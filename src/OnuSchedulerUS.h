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

#ifndef __NGPON2_ONUSCHEDULERUS_H_
#define __NGPON2_ONUSCHEDULERUS_H_

#include <omnetpp.h>
#include "BwmapAllocation.h"
#include "Dbru.h"

#include "Bwmap_m.h"
#include "OnuSdu_m.h"
#include "OnuBurst_m.h"

#include "OnuQueue.h"
#include "OnuDatabase.h"

using namespace omnetpp;

/**
 * TODO - Generated class
 */
class OnuSchedulerUS : public cSimpleModule
{
private:
    long onuId;
    OnuDatabase *onuDatabase;
    simsignal_t pktOutOfQueueSignal, utilizationSignal;
    cChannel *upstreamChannel;

  protected:
    void sendSignals(cQueue&);
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    ~OnuSchedulerUS();
};

#endif
