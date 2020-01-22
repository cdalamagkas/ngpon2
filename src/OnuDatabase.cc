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

#include "OnuDatabase.h"

Define_Module(OnuDatabase);

OnuDatabase::~OnuDatabase(){} //TODO - Destructor OnuDatabase is empty

void OnuDatabase::initialize(int stage) //stage 1
{
    if (stage == 1) {
        //will be executed after OltDatabase, meaning when each ONU knows exactly all the AllocIDs that it has.
        //assigns proper AllocIDs from allocIdVector[] to each queue and appCbr submodule

        this->submoduleIndexByAllocId = std::vector<int>(this->systemAllocIdCount); //new std::vector<int>(*std::max_element(this->allocIds->begin(), this->allocIds->end())+1);
        cModule *currentAllocId;

        for (int i=0; i<this->allocIdCount; i++)
        {
            currentAllocId = getParentModule()->getSubmodule("allocId", i);
            currentAllocId->par("allocId").setIntValue( this->allocIdVector.at(i) );
            this->submoduleIndexByAllocId.at( this->allocIdVector.at(i) ) = i;  //[ this->allocIds[i] ] = i;
        }
        this->gateId = getParentModule()->gate("out")->getId();

        /*RealTrafficInitializer *realTrafficInitializerModule = check_and_cast<RealTrafficInitializer *>(getSystemModule()->getSubmodule("realTrafficInitializer"));

        if ( realTrafficInitializerModule->par("isEnabled").boolValue() )    // OnuAppReal modules must be manually created, each module corresponds to each Traffic in folder
        {
            std::vector<RealTraffic> traffics = realTrafficInitializerModule->getAllTraffics();

            for (int i=0; i<this->allocIdCount; i++)
            {
                cModule *onuAllocId = getParentModule()->getSubmodule("allocId", i);
                for (int j=0; j<traffics.size(); j++)
                {
                    cModule *realTrafficModule = cModuleType::get("ngpon2.OnuAppReal")->create("realTraffic", onuAllocId);
                    realTrafficModule->par("trafficIndex").setIntValue(j);
                    realTrafficModule->finalizeParameters();
                    realTrafficModule->buildInside();
                    realTrafficModule->callInitialize();
                    //no need to connect realTrafficModule to onuQueue. Packets will be sent using sendDirect();
                }
            }
        }*/
        //EV << "OnuDatabase for [ONU " << (int) getParentModule()->par("onuId") << "] finished initialisation" << endl;
    }
}

void OnuDatabase::handleMessage(cMessage *msg) { delete msg; }
int OnuDatabase::numInitStages() const {return 2;}

//void OnuDatabase::calculateDistribution(boost::filesystem::path p)
//{

//}
