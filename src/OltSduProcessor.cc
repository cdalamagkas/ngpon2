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

#include "OltSduProcessor.h"

Define_Module(OltSduProcessor);

OltSduProcessor::~OltSduProcessor(){} //TODO - Destructor OltSduProcessor is empty

void OltSduProcessor::initialize(int stage)
{
    if (stage == 1)
    {
        this->oltDatabase = check_and_cast<OltDatabase *>(getParentModule()->getSubmodule("database"));
        pktSignal = registerSignal("pkt");
    }
}

void OltSduProcessor::handleMessage(cMessage *msg)
{
    OnuBurst *receivedBurst = check_and_cast<OnuBurst *>(msg);

    cQueue sduQueue = receivedBurst->getSduQueue();
    DbruVector dbruVec = receivedBurst->getDbruVector();

    OnuSdu *currentSdu;
    Dbru *currentDbru;


    EV << "[RX] SDU received by ONU ID: " << receivedBurst->getOnuId() << ". Number of SDUs: " << sduQueue.getLength() << endl;

    while ( !sduQueue.isEmpty() )
    {
        currentSdu = check_and_cast<OnuSdu *>(sduQueue.pop());

        EV << "\t AllocID: " << currentSdu->getAllocId() << ". Real byte length: " << currentSdu->getByteLength() << ". Original byte length: " << currentSdu->getOriginalByteLength() << ". Arrival time: " << currentSdu->getArrivalTime() << endl;

        currentSdu->setByteLength(currentSdu->getOriginalByteLength());

        /*Delay delay_pair;
        delay_pair.allocid = currentSdu->getAllocId();
        delay_pair.delay = simTime() - currentSdu->getInsertionTime();
        this->delayVec.push_back(delay_pair);*/

        emit(this->pktSignal, currentSdu);
        cancelAndDelete(currentSdu);
    }

    while ( !dbruVec.empty() )
    {
        currentDbru = &dbruVec.back();
        this->oltDatabase->setBufOcc(currentDbru->allocId, currentDbru->bufOcc);
        dbruVec.pop_back();
    }

    delete msg;
}

int OltSduProcessor::numInitStages() const { return 2; }

/*
void OltSduProcessor::finish() const {
    for (std::vector<Delay>::iterator it = delayVec.begin(); it != delayVec.end(); ++it)
        std::cout << ' ' << *it;
}*/
