#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>	// ??
#include <net/if.h>	// interface
#include <sys/ioctl.h>	//ioctl flags
#include <netpacket/packet.h> // struct sockaddr_ll

#include "eth0.h"
#include "ipv6.h"
#include "tcp.h"

/* Functions testing Ethernet/IPv6/ICMPv6/TCP parsing */

int parsed_ipv6(union ethframe* frame, struct ip6_hdr* iphdr); 
//int parse_eth_frame(union ethframe* frame, void* buffer);
int parsed_icmp(union ethframe* frame, struct icmp6_hdr* hdr);
int parsed_tcp(union ethframe* frame, struct tcp_header* hdr);

