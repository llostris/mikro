#ifndef ETH0_H
#define ETH0_H

#define ETH_FRAME_LEN	1514
#define ETH_DATA_LEN	1500
#define ETH_ADDR_LEN	6	/* Octets in Ethernet address */
#define ETH_HDR_LEN	14	/* Header length in octets */
#define ETH_TYPE_IP6	0x86DD	/* EtherType for IPv6 packets */

/* Socket flags/types */
#define ETH_P_ALL	0x0003	/* Receive all ethernet frames */
#define ETH_P_IPV6	0x86DD	/* Receive IPv6 packets only */


struct ethhdr {
        unsigned char  dest[ETH_ADDR_LEN];
        unsigned char  src[ETH_ADDR_LEN];
        unsigned short proto;
};

union ethframe {
        struct {
                struct ethhdr header;
                unsigned char data[ETH_DATA_LEN];
        } field;
        unsigned char buffer[ETH_FRAME_LEN];
};

int get_hardware_info(int* ifindex, unsigned char* hw_addr, int sockfd);

/* Performs big endian <-> little endian conversion of whole structures */
void hton_structure(void* buffer, unsigned int size);
void ntoh_structure(void* buffer, unsigned int size);

#endif

