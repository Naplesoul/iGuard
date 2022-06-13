#include <fstream>
#include <sstream>
#include "../utils/udpsender.h"

int main(){
    std::ifstream f("./node_info.json");
    std::stringstream ss;
    ss << f.rdbuf();

    UDPSender sender("59.78.8.125", 50001);
    sender.send_to_server(ss.str().c_str(), ss.str().size());
}