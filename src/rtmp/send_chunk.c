#include "send_chunk.h"

#define CHUNK_SIZE 255

Buffer *_rtmpWriteFrame(Buffer *frame)
{
    int length = RTMP_FRAME_HEADER_LENGTH + frame->length;

    Buffer *buffer = createBuffer(length);
    if (!buffer)
        return NULL;

    bs_t *b = bs_new(buffer->data, buffer->length);
    if (!b)
        return NULL;

    if (NAL_UNIT_TYPE_CODED_SLICE_IDR == frame->frame_type)
        bs_write_u8(b, 0x17); //0x27
    else 
        bs_write_u8(b, 0x27); //0x27
    
    bs_write_u8(b, 0x01);
    bs_write_u8(b, 0x00);
    bs_write_u8(b, 0x00);
    bs_write_u8(b, 0x00);

    bs_write_u(b, 32, frame->length);

    bs_write_bytes(b, frame->data, frame->length);

    FREE(b);

    return buffer;
}

Buffer *_rtmpAvcSequence(Buffer *sps_frame, Buffer *pps_frame)
{
    int length = sps_frame->length + pps_frame->length + RTMP_AVC_HEADER_LENGTH;

    Buffer *buffer = createBuffer(length);
    if (!buffer)
        return NULL;

    bs_t *b = bs_new(buffer->data, buffer->length);
    if (!b)
        return NULL;

    bs_write_u8(b, 0x17);
    bs_write_u8(b, 0x00);
    bs_write_u8(b, 0x00);
    bs_write_u8(b, 0x00);
    bs_write_u8(b, 0x00);

    bs_write_u8(b, 0x01);
    bs_write_u8(b, sps_frame->data[1]);
    bs_write_u8(b, sps_frame->data[2]);
    bs_write_u8(b, sps_frame->data[3]);
    bs_write_u8(b, 0xff);

    bs_write_u8(b, 0xe1);
    bs_write_u(b, 16, sps_frame->length);
    bs_write_bytes(b, sps_frame->data, sps_frame->length);
    
    bs_write_u8(b, 0x01);
    bs_write_u(b, 16, pps_frame->length);
    bs_write_bytes(b, pps_frame->data, pps_frame->length);

    FREE(b);

    return buffer;
}

int sendFrameStream(RtmpSession *session, Buffer *frame)
{
    Buffer *buffer = _rtmpWriteFrame(frame);
    if (!buffer)
        return NET_FAIL;
    
    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_0,
        .csid = RTMP_CHANNEL_DATA,
        .timestamp = buffer->timestamp,
        .length = buffer->length,
        .type_id = RTMP_TYPE_VIDEO,
        .stream_id = 0,
    };

    return sendRtmpPacket(session, &header, buffer);
}

int sendAudioStream(RtmpSession *session, Buffer *frame)
{
    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_1,
        .csid = RTMP_CHANNEL_AUDIO,
        .timestamp = frame->timestamp,
        .length = frame->length,
        .type_id = RTMP_TYPE_AUDIO,
        .stream_id = 0,
    };

    return sendRtmpPacket(session, &header, frame);
}

int sendScriptStream(RtmpSession *session, Buffer *sps_frame, Buffer *pps_frame)
{
  
    Buffer * buffer = _rtmpAvcSequence(sps_frame, pps_frame);
    if (!buffer)
        return NET_FAIL;

    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_1,
        .csid = RTMP_CHANNEL_INVOKE,
        .timestamp = buffer->timestamp,
        .length = buffer->length,
        .type_id = RTMP_TYPE_DATA,
        .stream_id = 0,
    };

    return sendRtmpPacket(session, &header, buffer);
}

int sendRtmpPacket(RtmpSession *session, HeaderChunk *header, Buffer *frame)
{
    if (!session || !header || !frame)
        return NET_FAIL;

    if (header->length <= 0)
        return NET_FAIL;

    Buffer *buffer = createBuffer(header->header_len + CHUNK_SIZE);
    if (!buffer)
        return NET_FAIL;

    bs_t *b = bs_new(buffer->data, buffer->length);
    if (!b)
        return NET_FAIL;

    while (frame->length <= frame->index) {

        bs_init(b, buffer->data, buffer->length);

        writeChunkHeader(b, header);

        int chunk_size = frame->length - frame->index < CHUNK_SIZE ? frame->length - frame->index : CHUNK_SIZE;
        
        bs_write_bytes(b, frame->data + frame->index, chunk_size);
        
        int code = sendToClient(session->conn->fd, buffer->data, bs_pos(b));
        if (code <= 0)
            break;

        frame->index += chunk_size;

        header->fmt  = RTMP_CHUNK_TYPE_3;
    }
    FREE(b);
    FREE(buffer);

    return NET_SUCCESS;
}


