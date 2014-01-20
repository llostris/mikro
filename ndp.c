#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>	// ??
#include <net/if.h>	// interface
#include <sys/ioctl.h>	//ioctl flags
#include <netpacket/packet.h> // struct sockaddr_ll

#include <types.h>
#include <ipv6.h>
#include <eth0.h>


/* Sends ICMP packet with NDP solicitation message */
int send_ndp_solicitation(uint16_t* ip_addr) {
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
	int ifindex = 0;
	if ( get_hardware_info(&ifindex, src_hw, sockfd) < 0 ) {
		printf("Error: could not get hardware's information\n");
		return -1;
	}

	uint16_t src_ipaddr[IPV6_ADDR_LEN] = { 0xfe80, 0x0, 0xa00, 0x27ff, 0xfe5c, 0x2c16, 0x0, 0x0 };
	memcpy(&src_ipaddr, &src_ip_address, 2 * IPV6_ADDR_LEN );


	/* Convert to little endian */
	hton_ip_address(src_ipaddr);	
	hton_ip_address(ip_addr);	

	/* Create ICMP header */
	struct icmp6_hdr icmphdr;
	icmphdr.type = ICMP_NDP_SOLICIT;
	icmphdr.code = 0;
	icmphdr.checksum = 0;
	memset(icmphdr.data, 0, ICMP_RESERVED_LEN);
	memcpy(icmphdr.data + ICMP_RESERVED_LEN, ip_addr, IPV6_ADDR_LEN * 2);	

	/* Create IPv6 Header */
	struct ip6_hdr ipv6hdr;
	ipv6hdr.version = 6;
	ipv6hdr.traffic_class1 = 0;
	ipv6hdr.traffic_class2 = 0;
	ipv6hdr.flow_label1 = 0;
	ipv6hdr.flow_label2 = 0;
	ipv6hdr.payload_len = htons(ICMP_HDR_LEN + ICMP_NDP_LEN); // big/little endian
	ipv6hdr.next_hdr = NEXT_HDR_ICMP;	
	ipv6hdr.hop_limit = DEFAULT_TTL;
	memcpy(ipv6hdr.destination_address, ip_addr, IPV6_ADDR_LEN * 2); // bo mamy 16-bitowe pola - do zmiany?
	memcpy(ipv6hdr.source_address, src_ipaddr, IPV6_ADDR_LEN * 2);

	/* calculate checksum */	
	icmphdr.checksum = checksum_pseudo(&icmphdr, ipv6hdr.source_address, ipv6hdr.destination_address, ipv6hdr.next_hdr, ICMP_HDR_LEN + ICMP_NDP_LEN);
	printf("\nip header created\n");

	/* Create ethernet frame */
	union ethframe frame;
	memset(frame.field.header.dest, 0, ETH_ADDR_LEN);
	memcpy(frame.field.header.src, src_hw, ETH_ADDR_LEN);
	frame.field.header.proto = htons(ETH_TYPE_IP6);
	memcpy(frame.field.data, &ipv6hdr, IPV6_HDR_LEN);
	//memcpy(frame.field.data + IPV6_HDR_LEN, &icmphdr, ICMP_HDR_LEN + ICMP_NDP_LEN);
	memcpy(frame.field.data + IPV6_HDR_LEN, &icmphdr, ntohs(ipv6hdr.payload_len));


	int frame_len = ICMP_HDR_LEN + IPV6_HDR_LEN + ETH_HDR_LEN + ICMP_NDP_LEN;
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



/* Sends ICMP packet with NDP advertisement message */
int send_ndp_advertisement(int solicited, uint16_t* dest_ip_addr, uint8_t* dest_hw_addr) {
	int proto = ETH_TYPE_IP6;
	unsigned char src_hw[ETH_ADDR_LEN];
	unsigned char dest_hw[ETH_ADDR_LEN];
	uint16_t src_ip_addr[IPV6_ADDR_LEN] = { 0xfe80, 0x0, 0xa00, 0x27ff, 0xfe5c, 0x2c16, 0x0, 0x0 };
	memcpy(&src_ip_addr, &src_ip_address, 2 * IPV6_ADDR_LEN );

	uint16_t ipv6_multicast[IPV6_ADDR_LEN] = { 0xff02, 0x0, 0x0001, 0x0, 0x0, 0x0, 0x0, 0x0 };

	int sockfd;
	if ( (sockfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) < 0 ) {
		printf("Error: could not open a socket.\n");
		return -1;
	}

//	printf("socket opened\n");

	/* Get interface index and hardware address of host */
	int ifindex = 0;
	if ( get_hardware_info(&ifindex, src_hw, sockfd) < 0 ) {
		printf("Error: could not get hardware's information\n");
		return -1;
	}

//	printf("address aquaired\n");

	/* Convert to little endian */
	hton_ip_address(src_ip_addr);	
	if ( solicited > 0 )
		hton_ip_address(dest_ip_addr);
	hton_ip_address(ipv6_multicast);	

//	printf("addresses hton'ed\n");

	/* Create ICMP header */
	struct icmp6_hdr icmphdr;
	icmphdr.type = ICMP_NDP_ADVERT;
	icmphdr.code = 0;
	icmphdr.checksum = 0;	
	memset(icmphdr.data, 0, ICMP_RESERVED_LEN);
	if ( solicited )
		memset(icmphdr.data, ICMP_FLAG_SOLICITED, 1);	// set a proper flag
	else
		memset(icmphdr.data, ICMP_FLAG_OVERRIDE, 1);

	/* Target Address */
	if ( solicited )
		memcpy(icmphdr.data + ICMP_RESERVED_LEN, dest_ip_addr, IPV6_ADDR_LEN * 2);
	else
		memcpy(icmphdr.data + ICMP_RESERVED_LEN, src_ip_addr, IPV6_ADDR_LEN * 2);

//	printf("icmpv6 header created\n");	

	/* Create IPv6 Header */
	struct ip6_hdr ipv6hdr;
	ipv6hdr.version = 6;
	ipv6hdr.traffic_class1 = 0;
	ipv6hdr.traffic_class2 = 0;
	ipv6hdr.flow_label1 = 0;
	ipv6hdr.flow_label2 = 0;
	ipv6hdr.payload_len = htons(ICMP_HDR_LEN + ICMP_NDP_LEN); // big/little endian
	ipv6hdr.next_hdr = NEXT_HDR_ICMP;	
	ipv6hdr.hop_limit = DEFAULT_TTL;

	memcpy(ipv6hdr.source_address, src_ip_addr, IPV6_ADDR_LEN * 2);
	if ( solicited )
		memcpy(ipv6hdr.destination_address, dest_ip_addr, IPV6_ADDR_LEN * 2); // bo mamy 16-bitowe pola - do zmiany?
	else 
		memcpy(ipv6hdr.destination_address, ipv6_multicast, IPV6_ADDR_LEN * 2);
	/* Calculate ICMP checksum */	
	icmphdr.checksum = checksum_pseudo(&icmphdr, ipv6hdr.source_address, ipv6hdr.destination_address, ipv6hdr.next_hdr, ICMP_HDR_LEN + ICMP_NDP_LEN);
	printf("\nip header created\n");

	/* Create ethernet frame */
	union ethframe frame;
	if ( solicited )
		memcpy(frame.field.header.dest, dest_hw_addr, ETH_ADDR_LEN);
	else {
		/* IPV6 multicast */
		memset(frame.field.header.dest, 0x33, 2);
		memset(frame.field.header.dest + 2, 0x0, 1);
		memcpy(frame.field.header.dest + 3, src_hw + 3, 3);
	}
	memcpy(frame.field.header.src, src_hw, ETH_ADDR_LEN);
	frame.field.header.proto = htons(ETH_TYPE_IP6);
	memcpy(frame.field.data, &ipv6hdr, IPV6_HDR_LEN);
	memcpy(frame.field.data + IPV6_HDR_LEN, &icmphdr, ICMP_HDR_LEN + ICMP_NDP_LEN);

	int frame_len = ICMP_HDR_LEN + IPV6_HDR_LEN + ETH_HDR_LEN + ICMP_NDP_LEN;

	/* prepare the socket */

	struct sockaddr_ll sockaddr;
	memset((void*)&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sll_family = PF_PACKET;
	sockaddr.sll_ifindex = ifindex;
	sockaddr.sll_halen = ETH_ADDR_LEN;
	memcpy((void*)sockaddr.sll_addr, (void*) frame.field.header.dest, ETH_ADDR_LEN);
	
	if ( sendto(sockfd, frame.buffer, frame_len, 0, (struct sockaddr*) &sockaddr, sizeof(sockaddr)) <= 0 ) {
		printf("Error sending a frame");
		return -1;
	} 

	close(sockfd);

//	printf("Everything worked!\n");
}

void echo_reply(union ethframe* frame) {
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

	printf("-- addresses swapped --\n");

	/* Change ICMP Packet */
	struct icmp6_hdr* icmphdr;
	icmphdr = (struct icmp6_hdr*) (frame->field.data + IPV6_HDR_LEN);
	icmphdr->type = ICMP_ECHO_REPLY;
	icmphdr->code = 0;
	icmphdr->checksum = 0;

	printf("-- icmp packet done \n");
//	printf("Payload: %d\n", iphdr->payload_len);

	/* Calculate Checksum */
	icmphdr->checksum = checksum_pseudo(icmphdr, iphdr->source_address, iphdr->destination_address, NEXT_HDR_ICMP, (uint32_t) ntohs(iphdr->payload_len));

	printf("\n checksum calculated\n");
	
	send_frame(frame, ETH_HDR_LEN + IPV6_HDR_LEN + ntohs(iphdr->payload_len));
	printf("\nEnd of echo_reply()\n"); 
}


int send_ndp_advert(union ethframe* frame) {
	printf("\nBeggining of ndp advert()\n");
	unsigned char src_hw[ETH_ADDR_LEN];	
	unsigned char dest_hw[ETH_ADDR_LEN] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };	// broadcast

	int sockfd;
	if ( (sockfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) < 0 ) {
		printf("Error: could not open a socket.\n");
		return -1;
	}

	/* Get interface index and hardware address of host */
	int ifindex = 0;
	if ( get_hardware_info(&ifindex, src_hw/*frame->field.header.src*/, sockfd) < 0 ) {
		printf("Error: could not get hardware's information\n");
		return -1;
	}

	/* Create ICMP Header */

	struct icmp6_hdr icmphdr;
	icmphdr.code = 0;
	icmphdr.type = ICMP_NDP_ADVERT;
	icmphdr.checksum = 0;
	memset(icmphdr.data, ICMP_FLAG_SOLICITED + ICMP_FLAG_OVERRIDE, 1); 
	memset(icmphdr.data + 1, 0, 3);

	void* buffer = (void*) malloc(100);

	/* Change MAC addresses */
	memcpy(buffer, frame->field.header.dest, ETH_ADDR_LEN);
	memcpy(frame->field.header.dest, frame->field.header.src, ETH_ADDR_LEN);
	memcpy(frame->field.header.src, buffer, ETH_ADDR_LEN);
	
	/* Change IP addresses */
	struct ip6_hdr* iphdr;
	iphdr = (struct ip6_hdr*) frame->field.data;
	memcpy(iphdr->destination_address, iphdr->source_address, 2 * IPV6_ADDR_LEN);
	memcpy(iphdr->source_address, src_ip_address, 2 * IPV6_ADDR_LEN);
	hton_structure(iphdr->source_address, 2 * IPV6_ADDR_LEN);

	/* Replace the previous ICMP header by a new one */
	memcpy(frame->field.data + IPV6_HDR_LEN, &icmphdr, ICMP_HDR_LEN + ICMP_RESERVED_LEN);
	struct icmp6_hdr* ptr;
	ptr = (struct icmp6_hdr*) (frame->field.data + IPV6_HDR_LEN);
	
	// if options == 1 -> set options = 2 & MAC addr
	memset(frame->field.data + IPV6_HDR_LEN + ICMP_HDR_LEN + ICMP_NDP_LEN, 2, 1);
	memcpy(frame->field.data + IPV6_HDR_LEN + ICMP_HDR_LEN + ICMP_NDP_LEN + 2, src_hw, ETH_ADDR_LEN);

	/* Calculate TCP Checksum */
	int len = ntohs(iphdr->payload_len);
	ptr->checksum = checksum_pseudo(ptr, iphdr->source_address, iphdr->destination_address, NEXT_HDR_ICMP, len);
	printf("\n checksum calculated\n");

	/* Send frame */
	int frame_len = ETH_HDR_LEN + IPV6_HDR_LEN + ntohs(iphdr->payload_len);
	memcpy(frame->field.header.src, src_hw, ETH_ADDR_LEN);

	struct sockaddr_ll sockaddr;
	memset((void*)&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sll_family = PF_PACKET;
	sockaddr.sll_ifindex = ifindex;
	sockaddr.sll_halen = ETH_ADDR_LEN;
	memcpy((void*)sockaddr.sll_addr, (void*) dest_hw /*frame->field.header.dest*/, ETH_ADDR_LEN);
	
	if ( sendto(sockfd, frame->buffer, frame_len, 0, (struct sockaddr*) &sockaddr, sizeof(sockaddr)) <= 0 ) {
		printf("Error sending a frame");
		return -1;
	} 

	printf("\n *** FRAME SENT.");	// debugging

	close(sockfd);

	printf("\nEnd of ndp_advert()\n"); 
	
}



void icmp_actions(union ethframe* frame, struct ip6_hdr* iphdr, struct icmp6_hdr* icmphdr, uint16_t* src_ipaddr) {
	uint16_t ipaddr[IPV6_ADDR_LEN];
	memcpy(&ipaddr, icmphdr->data + ICMP_RESERVED_LEN, 2 * IPV6_ADDR_LEN);
	ntoh_structure(ipaddr, 2 * IPV6_ADDR_LEN);
	
	if ( icmphdr->type == ICMP_NDP_SOLICIT ) {
		if (ipv6_addr_compare(iphdr->destination_address, src_ipaddr) || ipv6_addr_compare(ipaddr, src_ip_address) ) {
			printf("-- SENDING NDP ADVERTISEMENT\n");
			send_ndp_advert(frame);
		}
	} else if ( icmphdr->type == ICMP_NDP_ADVERT ) {
		update_mac_table(iphdr->source_address, frame->field.header.src);
	}
	if ( ipv6_addr_compare(iphdr->destination_address, src_ip_address) && icmphdr->type == ICMP_ECHO_REQUEST && icmphdr->code == 0) {
		printf("-- REPLYING TO ECHO\n");
		echo_reply(frame);
	}
}

void update_mac_table(uint16_t* ipaddr, uint8_t* macaddr) {
	/* testing
	printf("src address: ");
	int i;
	for ( i = 0; i < ETH_ADDR_LEN; i++ )
		printf("%x ", macaddr[i]);
	printf(" --- ");
	for ( i = 0; i < 2 * IPV6_ADDR_LEN; i++ )
		printf("%x ", ipaddr[i]);
	*/

	int iter = 0;
	for ( iter = 0; iter < mac_index; iter++ ) {
		if ( ipv6_addr_compare(ipaddr, mac_table[iter].ip_addr) == 1 )
			break;
	}
	if ( iter == mac_index ) {
		memcpy(mac_table[iter].ip_addr, ipaddr, 2 * IPV6_ADDR_LEN);
		mac_index++;
	}
	memcpy(mac_table[iter].mac_addr, macaddr, ETH_ADDR_LEN);
	mac_table[iter].ref_time = MAC_TABLE_TIME;

//	printf("------- IP/MAC ADDRESS SAVED ------ \n");
}

uint8_t lookup_mac_address(uint16_t* ip_addr, uint8_t* mac_buffer) {
	int iter = 0;
	for ( iter = 0; iter < mac_index; iter++ ) {
		if ( ipv6_addr_compare(ip_addr, mac_table[iter].ip_addr) == 1 ) {
			memcpy(mac_buffer, mac_table[iter].ip_addr, IPV6_ADDR_LEN);
			break;
		}
	}

	if ( iter == mac_index )
		return -1;	// IP address not found in a table
	return 0;
}

void print_mac_table() {
	int iter = 0;
	int i = 0;
	printf("------ MAC TABLE ------ \n");
	for ( iter = 0; iter < mac_index; iter++ ) {
		printf("\nIP: ");
		for ( i = 0; i < IPV6_ADDR_LEN; i++ )
			printf("%x ", mac_table[iter].ip_addr[i]);
		printf("\nMAC: ");
		for ( i = 0; i < ETH_ADDR_LEN; i++ )
			printf("%x ", mac_table[iter].mac_addr[i]);
	}
}



