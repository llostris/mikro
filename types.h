#ifndef TYPES_H
#define TYPES_H

#define uint8_t unsigned char
#define uint16_t unsigned short
#define uint32_t unsigned int


#if BYTE_ORDER == BIG_ENDIAN

#define HTONS(n) (n)
#define NTOHS(n) (n)
#define HTONL(n) (n)
#define NTOHL(n) (n)

#else

#define HTONS(n) ( ((((uint16_t)(n) & 0xFF)) << 8) | (((unsigned short)(n) & 0xFF00) >> 8))
#define NTOHS(n) ( ((((uint16_t)(n) & 0xFF)) << 8) | (((unsigned short)(n) & 0xFF00) >> 8))
/*
#define HTONL(n) (n)	// TODO
#define NTOHL(n) (n)	// TODO
*/
#endif

#define htons(n) HTONS(n)
#define ntohs(n) NTOHS(n)

/*
#define htonl(n) HTONL(n)
#define ntohl(n) NTOHL(n)
*/

/*
unsigned short htons(unsigned short n);
unsigned short ntohs(unsigned short n);
unsigned long htonl(unsigned long n);
unsigned long ntohl(unsigned long n);

*/

#endif

