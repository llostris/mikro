#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>	
#include <net/if.h>	
#include <sys/ioctl.h>
#include <netpacket/packet.h> // struct sockaddr_ll

#include <eth0.h>
#include <types.h>
#include <ipv6.h>
#include <tcp.h>

int get_hardware_info(int* ifindex, unsigned char* hw_addr, int sockfd) {
	char *iface = "eth0";

	*ifindex = 0;
	struct ifreq buffer;
	memset(&buffer, 0x00, sizeof(buffer));
	strncpy(buffer.ifr_name, iface, IFNAMSIZ);
	if ( ioctl(sockfd, SIOCGIFINDEX, &buffer) < 0 ) {
		printf("Error: could not get interface index.\n");
		return -1;
	}
	*ifindex = buffer.ifr_ifindex;

	if (ioctl(sockfd, SIOCGIFHWADDR, &buffer) < 0) {
		printf("Error: could not get interface address\n");
 		close(sockfd);
		return -1;
  	}

	memcpy((void*)hw_addr, (void*)(buffer.ifr_hwaddr.sa_data), ETH_ADDR_LEN);

	return 0;
}


// eksperymentalne - dziala?

void hton_structure(void* buffer, unsigned int size) {
	uint16_t* ptr = buffer;
	int index = 0;
	for (index = 0; index < size/2; index++) {
		*ptr = htons(*ptr);
		ptr++;
	}
}

void ntoh_structure(void* buffer, unsigned int size) {
	uint16_t* ptr = buffer;
	int index = 0;
	for (index = 0; index < size/2; index++) {
		*ptr = ntohs(*ptr);
		ptr++;
	}
}

void htonl_structure(void* buffer, unsigned int size) {
	uint32_t* ptr = buffer;
	int index = 0;
	for (index = 0; index < size/4; index++) {
		*ptr = htonl(*ptr);
		ptr++;
	}
}




int send_frame(union ethframe* frame, int frame_len) {
	int proto = ETH_TYPE_IP6;

	unsigned char src_hw[ETH_ADDR_LEN];	
	unsigned char dest_hw[ETH_ADDR_LEN] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };	// broadcast

	int sockfd;
	if ( (sockfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) < 0 ) {
		printf("Error: could not open a socket.\n");
		return -1;
	}

	/* Get interface index and hardware address of host */
	int ifindex = 0;
	if ( get_hardware_info(&ifindex, src_hw/*frame->field.header.src*/, sockfd) < 0 ) {
		printf("Error: could not get hardware's information\n");
		return -1;
	}

	memcpy(frame->field.header.src, src_hw, ETH_ADDR_LEN);

	struct sockaddr_ll sockaddr;
	memset((void*)&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sll_family = PF_PACKET;
	sockaddr.sll_ifindex = ifindex;
	sockaddr.sll_halen = ETH_ADDR_LEN;
	memcpy((void*)sockaddr.sll_addr, (void*) dest_hw /*frame->field.header.dest*/, ETH_ADDR_LEN);

	if ( sendto(sockfd, frame->buffer, frame_len, 0, (struct sockaddr*) &sockaddr, sizeof(sockaddr)) <= 0 ) {
		printf("Error sending a frame");
		return -1;
	} 

	printf("\n *** FRAME SENT.");	// debugging

	close(sockfd);

}
