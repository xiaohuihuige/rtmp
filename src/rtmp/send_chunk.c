#include "send_chunk.h"

#define CHUNK_SIZE 255

Buffer *rtmpWriteFrame(uint8_t *data, int size, int type)
{
    int length = RTMP_FRAME_HEADER_LENGTH + size;

    Buffer *buffer = createBuffer(length);
    if (!buffer)
        return NULL;

    bs_t *b = bs_new(buffer->data, buffer->length);
    if (!b)
        return NULL;

    buffer->frame_type = type;

    if (NAL_UNIT_TYPE_CODED_SLICE_IDR == type)
        bs_write_u8(b, 0x17); //0x27
    else 
        bs_write_u8(b, 0x27); //0x27
    
    bs_write_u8(b, 0x01);
    bs_write_u8(b, 0x00);
    bs_write_u8(b, 0x00);
    bs_write_u8(b, 0x00);

    bs_write_u(b, 32, size);

    bs_write_bytes(b, data, size);

    FREE(b);

    return buffer;
}

Buffer *rtmpAvcSequence(Buffer *sps_frame, Buffer *pps_frame)
{
    if (!sps_frame || !pps_frame) {
        ERR("args sps frame, pps frame is null");
        return NULL;
    }

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

    //LOG("pos %d, %d", length, bs_pos(b));

    //printfChar(buffer->data, length);

    FREE(b);
    
    return buffer;
}

int sendFrameStream(RtmpSession *session, Buffer *frame, uint32_t timestamp)
{
    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_0,
        .csid = RTMP_CHANNEL_DATA,
        .timestamp = timestamp,
        .length = frame->length,
        .type_id = RTMP_TYPE_VIDEO,
        .stream_id = 0,
    };

    return sendRtmpPacket(session, &header, frame);
}

int sendAudioStream(RtmpSession *session, Buffer *frame, uint32_t timestamp)
{
    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_1,
        .csid = RTMP_CHANNEL_AUDIO,
        .timestamp = timestamp,
        .length = frame->length,
        .type_id = RTMP_TYPE_AUDIO,
        .stream_id = 0,
    };

    return sendRtmpPacket(session, &header, frame);
}

int sendScriptStream(RtmpSession *session, Buffer *frame, uint32_t timestamp)
{
    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_1,
        .csid = RTMP_CHANNEL_INVOKE,
        .timestamp = timestamp,
        .length = frame->length,
        .type_id = RTMP_TYPE_DATA,
        .stream_id = 0,
    };

    return sendRtmpPacket(session, &header, frame);
}

int sendRtmpPacket(RtmpSession *session, HeaderChunk *header, Buffer *frame)
{
    if (!session || !header || !frame || header->length <= 0)
    {
        ERR("args error");
        return NET_FAIL;
    }
 
    int index = frame->index;

    while (frame->length > index) {
        bs_init(session->b, session->buffer->data, session->buffer->length);

        writeChunkHeader(session->b, header);

        int chunk_size = frame->length - index < RTMP_OUTPUT_CHUNK_SIZE ? frame->length - index : RTMP_OUTPUT_CHUNK_SIZE;
        
        bs_write_bytes(session->b, frame->data + index, chunk_size);

        int code = sendToClient(session, session->buffer->data, bs_pos(session->b));
        if (code <= 0)
            return NET_FAIL;

        index += chunk_size;

        header->fmt  = RTMP_CHUNK_TYPE_3;
    }

    return NET_SUCCESS;
}


