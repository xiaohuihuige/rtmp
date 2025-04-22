#ifndef __CONTRLOL_MESSAGES__H_
#define __CONTRLOL_MESSAGES__H_

#include "rtmp_session.h"

#define RTMP_FMSVER				"FMS/3,0,1,123"
#define RTMP_CAPABILITIES		31

#define RTMP_STREAM_LIVE	"live"
#define RTMP_STREAM_RECORD	"record"
#define RTMP_STREAM_APPEND	"append"

#define RTMP_LEVEL_WARNING	"warning"
#define RTMP_LEVEL_STATUS	"status"
#define RTMP_LEVEL_ERROR	"error"
#define RTMP_LEVEL_FINISH	"finish" // ksyun cdn
#define RTMP_WINDOW_SIZE    5000000

int sendPeerBandwidth(RtmpSession *session, Buffer *buffer, uint32_t window_size, uint8_t limit_type);
int sendConnectResult(RtmpSession *session, Buffer *buffer, double transactionId);
int sendChunkSize(RtmpSession *session, Buffer *buffer, int chunk_size);
int sendAcknowledgement(RtmpSession *session, Buffer *buffer, uint32_t acknowledgement);

#endif // !__CONTRLOL_MESSAGES__H_
