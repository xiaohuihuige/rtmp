#ifndef __INVOKE_H__
#define __INVOKE_H__

#include <schedule/net-common.h>
#include "rtmp_session.h"

int rtmp_read_onconnect(RtmpSession *session, bs_t *b);
int rtmp_read_oncreate_stream(RtmpSession *session, bs_t *b);
int rtmp_read_onplay(RtmpSession *session, bs_t *b);
int rtmp_read_onget_stream_length(RtmpSession *session, bs_t *b);
int rtmp_read_ondelete_stream(RtmpSession *session, bs_t *b);
int rtmp_read_onreceive_audio(RtmpSession *session, bs_t *b);
int rtmp_read_onreceive_video(RtmpSession *session, bs_t *b);
int rtmp_read_onpublish(RtmpSession *session, bs_t *b);
int rtmp_read_onseek(RtmpSession *session, bs_t *b);
int rtmp_read_onpause(RtmpSession *session, bs_t *b);
int rtmp_read_fcpublish(RtmpSession *session, bs_t *b);
int rtmp_read_onfcpublish(RtmpSession *session, bs_t *b);
int rtmp_read_onfcunpublish(RtmpSession *session, bs_t *b);
int rtmp_read_onfcsubscribe(RtmpSession *session, bs_t *b);
int rtmp_read_onfcunsubscribe(RtmpSession *session, bs_t *b);
int rtmp_read_releaseStream(RtmpSession *session, bs_t *b);

#endif // !__INVOKE_H__
