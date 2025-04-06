#ifndef __TYPE_H__
#define __TYPE_H__

#include "chunk_header.h"

#define RTMP_VERSION 3
#define RTMP_HANDSHAKE_SIZE	 1536

#define INIT_PACKET       0
#define WHOLE_PACKET      1
#define MUTILAtion_PACKET 2

enum rtmp_chunk_type_t
{
	RTMP_CHUNK_TYPE_0 = 0, // 11-bytes: timestamp(3) + length(3) + stream type(1) + stream id(4)
	RTMP_CHUNK_TYPE_1 = 1, // 7-bytes: delta(3) + length(3) + stream type(1)
	RTMP_CHUNK_TYPE_2 = 2, // 3-bytes: delta(3)
	RTMP_CHUNK_TYPE_3 = 3, // 0-byte
};

typedef enum
{
	RTMP_HANDSHAKE_UNINIT, // Uninitialized 
	RTMP_HANDSHAKE_0, // received C0/S0, wait C1/S1
	RTMP_HANDSHAKE_1, // received C1/S1, wait C2/S2
	RTMP_HANDSHAKE_2, // received C2/S2, handshake done
} Handshake;

enum RTMPMessageTypeId
{
	/* Protocol Control Messages */
	RTMP_TYPE_SET_CHUNK_SIZE = 1,
	RTMP_TYPE_ABORT = 2,
	RTMP_TYPE_ACKNOWLEDGEMENT = 3,			   // bytes read report
	RTMP_TYPE_WINDOW_ACKNOWLEDGEMENT_SIZE = 5, // server bandwidth
	RTMP_TYPE_SET_PEER_BANDWIDTH = 6,		   // client bandwidth

	/* User Control Messages Event (4) */
	RTMP_TYPE_EVENT = 4,

	RTMP_TYPE_AUDIO = 8,
	RTMP_TYPE_VIDEO = 9,

	/* Data Message */
	RTMP_TYPE_FLEX_STREAM = 15, // AMF3
	RTMP_TYPE_DATA = 18,		// AMF0

	/* Shared Object Message */
	RTMP_TYPE_FLEX_OBJECT = 16,	  // AMF3
	RTMP_TYPE_SHARED_OBJECT = 19, // AMF0

	/* Command Message */
	RTMP_TYPE_FLEX_MESSAGE = 17, // AMF3
	RTMP_TYPE_INVOKE = 20,		 // AMF0

	/* Aggregate Message */
	RTMP_TYPE_METADATA = 22,
};

typedef struct 
{
	int state;
	int index;
	HeaderChunk header;
	Buffer *buffer;
} RtmpPacket;

#endif