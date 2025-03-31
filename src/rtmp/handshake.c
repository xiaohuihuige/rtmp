#include "handshake.h"
#include "type.h"

static void _buildHandShakeRandom(Buffer *buffer, int index)
{
    assert(buffer || buffer->length > index);

    srand(time(NULL));
    int indexs = index;
    while (buffer->length > indexs) {
        writeBufferU1(buffer, indexs, 8, rand());
        indexs++;
    }
}

static Buffer *_buildHandShakeS0()
{
    Buffer *buffer = createBuffer(1);
    if (!buffer) {
        return NULL;
    }

    writeBufferU(buffer, 0, 8, 8, RTMP_VERSION);

    return buffer;
}

static Buffer *_buildHandShakeS1()
{
    Buffer *buffer = createBuffer(RTMP_HANDSHAKE_SIZE);
    if (!buffer) {
        return NULL;
    }

    writeBufferU(buffer, 0, 8, 32, time(NULL));
    writeBufferU(buffer, 4, 8, 32, 0);
    _buildHandShakeRandom(buffer, 8);
    return buffer;
}

static Buffer *_buildHandShakeS2()
{
    Buffer *buffer = createBuffer(RTMP_HANDSHAKE_SIZE);
    if (!buffer) {
        return NULL;
    }

    writeBufferU(buffer, 0, 8, 32, time(NULL));
    writeBufferU(buffer, 4, 8, 32, 0);
    _buildHandShakeRandom(buffer, 8);
    return buffer;
}

static int _sendRtmpHandShakeS0S1S2(RtmpSession *session, Buffer *buffer)
{
    Buffer *send_buffer = createBuffer(2 * RTMP_HANDSHAKE_SIZE + 1);
    if (!send_buffer)
        return NET_FAIL;

    Buffer *v_buffer = NULL;
    Buffer *s1_buffer = NULL;
    Buffer *s2_buffer = NULL;

    do {
        v_buffer = _buildHandShakeS0();
        if (!v_buffer)
            break;

        s1_buffer = _buildHandShakeS1();
        if (!s1_buffer)
            break;

        s2_buffer = _buildHandShakeS2();
        if (!s2_buffer)
            break;

        writeBuffer(send_buffer, 0, v_buffer->data, v_buffer->length);
        writeBuffer(send_buffer, v_buffer->length, s1_buffer->data, s1_buffer->length);
        writeBuffer(send_buffer, v_buffer->length + s1_buffer->length, s2_buffer->data, s2_buffer->length);

        send(session->conn->fd, send_buffer->data, send_buffer->length, 0);

    } while (0);

    FREE(v_buffer);
    FREE(s1_buffer);
    FREE(s2_buffer);
    FREE(send_buffer);

    return NET_SUCCESS;
}


void createRtmpHandShake(RtmpSession *session, Buffer *buffer)
{
    assert(session || buffer);

    if (session->state == RTMP_HANDSHAKE_UNINIT) {
        session->state = RTMP_HANDSHAKE_0;
        _sendRtmpHandShakeS0S1S2(session, buffer);
    }


    if (session->state == RTMP_HANDSHAKE_0) {
        session->state = RTMP_HANDSHAKE_1;
    }
}
