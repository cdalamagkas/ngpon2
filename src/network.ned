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

package ngpon2;

network Network
{
    parameters:
        int onus;
        double speedLight @unit(km) = 299792.458 * 0.7 km;
        bool symmetricMode = default(false);

        @statistic[dropRate](source=count(drop)/count(insert); record=last);
        @statistic[bytesReceived](source=packetBytes(pkt); record=sum; unit=bytes);
        @statistic[eed](source=delay(pkt); record=mean,vector; unit=s);
        @statistic[allocid](source=allocid(pkt); record=vector);
        @statistic[queueingDelay](source=delay(packetOutOfQueue); record=mean; unit=s);
        @statistic[packetsNum](source=pkt; record=count);
        @statistic[utilization](source=utilization; record=mean);
        
    submodules:
        olt: Olt {
            ONUS = onus;
        }
        onu[onus]: Onu {
            onuId = index;
            distance = default(uniform(20km,60km));
        }

        //realTrafficInitializer: RealTrafficInitializer {
        //    @display("i=block/cogwheel");
        //}
    connections:
        for i=0..onus-1 {
            olt.out++ --> { datarate=9.95328 Gbps; delay=(onu[i].distance/speedLight)*1s; } --> onu[i].in;
            olt.in++ <--  { datarate=2.48832 Gbps; delay=(onu[i].distance/speedLight)*1s; } <-- onu[i].out if !symmetricMode;
            olt.in++ <--  { datarate=9.95328 Gbps; delay=(onu[i].distance/speedLight)*1s; } <-- onu[i].out if symmetricMode;
        }
}