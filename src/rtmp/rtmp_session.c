#include "rtmp_session.h"
#include "type.h"
#include "handshake.h"
#include "chunk_header.h"
#include "rtmp_event.h"
#include "send_chunk.h"
#include "rtmp_server.h"
#include <schedule/amf0.h>

static int _sendVideoFrameTimer(void *args)
{
    assert(args);

    RtmpSession *session = (RtmpSession *)args;

    Buffer *frame = fifoQueuePopUnblock(session->queue);
    if (!frame)
        return NET_FAIL;

    sendFrameStream(session, frame, session->channle[VIDEO_CHANNL].time_base);

    session->channle[VIDEO_CHANNL].time_base += frame->timestamp;

    bufferReleaseSpace(frame);

    return NET_SUCCESS;
}

int createSessionStreamTimer(RtmpSession *session)
{
    if (!session || !session->media->video)
        return NET_FAIL;

    session->gop_count = session->media->video->fps * 6;

    LOG("session gop count %d", session->gop_count);

    while (session->gop_count--) {
        if (NET_FAIL == _sendVideoFrameTimer(session))
            break;
    }

    session->pull_stream_timer = addTimerTask(session->conn->tcps->scher,  
                                                session->media->video->duration,
                                                session->media->video->duration - 10,
                                                 _sendVideoFrameTimer, 
                                                 (void *)session);
    if (!session->pull_stream_timer)
        return NET_FAIL;

    return NET_SUCCESS;

}

static void _parseRtmpHandshake(RtmpSession *session, Buffer *buffer)
{
    if (!session->temp_buffer) 
        session->temp_buffer = createBuffer(2 * RTMP_HANDSHAKE_SIZE + 1);
    
    int actual_len = writeBuffer(session->temp_buffer, session->temp_buffer->index, buffer->data, buffer->length);
    session->temp_buffer->index += actual_len;

    if (session->state == RTMP_HANDSHAKE_UNINIT)
    {
        if (session->temp_buffer->index < RTMP_HANDSHAKE_SIZE + 1)
            return;

        sendHandShakeS0S1S2(session, buffer);
        session->state = RTMP_HANDSHAKE_0;
    }

    if (session->state == RTMP_HANDSHAKE_0)
    {
        if (session->temp_buffer->index < 2 * RTMP_HANDSHAKE_SIZE + 1)
            return;

        buffer->index = actual_len;
        session->state = RTMP_HANDSHAKE_1;
        FREE(session->temp_buffer);
    }
}

static void _checkChunkComplete(Buffer *buffer)
{
    assert(buffer);

    int count = buffer->index;

    for (int i = buffer->index; i < buffer->length; i++)
    {
        if (buffer->data[i] != 0xC3) 
            buffer->data[count++] = buffer->data[i];
    }

    buffer->length = count;
}

static RtmpPacket *_completeRtmpChunk(RtmpSession *session, Buffer *buffer)
{
    assert(session || buffer);

    // RtmpPacket *packet = CALLOC(1, RtmpPacket);
    // if (!packet)
    //     return NULL;

    LOG("buffer length %d, ", buffer->length, buffer->index);

    RtmpPacket *packet = (RtmpPacket *)malloc(sizeof(RtmpPacket));
    if (!packet)
        return NULL;

    LOG("CALLOC %p", packet);

    int overturn = readHeaderChunk(buffer, &packet->header);
    if (overturn) {
        ERR("overturn error %p", packet);
        FREE(packet);
        return NULL;
    }

    if (packet->header.length <= 0 || packet->header.length > RTMP_OUTPUT_CHUNK_SIZE 
        ||(packet->header.length + packet->header.header_len) > (buffer->length - buffer->index)) {
        ERR("header length error %p", packet);
        FREE(packet);
        return NULL;
    }

    packet->buffer = createFrameBuffer(buffer->data + buffer->index + packet->header.header_len, packet->header.length, 0, 0);
    if (!packet->buffer) {
        ERR("createFrameBuffer error %p", packet);
        FREE(packet);
        return NULL;
    }

    buffer->index += packet->header.length + packet->header.header_len;

    return packet;
}

static void _parseRtmpChunk(RtmpSession *session, Buffer *buffer)
{
    assert(buffer);

    if (session->state != RTMP_HANDSHAKE_1 || buffer->index >= buffer->length)
        return;

    _checkChunkComplete(buffer);

    if (!session->temp_buffer) 
    {
        session->temp_buffer = createFrameBuffer(buffer->data + buffer->index, buffer->length - buffer->index, 0, 0);
    }
    else 
    {
        LOG("session->temp_buffer %d, %d", session->temp_buffer->length, buffer->length - buffer->index);
        session->temp_buffer = reinitializeBuffer(session->temp_buffer, buffer->data + buffer->index, buffer->length - buffer->index);
    }
    
    
    while (1)
    {
        RtmpPacket *packet = _completeRtmpChunk(session, session->temp_buffer);
        if (!packet)
            break;
        
        handleRtmpEvent(session, packet);

        FREE(packet->buffer);
        FREE(packet);

        if (session->temp_buffer->index >= session->temp_buffer->length) {
            LOG("free %p", session->temp_buffer);
            FREE(session->temp_buffer);
            break;
        }
    }
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

        session->queue = createFifoQueue();
        if (!session->queue)
            break;

        MUTEX_INIT(&session->myMutex);

        session->media          = NULL;
        session->conn           = conn;
        session->state          = RTMP_HANDSHAKE_UNINIT;
        session->temp_buffer    = NULL;
        session->gop_count      = -1;
        session->pull_stream_timer = NULL;

        session->channle[VIDEO_CHANNL].index = 0;
        session->channle[AUDIO_CHANNL].index = 0;
        session->channle[VIDEO_CHANNL].time_base = 10;
        session->channle[AUDIO_CHANNL].time_base = 10;

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

    removeRtmpSessionByMedia(session->media, session);

    if (session->pull_stream_timer) {
        deleteTimerTask(session->pull_stream_timer);
        session->pull_stream_timer = NULL;
    }

    if (session->queue) {
        releaseFifoQueue(session->queue);
        session->queue = NULL;
    }

    MUTEX_DESTROY(&session->myMutex);
    session->media = NULL;
    session->conn = NULL;

    FREE(session->temp_buffer);
    FREE(session->buffer);
    FREE(session->b);
    FREE(session);
}

void recvRtmpSession(RtmpSession *session, Buffer *buffer)
{
    assert(session || buffer);

    if (session->state == RTMP_HANDSHAKE_UNINIT || session->state == RTMP_HANDSHAKE_0) 
        _parseRtmpHandshake(session, buffer);
    
    return _parseRtmpChunk(session, buffer);
}


