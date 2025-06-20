#include "rtmp_session.h"
#include "type.h"
#include "handshake.h"
#include "chunk_header.h"
#include "rtmp_event.h"
#include "send_chunk.h"
#include "rtmp_server.h"
#include <schedule/amf0.h>

static int _parseFirstChunkPacket(RtmpPacket *packet, Buffer *buffer)
{
    assert(packet || buffer);

    if (0 > readHeaderChunk(buffer, &packet->header))
        return NET_FAIL;

    if (packet->header.length <= 0)
        return NET_FAIL;

    packet->buffer = createBuffer(packet->header.length);

    int leave_over = buffer->length - packet->header.header_len - buffer->index;

    if (packet->buffer && leave_over > 0)
    {
        int residue = packet->header.length > leave_over ? leave_over : packet->header.length;
        writeBuffer(packet->buffer, 0, buffer->data + buffer->index + packet->header.header_len, residue);
        packet->index += residue;
        buffer->index += residue + packet->header.header_len;
    }

    if (packet->index != packet->header.length)
        return NET_FAIL;

    return NET_SUCCESS;
}

static int _parseLastChunkPacket(RtmpPacket *packet, Buffer *buffer)
{
    assert(packet || buffer);

    int residue = packet->header.length - packet->index;

    if (residue > 0)
    {

        int need_len = residue > (buffer->length - buffer->index) ? buffer->length - buffer->index : residue;

        writeBuffer(packet->buffer, packet->index, buffer->data + buffer->index, need_len);

        packet->index += need_len;

        buffer->index += need_len;
    }

    if (packet->index != packet->header.length)
        return NET_FAIL;

    return NET_SUCCESS;
}

static int _incompleteRtmpChunk(RtmpSession *session, Buffer *buffer)
{
    assert(session || buffer);

    if (_parseLastChunkPacket(session->packet, buffer))
        return NET_FAIL;

    handleRtmpEvent(session, session->packet);

    session->packet = NULL;

    return NET_SUCCESS;
}

static int _completeRtmpChunk(RtmpSession *session, Buffer *buffer)
{
    assert(session || buffer);

    RtmpPacket *packet = CALLOC(1, RtmpPacket);
    if (!packet)
        return NET_FAIL;

    if (_parseFirstChunkPacket(packet, buffer))
    {
        if (packet->header.length <= 0)
        {
            FREE(packet->buffer);
            FREE(packet);
            return NET_FAIL;
        }
        session->packet = packet;
        return NET_FAIL;
    }

    handleRtmpEvent(session, packet);

    return NET_SUCCESS;
}

static void _parseRtmpChunk(RtmpSession *session, Buffer *buffer)
{
    assert(buffer);

    while (1)
    {
        if (session->packet)
        {
            if (_incompleteRtmpChunk(session, buffer))
                break;
        }
        else
        {
            if (_completeRtmpChunk(session, buffer))
                break;
        }

        if (buffer->index >= buffer->length)
            break;
    }
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
    assert(buffer);

    int count = 0;

    for (int i = 0; i < buffer->length; i++)
    {
        if (buffer->data[i] != 0xC3)
            buffer->data[count++] = buffer->data[i];
    }

    buffer->length = count;
}

RtmpSession *createRtmpSession(Seesion *conn)
{
    RtmpSession *session = NULL;

    do {
        session = CALLOC(1, RtmpSession);
        if (!session)  
            break;

        session->buffer = createBuffer(20 + RTMP_OUTPUT_CHUNK_SIZE);
        if (!session->buffer)
            break;


        session->b = bs_new(session->buffer->data, session->buffer->length);
        if (!session->b) 
            break;

        MUTEX_INIT(&session->myMutex);

        session->media          = NULL;
        session->conn           = conn;
        session->state          = RTMP_HANDSHAKE_UNINIT;
        session->packet         = NULL;

        session->channle[VIDEO_CHANNL].index = 0;
        session->channle[AUDIO_CHANNL].index = 0;
        session->channle[VIDEO_CHANNL].time_base = 1000;
        session->channle[AUDIO_CHANNL].time_base = 1000;

        LOG("create rtmp session success %p", session);

        return session;
    } while(0);

    destroyRtmpSession(session);

    return NULL;
}

void destroyRtmpSession(RtmpSession *session)
{
    if (!session)
        return;

    LOG("destroy Rtmp Session %p", session);

    deleteRtmpSessionToMedia(session->media, session);
    MUTEX_DESTROY(&session->myMutex);
    session->media = NULL;
    session->conn = NULL;
    FREE(session->buffer);
    FREE(session->b);
    FREE(session->packet);
    FREE(session);
}

void recvRtmpSession(RtmpSession *session, Buffer *buffer)
{
    if (!session || !buffer)
        return;

    _checkChunkComplete(buffer);

    return _parseRtmpPacket(session, buffer);
}


