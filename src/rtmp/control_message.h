#ifndef __CONTRLOL_MESSAGES__H_
#define __CONTRLOL_MESSAGES__H_

#include "rtmp_session.h"

int sendPeerBandwidth(RtmpSession *session, uint32_t window_size, uint8_t limit_type);

#endif // !__CONTRLOL_MESSAGES__H_
