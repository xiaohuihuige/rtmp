#include "rtmp_media.h"
#include "send_chunk.h"
#include "rtmp_server.h"

static int _sendVideoFrameTimer(void *args)
{
    assert(args);

    FifoQueue *task_node = NULL;
    RtmpMedia *media = (RtmpMedia *)args;

    VideoMedia *video  = getRtmpVideoMeida(media);
    RtmpConfig *config = getRtmpConfig(media);

    Buffer *frame = getH264Frame(config, video);
    if (!frame)
        return NET_FAIL;

    bufferReferenceCount(frame);

    MUTEX_LOCK(&media->myMutex);
    list_for_each_entry(task_node, &media->sessions->list, list)
    {
        if (!task_node || !task_node->task)
            continue;
        bufferReferenceCount(frame);
        fifoQueuePush(((RtmpSession *)task_node->task)->queue, frame);
    }

    pullFrameToGopCache(media->video_gop, frame);

    MUTEX_UNLOCK(&media->myMutex);
    return NET_SUCCESS;
}

static int _sendAudioFrameTimer(void *args)
{
    assert(args);

    FifoQueue *task_node = NULL;
    RtmpMedia *media = (RtmpMedia *)args;

    AudioMedia *audio = getRtmpAudioMedia(media);
    RtmpConfig *config = getRtmpConfig(media);
    Buffer *frame = getHAacFrame(config, audio);
    if (!frame)
        return NET_FAIL;

    bufferReferenceCount(frame);

    MUTEX_LOCK(&media->myMutex);
    list_for_each_entry(task_node, &media->sessions->list, list)
    {
        if (!task_node || !task_node->task)
            continue;
        bufferReferenceCount(frame);
        fifoQueuePush(((RtmpSession *)task_node->task)->queue, frame);
    }

    pullFrameToGopCache(media->audio_gop, frame);

    MUTEX_UNLOCK(&media->myMutex);
    return NET_SUCCESS;
}

static void _startPushSessionStream(RtmpMedia *media)
{
    if (!media)
        return;

    VideoMedia *video = getRtmpVideoMeida(media);
    if (video) {
        media->vtimer = addTimerTask(media->scher,  
                                    video->duration,
                                    video->duration, 
                                    _sendVideoFrameTimer, (void *)media);
    }

    AudioMedia *audio = getRtmpAudioMedia(media);
    if (audio) {
        media->atimer = addTimerTask(media->scher,  
                                    audio->duration,
                                    audio->duration, 
                                    _sendAudioFrameTimer, (void *)media);
    }

    return;
}

void addRtmpSessionToMedia(RtmpMedia *media, RtmpSession *session)
{
    if (!media || !media->sessions || !session)
        return;

    VideoMedia *video = getRtmpVideoMeida(media);
    if (video && video->avc_sequence) 
    {
        sendVideoAVCStream(session, video->avc_sequence, session->channle[VIDEO_CHANNL].time_base);
    }

    AudioMedia *audio = getRtmpAudioMedia(media);
    if (audio && audio->adts_sequence) 
        sendAudioAdtsStream(session, audio->adts_sequence, session->channle[AUDIO_CHANNL].time_base);

    MUTEX_LOCK(&media->myMutex);
    if (audio) {
        sendGopCacheToClient(media->audio_gop, fifoQueuePush, session->queue);
    }
        
    if (video) {
        sendGopCacheToClient(media->video_gop, fifoQueuePush, session->queue);
    }
    enqueue(media->sessions, session);
    MUTEX_UNLOCK(&media->myMutex);

}

void removeRtmpSessionByMedia(RtmpMedia *media, RtmpSession *session)
{
    if (!media || !media->sessions || !session)
        return;

    MUTEX_LOCK(&media->myMutex);
    DeleteTargetTaskNoFree(media->sessions, session);
    MUTEX_UNLOCK(&media->myMutex);
}

RtmpMedia *createRtmpMedia(RtmpConfig *config)
{   
    if (!config)
        return NULL;

    RtmpMedia *media = CALLOC(1, RtmpMedia);
    if (!media)
        return NULL;

    media->video = createVideoChannl(config);
    media->audio = createAudioChannl(config);

    media->app = config->app;
    media->config = config;
    MUTEX_INIT(&media->myMutex);

    do {
        if (!media->video && !media->audio)
            break;

        media->scher = createTaskScheduler();
        if (!media->scher)
            break;
            
        media->sessions = createFifiQueue();
        if (!media->sessions)
            break;

        media->video_gop = createGopCache(config->idr_count);
        if (!media->video_gop)
            break;

        media->audio_gop = createGopCache(config->idr_count);
        if (!media->audio_gop)
            break;

        _startPushSessionStream(media);

        LOG("create the name is:[%s] media success, %p", media->app, media);

        return media;

    } while(0);

    ERR("create %s media fail", media->app);

    destroyRtmpMedia(media);

    return NULL;
}

void destroyRtmpMedia(RtmpMedia *media)
{
    if (!media)
        return;

    if (media->vtimer) {
        deleteTimerTask(media->vtimer);
        media->vtimer = NULL;
    }

    if (media->atimer) {
        deleteTimerTask(media->atimer);
        media->atimer = NULL;
    }

    if (media->scher) {
        destroyTaskScheduler(media->scher);
        media->scher  = NULL;
    }

    AudioMedia *audio  = getRtmpAudioMedia(media);
    VideoMedia *video  = getRtmpVideoMeida(media);
    RtmpConfig *config = getRtmpConfig(media);

    destroyVideoChannl(config, video);
    media->video  = NULL;

    destroyAudioChannl(config, audio);
    media->audio  = NULL;

    if (!media->sessions) {
        destroyFifoQueue(media->sessions);
        media->sessions = NULL;
    }

    if (media->video_gop) {
        destroyGopCache(media->video_gop);
        media->video_gop = NULL;
    }

    if (media->audio_gop) {
        destroyGopCache(media->audio_gop);
        media->audio_gop = NULL;
    }

    MUTEX_DESTROY(&media->myMutex);

    media->config = NULL;

    FREE(media);
}






