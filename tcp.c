#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>	
#include <net/if.h>
#include <sys/ioctl.h>	//ioctl flags
#include <netpacket/packet.h> // struct sockaddr_ll

#include <types.h>
#include <ipv6.h>
#include <eth0.h>
#include <tcp.h>

void parse_tcp(union ethframe* frame, struct tcp_header* hdr) {
	memcpy(hdr, frame->field.data + IPV6_HDR_LEN, TCP_HDR_LEN);
	hdr->source_port = ntohs(hdr->source_port);
	hdr->destination_port = ntohs(hdr->destination_port);
	hdr->sequence_number = ntohl(hdr->sequence_number);
	hdr->acknowledgement_number = ntohl(hdr->acknowledgement_number);
	struct tcp_header buff;
/*	memcpy(&buff, frame->field.data + IPV6_HDR_LEN, TCP_HDR_LEN);
	ntoh_structure(&buff, TCP_HDR_LEN);
	hdr->flags = buff.flags;
	hdr->data_offset = buff.data_offset;
	hdr->reserved = buff.reserved; 
//	hdr->flags = ntohs(hdr->flags); */
	printf("%d %d %d\n", hdr->flags, hdr->reserved, hdr->data_offset); 
	
	int i;
	int data = hdr->data_offset;
	for( i = 3; i>=0; i--) {
		printf("%c", ((data & 1) + '0'));
		data >>= 1;
	}
	data = hdr->reserved;
	for( i = 2; i>=0; i--) {
		printf("%c", ((data & 1) + '0'));
		data >>= 1;
	}
	data = hdr->flags;
	for( i = 8; i>=0; i--) {
		printf("%c", ((data & 1) + '0'));
		data >>= 1;
	}
	printf("\n");
	hdr->window_size = ntohs(hdr->window_size);
	hdr->checksum = ntohs(hdr->checksum);
	hdr->urgent_pointer = ntohs(hdr->urgent_pointer);
//	ntoh_structure(hdr->flags, 4);
	//ntoh_structure(hdr, TCP_HDR_LEN);
}

void create_tcp_hdr(struct tcp_header* hdr, int dest_port, int flags, int ack_num, int seq_num, int window_size) {
	printf("\nNew tcp header");

	hdr->source_port = htons(PORT_HTTP);
	hdr->destination_port = htons(dest_port);
	hdr->sequence_number = 0;//seq_num;
	hdr->acknowledgement_number = htonl(ack_num);
	hdr->data_offset = 5;
	hdr->reserved = 0;
	hdr->flags = flags;
	hdr->window_size = htons(window_size); //window_size;
	hdr->checksum = 0;
	hdr->urgent_pointer = 0;
/*
	struct tcp_header buff;
	memcpy(&buff, hdr, TCP_HDR_LEN);
	hton_structure(&buff, TCP_HDR_LEN);
	hdr->data_offset = buff.data_offset;
	hdr->reserved = buff.reserved;
	hdr->flags = buff.flags; */
	printf("... created");
};

void reply_tcp(union ethframe* frame, struct tcp_header* tcphdr) {
	printf("\nBeggining of reply_tcp()\n");

	void* buffer = (void*) malloc(100);

	/* Change MAC addresses */
	memcpy(buffer, frame->field.header.dest, ETH_ADDR_LEN);
	memcpy(frame->field.header.dest, frame->field.header.src, ETH_ADDR_LEN);
	memcpy(frame->field.header.src, buffer, ETH_ADDR_LEN);
	
	/* Change IP addresses */
	struct ip6_hdr* iphdr;
	iphdr = (struct ip6_hdr*) frame->field.data;
	memcpy(buffer, iphdr->destination_address, 2 * IPV6_ADDR_LEN);
	memcpy(iphdr->destination_address, iphdr->source_address, 2 * IPV6_ADDR_LEN);
	memcpy(iphdr->source_address, buffer, 2 * IPV6_ADDR_LEN);

	/* Calculate TCP Checksum */
	tcphdr->checksum = checksum_pseudo(tcphdr, iphdr->source_address, iphdr->destination_address, NEXT_HDR_TCP, TCP_HDR_LEN/* ntohs(iphdr->payload_len) */);

	/* Replace the previous TCP header by a new one */
	memcpy(frame->field.data + IPV6_HDR_LEN, tcphdr, TCP_HDR_LEN /* ntohs(iphdr->payload_len)*/);
	printf("\nEnd of reply_tcp()\n"); 
}





void tcp_actions(union ethframe* frame, struct tcp_header* hdr) {
	printf("\n****** TCP ACTIONS *******\n");
	srand(time(NULL));
	switch ( hdr->flags ) {
		case 2 : {
			printf("\n---- SENDING TCP SYN-ACK ----\n");
			/* We got a TCP packet with a SYN flag: reply with SYN-ACK */
			struct tcp_header response_tcp;
			printf("%u %u\n", hdr->sequence_number, (hdr->sequence_number + 1));
			create_tcp_hdr(&response_tcp, hdr->source_port, TCP_FLAG_SYN + TCP_FLAG_ACK, hdr->sequence_number + 1, 0/* rand() */, hdr->window_size);	
			// sequence_number is a random number TO BE REMEMBERED
			printf("... getting to reply_tcp()");
			reply_tcp(frame, &response_tcp);
			printf("... sending frame ... ");
			send_frame(frame, ETH_HDR_LEN + IPV6_HDR_LEN + TCP_HDR_LEN);
			break;
		}
		case 18 : {
			/* We got ACK reply: connection established */
			break;
		}
		default : break;
	}
};

