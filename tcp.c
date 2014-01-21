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
	hdr->window_size = ntohs(hdr->window_size);
	hdr->urgent_pointer = ntohs(hdr->urgent_pointer);
}

void create_tcp_hdr(struct tcp_header* hdr, int dest_port, int flags, int ack_num, int seq_num, int window_size, int data_offset) {
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
};

void reply_tcp(union ethframe* frame, struct tcp_header* tcphdr) {
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

	/* Calculate TCP Checksum */
	tcphdr = (struct tcp_header*) (frame->field.data + IPV6_HDR_LEN);
	tcphdr->checksum = checksum_pseudo(tcphdr, iphdr->source_address, iphdr->destination_address, NEXT_HDR_TCP, ntohs(iphdr->payload_len));

	free(buffer);
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
			create_tcp_hdr(&response_tcp, hdr->source_port, TCP_FLAG_SYN + TCP_FLAG_ACK, hdr->sequence_number + 1, rand() * 24000, hdr->window_size, hdr->data_offset);	

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
			/* We got a HTTP GET REQUEST */			

			unsigned char* request;
			request = (unsigned char*) (frame->field.data + IPV6_HDR_LEN + TCP_HDR_LEN);
			if ( is_httpget(request) == -1 )
				break;	/* This is not our HTTP GET */
			printf("\n------ HTTP GET ------\n");

//			printf("%s\n", request);

			unsigned char* file = malloc(300);
			unsigned char* response = malloc(400);
			memset(file, 0, 300);
			memset(response, 0, 400);
//			parse_file(file);
			choose_file(request, file);
			sprintf(response, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n%s", strlen(file), file);
			//printf("RESP: \n%s\n", response);

			struct tcp_header response_tcp;
			create_tcp_hdr(&response_tcp, hdr->source_port, TCP_FLAG_PSH + TCP_FLAG_ACK, hdr->sequence_number + 1, hdr->acknowledgement_number, hdr->window_size, hdr->data_offset);
			
			struct ip6_hdr* iphdr;
			iphdr = (struct ip6_hdr*) frame->field.data;
			iphdr->payload_len = htons(TCP_HDR_LEN + strlen(response));

			add_http(frame, response, strlen(response));
			reply_tcp(frame, &response_tcp);	

			send_frame(frame, ETH_HDR_LEN + IPV6_HDR_LEN + ntohs(iphdr->payload_len));
			
			//finalize(frame);
			
			free(file);
			free(response);
			break;
		}

		case TCP_FLAG_FIN : {
			struct ip6_hdr* iphdr;
			iphdr = (struct ip6_hdr*) frame->field.data;
			iphdr->payload_len = iphdr->payload_len;

			struct tcp_header* hdr;
			hdr = (struct tcp_header*) (frame->field.data + IPV6_HDR_LEN);
			create_tcp_hdr(hdr, hdr->source_port, TCP_FLAG_FIN + TCP_FLAG_ACK, hdr->sequence_number + 1, hdr->acknowledgement_number, hdr->window_size, hdr->data_offset);

		//	oldtcp->checksum = checksum_pseudo(oldtcp, iphdr->source_address, iphdr->destination_address, NEXT_HDR_TCP, TCP_HDR_LEN);
			reply_tcp(frame, hdr);	

			send_frame(frame, ETH_HDR_LEN + IPV6_HDR_LEN + ntohs(iphdr->payload_len));
		}
		
		default : break;
	}
};


void add_http(union ethframe* frame, char* file, int file_len) {
	unsigned char* tmp;
	tmp = (unsigned char*) (frame->field.data + IPV6_HDR_LEN + TCP_HDR_LEN);
	strncpy(tmp, file, file_len);
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



