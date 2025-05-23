#include "rtmp_session.h"
#include "type.h"
#include "handshake.h"
#include "chunk_header.h"
#include "rtmp_event.h"
#include "send_chunk.h"
#include "rtmp_server.h"
#include "amf0.h"

RtmpSession *createRtmpSession(Seesion *conn)
{
    RtmpSession *session = CALLOC(1, RtmpSession);
    if (!session)  {
        ERR("create session malloc error");
        return NULL;
    }

    session->buffer = createBuffer(20 + RTMP_OUTPUT_CHUNK_SIZE);
    if (!session->buffer) {
        ERR("create buffer fail");
        FREE(session);
        return NULL;
    }


    session->b = bs_new(session->buffer->data, session->buffer->length);
    if (!session->b) {
        ERR("create b bytes fail");
        FREE(session->buffer);
        FREE(session);
        return NULL;
    }

    session->media          = NULL;
    session->conn           = conn;
    session->state          = RTMP_HANDSHAKE_UNINIT;
    session->packet         = NULL;
    session->video_task     = NULL;
    session->audio_task     = NULL;
    
    session->channle[VIDEO_CHANNL].index = 0;
    session->channle[AUDIO_CHANNL].index = 0;
    session->channle[VIDEO_CHANNL].time_base = 1000;
    session->channle[AUDIO_CHANNL].time_base = 1000;

    LOG("create rtmp session success %p", session);

    return session;
}

void destroyRtmpSession(RtmpSession *session)
{
    LOG("destroy Rtmp Session %p", session);
    stopPushSessionstream(session);
    session->media = NULL;
    FREE(session->buffer);
    FREE(session->b);
    FREE(session->packet);
    FREE(session);
}

static int _parseFirstChunkPacket(RtmpPacket *packet, Buffer *buffer)
{
    assert(buffer);

    if (0 > readHeaderChunk(buffer, &packet->header))
        return NET_FAIL;

    if (packet->header.length <= 0)
        return NET_FAIL;

    packet->buffer = createBuffer(packet->header.length);

    int leave_over = buffer->length - packet->header.header_len - buffer->index;

    if (packet->buffer && leave_over > 0) {
        int residue = packet->header.length > leave_over ? leave_over : packet->header.length;
        writeBuffer(packet->buffer, 0, buffer->data + buffer->index + packet->header.header_len, residue);
        packet->index += residue;
        buffer->index += residue + packet->header.header_len;
    }

    //LOG("first packet, [%p, body length %d, index %d], [%p, revc buffer length %d, index %d], ", packet, packet->header.length, packet->index, buffer, buffer->length, buffer->index);
    
    if (packet->index != packet->header.length) 
        return NET_FAIL;

    return NET_SUCCESS;
}

static int _parseLastChunkPacket(RtmpPacket *packet, Buffer *buffer)
{
    assert(packet || buffer);

    int residue = packet->header.length - packet->index;

    // LOG("last packet, [%p, body length %d, index %d], [%p revc buffer length %d, index %d]", 
    //     packet, packet->header.length, packet->index, buffer, buffer->length, buffer->index);

    if (residue > 0) {

        int need_len = residue > (buffer->length - buffer->index) ? buffer->length - buffer->index : residue;

        writeBuffer(packet->buffer, packet->index, buffer->data + buffer->index, need_len);

        packet->index += need_len;

        buffer->index += need_len;

        // LOG("end packet, [%p, body length %d, index %d], [%p, revc buffer length %d, index %d]", 
        //     packet, packet->header.length, packet->index, buffer, buffer->length, buffer->index);
    }

    if (packet->index != packet->header.length) 
        return NET_FAIL;

    return NET_SUCCESS;
}

static int _incompleteRtmpChunk(RtmpSession *session, Buffer *buffer)
{
    if (_parseLastChunkPacket(session->packet, buffer)) 
        return NET_FAIL;

    handleRtmpEvent(session, session->packet);

    session->packet = NULL;

    return NET_SUCCESS;
}

static int _completeRtmpChunk(RtmpSession *session, Buffer *buffer)
{
    RtmpPacket *packet = CALLOC(1, RtmpPacket);
    if (!packet) 
        return NET_FAIL;

    if (_parseFirstChunkPacket(packet, buffer)) {
        if (packet->header.length <= 0) {
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

    while (1) {
        if (session->packet) {
            if (_incompleteRtmpChunk(session, buffer))
                break;
        }      
        else {
            if (_completeRtmpChunk(session, buffer))
                break;
        }
         
        if (buffer->index  >= buffer->length) 
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
    //LOG("recv size:%d, %p", buffer->length, session);

    _checkChunkComplete(buffer);

    return _parseRtmpPacket(session, buffer);
}


