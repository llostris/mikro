#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <linux/in.h>	// ??
#include <linux/if.h>	// interface
#include <sys/ioctl.h>	//ioctl flags
#include <netpacket/packet.h> // struct sockaddr_ll

#include "eth0.h"
//#include <eth0.h>

int main(int argc, char* argv[])
{
	int sockfd;
	unsigned char src[ETH_ADDR_LEN];
	// htons(proto) jako 3 argument ??
	//int proto = IPPROTO_RAW;
	if ( (sockfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) < 0 ) {
		printf("Error: could not open a socket.\n");
		return -1;
	}

	int ifindex = 0;
	if ( get_hardware_info(&ifindex, src, sockfd) < 0 ) {
		printf("Error getting host's data\n");
		return -1;
	}


	void* buffer = (void*)malloc(ETH_FRAME_LEN);
	int frame_len = 0;

	printf("waiting for frames...\n");

	while ( 1 ) {
	
		frame_len = recvfrom(sockfd, buffer, ETH_FRAME_LEN, 0, NULL, NULL);
		if (frame_len == -1) { 
			printf("no frame... ");
			//errorhandling .... 
		}
	}

	/* get frame's data */
	

	close(sockfd);

	printf("Everything worked!\n");
	return 0;
}
