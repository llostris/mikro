#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <linux/in.h>	// ??
#include <linux/if.h>	// interface
#include <sys/ioctl.h>	//ioctl flags
#include <netpacket/packet.h> // struct sockaddr_ll

#include "ipv6.h"
#include "types.h"
#include "eth0.h"
#include "tcp.h"
#include "testing.h"


void create_tcp_hdr() {	
}


int send_tcp(uint16_t* dest_ipaddr) {
	int proto = ETH_TYPE_IP6;
	unsigned char src_hw[ETH_ADDR_LEN];	
	unsigned char dest_hw[ETH_ADDR_LEN] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };	// broadcast

	int sockfd;
	if ( (sockfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) < 0 ) {
		printf("Error: could not open a socket.\n");
		return -1;
	}

	printf("socket opened\n");

	/* Get interface index and hardware address of host */
	// wrzucic to do osobnej funkcji - DONE
	// i przechowywac w jakiejs zmiennej TODO ??

	int ifindex = 0;
	if ( get_hardware_info(&ifindex, src_hw, sockfd) < 0 ) {
		printf("Error: could not get hardware's information\n");
		return -1;
	}

	uint16_t src_ipaddr[IPV6_ADDR_LEN] = { 0xfe80, 0x0, 0xa00, 0x27ff, 0xfe5c, 0x2c16, 0x0, 0x0 };

	/* Convert to little endian */
	hton_ip_address(src_ipaddr);	
	hton_ip_address(dest_ipaddr);	

	/* Create TCP Header */
	struct tcp_header tcpheader;
	tcpheader.source_port = PORT_HTTP;
	tcpheader.destination_port = 30;
	tcpheader.sequence_number = 0;
	tcpheader.acknowledgement_number = 0;
	tcpheader.data_offset = 5;
	tcpheader.flags = TCP_FLAG_SYN;
	tcpheader.window_size = 10;
	tcpheader.checksum = checksum_pseudo(NULL, src_ipaddr, dest_ipaddr, NEXT_HDR_TCP, 0);
	tcpheader.urgent_pointer = 0;

	hton_structure(&tcpheader, TCP_HDR_LEN);


	/* Create IPv6 Header */
	struct ip6_hdr ipv6hdr;
/*
	ipv6hdr.version = 0x6;
	ipv6hdr.traffic_class = 0x0;
	ipv6hdr.flow_label = 0x0;
*/
	ipv6hdr.version = 6;
	ipv6hdr.traffic_class1 = 0;
	ipv6hdr.traffic_class2 = 0;
	ipv6hdr.flow_label1 = 0;
	ipv6hdr.flow_label2 = 0;
	ipv6hdr.payload_len = htons(TCP_HDR_LEN); // big/little endian
	ipv6hdr.next_hdr = NEXT_HDR_TCP;	
	ipv6hdr.hop_limit = DEFAULT_TTL;
	memcpy(ipv6hdr.destination_address, dest_ipaddr, IPV6_ADDR_LEN * 2); // bo mamy 16-bitowe pola - do zmiany?
	memcpy(ipv6hdr.source_address, src_ipaddr, IPV6_ADDR_LEN * 2);

	/* calculate checksum */	
//	icmphdr.checksum = checksum_pseudo(&icmphdr, ipv6hdr.source_address, ipv6hdr.destination_address, ipv6hdr.next_hdr, TCP_HDR_LEN);
	printf("\nip header created\n");

	/* Create ethernet frame */
	union ethframe frame;
	memset(frame.field.header.dest, 0, ETH_ADDR_LEN);
	memcpy(frame.field.header.src, src_hw, ETH_ADDR_LEN);
	frame.field.header.proto = htons(ETH_TYPE_IP6);
	memcpy(frame.field.data, &ipv6hdr, IPV6_HDR_LEN);
	memcpy(frame.field.data + IPV6_HDR_LEN, &tcpheader, TCP_HDR_LEN);

	//TODO
	int frame_len = ICMP_HDR_LEN + IPV6_HDR_LEN + TCP_HDR_LEN;
	printf("frame created\n");

	/* prepare the socket */

	struct sockaddr_ll sockaddr;
	memset((void*)&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sll_family = PF_PACKET;
	sockaddr.sll_ifindex = ifindex;
	sockaddr.sll_halen = ETH_ADDR_LEN;
	memcpy((void*)sockaddr.sll_addr, (void*) dest_hw, ETH_ADDR_LEN);
	
	if ( sendto(sockfd, frame.buffer, frame_len, 0, (struct sockaddr*) &sockaddr, sizeof(sockaddr)) <= 0 ) {
		printf("Error sending a frame");
		return -1;
	} 

	close(sockfd);


	printf("Everything worked!\n");
}




int main(int argc, char** argv) {
	uint16_t dest_ipaddr[IPV6_ADDR_LEN] = { 0xfe80, 0x0, 0xa00, 0x27ff, 0xfe5c, 0x2c16, 0x0, 0x0 };
	uint16_t dest_ipaddr2[IPV6_ADDR_LEN] = { 0xfe80, 0x0, 0xbd41, 0x2638, 0x69a0, 0x87d8, 0x0, 0x0 };
//	uint16_t dest_ipaddr[IPV6_ADDR_LEN] =  {0xfe80, 0x0, 0x5882, 0xd64b, 0xb962, 0x6ada, 0x0, 0x0 };
	unsigned char dest_hwaddr[6] = { 0x08, 0x00, 0x27, 0xb9, 0x87, 0x5a };
	send_tcp(dest_ipaddr2);

	return 0;
}

