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
    session->packet = NULL;
    session->stream_task = NULL;

    session->media = createMediaChannl("app", 0, "test.h264");

    session->interval == 33;

    return session;
}

void destroyRtmpSession(RtmpSession *session)
{
    LOG("close");
    FREE(session);
}

static int _runRtmpPushStream(RtmpSession *session)
{
    if (session->media->buffer->index >= session->media->buffer->length)
        return NET_FAIL;

    int nal_start = 0, nal_end = 0;

    int resp = find_nal_unit(session->media->buffer->data + session->media->buffer->index, session->media->buffer->length - session->media->buffer->index, &nal_start, &nal_end);
    if (resp <= 0)
        return NET_FAIL;

    uint8_t *nalu_start = session->media->buffer->data + session->media->buffer->index + nal_start;

    rtmp_paser_packet(nalu_start, nal_end - nal_start, (*nalu_start) & 0x1F);

    static int count = 0;
    Buffer *frame = createFrameBuffer(nalu_start, nal_end - nal_start, (*nalu_start) & 0x1F, count);
    if (frame->frame_type == NAL_UNIT_TYPE_SPS)
    {
        if (!session->sps_frame)
            session->sps_frame = frame;
    }

    if (frame->frame_type == NAL_UNIT_TYPE_PPS) {
        if (!session->pps_frame)
            session->pps_frame = frame;
    }

    if (session->pps_frame && session->sps_frame) 
    {
        if (frame->frame_type == NAL_UNIT_TYPE_SPS || frame->frame_type == NAL_UNIT_TYPE_PPS) {
            rtmpAvcSequence(session->sps_frame, session->pps_frame);
        }
    }


    Buffer *rtmpWriteFrame(Buffer *frame);
    Buffer *rtmpAvcSequence(Buffer *sps_frame, Buffer *pps_frame);

    sendFrameStream(RtmpSession *session, Buffer *frame);
    sendScriptStream(RtmpSession *session, Buffer *frame);

    count += session->interval;

    session->media->buffer->index += nal_end - nal_start;

    return NET_SUCCESS;
}

int addRtmpPushTask(RtmpSession *session)
{
    session->stream_task = addTimerTask(session->conn->tcps->scher, 0, 30, _runRtmpPushStream, session);
    if (!session->stream_task)
        return NET_FAIL;
}

static int _parseFirstChunkPacket(RtmpPacket *packet, Buffer *buffer)
{
    assert(buffer);

    if (0 > readHeaderChunk(buffer, &packet->header))
        return NET_FAIL;

    if (packet->header.length > 0)
        packet->buffer = createBuffer(packet->header.length);

    int leave_over = buffer->length - packet->header.header_len - buffer->index;

    if (packet->buffer && leave_over > 0) {
        int residue = packet->header.length > leave_over ? leave_over : packet->header.length;
        writeBuffer(packet->buffer, 0, buffer->data + buffer->index + packet->header.header_len, residue);
        packet->index += residue;
        buffer->index += residue + packet->header.header_len;
    }

    LOG("first packet, [%p, body length %d, index %d], [%p, revc buffer length %d, index %d], ", packet, packet->header.length, packet->index, buffer, buffer->length, buffer->index);
    
    if (packet->index != packet->header.length) 
        return NET_FAIL;

    return NET_SUCCESS;
}

static int _parseLastChunkPacket(RtmpPacket *packet, Buffer *buffer)
{
    assert(packet || buffer);

    int residue = packet->header.length - packet->index;

    LOG("last packet, [%p, body length %d, index %d], [%p revc buffer length %d, index %d]", 
        packet, packet->header.length, packet->index, buffer, buffer->length, buffer->index);

    if (residue > 0) {

        int need_len = residue > (buffer->length - buffer->index) ? buffer->length - buffer->index : residue;

        writeBuffer(packet->buffer, packet->index, buffer->data + buffer->index, need_len);

        packet->index += need_len;

        buffer->index += need_len;

        LOG("end packet, [%p, body length %d, index %d], [%p, revc buffer length %d, index %d]", 
            packet, packet->header.length, packet->index, buffer, buffer->length, buffer->index);
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
        session->packet = packet; 
        return NET_FAIL;
    }

    handleRtmpEvent(session, packet);

    return NET_SUCCESS;
}

static void _parseRtmpChunk(RtmpSession *session, Buffer *buffer)
{
    assert(buffer);

    //printfChar(buffer->data, buffer->length);

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
    LOG("recv size:%d, %p", buffer->length, session);

    _checkChunkComplete(buffer);

    return _parseRtmpPacket(session, buffer);
}


