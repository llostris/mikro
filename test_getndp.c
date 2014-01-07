#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <linux/in.h>	// ??
#include <linux/if.h>	// interface
#include <sys/ioctl.h>	//ioctl flags
#include <netpacket/packet.h> // struct sockaddr_ll

#include "eth0.h"
#include "ipv6.h"
//#include <eth0.h>

/* Functions testing Ethernet/IPv6/ICMP parsing */

int parse_eth_frame(union ethframe* frame, void* buffer) {
	memcpy(frame->buffer, buffer, ETH_FRAME_LEN);
	int i;
	printf("\ndestination and source address: \n");
	for ( i = 0; i < 12; i++ )
		printf("%x ", frame->field.header.dest[i]);
	if ( frame->field.header.proto == 0x1234 )
		for ( i = 0; i < 20; i++ )
			printf("%c", frame->field.data[i]);
	printf("proto: %x\n", ntohs(frame->field.header.proto));
	printf("\ndata: \n");

	//if ( frame->field.header.proto == htons(ETH_TYPE_IP6))
	for ( i = 0; i < 100; i++ )
		printf("%x", frame->field.data[i]);

}

int parsed_icmp(union ethframe* frame, struct icmp6_hdr* hdr) {
	parse_icmp(frame, hdr);
	printf("\nType: %d\n", hdr->type);
	printf("Code: %d\n", hdr->code);
	printf("Checksum: %x\n", hdr->checksum);
	printf("Reserved: %d\n", hdr->data[0]);	
	printf("Address:");

	int ind;
	for ( ind = 4; ind < ICMP_NDP_LEN; ind++ )
		printf("%x ", hdr->data[ind]);

}

int parsed_ipv6(union ethframe* frame, struct ip6_hdr* iphdr) {
	parse_ipv6(frame, iphdr);
	struct ip6_hdr_be hdr;
	memcpy(&hdr, frame->field.data, IPV6_HDR_LEN);

	printf("\nPayload: %d\n", iphdr->payload_len);
	printf("Next header: %d\n", iphdr->next_hdr);
	printf("TTL: %d\n", iphdr->hop_limit);
	printf("Version: %d\n", iphdr->version);

	ntoh_structure(&hdr, IPV6_HDR_LEN);
	printf("\nPayload: %d\n", hdr.payload_len);
}


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
		printf("-");
		
		frame_len = recvfrom(sockfd, buffer, ETH_FRAME_LEN, 0, NULL, NULL);
		if (frame_len == -1) { 
			printf("no frame... ");
			//errorhandling .... 
		} else {
			printf("%d\n", frame_len);
			union ethframe frame;
			parse_eth_frame(&frame, buffer);
			if ( ntohs(frame.field.header.proto) == ETH_TYPE_IP6 ) {
				struct ip6_hdr iphdr;
				parsed_ipv6(&frame, &iphdr);
				struct icmp6_hdr icmphdr;
				parsed_icmp(&frame, &icmphdr);
				icmp_actions(&frame, &iphdr, &icmphdr, src_ipaddr);
			}
		}
		printf("\n\n");
	}

	/* get frame's data */
	

	close(sockfd);

	printf("Everything worked!\n");
	return 0;
}

