#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <linux/in.h>	// ??
#include <linux/if.h>	// interface
#include <sys/ioctl.h>	//ioctl flags
#include <netpacket/packet.h> // struct sockaddr_ll

#include <eth0.h>

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
