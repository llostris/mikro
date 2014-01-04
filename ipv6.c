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


/* Converts IPv6 address to big endian */
void hton_ip_address(uint16_t* ip_address) {
	int index = 0;
	for(index = 0; index < IPV6_ADDR_LEN; index++) {
		ip_address[index] = htons(ip_address[index]);
	}
}

/* Calculates ICMP checksum */
uint16_t checksum(void* data, uint16_t length) {
	printf("checksum calculation\n");
	uint16_t *dataptr = data;
	long sum = 0;

	/* Add 16-bit words to accumulator */
	while (length > 1)  {
		sum += *dataptr++;
		length -= 2;
	}

	/* Odd byte handling */
	if (length == 1) {
		sum += *(unsigned char *)dataptr;
	}

	sum = (sum >> 16) + (sum & 0xffff);	/* add high 16 to low 16 */
	sum += (sum >> 16);			/* add carry */
	sum = ~sum;				/* truncate to 16 bits */

	return sum;
}

/* Calculates checksum for IPv6 pseudoheader */
// might not work for not-ndp packets - TOCHECK
uint16_t checksum_pseudo(void* data, uint16_t* src_addr, uint16_t* dest_addr, uint8_t next_hdr, uint8_t data_len) {
	/* ipv6 pseudo-header used to calculate the checksum contains: source address, destination address, icmpv6 length, 3 octets of 0s and next header's id */

	uint16_t pseudo_hdr_len = 4 * IPV6_ADDR_LEN + 8 + data_len;
	uint8_t buf[pseudo_hdr_len];
	memset(buf, 0x0, pseudo_hdr_len);
	memcpy(buf, src_addr, IPV6_ADDR_LEN * 2);
	memcpy(buf + 2 * IPV6_ADDR_LEN, dest_addr, IPV6_ADDR_LEN * 2);
	memset(buf + 4 * IPV6_ADDR_LEN + 3, data_len, 1);	// uwaga! problem dla duzych pakietow - wykorzystac htons/htonl? // TODO
	memset(buf + 4 * IPV6_ADDR_LEN + 7, next_hdr, 1);	/* 3 octets of 0's and next header's id */
	memcpy(buf + 4 * IPV6_ADDR_LEN + 8, data, data_len);

	// for testing
/*
	int i;
	for ( i = 0; i < pseudo_hdr_len; i++ )
	{
		printf("%x ", buf[i]);
		if ( (i + 1) % 16 == 0 )
			printf("\n");
	}
*/	

	return checksum(&buf, pseudo_hdr_len);
}

/* Returns 1 when addr1 and addr2 are the same, 0 otherwise */
uint8_t ipv6_addr_compare(uint16_t* addr1, uint16_t* addr2) {
	uint8_t result = 1;
	uint8_t ind = 0;
	for ( ind = 0; ind < IPV6_ADDR_LEN; ind++ )
		result = result && ( addr1[ind] == addr2[ind]);
	return result;
}

/* Converts little endian to big endian where needed */
void parse_ipv6(union ethframe* frame, struct ip6_hdr* iphdr) {
	memcpy(iphdr, frame->field.data, IPV6_HDR_LEN);
	iphdr->payload_len = ntohs(iphdr->payload_len);
	hton_ip_address(iphdr->source_address);
	hton_ip_address(iphdr->destination_address);
	iphdr->flow_label2 = ntohs(iphdr->flow_label2);
}

// na razie dziala tylko dla NDP
void parse_icmp(union ethframe* frame, struct icmp6_hdr* hdr) {
	memcpy(hdr, frame->field.data + IPV6_HDR_LEN, ICMP_HDR_LEN + ICMP_NDP_LEN);
	ntoh_structure(hdr, ICMP_HDR_LEN);
}

