#ifndef TESTING_H
#define TESTING_H


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
#include "tcp.h"

/* Functions testing Ethernet/IPv6/ICMP parsing */

int parsed_ipv6(union ethframe* frame, struct ip6_hdr* iphdr) {
	parse_ipv6(frame, iphdr);
	struct ip6_hdr_be hdr;
	memcpy(&hdr, frame->field.data, IPV6_HDR_LEN);
/*
	printf("\nPayload: %d\n", iphdr->payload_len);
	printf("Next header: %d\n", iphdr->next_hdr);
	printf("TTL: %d\n", iphdr->hop_limit);
	printf("Version: %d\n", iphdr->version);

	ntoh_structure(&hdr, IPV6_HDR_LEN);
	printf("\nSource/dest addr: \n");
	int ind;
	for ( ind = 0; ind < 2 * IPV6_ADDR_LEN; ind++ )
		printf("%x ", iphdr->source_address[ind]);
*/

}


int parse_eth_frame(union ethframe* frame, void* buffer) {
	memcpy(frame->buffer, buffer, ETH_FRAME_LEN);
	int i;
	printf("\ndestination and source address: \n");
	for ( i = 0; i < 12; i++ )
		printf("%x ", frame->field.header.dest[i]);
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


int parsed_tcp(union ethframe* frame, struct tcp_header* hdr) {
	parse_tcp(frame, hdr);
	printf("\nSource port: %d\n", hdr->source_port);
	printf("Destination port: %d\n", hdr->destination_port);
	printf("Sequence number: %d\n", hdr->sequence_number);
	printf("Acknowledgement number: %d\n", hdr->acknowledgement_number);	
	printf("Flags: %d\n", hdr->flags);
	printf("Data offset: %d\n", hdr->data_offset);
	printf("Window size: %d\n", hdr->window_size);
	printf("Checksum: %x\n", hdr->checksum);	
	//printf("Urgent pointer:");
	printf("\n\n");
};

#endif
