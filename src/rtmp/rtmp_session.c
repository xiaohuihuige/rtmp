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

static int _parseChunkPacket(RtmpPacket *packet, Buffer *buffer)
{
    if (packet->state == INIT_PACKET) {
        int header_len = readHeaderChunk(buffer, &packet->header);
        if (header_len <= 0)
            return NULL;

        if (packet->header.length > 0) {
            packet->buffer = createBuffer(packet->header.length);

            packet->index = packet->header.length > buffer->length - header_len ? buffer->length - header_len : packet->header.length;

            writeBuffer(packet->buffer, 0, buffer->data + header_len, packet->index);

        }

        return header_len
    }
}


static void _parseRtmpChunk(RtmpSession *session, Buffer *buffer)
{
    static RtmpPacket *temp_packet = NULL;

    while ()


    if (!temp_packet) {
        RtmpPacket *packet = _parseChunkPacket(session, buffer);



        return;
    } 

    temp_packet->state = 

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


