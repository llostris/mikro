#ifndef IPV6_H
#define IPV6_H

//#include <types.h>
#include "types.h"
#include "eth0.h"

#define IPV6_ADDR_LEN	8
#define IPV6_HDR_LEN	40
#define ICMP_HDR_LEN	4
#define ICMP_RESERVED_LEN	4
#define ICMP_NDP_LEN	ICMP_RESERVED_LEN + 16	/* Octets: 4 reserved + 16 for ipv6 address */

/* ICMP Types */
#define ICMP_NDP_SOLICIT	135
#define ICMP_NDP_ADVERT		136

#define ICMP_FLAG_SOLICITED	64	/* 0x01000000 */
#define ICMP_FLAG_OVERRIDE	32	/* 0x00100000 */
#define ICMP_ADVERT_SOLICITED	1

/* IPv6 Field Values */
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
	//uint32_t reserved;	// zmienic na reserved + payload?
	uint8_t data[ICMP_NDP_LEN];	// to be changed eventually
};

// big endian version
struct ip6_hdr_be {
	uint32_t
		version: 4,
		traffic_class: 8,	
		flow_label: 20;
	uint16_t payload_len;	/* Length of all data following IPv6 header */
	uint8_t next_hdr;	
	uint8_t hop_limit;	/* TTL: default 255 */
	uint16_t source_address[IPV6_ADDR_LEN];
	uint16_t destination_address[IPV6_ADDR_LEN];
};


struct ip6_hdr {
	uint32_t
		traffic_class1: 4,	// little endian
		version: 4,
		flow_label1: 4,
		traffic_class2: 4,
		flow_label2: 16;
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

void hton_ip_address(uint16_t* ip_address);

uint16_t checksum(void* data, uint16_t length);
uint16_t checksum_pseudo(void* data, uint16_t* src_addr, uint16_t* dest_addr, uint8_t next_hdr, uint8_t data_len);

int send_ndp_solicitation(uint16_t* ip_addr);
int send_ndp_advertisement(int solicited, uint16_t* dest_ip_addr, uint8_t* dest_hw_addr);

void parse_ipv6(union ethframe* frame, struct ip6_hdr* iphdr);
void parse_icmp(union ethframe* frame, struct icmp6_hdr* hdr);

// gdzie to wrzucic?
struct mac_table_record {
	uint16_t ip_addr[IPV6_ADDR_LEN];
	uint8_t mac_addr[ETH_ADDR_LEN];
	uint16_t ref_time;
};

/*
struct mac_table_record mac_table[MAC_TABLE_SIZE];
int mac_index = 0;
*/


#endif
