#ifndef __SEND_CHUNK_H__
#define __SEND_CHUNK_H_

#include "rtmp_session.h"

static inline int sendToClient(RtmpSession *session, uint8_t *data, int len)
{
    //printfChar(data, len);
    ssize_t bytes_sent = send(session->conn->fd, data, len, MSG_NOSIGNAL);
    if (bytes_sent <= 0) {
        if (errorReSend(session->conn->fd)) {
            ERR("send() failed: %s", strerror(errno));
        } else {
            bytes_sent = send(session->conn->fd, data, len, MSG_NOSIGNAL);
            if (bytes_sent <= 0) { 
                ERR("send() failed: %s", strerror(errno));
            }
        }
    } else {
        //DBG("[send message fd:%d, length: %d]", session->conn->fd, len);
    }

    return bytes_sent;
}

int sendFrameStream(RtmpSession *session, Buffer *frame, uint32_t timestamp);
int sendAudioStream(RtmpSession *session, Buffer *frame, uint32_t timestamp);
int sendScriptStream(RtmpSession *session, Buffer *frame, uint32_t timestamp);

int sendRtmpPacket(RtmpSession *session, HeaderChunk *header, Buffer *frame);

Buffer *rtmpWriteFrame(Buffer *frame);
Buffer *rtmpAvcSequence(Buffer *sps_frame, Buffer *pps_frame);

#endif