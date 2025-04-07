#include "rtmp_session.h"
#include "type.h"
#include "handshake.h"
#include "chunk_header.h"
#include "rtmp_event.h"

RtmpSession *createRtmpSession(Seesion *conn)
{
    RtmpSession *session = CALLOC(1, RtmpSession);
    if (!session)   
        return NULL;

    session->conn  = conn;
    session->state = RTMP_HANDSHAKE_UNINIT;

    session->packets = createFifiQueue();

    return session;
}

void destroyRtmpSession(RtmpSession *session)
{
    LOG("close");
    FREE(session);
}

static RtmpPacket *_parseFirstChunkPacket(Buffer *buffer)
{
    RtmpPacket *packet = CALLOC(1, RtmpPacket);
    if (!packet) 
        return NULL;

    if (0 > readHeaderChunk(buffer, &packet->header))
        return NULL;

    if (packet->header.length > 0) {
        packet->buffer = createBuffer(packet->header.length);
        packet->index = packet->header.length > buffer->length - packet->header.header_len
                            ? buffer->length - packet->header.header_len
                            : packet->header.length;
        writeBuffer(packet->buffer, 0, buffer->data + packet->header.header_len, packet->index);
    }
    return packet;
}

// static int _parseLastChunkPacket(RtmpPacket *packet, Buffer *buffer)
// {
//     int residue = packet->header.length - packet->index;

//     if (residue == 0)
//         return NET_SUCCESS; 
//     return NET_SUCCESS; 
// }

static void _parseRtmpChunk(RtmpSession *session, Buffer *buffer)
{


    RtmpPacket *packet = _parseFirstChunkPacket(buffer);

    handleRtmpEvent(session, packet);

    //FifoQueue *node = dequeue(session->packets);
        
    return;
}

static void _parseRtmpPacket(RtmpSession *session, Buffer *buffer)
{
    assert(session || buffer);

    if (session->state == RTMP_HANDSHAKE_UNINIT || session->state == RTMP_HANDSHAKE_0)
        return createRtmpHandShake(session, buffer);

    return _parseRtmpChunk(session, buffer);
}

void recvRtmpSession(RtmpSession *session, Buffer *buffer)
{
    LOG("recv size:%d, %p", buffer->length, session);
    return _parseRtmpPacket(session, buffer);
}


