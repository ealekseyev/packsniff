#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/ether.h>
#include <linux/if_packet.h>
#include <sys/ioctl.h>
#include <csignal>
#include "InetAddress.h"

using namespace std;
typedef unsigned char uint8;

// func protos
void copyFields(uint8* pack_final, uint8* a, uint8* b, uint8* c, uint8* d);
void getIfaceMAC(uint8* retval, int sockfd, char* name);
void printMAC(uint8* mac);
void dtor(int sig);

// eth payload
uint8 eth_header[14] = {
        0xc0, 0xff, 0xee, 0xd0, 0x0d, 0x00, // src
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // destination - broadcast
        0x08, 0x06                          // ARP proto identifier
};
uint8 proto_info[6] = {
        0x00, 0x01, // hardware tye - ethernet
        0x08, 0x00, // IPv4
        0x06, 0x04  // MAC len, IP len (bytes)
};
uint8 arp_opcode[2] = {
        0x00, 0x01  // Who is __?
};
uint8 payload[20];
uint8 packet[64];

// message buffer from socket - a bit more than eth max buflen
uint8* buffer = new uint8[1600];

int main(int argc, char** argv) {
    // register ^C function
    signal(SIGINT, reinterpret_cast<__sighandler_t>(dtor));
    InetAddress src_addr = InetAddress("192.168.1.162");
    InetAddress dst_addr = InetAddress("192.168.1.168");//string(argv[1]));

    // create the socket
    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL)); //IPPROTO_RAW);

    // bind to interface at argv[1]
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), argv[1]);
    if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
        fprintf(stderr, "Error binding socket to device %s", argv[1]);
    }

    uint8 selfMAC[6]; getIfaceMAC(selfMAC, sockfd, argv[1]); printMAC(selfMAC);

    // one loop captures one packet
    while(1) {
        // get a packet
        struct sockaddr saddr; int saddr_len = sizeof saddr;
        int packlen;
        if((packlen = recvfrom(sockfd, buffer, 65536, 0, &saddr, reinterpret_cast<socklen_t*>(&saddr_len))) < 0) {
            perror("Socket Error");
            exit(0);
        }
        int counter = 0;
        // source mac
        for(int i = 0; i < 6; i++, counter++) {
            payload[counter] = eth_header[i]; //TODO:replace
        }
        // source ip
        for(int i = 0; i < 4; i++, counter++) {
            payload[counter] = src_addr.getArray()[i];
        }
        // dest mac
        for(int i = 0; i < 6; i++, counter++) {
            payload[counter] = 0; // empty
        }
        // dest mac
        for(int i = 0; i < 4; i++, counter++) {
            payload[counter] = dst_addr.getArray()[i];
        }

        copyFields(packet, eth_header, proto_info, arp_opcode, payload);

    }
    return 0;
}

void dtor(int sig) {
    signal(sig, SIG_IGN);
    cout << "\n~~Aborting~~" << endl;
    delete[] buffer;
    exit(0);
}

void copyFields(uint8* pack_final, uint8* a, uint8* b, uint8* c, uint8* d) {
    int counter = 0;
    for(int i = 0; i < 14; i++, counter++) {
        pack_final[counter] = a[i];
    }
    for(int i = 0; i < 6; i++, counter++) {
        pack_final[counter] = b[i];
    }
    for(int i = 0; i < 2; i++, counter++) {
        pack_final[counter] = c[i];
    }
    for(int i = 0; i < 20; i++, counter++) {
        pack_final[counter] = d[i];
    }
    // padding
    for(int i = 0; i < 20; i++, counter++) {
        pack_final[counter] = 0;
    }

}

void getIfaceMAC(uint8* retval, int sockfd, char* name) {
    struct ifreq if_mac;

    memset(&if_mac, 0, sizeof(struct ifreq));
    strncpy(if_mac.ifr_name, name, IFNAMSIZ-1);
    if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0)
        perror("SIOCGIFHWADDR");
    for(int i = 0; i < 6; i++) {
        retval[i] = (unsigned char) if_mac.ifr_hwaddr.sa_data[i] ;
    }
}

void printMAC(uint8* mac) {
    printf("%0.2x:%0.2x:%0.2x:%0.2x:%0.2x:%0.2x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

}