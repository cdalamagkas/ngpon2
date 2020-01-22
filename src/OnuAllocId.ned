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

module OnuAllocId
{
    parameters:
        int allocId = default(0);
        int tcont = default(5);
        //traffic descriptor D = <Rf, Ra, Rm, Xab, P, omega>
        //xml profile = xmldoc("profile.xml");
        
        @display("bgb=288,197");
    submodules:
    	queue: OnuQueue {
            @display("p=137,151");
        }
       	appCbr: OnuAppCbr {
            @display("p=42,39");
        }
        appPoisson: OnuAppPoisson {
            @display("p=137,39");
        }
        //appPareto: OnuAppPareto;
    connections:
        appCbr.out --> queue.in++;
        appPoisson.out --> queue.in++;
        //for i=0..realTraces-1 {
        //    appReal[i].out --> queue.in++;
        //}
}
