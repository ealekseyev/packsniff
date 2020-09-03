#include <string>
#include <iostream>
#include <cstring>

class InetAddress {
private:
    std::string inetaddr;
    int inetaddr_uint8[4];
public:
    InetAddress(std::string inetaddr) {
        this->inetaddr = std::string(inetaddr);
        std::string inetaddr_temp = std::string(inetaddr);
        for (int i = 0; i < 4; i++) {
            if (i < 3) {
                inetaddr_uint8[i] = std::stoi(inetaddr_temp.substr(0, inetaddr_temp.find('.')));
                inetaddr_temp = inetaddr_temp.substr(inetaddr_temp.find('.') + 1, inetaddr_temp.length() - 1);
            } else if (i >= 3) {
                inetaddr_uint8[i] = std::stoi(inetaddr_temp);
            }
        }
    }

    InetAddress(unsigned char one, unsigned char two, unsigned char three, unsigned char four) {
        inetaddr_uint8[0] = one;
        inetaddr_uint8[1] = two;
        inetaddr_uint8[2] = three;
        inetaddr_uint8[3] = four;
        this->inetaddr = std::to_string((int) one) +
                         "." + std::to_string((int) two) +
                         "." + std::to_string((int) three) +
                         "." + std::to_string((int) four);
    }

    std::string getString() {
        return inetaddr;
    }

    char* getCString() {
        char* inetaddr_cstr = (char*) malloc(16);
        strcpy(inetaddr_cstr, const_cast<char *>(inetaddr.c_str()));
        strcat(inetaddr_cstr, "\0");
        return inetaddr_cstr;
    }

    int* getArray() {
        return inetaddr_uint8;
    }
    //static std::string getSelfAddr() {
    // }
};