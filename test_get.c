#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>	
#include <net/if.h>	
#include <sys/ioctl.h>	
#include <netpacket/packet.h> 

#include "eth0.h"
#include "ipv6.h"
#include "tcp.h"
#include "testing.h"

//#include <eth0.h>

int main(int argc, char* argv[])
{
	int sockfd;
	unsigned char src[ETH_ADDR_LEN];
	// htons(proto) jako 3 argument ??
	if ( (sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL /*ETH_TYPE_IP6 */))) < 0 ) {
		printf("Error: could not open a socket.\n");
		return -1;
	}

	int ifindex = 0;
	if ( get_hardware_info(&ifindex, src, sockfd) < 0 ) {
		printf("Error getting host's data\n");
		return -1;
	}
	uint16_t src_ipaddr[IPV6_ADDR_LEN] = { 0xfe80, 0x0, 0xa00, 0x27ff, 0xfe5c, 0x2c16, 0x0, 0x0 };

	void* buffer = (void*)malloc(ETH_FRAME_LEN);
	int frame_len = 0;

	printf("waiting for frames...\n");

	while ( 1 ) {		
		frame_len = recvfrom(sockfd, buffer, ETH_FRAME_LEN, 0, NULL, NULL);
		if (frame_len == -1) { 
			printf("no frame... ");
			//errorhandling .... 
		} else {
			union ethframe* frame;
			frame = malloc(sizeof(union ethframe));
			parse_eth_frame(frame, buffer);
			if ( ntohs(frame->field.header.proto) == ETH_TYPE_IP6  && mac_address_compare(frame->field.header.dest, src_mac_address) ) {
				struct ip6_hdr iphdr;
				parsed_ipv6(frame, &iphdr);
				if ( iphdr.next_hdr == NEXT_HDR_TCP ) {
					printf("\n---- TCP PACKET!!! ----- \n");
					struct tcp_header tcphdr;
					parsed_tcp(frame, &tcphdr);
					tcp_actions(frame, &tcphdr);
				}
			}
			free(frame);
		}
	}

	close(sockfd);

	printf("Everything worked!\n");
	return 0;
}

