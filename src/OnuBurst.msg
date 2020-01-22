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

//
// TODO generated message class
//
cplusplus{{
#include <vector>
#include "Dbru.h"
#include "OnuSdu_m.h"

typedef std::vector<Dbru> DbruVector;

}}

class noncobject OnuSduVector;
class noncobject DbruVector;
class omnetpp::cQueue;

packet OnuBurst {
    int onuId;
    omnetpp::cQueue sduQueue;
    DbruVector dbruVector;
    int64_t payload;
}
