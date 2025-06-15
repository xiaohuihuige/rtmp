#include "rtmp_media.h"
#include "send_chunk.h"
#include "rtmp_server.h"

static int _sendVideoFrameToclient(RtmpSession *session)
{
    assert(session);

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

static int _sendAudioFrameToclient(RtmpSession *session)
{
    assert(session);

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

    MUTEX_LOCK(&media->myMutex);
    list_for_each_entry_safe(task_node, temp_node, &media->sessions->list, list)
    {
        if (!task_node || !task_node->task)
            continue;
        _sendVideoFrameToclient((RtmpSession *)task_node->task);
    }
    MUTEX_UNLOCK(&media->myMutex);

    return NET_SUCCESS;
}

static int _sendAudioFrameTimer(void *args)
{
    assert(args);

    FifoQueue *task_node = NULL;
    FifoQueue *temp_node = NULL;
    RtmpMedia *media = (RtmpMedia *)args;

    MUTEX_LOCK(&media->myMutex);
    list_for_each_entry_safe(task_node, temp_node, &media->sessions->list, list)
    {
        if (!task_node || !task_node->task)
            continue;
        _sendAudioFrameToclient((RtmpSession *)task_node->task);
    }
    MUTEX_UNLOCK(&media->myMutex);
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

    MUTEX_LOCK(&media->myMutex);
    enqueue(media->sessions, session);
    MUTEX_UNLOCK(&media->myMutex);
}

void deleteRtmpSessionToMedia(RtmpMedia *media, RtmpSession *session)
{
    if (!media || !media->sessions || !session)
        return;

    MUTEX_LOCK(&media->myMutex);
    FindDeleteFifoQueueTask(media->sessions, RtmpSession, session);
    MUTEX_UNLOCK(&media->myMutex);
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

    MUTEX_INIT(&media->myMutex);

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

    destroyFifoQueueTask(media->sessions, RtmpSession);
    MUTEX_DESTROY(&media->myMutex);

    media->scher  = NULL;
    media->atimer = NULL;
    media->vtimer = NULL;
    media->sessions = NULL;
    
    media->audio  = NULL;
    media->video  = NULL;
    media->config = NULL;

    FREE(media);
}