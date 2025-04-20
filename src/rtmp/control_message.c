#include "control_message.h"
#include "rtmp_session.h"
#include "type.h"

int sendPeerBandwidth(RtmpSession *session, uint32_t window_size, uint8_t limit_type)
{
    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_0,
        .csid = RTMP_CHANNEL_PROTOCOL,
        .timestamp = 0,
        .length = 0,
        .type_id = RTMP_TYPE_WINDOW_ACKNOWLEDGEMENT_SIZE,
        .stream_id = 0,
    };

    Buffer *buffer = createBuffer(100);
    if (!buffer)
        return NET_FAIL;

    bs_t *b = bs_new(buffer->data, buffer->length);
    if (!b) {
        FREE(buffer);
        return NET_FAIL;
    }

    int len = writeChunkHeader(b, &header);
    if (len < 0)
        return NET_FAIL;

    bs_write_u(b, 32, window_size);
    bs_write_u8(b, limit_type);

    send(session->conn->fd, buffer->data, len + 5, 0);

    FREE(buffer);
    FREE(b);
    
    return NET_SUCCESS;
}