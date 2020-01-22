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

#include "RealTrafficInitializer.h"

Define_Module(RealTrafficInitializer);

void RealTrafficInitializer::initialize()
{
    if (par("isEnabled").boolValue())
    {
        chdir("./Traffic");

        std::vector<boost::filesystem::path> filesVector; //each file is a Traffic
        boost::filesystem::path current_file;

        std::copy(boost::filesystem::directory_iterator(boost::filesystem::current_path()), boost::filesystem::directory_iterator(), back_inserter(filesVector));

        for ( std::vector<boost::filesystem::path>::const_iterator it (filesVector.begin()); it != filesVector.end(); ++it)
        {
            if ( boost::filesystem::extension(*it).compare(".dat") == 0 ) //ignore everything else than .dat
            {
                current_file = *it;
                RealTraffic newTraffic;

                std::ifstream inFile(current_file.string());
                if (!inFile.is_open() )
                    error("Unable to open .dat");
                else
                {
                    std::string currentLine;
                    while (std::getline(inFile, currentLine))
                    {
                        std::istringstream iss(currentLine);
                        std::vector<std::string> results(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());

                        if (results.at(0).compare("p") == 0) //is a packetSize
                        {
                            RealTrafficEntityPacketSize newEntry;
                            newEntry.value  = std::stoi(results.at(1));
                            newEntry.propability = std::stod(results.at(2));
                            newTraffic.packetSize.push_back(newEntry);
                        }
                        else                                 // is an interarrival
                        {
                            RealTrafficEntityInterarrival newEntry;
                            newEntry.value        = boost::lexical_cast<double>(results.at(1));
                            newEntry.propability  = std::stod(results.at(2));
                            newTraffic.interarrival.push_back(newEntry);
                        }
                    }
                    inFile.close();
                }
                realTraffics.push_back(newTraffic);
            }
        }

        chdir("../");

        std::cout << "RealTraffic has finished initialisation";
    }
}

void RealTrafficInitializer::handleMessage(cMessage *msg)         {delete msg;}
std::vector<RealTraffic> RealTrafficInitializer::getAllTraffics() {return this->realTraffics;}
RealTraffic RealTrafficInitializer::getTraffic(int i)             {return this->realTraffics.at(i);}
