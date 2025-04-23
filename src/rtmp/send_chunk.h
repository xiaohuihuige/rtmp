#ifndef __SEND_CHUNK_H__
#define __SEND_CHUNK_H_

#include "rtmp_session.h"

int sendFrameStream(RtmpSession *session, Buffer *frame);
int sendAudioStream(RtmpSession *session, Buffer *frame);
int sendScriptStream(RtmpSession *session, Buffer *frame);
int sendRtmpPacket(RtmpSession *session, HeaderChunk *header, Buffer *frame);

#endif