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

    // one loop captures and prints one packet
    int loopcount; // amount of packets captured
    while(1) {
        // get a packet
        struct sockaddr saddr; int saddr_len = sizeof saddr;
        int packlen;
        if((packlen = recvfrom(sockfd, buffer, 65536, 0, &saddr, reinterpret_cast<socklen_t*>(&saddr_len))) < 0) {
            perror("Socket Error");
            exit(0);
        }

        // print the hexes
        printf("[%d]: Packlen = %d, Addr = %s\n\t", loopcount, packlen, saddr.sa_data );
        for(int pack_ind = 0; pack_ind < packlen; pack_ind++) {
            // print a raw hex
            printf("%.2X ", buffer[pack_ind]);

            // if the loop is on its last iter and the hex isn't in a perfect block
            if(pack_ind + 1 == packlen && packlen % 16 != 0) {
                for(int i = 0; i < 16-(packlen%16); i++) {
                    printf("-- ");
                    // makes the next code block think it's time to print ascii (for the last row)
                    pack_ind++;
                }
            }

            // once 16 chars have been printed, print the ascii table
            if((pack_ind+1) % 16 == 0 && pack_ind > 1) {
                // print the ascii table
                printf("   ");
                for(int i = pack_ind - 16; i < pack_ind; i++) {
                    if(buffer[i] > 32 && buffer[i] < 126) {
                        printf("%c", (char) buffer[i]);
                    } else {
                        printf("-");
                    }
                }
                // also start a new line with tab
                printf("\n\t");
            }
        }
        // after done printing a packet
        printf("\n\n");
        loopcount++;
    }
    return 0;
}

void dtor(int sig) {
    signal(sig, SIG_IGN);
    cout << "\n~~Aborting~~" << endl;
    delete[] buffer;
    exit(0);
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