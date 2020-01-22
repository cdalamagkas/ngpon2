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

#include "CustomFilters.h"

Register_ResultFilter("delay", DelayFilter);
Register_ResultFilter("allocid", AllocidFilter);

void DelayFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details)
{
    if (object)
        if (OnuSdu *packet = check_and_cast<OnuSdu *>(object))
        {
            simtime_t delay = t - packet->getInsertionTime();
            fire(this, t, delay, details);
        }
}

void AllocidFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details)
{
    if (object)
        if (OnuSdu *packet = check_and_cast<OnuSdu *>(object))
        {
            long allocid = packet->getAllocId();
            fire(this, t, allocid, details);
        }
}
