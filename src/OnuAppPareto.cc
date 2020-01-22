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

#include "OnuAppPareto.h"

Define_Module(OnuAppPareto);

OnuAppPareto::~OnuAppPareto(){
    //TODO - Destructor OnuAppPoisson is empty
}

void OnuAppPareto::initialize(int stage)
{
    if (stage==2)
    {
        this->interarrival  = (double) par("interarrival");
        if ( this->interarrival > 0.0 && ((int) par("packetSize")) > 0 )
        {
            this->allocId = getParentModule()->par("allocId");
            cMessage *msg = new cMessage();
            scheduleAt(0.0, msg);
            //std::cout << "OnuAppPareto for [AllocID " << this->allocId << "] finished initialization" << endl;
        }
    }
}

void OnuAppPareto::handleMessage(cMessage *msg)
{
    OnuSdu *pkt = new OnuSdu();
    int size = par("packetSize"); //intuniform(64, 1518);

    pkt->setAllocId(this->allocId);
    pkt->setOriginalByteLength(size);
    pkt->setByteLength(size);

    send(pkt, "out");

    //scheduleAt(simTime() + pareto_shifted(this->interarrival), msg);
}

int OnuAppPareto::numInitStages() const { return 3; }
