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

#include "OnuQueue.h"

Define_Module(OnuQueue);

OnuQueue::~OnuQueue(){} //TODO - OnuQueue deconstructor is empty

void OnuQueue::initialize(int stage) {
    if (stage==2)
    {
        std::string capacityUnit(par("capacity").getUnit());

        this->capacity = par("capacity").intValue();
        this->allocId  = getParentModule()->par("allocId");

        this->dropSignal = registerSignal("drop");
        this->insertedSignal = registerSignal("insert");
        //std::cout << "OnuQueue for [AllocID " << this->allocId << "] finished initialisation" << endl;
    }
}

void OnuQueue::handleMessage(cMessage *msg)
{
    OnuSdu *pkt = check_and_cast<OnuSdu *>(msg);
    if (pkt->getByteLength() + this->queue.getByteLength() <= this->capacity || this->capacity == 0)
    {
        this->queue.insert(pkt);
        emit(insertedSignal, 1);
    }
    else
        emit(dropSignal, 1);

}

int OnuQueue::numInitStages() const { return 3;}
cPacketQueue * OnuQueue::getQueue() { return &this->queue; }
