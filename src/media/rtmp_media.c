#include "rtmp_media.h"
#include "send_chunk.h"
#include "rtmp_server.h"
#include "aac.h"


RtmpMedia *createRtmpMedia(RtmpConfig *config)
{
    RtmpMedia *media = CALLOC(1, RtmpMedia);
    if (!media)
        return NULL;

    snprintf(media->app, sizeof(media->app), "%s", config->app);

    if (config->type == FILE_MEDIA) {

        media->createH264Stream  = createH264Media;
        media->destroyH264Stream = destroyH264Media;
        media->getH264Stream     = getH264MediaFrame;

        media->destroyAacStream  = destroyAacMedia;
        media->getAacStream      = getAacMediaFrame;
        media->createAacStream   = createAacMedia;

        // media->video  = createH264Media(config->aac_file);
        // media->audio  = createAacMedia(config->h264_file);
    } else if (config->type == LOCAL_MEDIA)
    {

    }
    return media;
}

void setRtmpMediaCallBack()
{

}

void destroyRtmpMedia(RtmpMedia *media)
{
    destroyH264Media(media->video);
    destroyAacMedia(media->audio);
    media->audio = NULL;
    media->video = NULL;
    FREE(media);
}

static int sendVideoFrameToclient(void *args)
{
    RtmpSession *session = (RtmpSession *)args;

    Buffer *frame = getH264MediaFrame(session->media->video, session->channle[VIDEO_CHANNL].index);
    if (!frame) {
        session->channle[VIDEO_CHANNL].index = 0;
        frame = getH264MediaFrame(session->media->video, session->channle[VIDEO_CHANNL].index);
        if (!frame) 
            return NET_SUCCESS;
    }

    if (NET_SUCCESS != sendFrameStream(session, frame, session->channle[VIDEO_CHANNL].time_base))
    {
        ERR("send video error");
        return NET_FAIL;
    }

    session->channle[VIDEO_CHANNL].time_base += frame->timestamp;
    session->channle[VIDEO_CHANNL].index++;

    return frame->frame_type;
}

static int sendAudioFrameToclient(void *args)
{
    RtmpSession *session = (RtmpSession *)args;

    Buffer *frame = getAacMediaFrame(session->media->audio, session->channle[AUDIO_CHANNL].index);
    if (!frame) {
        session->channle[AUDIO_CHANNL].index = 0;
        frame = getAacMediaFrame(session->media->audio, session->channle[AUDIO_CHANNL].index);
        if (!frame)
            return NET_SUCCESS;
    }

    if (NET_SUCCESS != sendAudioStream(session, frame, frame->timestamp))
    {
        ERR("send audio error");
        return NET_FAIL;
    }
    
    session->channle[AUDIO_CHANNL].index++;

    return frame->frame_type;
}

static void _sendGopVideoFrameToclient(RtmpSession *session, int long_time)
{
    if (!session)
        return ;
    
    int base_time = 0;
    int video_time = 0;
    int audio_time = 0;

    while (base_time <= long_time)
    {
        if (base_time <= video_time && (base_time + 10) > video_time && session->media->video)
        {
            sendVideoFrameToclient(session);
            video_time += session->media->video->duration;
            continue;
        }

        if (base_time <= audio_time && (base_time + 10) > audio_time && session->media->audio)
        {
            sendAudioFrameToclient(session);
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

    if (session->media->video)
        sendVideoAVCStream(session, session->media->video->avc_sequence, session->channle[VIDEO_CHANNL].time_base);

    if (session->media->audio)
        sendAudioAdtsStream(session, session->media->audio->adts_sequence, session->channle[AUDIO_CHANNL].time_base);

    _sendGopVideoFrameToclient(session, 6000);

    if (session->media->video && session->media->video->queue) {
        session->video_task = addTimerTask(session->conn->tcps->scher, 
                                            session->media->video->duration,
                                            session->media->video->duration, 
                                            sendVideoFrameToclient, (void *)session);
        if (!session->video_task)
            return NET_FAIL;
    }

    if (session->media->audio && session->media->audio->queue) {
        session->audio_task = addTimerTask(session->conn->tcps->scher, 
                                            session->media->audio->duration, 
                                            session->media->audio->duration, 
                                            sendAudioFrameToclient, (void *)session);
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