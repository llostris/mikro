#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <linux/in.h>	// ??
#include <linux/if.h>	// interface
#include <sys/ioctl.h>	//ioctl flags
#include <netpacket/packet.h> // struct sockaddr_ll

#include <types.h>
#include <ipv6.h>
#include <eth0.h>


// gdzie to wrzucic?
struct mac_table_record {
	uint16_t ip_addr[IPV6_ADDR_LEN];
	uint8_t eth_addr[ETH_ADDR_LEN];
	uint8_t ref_time;
};

struct mac_table_record mac_table[MAC_TABLE_SIZE];

/* sends ICMP packet with NDP solicitation message */
int send_ndp_solicitation(uint16_t* ip_addr) {

	char *iface = "eth0";
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
	// wrzucic to do osobnej funkcji
	// i przechowywac w jakiejs zmiennej
	int ifindex = 0;
	struct ifreq buffer;
	memset(&buffer, 0x00, sizeof(buffer));
	strncpy(buffer.ifr_name, iface, IFNAMSIZ);
	if ( ioctl(sockfd, SIOCGIFINDEX, &buffer) < 0 ) {
		printf("Error: could not get interface index.\n");
		return -1;
	}
	ifindex = buffer.ifr_ifindex;

	if (ioctl(sockfd, SIOCGIFHWADDR, &buffer) < 0) {
		printf("Error: could not get interface address\n");
 		close(sockfd);
	return -1;
  	}
	memcpy((void*)src_hw, (void*)(buffer.ifr_hwaddr.sa_data), ETH_ADDR_LEN);

	uint16_t src_ipaddr[IPV6_ADDR_LEN] = { 0xfe80, 0x0, 0xa00, 0x27ff, 0xfe5c, 0x2c16, 0x0, 0x0 };

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



/* sends ICMP packet with NDP solicitation message */
int send_ndp_advertisement(int solicited, uint16_t* dest_ip_addr, uint8_t* dest_hw_addr) {

	char *iface = "eth0";
	int proto = ETH_TYPE_IP6;
	unsigned char src_hw[ETH_ADDR_LEN];
	uint16_t src_ip_addr[IPV6_ADDR_LEN] = { 0xfe80, 0x0, 0xa00, 0x27ff, 0xfe5c, 0x2c16, 0x0, 0x0 };


	int sockfd;
	if ( (sockfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) < 0 ) {
		printf("Error: could not open a socket.\n");
		return -1;
	}

	printf("socket opened\n");

	/* Get interface index and hardware address of host */
	// wrzucic to do osobnej funkcji
	// i przechowywac w jakiejs zmiennej ?
	int ifindex = 0;
	struct ifreq buffer;
	memset(&buffer, 0x00, sizeof(buffer));
	strncpy(buffer.ifr_name, iface, IFNAMSIZ);
	if ( ioctl(sockfd, SIOCGIFINDEX, &buffer) < 0 ) {
		printf("Error: could not get interface index.\n");
		return -1;
	}
	ifindex = buffer.ifr_ifindex;

	if (ioctl(sockfd, SIOCGIFHWADDR, &buffer) < 0) {
		printf("Error: could not get interface address\n");
 		close(sockfd);
	return -1;
  	}
	memcpy((void*)src_hw, (void*)(buffer.ifr_hwaddr.sa_data), ETH_ADDR_LEN);

	printf("address aquaired\n");

	/* Convert to little endian */
	hton_ip_address(src_ip_addr);	
	if ( solicited > 0 )
		hton_ip_address(dest_ip_addr);	

	printf("addresses hton'ed\n");

	/* Create ICMP header */
	struct icmp6_hdr icmphdr;
	icmphdr.type = ICMP_NDP_ADVERT;
	icmphdr.code = 0;
	icmphdr.checksum = 0;	
	memset(icmphdr.data, 0, ICMP_RESERVED_LEN);
	if ( solicited )
		memset(icmphdr.data + 1, 1, 1);
	memcpy(icmphdr.data + ICMP_RESERVED_LEN, src_ip_addr, IPV6_ADDR_LEN * 2);

	printf("icmpv6 header created\n");	

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
	ipv6hdr.payload_len = htons(ICMP_HDR_LEN + ICMP_NDP_LEN); // big/little endian
	ipv6hdr.next_hdr = NEXT_HDR_ICMP;	
	ipv6hdr.hop_limit = DEFAULT_TTL;

	if ( solicited )
		memcpy(ipv6hdr.destination_address, dest_ip_addr, IPV6_ADDR_LEN * 2); // bo mamy 16-bitowe pola - do zmiany?
	else 
		memset(ipv6hdr.destination_address, 0xff, IPV6_ADDR_LEN * 2);
	memcpy(ipv6hdr.source_address, src_ip_addr, IPV6_ADDR_LEN * 2);

	/* Calculate ICMP checksum */	
	icmphdr.checksum = checksum_pseudo(&icmphdr, ipv6hdr.source_address, ipv6hdr.destination_address, ipv6hdr.next_hdr, ICMP_HDR_LEN + ICMP_NDP_LEN);
	printf("\nip header created\n");

	/* Create ethernet frame */
	union ethframe frame;
	if ( solicited )
		memcpy(frame.field.header.dest, dest_hw_addr, ETH_ADDR_LEN);
	else
		memset(frame.field.header.dest, 0, ETH_ADDR_LEN);
	memcpy(frame.field.header.src, src_hw, ETH_ADDR_LEN);
	frame.field.header.proto = htons(ETH_TYPE_IP6);
	memcpy(frame.field.data, &ipv6hdr, IPV6_HDR_LEN);
	memcpy(frame.field.data + IPV6_HDR_LEN, &icmphdr, ntohs(ipv6hdr.payload_len));

	int frame_len = ICMP_HDR_LEN + IPV6_HDR_LEN + ETH_HDR_LEN + ICMP_NDP_LEN;
	printf("frame created\n");

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

	printf("Everything worked!\n");
}

