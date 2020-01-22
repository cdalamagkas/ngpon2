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

#ifndef __NGPON2_OLTDATABASE_H_
#define __NGPON2_OLTDATABASE_H_

#include <omnetpp.h>
#include <vector>
#include <cstdlib>
#include <numeric>
#include "OnuDatabase.h"
#include <fstream>
#include <sstream>
#include <iterator>
#include <random>

using namespace omnetpp;

class OltDatabase : public cSimpleModule
{
    protected:
        virtual void initialize();
        virtual void handleMessage(cMessage *msg);
        ~OltDatabase();

        std::vector<int> onuIdByAllocId, channelByOnuId, gateIdByOnuId, allocIdCountPerOnu, tcontByAllocId;
        int64_t UPSTREAM_SPEED;
        int ALLOC_IDs, ONUS, CHANNELS, UPSTREAM_BANDWIDTH, TCONTS;
        std::vector<int64_t> bufOcc;
        std::vector<simtime_t> pdtByOnuId;
        bool symmetricMode;
        simtime_t guardTime;

    public:
        int getAllocIDCount(), getOnuCount(), getChannelCount(), getUpstreamBandwidth(), getOnuIdByAllocId(int), getGateIdByOnuId(int), getChannelByOnuId(int);
        simtime_t getPdt(int);
        int64_t getBufOcc(int), getUpstreamSpeed();
        bool isSymmetric();
        simtime_t getGuardTime();

        void setBufOcc(int, int64_t), setChannelByOnuId(int, int);

        std::random_device rd;    // only used once to initialise (seed) engine
        std::mt19937 rng;  // random-number engine used (Mersenne-Twister in this case)
        std::uniform_int_distribution<int> randomChannel; // guaranteed unbiased

        std::vector<int64_t> getBufOccVector();
};

#endif
