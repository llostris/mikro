#ifndef TCP_H
#define TCP_H

#define FLAGS_SIZE	9
#define OPTIONS_LENGTH 10

struct tcp_header {
	uint16_t source_port;
	uint16_t destination_port;
	uint32_t sequence_number;
	uint32_t acknowledgement_number;
	uint32_
        	data_offset: 4,	
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
