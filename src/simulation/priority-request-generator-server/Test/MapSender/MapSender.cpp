#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <UdpSocket.h>
#include <unistd.h>
#include "json/json.h"
using std::vector;
using std::ofstream;
unsigned int microseconds = 1000000;

int main()
{

    UdpSocket mapSenderSocket(20061);
    // const string LOCALHOST = "127.0.0.1";
    const string LOCALHOST = "10.12.6.56";
    const int receiverPortNo = 20022;
    Json::Value jsonObject;
    Json::FastWriter fastWriter;
    Json::StyledStreamWriter styledStreamWriter;
    ofstream outputter("map.json");
    std::string sendingJsonString{};
   
    jsonObject["MsgType"] = "MAP";
    jsonObject["IntersectionName"] = "Map26379";
    jsonObject["MapPayload"] = "001282313806302030ce161948dbba702927d34f2bf802dc051870a96008a000001480022d4ad486f40c404e01450da4ef80a40282852250000c916200062c0214000002000045b2d7f6de4ef80af02879b50170148050a0244600018b00c500000080001170d53db7a7fe036c0a286d12ec052014140910800062c0414000001400005cd9210cfd6606b0282852168000290510001180522000002000005cb2c5edb7aa80390285060188800000800001739a54b6dede004c0a14180722000002000005d706dcdb74c7fae02850b020d000000240001d200df838648218120e4001058126800000400010e468715242b4428b6023d71ac09f61c1f81240916800082c0a340000010000070d238d92174238412208000630168400000400000de626f5243323d99607aa000000a0000222e6eb6a813c04e813ec2916400054880800296062a0000014800222df2cb69fbb40a6013ecd37ee80c902788520f0000490b200022c0d5400000200004452687cd3d6380fb027b1a74cd02d804f60241a00008b0395000000800011120adbb4f24e038409ec69e03c0cf813d80905800021810620000020000042d5656d53177f76027d8604588000008000010a55fab54ca9ff6809f618126200000200000426fab6d53308013027d8b04dd000000240001aedb31e37ba7c241224400085828e800000400010db5f98823ee9ecd95ff7aeaa9b0067c58604fb0242e00010b055d000000400001bcab31047c3f4e80482a00038c0b41000001000003878e5bcafc8944400";
    jsonObject["IntersectionID"] = 26379;
    jsonObject["RegionalID"] = 0;
    sendingJsonString = fastWriter.write(jsonObject);


    std::cout << "JsonString: " << sendingJsonString << std::endl;
    // mapSenderSocket.sendData(LOCALHOST, receiverPortNo, sendingJsonString);
    styledStreamWriter.write(outputter, jsonObject);
    while(true)
    {
        mapSenderSocket.sendData(LOCALHOST, receiverPortNo, sendingJsonString);
        usleep(microseconds);
    }
    
}