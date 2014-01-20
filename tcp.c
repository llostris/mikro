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
#include <string.h>

#include <types.h>
#include <ipv6.h>
#include <eth0.h>
#include <tcp.h>
#include <pagehtml.h>

void parse_tcp(union ethframe* frame, struct tcp_header* hdr) {
	memcpy(hdr, frame->field.data + IPV6_HDR_LEN, TCP_HDR_LEN);
	hdr->source_port = ntohs(hdr->source_port);
	hdr->destination_port = ntohs(hdr->destination_port);
	hdr->sequence_number = ntohl(hdr->sequence_number);
	hdr->acknowledgement_number = ntohl(hdr->acknowledgement_number);
	struct tcp_header buff;
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
	//hdr->checksum = ntohs(hdr->checksum);
	hdr->urgent_pointer = ntohs(hdr->urgent_pointer);
}

void create_tcp_hdr(struct tcp_header* hdr, int dest_port, int flags, int ack_num, int seq_num, int window_size, int data_offset) {
	printf("\nNew tcp header");

	hdr->source_port = htons(PORT_HTTP);
	hdr->destination_port = htons(dest_port);
	hdr->sequence_number = htonl(seq_num);
	hdr->acknowledgement_number = htonl(ack_num);
	hdr->data_offset = data_offset;
	hdr->reserved = 0;
	hdr->flags = flags;
	hdr->window_size = htons(window_size);
	hdr->checksum = 0;
	hdr->urgent_pointer = 0;
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

	/* Replace the previous TCP header by a new one */
	memcpy(frame->field.data + IPV6_HDR_LEN, tcphdr, TCP_HDR_LEN);
	//tcphdr = frame->field.data + IPV6_HDR_LEN;
	/* Calculate TCP Checksum */
	tcphdr = (struct tcp_header*) (frame->field.data + IPV6_HDR_LEN);
	tcphdr->checksum = checksum_pseudo(tcphdr, iphdr->source_address, iphdr->destination_address, NEXT_HDR_TCP, ntohs(iphdr->payload_len));

	free(buffer);
	printf("\nChecksum: %x\n", tcphdr->checksum);
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
		//	printf("%u %u\n", hdr->sequence_number, (hdr->sequence_number + 1));
			create_tcp_hdr(&response_tcp, hdr->source_port, TCP_FLAG_SYN + TCP_FLAG_ACK, hdr->sequence_number + 1, 0/* rand() */, hdr->window_size, hdr->data_offset);	

			// sequence_number is a random number TO BE REMEMBERED

			reply_tcp(frame, &response_tcp);
			struct ip6_hdr* iphdr;
			iphdr = (struct ip6_hdr*) frame->field.data;
			hdr = (struct tcp_header*) (frame->field.data + IPV6_HDR_LEN);
		//	printf("\nChecksum before sending: %x", hdr->checksum);
			send_frame(frame, ETH_HDR_LEN + IPV6_HDR_LEN + ntohs(iphdr->payload_len));
			break;
		}

		case 24 : {
			printf("\n------ HTTP GET ------\n");
			/* We got a HTTP GET */			

			unsigned char* tmp;
			tmp = (unsigned char*) (frame->field.data + IPV6_HDR_LEN + TCP_HDR_LEN);
			if ( is_httpget(tmp) == -1 )
				break;	/* This is not our HTTP GET */
	
			printf("%s\n\n", tmp);

			unsigned char* file;
			file = malloc(sizeof(char));			
			printf("\n... parsing file ..\n");
			//parse_file(file);
			file = "HTTP/1.1 200 OK\r\nContent-Length: 76\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n<html><head><title>Mikro Project</title></head><body>Send page</body></html>";

			printf("FILE: %s\n", file);

			struct tcp_header response_tcp;
			create_tcp_hdr(&response_tcp, hdr->source_port, TCP_FLAG_PSH + TCP_FLAG_ACK, hdr->sequence_number, hdr->acknowledgement_number, hdr->window_size, hdr->data_offset);
			// sequence_number is a random number TO BE REMEMBERED
			
			struct ip6_hdr* iphdr;
			iphdr = (struct ip6_hdr*) frame->field.data;
			iphdr->payload_len = htons(TCP_HDR_LEN + strlen(file));

			add_http(frame, file, strlen(file));
			reply_tcp(frame, &response_tcp);	

			send_frame(frame, ETH_HDR_LEN + IPV6_HDR_LEN + ntohs(iphdr->payload_len));
			finalize(frame);
			break;
		}

		case TCP_FLAG_FIN : {
			break;
		}
		
		default : break;
	}
};


void add_http(union ethframe* frame, char* file, int file_len) {
	printf("Count %d: \n", file_len);
	unsigned char* tmp;
	tmp = (unsigned char*) (frame->field.data + IPV6_HDR_LEN + TCP_HDR_LEN);
	strncpy(tmp, file, file_len);
	printf("... added http.. \n");
}



void finalize(union ethframe* frame) {
	struct ip6_hdr* iphdr;
	iphdr = (struct ip6_hdr*) frame->field.data;
	iphdr->payload_len = htons(TCP_HDR_LEN);

	struct tcp_header* oldtcp;
	oldtcp = (struct tcp_header*) (frame->field.data + IPV6_HDR_LEN);

	oldtcp->checksum = 0;
	oldtcp->flags = TCP_FLAG_FIN;
	oldtcp->acknowledgement_number = 0;
	oldtcp->checksum = checksum_pseudo(oldtcp, iphdr->source_address, iphdr->destination_address, NEXT_HDR_TCP, TCP_HDR_LEN);

	send_frame(frame, ETH_HDR_LEN + IPV6_HDR_LEN + TCP_HDR_LEN);
}



