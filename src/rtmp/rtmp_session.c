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

    return session;
}

void destroyRtmpSession(RtmpSession *session)
{
    LOG("close");
    FREE(session);
}

static RtmpPacket *_parseFirstChunkPacket(Buffer *buffer)
{
    assert(buffer);

    RtmpPacket *packet = CALLOC(1, RtmpPacket);
    if (!packet) 
        return NULL;

    if (0 > readHeaderChunk(buffer, &packet->header))
        return NULL;

    if (packet->header.length > 0 && buffer->length > packet->header.header_len) {
        packet->buffer = createBuffer(packet->header.length);
        packet->index = packet->header.length > buffer->length - packet->header.header_len
                            ? buffer->length - packet->header.header_len
                            : packet->header.length;

        writeBuffer(packet->buffer, 0, buffer->data + buffer->index + packet->header.header_len, packet->index);
        buffer->index = packet->index + packet->header.header_len;
        LOG("packet index %d", packet->index);
    }
    return packet;
}

static int _checkChunkPacket(RtmpPacket *packet)
{
    assert(packet);

    if (packet->index == packet->header.length) 
        return NET_SUCCESS;

    LOG("MUTILAtion_PACKET %d, %d", packet->index, packet->header.length);
    return NET_FAIL;
}

static void _parseLastChunkPacket(RtmpPacket *packet, Buffer *buffer)
{
    assert(packet || buffer);

    int residue = packet->header.length - packet->index;
    LOG("%d, %d, %d", residue, packet->header.length, packet->index);
    if (residue > 0 && residue <= buffer->length) {
        writeBuffer(packet->buffer, packet->index, buffer->data + buffer->index, residue);
        packet->index += residue;
        buffer->index += residue;
    }
}

static int _nextChunkPacket(Buffer **tbuffer)
{
    assert(tbuffer || *tbuffer);

    Buffer *buffer = *tbuffer;
    if (buffer->index + 1 < buffer->length) {
        LOG("%d, %d", buffer->index, buffer->length);
        Buffer *next_buffer = createBuffer(buffer->length - buffer->index);
        if (!next_buffer) 
            return NET_FAIL;
        
        writeBuffer(next_buffer, 0, buffer->data + buffer->index, buffer->length - buffer->index);

        FREE(buffer);

        buffer = next_buffer;
        return NET_SUCCESS;
    }
    return NET_FAIL;
}

static void _parseRtmpChunk(RtmpSession *session, Buffer *buffer)
{
    assert(buffer);

    printfChar(buffer->data, buffer->length);

    RtmpPacket *packet = NULL;

    while (1) {
        if (session->packet) {
            _parseLastChunkPacket(session->packet, buffer);
            if (_checkChunkPacket(session->packet))
                break;

            packet = session->packet;
            session->packet = NULL;
            LOG("get last packet");
        } else {
            packet = _parseFirstChunkPacket(buffer);
            if (!packet)
                break;

            if (_checkChunkPacket(packet)) {
                session->packet = packet;
                packet = NULL;
                break;
            }
        } 

        handleRtmpEvent(session, packet);

        if (!_nextChunkPacket(&buffer))
            continue;

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

static void _checkChunkComplete(Buffer *buffer)
{
    int count = 0;

    for (int i = 0; i < buffer->length; i++)
    {
        if (buffer->data[i] != 0xC3)
            buffer->data[count++] = buffer->data[i];
    }

    buffer->length = count;
}

void recvRtmpSession(RtmpSession *session, Buffer *buffer)
{
    LOG("recv size:%d, %p", buffer->length, session);

    _checkChunkComplete(buffer);

    return _parseRtmpPacket(session, buffer);
}


