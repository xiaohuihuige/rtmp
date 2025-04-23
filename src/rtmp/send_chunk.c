#include "send_chunk.h"

#define CHUNK_SIZE 255

int sendFrameStream(RtmpSession *session, Buffer *frame)
{
    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_0,
        .csid = RTMP_CHANNEL_DATA,
        .timestamp = frame->timestamp,
        .length = frame->length,
        .type_id = RTMP_TYPE_VIDEO,
        .stream_id = 0,
    };

    return sendRtmpPacket(session, &header, frame);
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

int sendScriptStream(RtmpSession *session, Buffer *frame)
{
    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_1,
        .csid = RTMP_CHANNEL_INVOKE,
        .timestamp = frame->timestamp,
        .length = frame->length,
        .type_id = RTMP_TYPE_DATA,
        .stream_id = 0,
    };

    return sendRtmpPacket(session, &header, frame);
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

    while (frame->length <= frame->index) {
        bs_t *b = bs_new(buffer->data, buffer->length);
        if (!b) 
            break;

        writeChunkHeader(b, header);

        int chunk_size = frame->length - frame->index < CHUNK_SIZE ? frame->length - frame->index : CHUNK_SIZE;
        
        bs_write_bytes(b, frame->data + frame->index, chunk_size);
        
        int code = send(session->conn->fd, buffer->data, bs_pos(b), 0);
        if (code <= 0)
            break;

        frame->index += chunk_size;

        header->fmt  = RTMP_CHUNK_TYPE_3;

        FREE(b);
    }

    FREE(buffer);

    return NET_SUCCESS;
}


