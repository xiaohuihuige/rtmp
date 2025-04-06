#ifndef __CHUNK_HEADER_H__
#define __CHUNK_HEADER_H__

#include <schedule/net-common.h>
#include <schedule/tcp_server.h>
#include <schedule/timestamp.h>

typedef struct
{
    uint8_t  fmt;       // RTMP_CHUNK_TYPE_XXX
    uint8_t  csid;      // chunk stream id(22-bits)
    uint32_t timestamp; // delta(24-bits) / extended timestamp(32-bits)
    uint32_t length;    // message length (24-bits)
    uint8_t  type_id;   // message type id
    uint32_t stream_id; // message stream id
    uint8_t  header_len;
} HeaderChunk;

int readHeaderChunk(Buffer *buffer, HeaderChunk *header);

#endif