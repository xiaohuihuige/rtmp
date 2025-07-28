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

static void _checkChunkComplete(Buffer *buffer)
{
    assert(buffer);

    int count = buffer->rindex;

    for (int i = buffer->rindex; i < buffer->index; i++) {
        if (buffer->data[i] != 0xC3) 
            buffer->data[count++] = buffer->data[i];
    }

    buffer->index = count;
}

static void _parseRtmpChunk(RtmpSession *session, Buffer *buffer)
{
    assert(buffer || session);

    _checkChunkComplete(buffer);

    RtmpPacket *packet = (RtmpPacket *)malloc(sizeof(RtmpPacket));
    if (!packet)
        return;

    while (buffer->index - buffer->rindex > 0) {
        int overturn = readHeaderChunk(buffer, &packet->header);
        if (overturn) {
            if (buffer->index - buffer->rindex >= 12)
                buffer->rindex += packet->header.header_len;
            break;
        }

        if (packet->header.length <= 0 || packet->header.length >= RTMP_OUTPUT_CHUNK_SIZE) {
            buffer->rindex += packet->header.header_len;
            break;
        }
        
        if ((packet->header.length + packet->header.header_len) > (buffer->index - buffer->rindex)) 
            break;

        packet->buffer = createFrameBuffer(buffer->data + buffer->rindex + packet->header.header_len, packet->header.length, 0, 0);
        if (!packet->buffer) 
            break;

        buffer->rindex += packet->header.length + packet->header.header_len;

        handleRtmpEvent(session, packet);

        FREE(packet->buffer);

        memset(packet, 0x00, sizeof(RtmpPacket));
    }
    FREE(packet);
}

static void _parseRtmpPacket(RtmpSession *session, Buffer *buffer)
{
    assert(buffer || session);
    
    if (session->temp_buffer->index <= session->temp_buffer->rindex && session->temp_buffer->index != 0)
        session->temp_buffer->index = session->temp_buffer->rindex = 0;

    int actual_len = writeBuffer(session->temp_buffer, session->temp_buffer->index, buffer->data, buffer->length);
    session->temp_buffer->index += actual_len;
    buffer->index += actual_len;

    if (session->state == RTMP_HANDSHAKE_UNINIT)
    {
        if (session->temp_buffer->index < RTMP_HANDSHAKE_SIZE + 1)
            return;

        session->temp_buffer->rindex += RTMP_HANDSHAKE_SIZE + 1;
        sendHandShakeS0S1S2(session, buffer);
        session->state = RTMP_HANDSHAKE_0;
    }

    if (session->state == RTMP_HANDSHAKE_0)
    {
        if (session->temp_buffer->index - session->temp_buffer->rindex < RTMP_HANDSHAKE_SIZE)
            return;
        session->temp_buffer->rindex += RTMP_HANDSHAKE_SIZE;
        session->state = RTMP_HANDSHAKE_1;
    }

    if (session->state == RTMP_HANDSHAKE_1)
    {
        if (session->temp_buffer->index - session->temp_buffer->rindex <= 0)
            return;

        return _parseRtmpChunk(session, session->temp_buffer);
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

        session->temp_buffer = createBuffer(3 * RTMP_HANDSHAKE_SIZE);
        if (!session->temp_buffer)
            break;

        MUTEX_INIT(&session->myMutex);

        session->media          = NULL;
        session->conn           = conn;
        session->state          = RTMP_HANDSHAKE_UNINIT;
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

    return _parseRtmpPacket(session, buffer);
}


