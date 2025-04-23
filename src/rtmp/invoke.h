#ifndef __INVOKE_H__
#define __INVOKE_H__

#include <schedule/net-common.h>
#include "rtmp_session.h"
#include "chunk_header.h"

int handleInvokeEvent(RtmpSession *session, bs_t *b, HeaderChunk *header);

#endif // !__INVOKE_H__
