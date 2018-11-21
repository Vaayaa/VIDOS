#ifndef OSCLISTENER_H
#define OSCLISTENER_H

#include <string>
#include <iostream>
#include <cstring>
#include <sstream>

#if defined(__BORLANDC__) // workaround for BCB4 release build intrinsics bug
namespace std {
using ::__strcmp__;  // avoid error: E2316 '__strcmp__' is not a member of 'std'.
}
#endif

#include <lib/oscpack/osc/OscReceivedElements.h>
#include <lib/oscpack/osc/OscPrintReceivedElements.h>
#include <lib/oscpack/osc/OscPacketListener.h>
#include <lib/oscpack/ip/PacketListener.h>
#include <lib/oscpack/ip/UdpSocket.h>

#include <src/input.h>


#define PORT 7000

class MyPacketListener : public PacketListener{
public:
    MyPacketListener(Input* iptr){
        inputPtr = iptr;
    }
    void parseOSC(std::string packet){
        std::string temp;
        std::string address;
        std::string valBlock;
        std::istringstream iss(packet);
        //throw out beginning brace
        if(getline(iss, temp, '[') )
        {
            // std::cout << "TEMP: " << temp << std::endl;
        }
        //grab address
        if(getline(iss, address, ' ')){
            // std::cout << "Address: " << address << std::endl;
        }

        //grab value data
        if(getline(iss, valBlock, ']')){
            // std::cout << "Data: " << valBlock << std::endl;
        }

        std::istringstream issAdr(address);
        std::vector<std::string> addressArray;
        while(getline(issAdr, temp, '/')){
            // std::cout << "Address Token: " << temp << std::endl;
            addressArray.push_back(temp);
        }

        std::istringstream issVal(valBlock);
        //grab data type
        std::string type;
        std::string val;
        if(getline(issVal, type, ':')){
            // std::cout << "Data Type: " << type << std::endl;
        }
        if(getline(issVal, val, ':')){
            // std::cout << "Data Val: " << val << std::endl;
        }


        //time to output things back to our synth
        if(addressArray[1].compare("vidos") == 0){ //make sure this is a vidos message
            if(type.compare("float32") == 0){ // only support float32s for now

                int address = -1;
                try {
                  address = stoi(addressArray[2]);
                }
                catch(...){
                  // if no conversion could be performed
                }

                if(address != -1){
                    inputPtr->setCV(address, stof(val));
                }

                
            }
        }

    }

    virtual void ProcessPacket( const char *data, int size, 
            const IpEndpointName& remoteEndpoint )
    {
        (void) remoteEndpoint; // suppress unused parameter warning
        std::stringstream sout; 

        sout << osc::ReceivedPacket( data, size );
        //make packet print into a stringstream
        parseOSC(sout.str());
        
    }
private:
    Input* inputPtr;

};


#endif