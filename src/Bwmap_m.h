//
// Generated file, do not edit! Created by nedtool 5.5 from Bwmap.msg.
//

#ifndef __BWMAP_M_H
#define __BWMAP_M_H

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <omnetpp.h>

// nedtool version check
#define MSGC_VERSION 0x0505
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif



// cplusplus {{
    #include "BwmapAllocation.h"
// }}

/**
 * Class generated from <tt>Bwmap.msg:25</tt> by nedtool.
 * <pre>
 * packet Bwmap
 * {
 *     int onuId;
 *     int64_t burstLength;
 *     BwmapAllocation allocations[];
 * }
 * </pre>
 */
class Bwmap : public ::omnetpp::cPacket
{
  protected:
    int onuId;
    int64_t burstLength;
    BwmapAllocation *allocations; // array ptr
    unsigned int allocations_arraysize;

  private:
    void copy(const Bwmap& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const Bwmap&);

  public:
    Bwmap(const char *name=nullptr, short kind=0);
    Bwmap(const Bwmap& other);
    virtual ~Bwmap();
    Bwmap& operator=(const Bwmap& other);
    virtual Bwmap *dup() const override {return new Bwmap(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual int getOnuId() const;
    virtual void setOnuId(int onuId);
    virtual int64_t getBurstLength() const;
    virtual void setBurstLength(int64_t burstLength);
    virtual void setAllocationsArraySize(unsigned int size);
    virtual unsigned int getAllocationsArraySize() const;
    virtual BwmapAllocation& getAllocations(unsigned int k);
    virtual const BwmapAllocation& getAllocations(unsigned int k) const {return const_cast<Bwmap*>(this)->getAllocations(k);}
    virtual void setAllocations(unsigned int k, const BwmapAllocation& allocations);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const Bwmap& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, Bwmap& obj) {obj.parsimUnpack(b);}


#endif // ifndef __BWMAP_M_H

