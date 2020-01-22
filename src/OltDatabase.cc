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

#include "OltDatabase.h"

Define_Module(OltDatabase);

OltDatabase::~OltDatabase(){} //TODO - Destructor OltDatabase is empty

void OltDatabase::initialize() //stage 0
{
    ONUS               = getParentModule()->par("ONUS");           //Initialise some parameters
    CHANNELS           = getParentModule()->par("twdmChannels");  // 4 by default

    channelByOnuId     = std::vector<int>(ONUS);    //Initialise size of arrays that return information related to ONUs
    gateIdByOnuId      = std::vector<int>(ONUS);
    allocIdCountPerOnu = std::vector<int>(ONUS);
    pdtByOnuId         = std::vector<simtime_t>(ONUS);

    rng           = std::mt19937(rd());
    randomChannel = std::uniform_int_distribution<int>(1, CHANNELS);

    int onuId, allocId, j;
    cGate *currentGate;
    cModule *currentOnu;

    for (int i=0; i<ONUS; i++)  // iterate through every ONU
    {
        currentOnu = getSystemModule()->getSubmodule("onu",i);
        onuId = currentOnu->par("onuId");

        allocIdCountPerOnu.at(onuId) = currentOnu->par("numAllocIDs");  //get the amount of AllocIDs assigned to that ONU

        cDatarateChannel *c = check_and_cast<cDatarateChannel *>(currentOnu->gate("out")->getTransmissionChannel());
        pdtByOnuId.at(onuId) = c->getDelay();

        channelByOnuId.at(i) = randomChannel(rng);                     //assign initial channel to each ONU randomly
    }

    ALLOC_IDs      = std::accumulate(allocIdCountPerOnu.begin(), allocIdCountPerOnu.end(), 0); //Total number of AllocIDs
    onuIdByAllocId = std::vector<int>    (ALLOC_IDs);        //gets the ONU associated with a specific AllocID
    bufOcc         = std::vector<int64_t>(ALLOC_IDs, 0);    // Holds the value of BufOcc (bandwidth demands) for each AllocID

    allocId=0;
    for (int i=0; i<ONUS; i++)        //Assign AllocIDs to ONUs
        for (int j=0; j < allocIdCountPerOnu.at(i); j++)
        {
            onuIdByAllocId.at(allocId) = i;
            allocId++;
        }

    for (int i=0; i<getParentModule()->gateSize("out"); i++) //for each out OLT gate find the ID of the adjacent ONU
    {
        currentGate             = getParentModule()->gate("out", i); //return onu module
        onuId                   = currentGate->getNextGate()->getOwnerModule()->par("onuId");
        gateIdByOnuId.at(onuId) = currentGate->getId();
    }

    for (int i=0; i<ONUS; i++)
    {
        cModule *currentOnu = getSystemModule()->getSubmodule("onu",i);
        onuId = currentOnu->par("onuId");

        OnuDatabase *currentDatabase        = check_and_cast<OnuDatabase *>(currentOnu->getSubmodule("database"));
        currentDatabase->allocIdCount       = allocIdCountPerOnu.at(onuId);                    //assign to ONU database the count of AllocIDs
        currentDatabase->allocIdVector      = std::vector<int>(allocIdCountPerOnu.at(onuId)); //reserve a vector that will store the AllocID the ONU has
        currentDatabase->systemAllocIdCount = ALLOC_IDs;

        j=0;
        for (int k=0; k<ALLOC_IDs; k++)           //search all AllocIDs
            if (onuIdByAllocId.at(k) == onuId )  //and save those that match the current ONU
            {
                currentDatabase->allocIdVector.at(j) = k;
                j++;
            }
    }

    if (getSystemModule()->par("symmetricMode").boolValue()) {
        this->symmetricMode = true;
        UPSTREAM_BANDWIDTH = 155520;

    }
    else {
        this->symmetricMode = false;
        UPSTREAM_BANDWIDTH = 38880;
    }

    UPSTREAM_SPEED = UPSTREAM_BANDWIDTH/0.000125;
    guardTime      = (64/8)/UPSTREAM_SPEED;

    //std::cout << "OltDatabase finished initialisation" << endl;
}

simtime_t OltDatabase::getGuardTime() {return this->guardTime;}
simtime_t OltDatabase::getPdt(int i)  {return this->pdtByOnuId.at(i);}
void OltDatabase::setBufOcc(int i, int64_t value)       {this->bufOcc.at(i) = value;}
void OltDatabase::setChannelByOnuId(int i, int channel) {this->channelByOnuId.at(i) = channel;}
void OltDatabase::handleMessage(cMessage *msg)          {delete msg;}
int64_t OltDatabase::getBufOcc(int i)   {return this->bufOcc.at(i);}
int64_t OltDatabase::getUpstreamSpeed() {return this->UPSTREAM_SPEED;}
int OltDatabase::getAllocIDCount()        {return this->ALLOC_IDs;}
int OltDatabase::getOnuCount()            {return this->ONUS;}
int OltDatabase::getChannelCount()        {return this->CHANNELS;}
int OltDatabase::getUpstreamBandwidth()   {return this->UPSTREAM_BANDWIDTH;}
int OltDatabase::getOnuIdByAllocId(int i) {return this->onuIdByAllocId.at(i);}
int OltDatabase::getGateIdByOnuId(int i)  {return this->gateIdByOnuId.at(i);}
int OltDatabase::getChannelByOnuId(int i) {return this->channelByOnuId.at(i);}
bool OltDatabase::isSymmetric() {return this->symmetricMode;}
std::vector<int64_t> OltDatabase::getBufOccVector() {return this->bufOcc;}
