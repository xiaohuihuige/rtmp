#ifndef __TYPE_H__
#define __TYPE_H__

#define RTMP_VERSION 3
#define RTMP_HANDSHAKE_SIZE	 1536

typedef enum
{
	RTMP_HANDSHAKE_UNINIT, // Uninitialized 
	RTMP_HANDSHAKE_0, // received C0/S0, wait C1/S1
	RTMP_HANDSHAKE_1, // received C1/S1, wait C2/S2
	RTMP_HANDSHAKE_2, // received C2/S2, handshake done
} Handshake;


#endif