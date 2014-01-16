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
#include <tcp.h>

void parse_tcp(union ethframe* frame, struct tcp_header* hdr) {
	memcpy(hdr, frame->field.data + IPV6_HDR_LEN, TCP_HDR_LEN);
	ntoh_structure(hdr, TCP_HDR_LEN);
}


void tcp_actions();
