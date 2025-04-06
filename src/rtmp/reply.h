#ifndef __REPLY_H__
#define __REPLY_H__

#include "type.h"
#include "rtmp_session.h"

int rtmp_reply_connect(RtmpSession *session, int code, double transactionId);
int rtmp_reply_result(RtmpSession *session, int code, double transactionId);
int rtmp_reply_onplay(RtmpSession *session, int code, double transactionId);
int rtmp_reply_onstatus(RtmpSession *session, int code, double transactionId);

#endif // !__REPLY_H__