/*
 * CustomFilters.h
 *
 *  Created on: Sep 4, 2018
 *      Author: cdal
 */

#ifndef CUSTOMFILTERS_H_
#define CUSTOMFILTERS_H_

#include <omnetpp.h>
#include "OnuSdu_m.h"

using namespace omnetpp;

class DelayFilter : public cObjectResultFilter
{
    protected:
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
    public:
        using cObjectResultFilter::receiveSignal;
};

class AllocidFilter : public cObjectResultFilter
{
    protected:
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
    public:
        using cObjectResultFilter::receiveSignal;
};


#endif /* CUSTOMFILTERS_H_ */
