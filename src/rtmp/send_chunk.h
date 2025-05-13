#ifndef __SEND_CHUNK_H__
#define __SEND_CHUNK_H_

#include "rtmp_session.h"
#include <schedule/net-common.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

static inline void showSendBufferSize(SOCKET sockfd)
{
    int send_queue_length;
    if (ioctl(sockfd, TIOCOUTQ, &send_queue_length) < 0) {
        ERR("ioctl");
    }
    DBG("[getSendBufSize %d, send_queue_length %d]", getSendBufSize(sockfd), send_queue_length);
}

static inline int sendToClient(RtmpSession *session, uint8_t *data, int len)
{
    // DBG("[send message fd:%d, length: %d]", session->conn->fd, len);
    // showSendBufferSize(session->conn->fd);
    int send_bytes = send(session->conn->fd, data, len, MSG_NOSIGNAL);
    if (send_bytes <= 0) 
        ERR("send() failed: %s", strerror(errno));
    return send_bytes;
}

static inline int blockSendToClient(RtmpSession *session, uint8_t *data, int len)
{
    DBG("[send message fd:%d, length: %d]", session->conn->fd, len);
    return Send(session->conn->fd, data, len, 5);
}

int sendFrameStream(RtmpSession *session, Buffer *frame, uint32_t timestamp);
int sendAudioStream(RtmpSession *session, Buffer *frame, uint32_t timestamp);
int sendScriptStream(RtmpSession *session, Buffer *frame, uint32_t timestamp);

int sendRtmpPacket(RtmpSession *session, HeaderChunk *header, Buffer *frame);

Buffer *rtmpWriteFrame(uint8_t *data, int size, int type);
Buffer *rtmpAvcSequence(Buffer *sps_frame, Buffer *pps_frame);

#endif