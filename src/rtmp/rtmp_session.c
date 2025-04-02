#include "rtmp_session.h"
#include "type.h"
#include "handshake.h"
#include "chunk_header.h"

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

static RtmpPacket *_parseRtmpPacket(Buffer *buffer)
{
    RtmpPacket *packet = CALLOC(1, RtmpPacket);
    if (!packet) 
        return NULL;

    int len = readHeaderChunk(buffer, &packet->header);
    if (len <= 0)
        return NULL;

    if (packet->header.length > 0) {
        int left_buffer = buffer->length - len;
    }

    int left_buffer = buffer->length - len;

    left_buffer

}


static void _parseRtmpChunk(RtmpSession *session, Buffer *buffer)
{
    RtmpPacket *packet_queue = dequeue(session->packets);
    if (!packet_queue) {
        RtmpPacket *packet = _parseRtmpPacket(buffer);
    } else {

    }

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
    LOG("recv size:%d, %p, %s", buffer->length, session, buffer->data);

    //char *message = "Hello, server!";
    //send(session->conn->fd, message, strlen(message), 0);
    return _parseRtmpPacket(session, buffer);
}


