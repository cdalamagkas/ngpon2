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

#ifndef __NGPON2_OLTSCHEDULERUS_H_
#define __NGPON2_OLTSCHEDULERUS_H_

#include <omnetpp.h>
#include <vector>
#include <array>
#include <cmath>
#include <algorithm>
#include <limits>
#include <numeric>
#include <cfenv>
#include "BwmapAllocation.h"
#include "Bwmap_m.h"
#include "OltDatabase.h"

using namespace omnetpp;

class OltSchedulerUS : public cSimpleModule
{
    private:
        OltDatabase *oltDatabase;
        std::vector<simtime_t> CAT; // Channel Available Time
        std::vector<simtime_t> OAT; // ONU Available Time
        int r_f, r_a, r_m;
        int64_t grantedBytes;
        int upstreamBandwidthAvailable;
        int dwbaMode;

        std::vector<int> onuPriority;

    protected:
        virtual void initialize(int stage);
        virtual int numInitStages() const;
        virtual void handleMessage(cMessage *msg);

    public:
        std::vector<int64_t> roundRobinAllocation(std::vector<int64_t>, std::vector<int>, std::vector<int64_t>*, int);
        std::vector<int64_t> gameTheory(std::vector<int64_t>, std::vector<int>, std::vector<int64_t>*, int, int64_t);
        std::vector<int64_t> calculateBandwidthConsumptionOfOnus(std::vector<int64_t> ,bool);
        std::vector<simtime_t> calculateLoadOfOnus(std::vector<int64_t>, bool);
        int64_t sumAllocations(std::vector<int64_t>, std::vector<int>, int);
        int64_t calculateRemainingDemands(std::vector<int>, int, std::vector<int64_t>);
        int64_t calculatePhyBurstLength(int64_t,bool);
        int64_t calculateResidualAllocation(int, std::vector<int64_t>, std::vector<int>, int);
        int64_t calculateAvailableBandwidth(bool);
        std::vector<int> getOnusShortedByLoad(std::vector<int64_t>, bool);
        std::vector<int> channelAllocation(bool);
        simtime_t calculateTransmissionDelay(int64_t);
        ~OltSchedulerUS();
};

#endif
