#ifndef __REPLY_H__
#define __REPLY_H__

#include "type.h"
#include "rtmp_session.h"
#include "chunk_header.h"

int rtmpSendConnect(RtmpSession *session, HeaderChunk *header, int code, double transactionId);
int rtmpSendResult(RtmpSession *session, HeaderChunk *header, int code, double transactionId);
int rtmpSendOnplay(RtmpSession *session, HeaderChunk *header, int code, double transactionId);
int rtmpSendOnstatus(RtmpSession *session, HeaderChunk *header, int code, double transactionId);

#endif // !__REPLY_H__