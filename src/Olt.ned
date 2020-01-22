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

module Olt
{
    parameters:
        @display("i=device/router");
        int ONUS;
        int twdmChannels = default(4);
                      
    gates:
        output out[ONUS];
        input  in[ONUS];
        
    submodules:
        database: OltDatabase {
            @display("p=38.133335,36.4;is=s");
        }
        schedulerUS: OltSchedulerUS {
            @display("p=188.06667,165.53334");
        }
        sduProcessor: OltSduProcessor {
            @display("p=328.46667,128.26666");
        }
    connections:
        for i=0..sizeof(out)-1 {
            in[i] --> sduProcessor.in++;
            schedulerUS.out++ --> out[i];
        }

}