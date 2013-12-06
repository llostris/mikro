#ifndef IPV6_H
#define IPV6_H

//#include <types.h>
#include "types.h"

#define IPV6_HDR_LEN	40
#define ICMP_HDR_LEN	4
#define ICMP_NDP_LEN	20	/* Octets: 4 reserved + 16 for ipv6 address */
#define ICMP_RESERVED_LEN	4

#define ICMP_NDP_SOLICIT	135
#define ICMP_NDP_ADVERT		136
#define IPV6_ADDR_LEN	8

#define DEFAULT_TTL	255
#define NEXT_HDR_ICMP	58
#define NEXT_HDR_TCP	6
#define NEXT_HDR_NONE	59

#define MAC_TABLE_SIZE 	30
#define MAC_TABLE_TIME	1000


struct icmp6_hdr {
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	uint8_t data[ICMP_NDP_LEN];	// to be changed eventually
};


struct ip6_hdr {
	uint32_t
		traffic_class1: 4,	// little endian
		version: 4,
		flow_label1: 4,
		traffic_class2: 4,
		flow_label2: 16;
/*
		version: 4,
		traffic_class: 8,	
		flow_label: 20;
*/
	uint16_t payload_len;	/* Length of all data following IPv6 header */
	uint8_t next_hdr;
	uint8_t hop_limit;	/* TTL: default 255 */
	uint16_t source_address[IPV6_ADDR_LEN];
	uint16_t destination_address[IPV6_ADDR_LEN];
};

struct ip6_frame {
	struct ip6_hdr header;
	void* data;
};

int send_ndp_solicitation(uint16_t* ip_addr);
int send_ndp_advertisement(int solicited, uint16_t* dest_ip_addr, uint8_t* dest_hw_addr);

void hton_ip_address(uint16_t* ip_address);

uint16_t checksum(void* data, uint16_t length);

#endif
