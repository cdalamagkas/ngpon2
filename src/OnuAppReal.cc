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

#include "OnuAppReal.h"


Define_Module(OnuAppReal);

OnuAppReal::~OnuAppReal(){
    //TODO - Destructor OnuAppReal is empty
};

void OnuAppReal::initialize()
{
    this->traffic = check_and_cast<RealTrafficInitializer *>(getSystemModule()->getSubmodule("realTrafficInitializer"))->getTraffic(par("trafficIndex").intValue());
    this->allocId = getParentModule()->par("allocId");

    this->toQueue = getParentModule()->getSubmodule("queue")->gate("appRealIn");

    cMessage *msg = new cMessage();
    scheduleAt(0.0, msg);

    rng                 = std::mt19937(rd());
    random_propability  = std::uniform_real_distribution<double>(1, 100);

    //std::cout << "OnuAppReal for [AllocID " << allocId << "] finished initialisation" << endl;
}

void OnuAppReal::handleMessage(cMessage *msg)
{
    OnuSdu *pkt = new OnuSdu();
    int size = customCdf_packetsize(traffic.packetSize);

    pkt->setAllocId(this->allocId);
    pkt->setOriginalByteLength(size);
    pkt->setByteLength(size);

    sendDirect(pkt, this->toQueue);

    scheduleAt(simTime() + customCdf_interarrival(traffic.interarrival), msg);

}

int64_t OnuAppReal::customCdf_packetsize(std::vector<RealTrafficEntityPacketSize> F)
{
    double p = random_propability(rng);
    for (int i=0; i<F.size(); i++)
    {
        int sum = 0;
        for (int k=0; k<i; k++)
            sum += F.at(k).propability;
        if (p <= sum)
            return F.at(i).value;
    }
    return F.at(F.size()-1).value;
}

double OnuAppReal::customCdf_interarrival(std::vector<RealTrafficEntityInterarrival> F)
{
    double p = random_propability(rng);
    for (int i=0; i<F.size(); i++)
    {
        int sum = 0;
        for (int k=0; k<i; k++)
            sum += F.at(k).propability;
        if (p <= sum)
            return F.at(i).value;
    }
    return F.at(F.size()-1).value;
}

