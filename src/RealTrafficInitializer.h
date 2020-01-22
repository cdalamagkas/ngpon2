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

#ifndef __NGPON2_REALTRAFFICINITIALIZER_H_
#define __NGPON2_REALTRAFFICINITIALIZER_H_

#include <omnetpp.h>
#include <boost/filesystem.hpp> // apt install libboost-filesystem-dev
#include <boost/lexical_cast.hpp>
#include <fstream>
using namespace omnetpp;

typedef struct RealTrafficEntityPacketSize_s {
    int64_t value;
    double propability;
} RealTrafficEntityPacketSize;

typedef struct RealTrafficEntityInterarrival_s {
    double value;
    double propability;
} RealTrafficEntityInterarrival;

typedef struct RealTraffic_s {
    std::vector<RealTrafficEntityPacketSize> packetSize;
    std::vector<RealTrafficEntityInterarrival> interarrival;
} RealTraffic;


/**
 * TODO - Generated class
 */
class RealTrafficInitializer : public cSimpleModule
{
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    std::vector<RealTraffic> realTraffics;

  public:
    std::vector<RealTraffic> getAllTraffics();
    RealTraffic              getTraffic(int);
};

#endif
