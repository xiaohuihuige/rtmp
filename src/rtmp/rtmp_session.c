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
        writeBuffer(packet->buffer, 0, buffer->data + buffer->index + packet->header.header_len, packet->index);
        buffer->index = packet->index + packet->header.header_len;
    }
    return packet;
}

static int _checkChunkPacket(RtmpPacket *packet)
{
    if (packet->index == packet->header.length) {
        packet->state = WHOLE_PACKET;
        LOG("WHOLE_PACKET");
        return WHOLE_PACKET;
    } else {
        LOG("MUTILAtion_PACKET %d, %d", packet->index, packet->header.length);
        packet->state = MUTILAtion_PACKET;
        return MUTILAtion_PACKET;
    }
}

static int _parseLastChunkPacket(RtmpPacket *packet, Buffer *buffer)
{
    int residue = packet->header.length - packet->index;
    LOG("%d, %d, %d", residue, packet->header.length, packet->index);
    if (residue > 0 && residue < buffer->length) {
        writeBuffer(packet->buffer, packet->index, buffer->data + buffer->index, residue);
        packet->index += residue;
        buffer->index += residue;
    }
   
    return NET_SUCCESS; 
}

static void _parseRtmpChunk(RtmpSession *session, Buffer *buffer)
{
    RtmpPacket *packet = NULL;
    Buffer *next_buffer= NULL;
    
    while (1) {
        if (session->packet) {
            _parseLastChunkPacket(session->packet, buffer);
            if (WHOLE_PACKET != _checkChunkPacket(session->packet))
                break;

            packet = session->packet;
            session->packet = NULL;
            LOG("get last packet");
        } else {
            packet = _parseFirstChunkPacket(buffer);
            if (!packet)
                break;

            if (WHOLE_PACKET != _checkChunkPacket(packet)) {
                session->packet = packet;
                packet = NULL;
                break;
            }
        } 

        handleRtmpEvent(session, packet);

        if (buffer->index + 1 < buffer->length) {
            LOG("%d, %d",buffer->index, buffer->length);
            next_buffer = createBuffer(buffer->length - buffer->index);
            writeBuffer(next_buffer, 0, buffer->data + buffer->index, buffer->length - buffer->index);
            FREE(buffer);
            buffer = next_buffer;
            continue;
        } 
        break;
    }     
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


