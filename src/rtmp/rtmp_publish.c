#include "rtmp_media.h"
#include "send_chunk.h"
#include "rtmp_server.h"
#include "rtmp_publish.h"

static int _sendVideoFrameToclient(void *args)
{
    RtmpSession *session = (RtmpSession *)args;

    assert(args || session->media->config || session->media->config->getH264Stream);

    Buffer *frame = session->media->config->getH264Stream(session->media->video, session->channle[VIDEO_CHANNL].index);
    if (!frame) {
        session->channle[VIDEO_CHANNL].index = 0;
        frame = session->media->config->getH264Stream(session->media->video, session->channle[VIDEO_CHANNL].index);
        if (!frame) 
            return NET_SUCCESS;
    }

    if (NET_SUCCESS != sendFrameStream(session, frame, session->channle[VIDEO_CHANNL].time_base)) {
        ERR("send video error");
        return NET_FAIL;
    }

    session->channle[VIDEO_CHANNL].time_base += frame->timestamp;
    session->channle[VIDEO_CHANNL].index++;

    return frame->frame_type;
}

static int _sendAudioFrameToclient(void *args)
{
    RtmpSession *session = (RtmpSession *)args;

    assert(args || session->media->config || session->media->config->getAacStream);

    Buffer *frame = session->media->config->getAacStream(session->media->audio, session->channle[AUDIO_CHANNL].index);
    if (!frame) {
        session->channle[AUDIO_CHANNL].index = 0;
        frame = session->media->config->getAacStream(session->media->audio, session->channle[AUDIO_CHANNL].index);
        if (!frame)
            return NET_SUCCESS;
    }

    if (NET_SUCCESS != sendAudioStream(session, frame, frame->timestamp)) {
        ERR("send audio error");
        return NET_FAIL;
    }
    
    session->channle[AUDIO_CHANNL].index++;

    return frame->frame_type;
}

static void _sendGopVideoFrameToclient(RtmpSession *session, int long_time)
{
    if (!session)
        return;
    
    int base_time = 0;
    int video_time = 0;
    int audio_time = 0;

    while (base_time <= long_time)
    {
        if (base_time <= video_time && (base_time + 10) > video_time && session->media->video)
        {
            _sendVideoFrameToclient(session);
            video_time += session->media->video->duration;
            continue;
        }

        if (base_time <= audio_time && (base_time + 10) > audio_time && session->media->audio)
        {
            _sendAudioFrameToclient(session);
            audio_time += session->media->audio->duration;
            continue;
        }
        base_time += 10;
    }
}

int startPushSessionStream(RtmpSession *session)
{
    if (!session->media)
        return NET_FAIL;

    if (session->media->video && session->media->video->avc_sequence)
        sendVideoAVCStream(session, session->media->video->avc_sequence, session->channle[VIDEO_CHANNL].time_base);

    if (session->media->audio && session->media->audio->adts_sequence)
        sendAudioAdtsStream(session, session->media->audio->adts_sequence, session->channle[AUDIO_CHANNL].time_base);

    _sendGopVideoFrameToclient(session, 6000);

    if (session->media->video && session->media->video->queue) {
        session->video_task = addTimerTask(session->conn->tcps->scher,  
                                            session->media->video->duration,
                                            session->media->video->duration, 
                                            _sendVideoFrameToclient, (void *)session);
        if (!session->video_task)
            return NET_FAIL;
    }

    if (session->media->audio && session->media->audio->queue) {
        session->audio_task = addTimerTask(session->conn->tcps->scher, 
                                            session->media->audio->duration, 
                                            session->media->audio->duration, 
                                            _sendAudioFrameToclient, (void *)session);
        if (!session->audio_task)
            return NET_FAIL;
    }
    return NET_SUCCESS;
}

int findRtmpMedia(RtmpSession *session)
{
    if (!session)
        return NET_FAIL;

    session->media = findRtmpServerMedia((RtmpServer *)session->conn->tcps->parent, session->config.app);
    if (!session->media) {
        ERR("find rtmp stream error");
        return NET_FAIL;
    }

    LOG("find stream %p, name %s", session->media, session->media->app);

    return NET_SUCCESS;
}

void stopPushSessionstream(RtmpSession *session)
{
    if (!session->media)
        return;

    if (session->video_task)
        deleteTimerTask(session->video_task);

    if (session->audio_task)
        deleteTimerTask(session->audio_task);

    session->video_task = NULL;
    session->audio_task = NULL;
}