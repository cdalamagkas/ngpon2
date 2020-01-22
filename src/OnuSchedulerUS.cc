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

#include "OnuSchedulerUS.h"

Define_Module(OnuSchedulerUS);

OnuSchedulerUS::~OnuSchedulerUS(){} //TODO - Destructor of OnuSchedulerUS is empty

void OnuSchedulerUS::initialize()
{
    this->onuId               = getParentModule()->par("onuId");
    this->onuDatabase         = check_and_cast<OnuDatabase *>(getParentModule()->getSubmodule("database"));
    this->pktOutOfQueueSignal = registerSignal("packetOutOfQueue");
    this->utilizationSignal   = registerSignal("utilization");
    this->upstreamChannel     = this->gate("out")->getTransmissionChannel();

    //std::cout << "OnuSchedulerUS for [ONU " << this->onuId << "] finished initialization" << endl;
}

void OnuSchedulerUS::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {   //is a postponed transmission
        if ( this->upstreamChannel->isBusy() )
        {
            EV << "Channel is busy! ";
            simtime_t difference = this->upstreamChannel->getTransmissionFinishTime() - simTime();
            if (difference <= 0.000000000005) //5 picosecond
            {
                EV << "Time difference is insignificant (" << difference << "), so transmission will be forced.";
                this->upstreamChannel->forceTransmissionFinishTime(simTime());
            }
            else
                EV << "Difference: " << this->upstreamChannel->getTransmissionFinishTime() - simTime();
        }
        OnuBurst *burst = check_and_cast<OnuBurst *>(msg);
        sendSignals(burst->getSduQueue());
        send(burst, this->onuDatabase->gateId);
        EV << "Burst sent from [ONU: " << this->onuId << "]. Arrives at: " << burst->getArrivalTime() << endl;

    }
    else
    {
        Bwmap *bwmap = check_and_cast<Bwmap *>(msg);

        if (bwmap->getOnuId() != this->onuId) error("BWMAP SENT TO THE WRONG ONU");

        Dbru dbru;
        int currentAllocId;
        int64_t availablePayload, originalGrant;
        double utilization;

        OnuBurst *burst = new OnuBurst();
        cQueue sduQueue;
        cPacketQueue *currentQueue;
        OnuQueue *currentQueueSubmodule;

        BwmapAllocation currentAllocation;
        OnuSdu *currentSdu, *editedPacket;

        simtime_t startTime = bwmap->getAllocations(0).startTime; //at the fist allocation struct is where we set the startTime delay

        EV << "[RX] ONU " << bwmap->getOnuId() << " received a BWmap. Allocations: " << bwmap->getAllocationsArraySize() << ". StartTime: " << startTime << endl;
        burst->setOnuId(this->onuId);

        for (int i=0; i<bwmap->getAllocationsArraySize(); i++)  //for each allocation
        {
            currentAllocation     = bwmap->getAllocations(i);
            currentAllocId        = currentAllocation.allocId;   //get it's allocId
            currentQueueSubmodule = check_and_cast<OnuQueue *>(getParentModule()->getSubmodule("allocId", this->onuDatabase->submoduleIndexByAllocId.at(currentAllocId) )->getSubmodule("queue") );
            currentQueue          = currentQueueSubmodule->getQueue(); // get the pointer of the right queue

            EV << "\t[AllocID: " << currentAllocId <<"]. Bytes/Packets in queue: " << currentQueue->getByteLength() << "/" << currentQueue->getLength() << ". GrantSize: " << currentAllocation.grantSize;
            availablePayload = currentAllocation.grantSize - 4;     //DBRu + CRC reserved as allocation overhead
            originalGrant    = availablePayload;

            while (currentQueue->front() != nullptr)   //iterate through the queue to add packets
            {
                if (availablePayload >= currentQueue->front()->getByteLength() + 8) //packet fits to allocation of FS frame
                {
                    currentSdu = check_and_cast<OnuSdu *>(currentQueue->pop());

                    sduQueue.insert(currentSdu);
                    burst->setPayload(burst->getPayload() + currentSdu->getByteLength());

                    availablePayload = availablePayload - currentSdu->getByteLength() - 8;
                }
                else if (availablePayload >= 16 && availablePayload < currentQueue->front()->getByteLength() + 8) //must make fragmentation
                {
                    editedPacket = check_and_cast<OnuSdu *>(currentQueue->pop()); //return front packet

                    editedPacket->setByteLength( editedPacket->getByteLength() - (availablePayload - 8) ); //change it's length

                    burst->setPayload(burst->getPayload() + availablePayload - 8);

                    if (currentQueue->front() == nullptr)  //insert it back to the FIRST place
                        currentQueue->insert(editedPacket);
                    else
                        currentQueue->insertBefore(currentQueue->front(), editedPacket);

                    availablePayload = 0;

                    break;
                }
                else break;
            }

            if (originalGrant != 0) {
                utilization = (originalGrant - availablePayload)/originalGrant;
                emit(this->utilizationSignal, utilization);
            }

            dbru.allocId = currentAllocId;
            dbru.bufOcc = currentQueue->getByteLength();

            EV << ". DBRu: " << dbru.bufOcc << endl;

            burst->setSduQueue(sduQueue);
            burst->getDbruVector().push_back(dbru);

        }//next AllocId

        burst->setByteLength( bwmap->getBurstLength() );

        if (startTime > 0)
        {
            scheduleAt(simTime() + startTime, burst);
            EV << "Burst postponed for time: " << simTime() + startTime << endl;
        }
        else
        {
            sendSignals(burst->getSduQueue());
            send(burst, this->onuDatabase->gateId);
            EV << "[TX] Burst sent from [ONU: " << this->onuId << "]. Arrives at: " << burst->getArrivalTime() << endl;
        }

        delete msg;
    }
}

void OnuSchedulerUS::sendSignals(cQueue& sduQueue)
{
    for (cQueue::Iterator it(sduQueue); !it.end(); it++)
        emit(pktOutOfQueueSignal, *it);
}
    /*
    while ( !sduQueue.isEmpty() )
    {
        currentSdu = check_and_cast<OnuSdu *>(sduQueue.pop());
        emit(pktOutOfQueueSignal, currentSdu);
        cancelAndDelete(currentSdu);
    }*/
