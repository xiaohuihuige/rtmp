#ifndef __HANDSHAKE_H__
#define __HANDSHAKE_H__
#include <schedule/net-common.h>
#include <schedule/tcp_server.h>
#include <schedule/timestamp.h>
#include "rtmp_session.h"

int sendHandShakeS0S1S2(RtmpSession *session, Buffer *buffer);

#endif
