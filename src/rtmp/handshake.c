#include "handshake.h"
#include "type.h"
#include "send_chunk.h"

static void _buildHandShakeRandom(bs_t *b, int length)
{
    assert(b);

    srand(time(NULL));
    while (length--) bs_write_u8(b, rand());
}

static int _sendHandShakeS0S1(RtmpSession *session, Buffer *buffer)
{
    assert(session || buffer);

    Buffer *send_buffer = createBuffer(RTMP_HANDSHAKE_SIZE + 1);
    if (!send_buffer)
        return NET_FAIL;

    bs_t *b = bs_new(send_buffer->data, send_buffer->length);
    if (!b) {
        FREE(send_buffer); 
        return NET_FAIL;
    }

    bs_write_u8(b, RTMP_VERSION);

    bs_write_u(b, 32, time(NULL));
    bs_write_u(b, 32, 0);
    _buildHandShakeRandom(b, RTMP_HANDSHAKE_SIZE - 8);

    sendToClient(session, send_buffer->data, send_buffer->length);

    FREE(b);
    FREE(send_buffer);

    return NET_SUCCESS;
}

static int _sendHandShakeS2(RtmpSession *session, Buffer *buffer)
{
    assert(session || buffer);

    Buffer *send_buffer = createBuffer(RTMP_HANDSHAKE_SIZE);
    if (!send_buffer)
        return NET_FAIL;

    bs_t *b = bs_new(send_buffer->data, send_buffer->length);
    if (!b) {
        FREE(send_buffer); 
        return NET_FAIL;
    }

    bs_write_u(b, 32, time(NULL));
    bs_write_u(b, 32, 0);
    bs_write_bytes(b, buffer->data + 1, RTMP_HANDSHAKE_SIZE - 8);
   
    sendToClient(session, send_buffer->data, send_buffer->length);

    FREE(b);
    FREE(send_buffer);

    return NET_SUCCESS;
}

int createRtmpHandShake(RtmpSession *session, Buffer *buffer)
{
    if (!session || !buffer)
        return NET_SUCCESS;

    if (session->state == RTMP_HANDSHAKE_UNINIT) {
        int size = writeBuffer(session->buffer, session->buffer->index, buffer->data, buffer->length);
        session->buffer->index += buffer->length;
        if (session->buffer->index != 1529)
            return NET_SUCCESS;
        session->state = RTMP_HANDSHAKE_0;
        _sendHandShakeS0S1(session, session->buffer);
        _sendHandShakeS2(session, session->buffer);
        session->buffer->index = 0;
        return NET_SUCCESS;
    }

    if (session->state == RTMP_HANDSHAKE_0) {
        LOG("s1111 %d", buffer->length);
        session->buffer->index += buffer->length;
        if (session->buffer->index < 1528)
            return NET_SUCCESS;
        
        session->state = RTMP_HANDSHAKE_1;
        LOG("end----- %d", session->buffer->index - 1528);
        session->buffer->index = 0;
        buffer->index = session->buffer->index - 1528;
        buffer->index = buffer->length - buffer->index;
        if (buffer->index > 0)
            return NET_FAIL;
    }
    return NET_SUCCESS;
}
