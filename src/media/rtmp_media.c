#include "rtmp_media.h"
#include "send_chunk.h"
#include "rtmp_server.h"
#include "aac.h"
#include "h264.h"

static int _sendVideoFrameToclient(void *args)
{
    assert(args);

    RtmpSession *session = (RtmpSession *)args;

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
    assert(args);

    RtmpSession *session = (RtmpSession *)args;

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

static int _sendVideoFrameTimer(void *args)
{
    assert(args);

    FifoQueue *task_node = NULL;
    FifoQueue *temp_node = NULL;

    RtmpMedia *media = (RtmpMedia *)args;

    Buffer *frame = media->config->getH264Stream(media->video, 0);
    if (!frame)
        return NET_FAIL;
    
    list_for_each_entry_safe(task_node, temp_node, &media->sessions->list, list)
    {
        if (!task_node || !task_node->task)
            continue;

        RtmpSession *session = (RtmpSession *)task_node->task;

        addTriggerTask(session->conn->tcps->scher, _sendVideoFrameToclient, (void *)session, 0);
    }

    return NET_SUCCESS;
}


static int _sendAudioFrameTimer(void *args)
{
    assert(args);

    FifoQueue *task_node = NULL;
    FifoQueue *temp_node = NULL;

    list_for_each_entry_safe(task_node, temp_node, &((RtmpMedia *)args)->sessions->list, list)
    {
        if (!task_node || !task_node->task)
            continue;

        RtmpSession *session = (RtmpSession *)task_node->task;

        addTriggerTask(session->conn->tcps->scher, _sendAudioFrameToclient, (void *)session, 0);
    }

    return NET_SUCCESS;
}


static void _sendGopVideoFrameToclient(RtmpMedia *media, RtmpSession *session, int long_time)
{
    if (!session)
        return;
    
    int base_time = 0;
    int video_time = 0;
    int audio_time = 0;

    while (base_time <= long_time)
    {
        if (base_time <= video_time && (base_time + 10) > video_time && media->video)
        {
            _sendVideoFrameToclient(session);
            video_time += media->video->duration;
            continue;
        }

        if (base_time <= audio_time && (base_time + 10) > audio_time && media->audio)
        {
            _sendAudioFrameToclient(session);
            audio_time += media->audio->duration;
            continue;
        }
        base_time += 10;
    }
}

static int _startPushSessionStream(RtmpMedia *media)
{
    if (!media)
        return NET_FAIL;

    if (media->video && media->video->queue) {
        media->vtimer = addTimerTask(media->scher,  
                                    media->video->duration,
                                    media->video->duration, 
                                    _sendVideoFrameTimer, (void *)media);
        if (!media->vtimer)
            return NET_FAIL;
    }

    if (media->audio && media->audio->queue) {
        media->atimer = addTimerTask(media->scher, 
                                    media->audio->duration, 
                                    media->audio->duration, 
                                    _sendAudioFrameTimer, (void *)media);
        if (!media->atimer)
            return NET_FAIL;
    }
    return NET_SUCCESS;
}

int findRtmpMediaStream(RtmpSession *session)
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

static void _sendRtmpMediaGop(RtmpMedia *media, RtmpSession *session)
{
    if (media->video && media->video->avc_sequence)
        sendVideoAVCStream(session, media->video->avc_sequence, session->channle[VIDEO_CHANNL].time_base);

    if (media->audio && media->audio->adts_sequence)
        sendAudioAdtsStream(session, media->audio->adts_sequence, session->channle[AUDIO_CHANNL].time_base);

    _sendGopVideoFrameToclient(media, session, 6000);
}

void addRtmpSessionToMedia(RtmpMedia *media, RtmpSession *session)
{
    if (!media || !media->sessions || !session)
        return;

    _sendRtmpMediaGop(media, session);

    enqueue(media->sessions, session); 
}

void removeRtmpSessionFromMedia(RtmpMedia *media, RtmpSession *session)
{
    if (!media || !media->sessions || !session)
        return;

    FindDeleteFifoQueueTask(media->sessions, RtmpSession, session);
}

RtmpConfig *createRtmpConfig(const char *app, const char *h264_file, const char *aac_file, 
                            CreateH264Stream createH264Stream, DestroyH264Stream destroyH264Stream,
                            GetH264Stream getH264Stream, CreateAacStream createAacStream, 
                            DestroyAacStream destroyAacStream, GetAacStream getAacStream)
{

    RtmpConfig *config = CALLOC(1, RtmpConfig);
    if (!config)
        return NULL;

    config->app = app;

    config->aac_file  = aac_file;
    config->h264_file = h264_file;

    config->createH264Stream  = createH264Stream;
    config->destroyH264Stream = destroyH264Stream;
    config->getH264Stream     = getH264Stream;

    config->createAacStream   = createAacStream;
    config->destroyAacStream  = destroyAacStream;
    config->getAacStream      = getAacStream;

    return config;
}

RtmpConfig *createFileRtmpConfig(const char *app, const char *h264_file, const char *aac_file)
{
    return createRtmpConfig(app, h264_file, aac_file, 
                        createH264Media, destroyH264Media, getH264MediaFrame, 
                        createAacMedia,  destroyAacMedia, getAacMediaFrame);
}

RtmpMedia *createRtmpMedia(RtmpConfig *config)
{   
    if (!config)
        return NULL;

    RtmpMedia *media = CALLOC(1, RtmpMedia);
    if (!media)
        return NULL;

    snprintf(media->app, sizeof(media->app), "%s", config->app);

    media->config = config;

    do {
        if (config->createH264Stream)
            media->video = config->createH264Stream(config->h264_file);

        if (config->createAacStream)
            media->audio = config->createAacStream(config->aac_file);

        media->sessions = createFifiQueue();
        if (!media->sessions)
            break;

        media->scher = createTaskScheduler();
        if (!media->scher)
            break;

        if (_startPushSessionStream(media))
            break;

        return media;

    } while(0);


    destroyRtmpMedia(media);

    return NULL;
}

void destroyRtmpMedia(RtmpMedia *media)
{
    if (!media)
        return;

    if (media->vtimer)
        deleteTimerTask(media->vtimer);

    if (media->atimer)
        deleteTimerTask(media->atimer);

    if (media->scher)
        destroyTaskScheduler(media->scher);

    if (media->video && media->config && media->config->destroyH264Stream)
        media->config->destroyH264Stream(media->video);

    if (media->audio && media->config && media->config->destroyAacStream)
        media->config->destroyAacStream(media->audio);

    FREE(media->config);

    destroyFifoQueueTask(media->sessions, RtmpSession);

    media->scher  = NULL;
    media->atimer = NULL;
    media->vtimer = NULL;
    media->sessions = NULL;
    
    media->audio  = NULL;
    media->video  = NULL;
    media->config = NULL;

    FREE(media);
}