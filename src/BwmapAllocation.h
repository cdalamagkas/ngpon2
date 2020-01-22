/*
 * BwmapAllocation.h
 *
 *  Created on: 5 Apr 2018
 *      Author: cdal
 */

#ifndef BWMAPALLOCATION_H_
#define BWMAPALLOCATION_H_

typedef struct bwmapallocation {
    int allocId;
    int64_t grantSize;
    omnetpp::simtime_t startTime;
} BwmapAllocation;

#endif /* BWMAPALLOCATION_H_ */
