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

#include "OnuAppCbr.h"

Define_Module(OnuAppCbr);

OnuAppCbr::~OnuAppCbr() {
    //TODO - Destructor OnuAppCbr is empty
}

void OnuAppCbr::initialize(int stage)
{
    if (stage==2)
    {
        std::string rateUnit(par("rate").getUnit());
        this->packetSize = par("packetSize").intValue();

        if (rateUnit == "Mbps")
            this->interarrival = this->packetSize/(par("rate").doubleValue()*125000.0);
        else if (rateUnit == "Gbps")
            this->interarrival = this->packetSize/(par("rate").doubleValue()*125000000.0);
        else
            error("appCbr: Unit provided was unpredictable");

        this->allocId = getParentModule()->par("allocId");

        if (this->packetSize > 0 && this->interarrival > 0)  //if packetSize is 0 then appCbr has been turned off
        {
            cMessage *msg = new cMessage();
            scheduleAt(0.0, msg);
        }

        this->serialNumber = 0;

        //std::cout << "OnuAppCbr for [AllocID " << this->allocId << "] finished initialisation" << endl;
    }
}

void OnuAppCbr::handleMessage(cMessage *msg)
{
    OnuSdu *pkt = new OnuSdu();

    pkt->setInsertionTime(simTime());
    pkt->setSerialNumber(this->serialNumber);
    pkt->setAllocId(this->allocId);
    pkt->setOriginalByteLength(this->packetSize);
    pkt->setByteLength(pkt->getOriginalByteLength());

    this->serialNumber++;

    send(pkt, "out");
    scheduleAt(simTime() + this->interarrival, msg);
}

void OnuAppCbr::setAllocId(int value) { this->allocId = value; }
int OnuAppCbr::numInitStages() const { return 3; }
