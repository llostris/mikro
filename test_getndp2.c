#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>	// ??
#include <net/if.h>	// interface
#include <sys/ioctl.h>	//ioctl flags
#include <netpacket/packet.h> // struct sockaddr_ll

#include "eth0.h"
#include "ipv6.h"
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
	uint16_t src_ipaddr[IPV6_ADDR_LEN];
	memcpy(&src_ipaddr, &src_ip_address, 2 * IPV6_ADDR_LEN);

	void* buffer = (void*)malloc(ETH_FRAME_LEN);
	int frame_len = 0;

	printf("waiting for frames...\n");

	union ethframe frame;

	while ( 1 ) {
		frame_len = recvfrom(sockfd, buffer, ETH_FRAME_LEN, 0, NULL, NULL);
		if (frame_len == -1) { 
			printf("no frame... ");
			//errorhandling .... 
		} else {
		//	printf("%d\n", frame_len);
			memset(&frame, 0, ETH_FRAME_LEN);
			parse_eth_frame(&frame, buffer);
			if ( ntohs(frame.field.header.proto) == ETH_TYPE_IP6 ) {
				struct ip6_hdr iphdr;
				parsed_ipv6(&frame, &iphdr);
				struct icmp6_hdr icmphdr;
				parsed_icmp(&frame, &icmphdr);
				printf("\n\n");
				icmp_actions(&frame, &iphdr, &icmphdr, src_ipaddr);
				print_mac_table();
			}
		}
	}

	free(buffer);
	close(sockfd);

	printf("Everything worked!\n");
	return 0;
}

