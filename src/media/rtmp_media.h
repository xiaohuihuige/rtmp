#ifndef __MEDIA_H__
#define __MEDIA_H__

#include <schedule/net-common.h>
#include <schedule/tcp_server.h>
#include <schedule/timestamp.h>
#include "rtmp_session.h"
#include "type.h"
#include "util.h"

RtmpMedia *createRtmpMedia(RtmpConfig *config);
void destroyRtmpMedia(RtmpMedia *media);

void addRtmpSessionToMedia(RtmpMedia *media, RtmpSession *session);
void deleteRtmpSessionToMedia(RtmpMedia *media, RtmpSession *session);

#endif // !__MEDIA_H__




