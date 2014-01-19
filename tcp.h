#ifndef TCP_H
#define TCP_H

#include "types.h"
#include "eth0.h"

#define FLAGS_SIZE	9
#define OPTIONS_LENGTH	10
#define TCP_HDR_LEN	20	/* Octets in TCP Header without options */

#define PORT_HTTP	80

/* 9 flags for TCP */

#define TCP_FLAG_NS	256	/* ECN-nonce concealment protection */
#define TCP_FLAG_CWR	128	/* Congestion Window Reduced (CWR) flag is set by the sending host to indicate that it received a TCP segment with the ECE flag set and had responded in congestion control mechanism */
#define TCP_FLAG_ECN	64	/* ECN-Echo: 1 - ECN capable */
#define TCP_FLAG_URG	32	/* Urgent pointer field is significant */
#define TCP_FLAG_ACK	16	/* Acknowledgement field is significant. Set after the initial SYN packet sent by client */
#define TCP_FLAG_PSH	8	/*  Push function. Asks to push the buffered data to the receiving application. */
#define TCP_FLAG_RST	4	/* Reset the connection */
#define TCP_FLAG_SYN	2	/* Only first packet sent from each end */
#define TCP_FLAG_FIN	1	/* End the communication */

struct tcp_header {
	uint16_t source_port;
	uint16_t destination_port;
	uint32_t sequence_number;
	uint32_t acknowledgement_number;
	uint16_t
		reserved: 4,
		data_offset: 4,
		flags: 8;
	uint16_t window_size;

/*
	uint32_t
		flags: FLAGS_SIZE,
		data_offset: 4,	 /* Header size in 32-bit words. Min. 5, max. 15. */
//		reserved: 3,	/* Always 000 */

//		window_size: 16;
	uint16_t checksum;
	uint16_t urgent_pointer;
	uint32_t options[OPTIONS_LENGTH];
};

struct tcp_frame {
	struct tcp_header header;
	void* data;
};

void parse_tcp(union ethframe* frame, struct tcp_header* hdr);
void tcp_actions(union ethframe* frame, struct tcp_header* hdr);
void reply_tcp(union ethframe* frame, struct tcp_header* tcphdr);
void create_tcp_hdr(struct tcp_header* hdr, int dest_port, int flags, int ack_num, int seq_num, int window_size);

#endif
