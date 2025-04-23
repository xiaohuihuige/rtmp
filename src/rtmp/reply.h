#ifndef __REPLY_H__
#define __REPLY_H__

#include "type.h"
#include "rtmp_session.h"
#include "chunk_header.h"

int rtmp_reply_connect(RtmpSession *session, HeaderChunk *header, int code, double transactionId);
int rtmp_reply_result(RtmpSession *session, HeaderChunk *header, int code, double transactionId);
int rtmp_reply_onplay(RtmpSession *session, HeaderChunk *header, int code, double transactionId);
int rtmp_reply_onstatus(RtmpSession *session, HeaderChunk *header, int code, double transactionId);

#endif // !__REPLY_H__