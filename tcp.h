#ifndef TCP_H
#define TCP_H

#define FLAGS_SIZE	9
#define OPTIONS_LENGTH	10
#define TCP_HDR_LEN	30	/* Octets in TCP Header without options */

#define PORT_HTTP	80

/* 9 flags for TCP */

#define TCP_FLAG_NS	256	/* ECN-nonce concealment protection */
#define TCP_FLAG_CWR	128	/* Congestion Window Reduced (CWR) flag is set by the sending host to indicate that it received a TCP segment with the ECE flag set and had responded in congestion control mechanism */
#define TCP_FLAG ECE	64	/* ECN-Echo: 1 - ECN capable */
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
	uint32_t
		flags: FLAGS_SIZE,
		reserved: 3,
		data_offset: 4,	 /* Header size in 32-bit words. Min. 5, max. 15. */
		window_size: 16;	/* Always 000 */
	uint16_t checksum;
	uint16_t urgent_pointer;
	uint32_t options[OPTIONS_LENGTH];
};


/* big endian version */
struct tcp_header_be {
	uint16_t source_port;
	uint16_t destination_port;
	uint32_t sequence_number;
	uint32_t acknowledgement_number;
	uint32_t
        	data_offset: 4,	/* Header size in 32bit words. Min. 5, max 15 */
        	reserved: 3, //Always 000
        	flags: FLAGS_SIZE,
        	window_size: 16;
	/*
	uint32_t 
		reserved: 3,
		flag_ns: 1,
		data_offset: 4,
		flags: FLAGS_SIZE - 1,	// htons ?
		window_size: 16;	// htons
	*/	
	uint16_t checksum;
	uint16_t urgent_pointer;
	uint32_t options[OPTIONS_LENGTH];
};

struct tcp_frame {
	struct tcp_header header;
	void* data;
};

#endif
