#ifndef __INVOKE_H__
#define __INVOKE_H__

#include <schedule/net-common.h>
#include "rtmp_session.h"

int handleInvokeEvent(RtmpSession *session, bs_t *b);

#endif // !__INVOKE_H__
