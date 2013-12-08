#include <stdio.h>

//#include "ip.h"
#include "ipv6.h"
#include "types.h"
#include "eth0.h"

/*
#include <types.h>
#include <ipv6.h>
#include <eth0.h>
*/

int main(int argc, char** argv) {
//	uint16_t dest_ipaddr[IPV6_ADDR_LEN] = { 0xfe80, 0x0, 0xa00, 0x27ff, 0xfe5c, 0x2c16, 0x0, 0x0 };
//	uint16_t dest_ipaddr[IPV6_ADDR_LEN] = { 0xfe80, 0x0, 0xbd41, 0x2638, 0x69a0, 0x87d8, 0x0, 0x0 };
	uint16_t dest_ipaddr[IPV6_ADDR_LEN] =  {0xfe80, 0x0, 0x5882, 0xd64b, 0xb962, 0x6ada, 0x0, 0x0 };
	send_ndp_solicitation(dest_ipaddr);
	send_ndp_advertisement(0, 0, 0);

	return 0;
}
