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

	/* odd byte handling */
	if (length == 1) {
		sum += *(unsigned char *)dataptr;
	}

	sum = (sum >> 16) + (sum & 0xffff);	/* add high 16 to low 16 */
	sum += (sum >> 16);			/* add carry */
	sum = ~sum;				/* truncate to 16 bits */

	return sum;
}

/* Calculates checksum for IPv6 pseudoheader */
// might not work for not-ndp packets
/* ipv6 pseudo-header used to calculate the checksum contains source address, destination address, icmpv6 length, 3 octets of 0 and next header id */
uint16_t checksum_pseudo(void* data, uint16_t* src_addr, uint16_t* dest_addr, int next_hdr, uint8_t data_len) {
	uint16_t pseudo_hdr_len = 4 * IPV6_ADDR_LEN + 8 + data_len;
	uint8_t buf[pseudo_hdr_len];
	memset(buf, 0x0, pseudo_hdr_len);
	memcpy(buf, src_addr, IPV6_ADDR_LEN * 2);
	memcpy(buf + 2 * IPV6_ADDR_LEN, dest_addr, IPV6_ADDR_LEN * 2);
	memset(buf + 4 * IPV6_ADDR_LEN + 3, data_len, 1);	// uwaga! problem dla duzych pakietow - wykorzystac htonl? // TODO
	memset(buf + 4 * IPV6_ADDR_LEN + 7, next_hdr, 1);	/* 3 octets of 0's and next header's id */
	memcpy(buf + 4 * IPV6_ADDR_LEN + 8, data, data_len);

	printf("%d\n", ICMP_HDR_LEN + 4 * IPV6_ADDR_LEN + 8);

	// for testing
	int i;
	for ( i = 0; i < pseudo_hdr_len; i++ )
	{
		printf("%x ", buf[i]);
		if ( (i + 1) % 16 == 0 )
			printf("\n");
	}	

	return checksum(&buf, pseudo_hdr_len);
}


