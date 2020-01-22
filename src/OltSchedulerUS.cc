//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "OltSchedulerUS.h"

Define_Module(OltSchedulerUS);

OltSchedulerUS::~OltSchedulerUS(){} //TODO - Destructor OltSchedulerUS is empty

void OltSchedulerUS::initialize(int stage)
{
    if (stage == 1)  //must come after Olt database initialisation
    {
        oltDatabase = (OltDatabase *) getParentModule()->getSubmodule("database");

        CAT = std::vector<simtime_t>(oltDatabase->getChannelCount(), SIMTIME_ZERO);
        OAT = std::vector<simtime_t>(oltDatabase->getOnuCount(), 0);

        this->r_f = par("r_f").intValue();
        this->r_a = par("r_a").intValue();
        this->r_m = par("r_m").intValue();

        if (this->dwbaMode == 1)
        for (int i=0; i<this->oltDatabase->getOnuCount(); i++)
        {
            if (i%2==0)
                this->onuPriority.at(i) = 0; //0 is low priority
            else
                this->onuPriority.at(i) = 1; //1 is high priority
        }

        cMessage *msg = new cMessage("DWBA");
        scheduleAt(0.0, msg);

        fesetround(FE_UPWARD);

        this->upstreamBandwidthAvailable = calculateAvailableBandwidth(false);

        this->grantedBytes = 0;

        if ( (this->r_f + this->r_a)*oltDatabase->getAllocIDCount() > this->upstreamBandwidthAvailable )
            error("Assured bandwidth is not enough for all these AllocIDs!");

        this->dwbaMode = par("dwbaMode").intValue();

        //std::cout << "OltSchedulerUS finished initialisation" << endl;
    }
}

void OltSchedulerUS::handleMessage(cMessage *msg)
{
    int64_t sum, burstLength, bestEffortDemands;
    simtime_t startTime, arrivalTimeofBurst, transmissionStartFromOnu, extraDelay, transmissionFinishFromOnu;
    bool sendsPloamu = false;

    std::vector<int64_t> availableBandwidth            = std::vector<int64_t> (oltDatabase->getChannelCount(), this->upstreamBandwidthAvailable);
    std::vector<int64_t> bandwidthAllocationByAllocId  = std::vector<int64_t> (oltDatabase->getAllocIDCount());
    std::vector<int>     channelAllocationByOnuId;

    //if (this->dwbaMode == 0) {

    // STEP 1: Allocate channels to ONUs, based on load
    channelAllocationByOnuId = channelAllocation(sendsPloamu);

    // STEP 2: Allocate bandwidth to each Channel
    for (int c=0; c<oltDatabase->getChannelCount(); c++) //for each channel...
    {
        //STEP 2.1: Allocate fixed and guaranteed bandwidth (R_F and R_a)
        bestEffortDemands = 0;

        for (int i=0; i<oltDatabase->getAllocIDCount(); i++)
            if (channelAllocationByOnuId.at(oltDatabase->getOnuIdByAllocId(i)) == c ) //if current AllocID belongs to the current channel
            {
                if (oltDatabase->getBufOcc(i) <= r_f)  //if demand is less than the fixed, then assigned the fixed.
                {
                    bandwidthAllocationByAllocId.at(i) = r_f;
                    oltDatabase->setBufOcc(i, 0);
                }
                else if (oltDatabase->getBufOcc(i) <= r_f + r_a) //if demand is bellow the assured, then assign the whole demand.
                {
                    bandwidthAllocationByAllocId.at(i) = oltDatabase->getBufOcc(i);
                    oltDatabase->setBufOcc(i, 0);
                }
                else //if demand exceeds the assured, then assign all assured and the rest demand will join the DBA method.
                {
                    bandwidthAllocationByAllocId.at(i) = r_f + r_a;
                    oltDatabase->setBufOcc(i, oltDatabase->getBufOcc(i) - bandwidthAllocationByAllocId.at(i));
                }
                availableBandwidth.at(c) -= bandwidthAllocationByAllocId.at(i);
                bestEffortDemands += oltDatabase->getBufOcc(i); //gathers additional demands
            }

        //STEP 3.2: If at least one AllocID for the current channel has excessive demands
        if (bestEffortDemands > availableBandwidth.at(c) ) {// if users demand more bandwidth than the available, make round-robin allocation
            if (this->dwbaMode == 0)
                bandwidthAllocationByAllocId = roundRobinAllocation(bandwidthAllocationByAllocId, channelAllocationByOnuId, &availableBandwidth, c);
            else if (this->dwbaMode == 1)
                bandwidthAllocationByAllocId = gameTheory(bandwidthAllocationByAllocId, channelAllocationByOnuId, &availableBandwidth, c, bestEffortDemands);
        }
        else if (bestEffortDemands > 0 ) //there are demands and residual bandwidth is enough for everyone
            for (int i=0; i<oltDatabase->getAllocIDCount(); i++)
                if (channelAllocationByOnuId.at(oltDatabase->getOnuIdByAllocId(i)) == c )  //if current AllocID belongs to the current channel
                {
                    bandwidthAllocationByAllocId.at(i) += oltDatabase->getBufOcc(i);
                    oltDatabase->setBufOcc(i, 0); //bandwidth is enough for everyone, so bufOcc of each allocID gets 0
                }

        if (this->upstreamBandwidthAvailable < sumAllocations(bandwidthAllocationByAllocId, channelAllocationByOnuId, c))
            error("Allocated more bandwidth than the available.");

        //STEP 3.3: Finalise BWmaps and send() them through the correct gate based on OnuId
        for (int currentOnu=0; currentOnu<oltDatabase->getOnuCount(); currentOnu++)
            if (channelAllocationByOnuId.at(currentOnu)==c)
            {
                std::vector<BwmapAllocation> allocationStructs;     // Initialise a vector of BwmapAllocations that are included in BWmap
                sum=0;
                for (int i=0; i<oltDatabase->getAllocIDCount(); i++)                       //search for AllocIDs that ONU is subscribed to
                    if ( currentOnu == oltDatabase->getOnuIdByAllocId(i) )                //for every one of them
                    {
                        BwmapAllocation allocation;                                     // create an Allocation struct
                        allocation.allocId      = i;                                   // define AllocID
                        allocation.grantSize    = bandwidthAllocationByAllocId.at(i); //set grantSize from bandwidthAllocationByAllocId
                        allocation.startTime    = 0;
                        allocationStructs.push_back(allocation);                     //put allocation in queue
                        sum += bandwidthAllocationByAllocId.at(i);                  //Summarise grantSizes in order to calculate burst length of ONU
                    }
                this->grantedBytes += sum;

                burstLength = calculatePhyBurstLength(sum, sendsPloamu);

                transmissionStartFromOnu  = simTime() + (oltDatabase->getPdt(currentOnu) + 0.000125);
                transmissionFinishFromOnu = transmissionStartFromOnu + calculateTransmissionDelay(burstLength);
                arrivalTimeofBurst = transmissionFinishFromOnu + oltDatabase->getPdt(currentOnu);

                if (arrivalTimeofBurst > CAT.at(c) && arrivalTimeofBurst - CAT.at(c) >= oltDatabase->getGuardTime() )  //define proper startTime
                {
                    startTime = SIMTIME_ZERO;
                    CAT.at(c) = arrivalTimeofBurst;
                }
                else if (arrivalTimeofBurst >= CAT.at(c) && arrivalTimeofBurst - CAT.at(c) < oltDatabase->getGuardTime() )
                {
                    startTime = oltDatabase->getGuardTime() - ( arrivalTimeofBurst - CAT.at(c) ) ;
                    CAT.at(c) = arrivalTimeofBurst + startTime;
                }
                else if (arrivalTimeofBurst < CAT.at(c))
                {
                    startTime = CAT.at(c) - arrivalTimeofBurst + oltDatabase->getGuardTime();
                    CAT.at(c) = CAT.at(c) + oltDatabase->getGuardTime();
                }

                transmissionStartFromOnu += startTime;
                transmissionFinishFromOnu += startTime;
                arrivalTimeofBurst += startTime;

                if ( transmissionStartFromOnu < OAT.at(currentOnu) )  //can't transmit while other transmission of the same ONU is still in progress
                {
                    extraDelay = OAT.at(currentOnu) - transmissionStartFromOnu + oltDatabase->getGuardTime();
                    startTime += extraDelay;
                    CAT.at(c) += extraDelay;
                    transmissionFinishFromOnu += extraDelay;
                }

                OAT.at(currentOnu) = transmissionFinishFromOnu;

                allocationStructs.at(0).startTime = startTime;               //update only fist struct with new StartTime, if that is > 0

                Bwmap *pkt = new Bwmap();                                   //create a new Bwmap
                pkt->setByteLength(155520);                                // we want for the Bwmap to arrive when the entire downstream frame arrives
                pkt->setAllocationsArraySize( allocationStructs.size() );
                for (int j=0; j<allocationStructs.size(); j++ )          //put structs into the BWmap
                    pkt->setAllocations(j, allocationStructs.at(j));
                pkt->setOnuId(currentOnu);                              // set proper onuId;
                pkt->setBurstLength(burstLength);                      //just to save time for ONU calculations

                send(pkt, oltDatabase->getGateIdByOnuId(currentOnu));

                EV << "[OLT TX] BWmap to ONU " << currentOnu << " at channel " << c <<". Number of allocation structs: " << pkt->getAllocationsArraySize() << ". Arrives at: " << pkt->getArrivalTime() << ". Burst will be received at: " << arrivalTimeofBurst << endl;
                for (int a=0; a<pkt->getAllocationsArraySize(); a++)
                    EV << "\t\tAllocID: " << pkt->getAllocations(a).allocId << ", GrantSize: " << pkt->getAllocations(a).grantSize << ". StartTime: " << pkt->getAllocations(a).startTime << endl;
            } //continues to the next ONU
    } //continues to the next channel

    for (int i=0; i<oltDatabase->getOnuCount(); i++)
        oltDatabase->setChannelByOnuId(i, channelAllocationByOnuId.at(i));

    scheduleAt(simTime() + 0.000125, msg);
}

std::vector<int64_t> OltSchedulerUS::roundRobinAllocation(std::vector<int64_t> bandwidthAllocationByAllocId, std::vector<int> channelAllocationByOnuId, std::vector<int64_t> *availableBandwidth, int c)
{
    int64_t oldAllocation, tempMin, residual;
    bool noMoreBandwidth = false;

    while (true)
    {
        tempMin = std::numeric_limits<int64_t>::max(); //find minimum POSITIVE bufOcc value for all AllocIDs of the current channel
        for (int i=0; i<oltDatabase->getAllocIDCount() ; i++)
            if (channelAllocationByOnuId.at(oltDatabase->getOnuIdByAllocId(i)) == c )   //if current AllocID belongs to the current channel
                if (oltDatabase->getBufOcc(i) < tempMin && oltDatabase->getBufOcc(i) > 0)
                    tempMin = oltDatabase->getBufOcc(i);

        for (int i=0; i<oltDatabase->getAllocIDCount(); i++)
            if (channelAllocationByOnuId.at(oltDatabase->getOnuIdByAllocId(i)) == c && oltDatabase->getBufOcc(i) > 0)
            {
                oldAllocation = bandwidthAllocationByAllocId.at(i); //backup original allocation

                if ( bandwidthAllocationByAllocId.at(i) + tempMin > r_m && r_m > 0 )  //if this allocation plus the minimum exceeds R_m and R_m has been set
                    bandwidthAllocationByAllocId.at(i) = r_m;      //give the maximum bandwidth allowed (R_m)
                else                                               //else if this allocation plus the minimum doesn't exceed R_m,
                    bandwidthAllocationByAllocId.at(i) += tempMin; //then assign that minimum value

                availableBandwidth->at(c) -= (bandwidthAllocationByAllocId.at(i) - oldAllocation);  //update the value of current available bandwidth

                if (availableBandwidth->at(c) < 0) {                   //if we assigned more bandwidth than we could...
                    bandwidthAllocationByAllocId.at(i) = oldAllocation; //reset allocation

                    residual = this->upstreamBandwidthAvailable - sumAllocations(bandwidthAllocationByAllocId, channelAllocationByOnuId, c); //calculate what remains
                    if (bandwidthAllocationByAllocId.at(i) + residual <= r_m)
                        bandwidthAllocationByAllocId.at(i) += residual;         //assign the residual bandwidth

                    if (bandwidthAllocationByAllocId.at(i) < 0) error("Negative bandwidth allocation occured");

                    oltDatabase->setBufOcc(i, oltDatabase->getBufOcc(i) - bandwidthAllocationByAllocId.at(i) - oldAllocation);  //update the bufOcc by the difference of new and old allocation
                    noMoreBandwidth = true;                                                    // turn on flag to stop bandwidth allocation immediately
                    break;                                                                    // Don't check for other AllocIDs
                }
                oltDatabase->setBufOcc(i, oltDatabase->getBufOcc(i) - (bandwidthAllocationByAllocId.at(i) - oldAllocation));  //update the bufOcc by the difference of new and old allocation
            }
        if (noMoreBandwidth || calculateRemainingDemands(channelAllocationByOnuId, c, bandwidthAllocationByAllocId) == 0 ) break; //if bandwidth is not enough or everyone is satisfied for the current channel, then get out from this while

    } //continue round-robin
    return bandwidthAllocationByAllocId;
}

std::vector<int64_t> OltSchedulerUS::gameTheory(std::vector<int64_t> bandwidthAllocationByAllocId, std::vector<int> channelAllocationByOnuId, std::vector<int64_t> *availableBandwidth, int c, int64_t bestEffortDemands)
{
    fesetround(FE_DOWNWARD);
    int64_t beAvailableBandwidth = availableBandwidth->at(c), allocation;
    for (int i=0; i<oltDatabase->getAllocIDCount(); i++)    // allocate it proportionally
        if (channelAllocationByOnuId.at(oltDatabase->getOnuIdByAllocId(i)) == c )  //if current AllocID belongs to the current channel
        {
            allocation = llrint((oltDatabase->getBufOcc(i) * beAvailableBandwidth) / bestEffortDemands);
            oltDatabase->setBufOcc(i, oltDatabase->getBufOcc(i) - allocation );
            availableBandwidth->at(c) -= allocation;
            bandwidthAllocationByAllocId.at(i) += allocation;
        }
    fesetround(FE_UPWARD);
    return bandwidthAllocationByAllocId;
}

// 80 = 24 (PSBu) + 4 (FS header) + 48 (PLOAMu) + 4 (FS trailer)
int64_t OltSchedulerUS::calculatePhyBurstLength(int64_t x, bool sendsPloamu)
{
    if (sendsPloamu)
        if (this->oltDatabase->isSymmetric())
            return (80 + x + llrint((56+x)/216.0 )*32);
        else
            return (80 + x + llrint((56+x)/232.0 )*16);
    else
        if (this->oltDatabase->isSymmetric())
            return (32 + x + llrint((8+x)/216.0 )*32);
        else
            return (32 + x + llrint((8+x)/232.0 )*16);

    //return ( (this->oltDatabase->isSymmetric()) ? (32 + x + llrint((8+x)/216.0 )*32) : (32 + x + llrint((8+x)/232.0 )*16) ); }
}

int64_t OltSchedulerUS::sumAllocations(std::vector<int64_t> allocation, std::vector<int> channelAllocationByOnuId, int c)
{
    int64_t sum=0;
    for (int i=0; i<oltDatabase->getAllocIDCount(); i++)
        if (channelAllocationByOnuId.at(oltDatabase->getOnuIdByAllocId(i)) == c ) //if current AllocID belongs to the current channel
            sum += allocation.at(i);
    return sum;
}

int64_t OltSchedulerUS::calculateRemainingDemands(std::vector<int> channelAllocationByOnuId, int c, std::vector<int64_t> bandwidthAllocationByAllocId)
{
    int64_t sum = 0;
    for (int i=0; i<oltDatabase->getAllocIDCount(); i++)
        if (channelAllocationByOnuId.at(oltDatabase->getOnuIdByAllocId(i)) == c )   // if current AllocID belongs to the current channel
        {    if( this->r_m > 0 && bandwidthAllocationByAllocId.at(i) >= this->r_m )  // if R_M has been defined and R_M amount has been assigned to AllocID
                continue;                                                           // then ignore the demand
            else                                                                 // else
                sum += oltDatabase->getBufOcc(i);                                   // add current BufOcc value to demands
        }
    return sum;
}

std::vector<int> OltSchedulerUS::channelAllocation(bool sendsPloamu)
{
    int currentOnu, currentChannel;

    std::vector<int> channelAllocationByOnuId = std::vector<int> (oltDatabase->getOnuCount(), 0);
    std::vector<int> onusShortedByLoad;
    std::vector<simtime_t> loadOnus, tempCAT = CAT;

    onusShortedByLoad = getOnusShortedByLoad(oltDatabase->getBufOccVector(), sendsPloamu); //load is calculated from their current demands
    loadOnus = calculateLoadOfOnus(oltDatabase->getBufOccVector(), sendsPloamu);  //get load of each ONU in order to update tempCAT accoordingly.

    while (!onusShortedByLoad.empty())
    {
        currentOnu      = onusShortedByLoad.back();  //ONU with the biggest load
        currentChannel  = std::distance(tempCAT.begin(), std::min_element(tempCAT.begin(), tempCAT.end())); //channel with the smallest load

        channelAllocationByOnuId.at(currentOnu) = currentChannel;
        tempCAT.at(currentChannel) += loadOnus.at(currentOnu);

        onusShortedByLoad.pop_back();
    }

    return channelAllocationByOnuId;
}

std::vector<simtime_t> OltSchedulerUS::calculateLoadOfOnus(std::vector<int64_t> demands, bool sendsPloamu) //returns the load of each ONU
{
    std::vector<simtime_t> loadByOnuId = std::vector<simtime_t>( oltDatabase->getOnuCount() );
    std::vector<int64_t> bandwidthConsumptionByOnuId;

    bandwidthConsumptionByOnuId = calculateBandwidthConsumptionOfOnus(demands, sendsPloamu);
    for (int i=0; i<oltDatabase->getOnuCount(); i++)
        loadByOnuId.at(i) = oltDatabase->getPdt(i) + calculateTransmissionDelay( bandwidthConsumptionByOnuId.at(i) );

    return loadByOnuId;
}

std::vector<int> OltSchedulerUS::getOnusShortedByLoad(std::vector<int64_t> demands, bool sendsPloamu) //Ascending order
{
    std::vector<int> onusShortedByLoad;
    std::vector<int64_t> bandwidthConsumptionByOnuId;// = std::vector<int64_t>(oltDatabase->getOnuCount());
    std::vector<simtime_t> loadByOnuId = std::vector<simtime_t>(oltDatabase->getOnuCount());
    int onuWithTheSmallestLoad;

    loadByOnuId = calculateLoadOfOnus(demands, sendsPloamu);

    for (int i=0; i<oltDatabase->getOnuCount(); i++)
    {
        onuWithTheSmallestLoad = std::distance(loadByOnuId.begin(), std::min_element(loadByOnuId.begin(), loadByOnuId.end())); //get index (=ONU) with the smallest value (=load)
        onusShortedByLoad.push_back(onuWithTheSmallestLoad);
        loadByOnuId.at(onuWithTheSmallestLoad) = SIMTIME_MAX;
    }

    return onusShortedByLoad;
}

std::vector<int64_t> OltSchedulerUS::calculateBandwidthConsumptionOfOnus(std::vector<int64_t> allocation, bool sendsPloamu )
{
    std::vector<int64_t> bandwidthConsumptionByOnuId = std::vector<int64_t>(oltDatabase->getOnuCount());

    for (int i=0; i<oltDatabase->getAllocIDCount(); i++) //calculates total FS payload for each ONU
        bandwidthConsumptionByOnuId.at( oltDatabase->getOnuIdByAllocId(i) ) += allocation.at(i);

    for (int i=0; i<oltDatabase->getOnuCount(); i++)
        bandwidthConsumptionByOnuId.at(i) = calculatePhyBurstLength( bandwidthConsumptionByOnuId.at(i), sendsPloamu );

    return bandwidthConsumptionByOnuId;
}

simtime_t OltSchedulerUS::calculateTransmissionDelay(int64_t bytes) { return SimTime(bytes/oltDatabase->getUpstreamSpeed()); }

int64_t OltSchedulerUS::calculateAvailableBandwidth(bool sendsPloamu)
{
    int64_t upstreamBandwidthAvailable;

    upstreamBandwidthAvailable = check_and_cast<OltDatabase *>(getParentModule()->getSubmodule("database"))->getUpstreamBandwidth();
    if (sendsPloamu)
        upstreamBandwidthAvailable -= 24*oltDatabase->getOnuCount() - (4+48+4+16)*oltDatabase->getOnuCount();  // PSBu + FS header + PLOAMu + FS trailer + FEC overhead
    else
        upstreamBandwidthAvailable -= 24*oltDatabase->getOnuCount() - (4+4+16)*oltDatabase->getOnuCount();  // PSBu + FS header + FS trailer + FEC overhead

    if (oltDatabase->isSymmetric())
        upstreamBandwidthAvailable -= llrint((upstreamBandwidthAvailable/216.0)*32);
    else
        upstreamBandwidthAvailable -= llrint((upstreamBandwidthAvailable/232.0)*16);
    return upstreamBandwidthAvailable;
}

int OltSchedulerUS::numInitStages() const { return 2; }
