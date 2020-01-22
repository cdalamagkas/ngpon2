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

#ifndef __NGPON2_ONUAPPREAL_H_
#define __NGPON2_ONUAPPREAL_H_

#include <omnetpp.h>
#include <boost/filesystem.hpp> // apt install libboost-filesystem-dev
#include "RealTrafficInitializer.h"
#include <random>
#include <vector>
#include "OnuSdu_m.h"

using namespace omnetpp;

class OnuAppReal : public cSimpleModule {

    protected:
        int allocId;
        virtual void initialize();
        virtual void handleMessage(cMessage *msg);
        RealTraffic traffic;
        cGate *toQueue;

        std::random_device rd;    // only used once to initialise (seed) engine
        std::mt19937 rng;  // random-number engine used (Mersenne-Twister in this case)
        std::uniform_real_distribution<double> random_propability; // guaranteed unbiased

    public:
        ~OnuAppReal();

        int64_t customCdf_packetsize(std::vector<RealTrafficEntityPacketSize>);
        double  customCdf_interarrival(std::vector<RealTrafficEntityInterarrival>);
};

#endif
