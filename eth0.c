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

int getInterfaceIndex() {
	
	return 0;
}

int main(int argc, char* argv[])
{
	char *iface = "eth0";
	int proto = 0x1234;
	unsigned char src[ETH_ADDR_LEN] = { 0x08, 0x00, 0x27, 0x5c, 0x2c, 0x16 };	
	unsigned char dest[ETH_ADDR_LEN] = { 0x89, 0x9F, 0xFA, 0x5B, 0x28, 0xE1 };
	unsigned char* data = "hello world";
	unsigned short data_len = strlen(data);

	int i;
	for ( i = 0; i < ETH_ADDR_LEN; i++ ) {
		printf("%x ", src[i]);
	}

	for ( i = 0; i < ETH_ADDR_LEN; i++ ) {
		printf("%x ", dest[i]);
	}
	printf("\n%s\n", data);


	int sockfd;
	// htons(proto) jako 3 argument ??
	//int proto = IPPROTO_RAW;
	if ( (sockfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) < 0 ) {
		printf("Error: could not open a socket.\n");
		return -1;
	}

	// filling packet fields
	union ethframe frame;
	memcpy(frame.field.header.dest, dest, ETH_ADDR_LEN);
	//memcpy(frame.field.header.dest, src, ETH_ADDR_LEN);
	memcpy(frame.field.header.src, src, ETH_ADDR_LEN);
	frame.field.header.proto = htons(proto);
	memcpy(frame.field.data, data, data_len);

	int frame_len = data_len + ETH_HDR_LEN;


	// get hardware MAC
	int ifindex = 0;
	struct ifreq buffer;
	memset(&buffer, 0x00, sizeof(buffer));
	strncpy(buffer.ifr_name, iface, IFNAMSIZ);
	if ( ioctl(sockfd, SIOCGIFINDEX, &buffer) < 0 ) {
		printf("Error: could not get interface index.\n");
		return -1;
	}
	ifindex = buffer.ifr_ifindex;



	struct sockaddr_ll sockaddr;
	memset((void*)&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sll_family = PF_PACKET;
	sockaddr.sll_ifindex = ifindex;
	sockaddr.sll_halen = ETH_ADDR_LEN;
	memcpy((void*)sockaddr.sll_addr, (void*) dest, ETH_ADDR_LEN);
	
	if ( sendto(sockfd, frame.buffer, frame_len, 0, (struct sockaddr*) &sockaddr, sizeof(sockaddr)) <= 0 ) {
		printf("Error sending a frame");
		return -1;
	} 

	close(sockfd);


	printf("Everything worked!\n");
	return 0;
}
