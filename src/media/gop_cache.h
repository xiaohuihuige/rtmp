#ifndef __GOP_CACHE_H__
#define __GOP_CACHE_H__

#include "type.h"
#include <schedule/net-common.h>
#include <schedule/tcp_server.h>
#include <schedule/timestamp.h>

GopCache *createGopCache(int idr_count);
void destroyGopCache(GopCache *gop);

void pullFrameToGopCache(GopCache *gop, Buffer *frame);
void sendGopCacheToClient(GopCache *gop, sendFrameToClient func, void *args);

#endif