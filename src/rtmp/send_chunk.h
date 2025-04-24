#ifndef __SEND_CHUNK_H__
#define __SEND_CHUNK_H_

#include "rtmp_session.h"

static inline int sendToClient(RtmpSession *session, uint8_t *data, int len)
{
    DBG("[send message fd:%d, length: %d]", session->conn->fd, len);
    return send(session->conn->fd, data, len, 0);
}

int sendFrameStream(RtmpSession *session, Buffer *frame);
int sendAudioStream(RtmpSession *session, Buffer *frame);
int sendScriptStream(RtmpSession *session, Buffer *frame);
int sendRtmpPacket(RtmpSession *session, HeaderChunk *header, Buffer *frame);

Buffer *rtmpWriteFrame(Buffer *frame);
Buffer *rtmpAvcSequence(Buffer *sps_frame, Buffer *pps_frame);

#endif