#ifndef __CONTRLOL_MESSAGES__H_
#define __CONTRLOL_MESSAGES__H_

#include "rtmp_session.h"

int sendPeerBandwidth(RtmpSession *session, Buffer *buffer, uint32_t window_size, uint8_t limit_type);
int sendConnectResult(RtmpSession *session, Buffer *buffer, double transactionId);
int sendChunkSize(RtmpSession *session, Buffer *buffer, int chunk_size);
int sendAcknowledgement(RtmpSession *session, Buffer *buffer, uint32_t acknowledgement);
int sendCreateStreamResult(RtmpSession *session, Buffer *buffer, double transactionId, uint32_t stream_id);
int sendSetStreamBegin(RtmpSession *session, Buffer *buffer, uint32_t stream_id);
int sendOnMetaData(RtmpSession *session, Buffer *buffer);
int sendSampleAccess(RtmpSession *session, Buffer *buffer, uint32_t stream_id);
int sendOnstatus(RtmpSession *session, Buffer *buffer, double transactionId, const char *app);

#endif // !__CONTRLOL_MESSAGES__H_
