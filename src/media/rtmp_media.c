#include "rtmp_media.h"
#include "send_chunk.h"
#include "rtmp_server.h"

static int _sendVideoFrameTimer(void *args)
{
    assert(args);

    FifoQueue *task_node = NULL;
    RtmpMedia *media = (RtmpMedia *)args;

    Buffer *frame = media->config->getH264Stream(media->video, 0);
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

    pullFrameToGopCache(media->gop, frame);

    MUTEX_UNLOCK(&media->myMutex);
    return NET_SUCCESS;
}

static void _startPushSessionStream(RtmpMedia *media)
{
    if (!media)
        return;

    if (media->video) {
        media->vtimer = addTimerTask(media->scher,  
                                    media->video->duration,
                                    media->video->duration, 
                                    _sendVideoFrameTimer, (void *)media);
    }

    return;
}

void addRtmpSessionToMedia(RtmpMedia *media, RtmpSession *session)
{
    if (!media || !media->sessions || !session)
        return;

    if (media->video && media->video->avc_sequence) 
        sendVideoAVCStream(session, media->video->avc_sequence, 
                            session->channle[VIDEO_CHANNL].time_base);

    MUTEX_LOCK(&media->myMutex);
    sendGopCacheToClient(media->gop, fifoQueuePush, session->queue);
    enqueue(media->sessions, session);
    MUTEX_UNLOCK(&media->myMutex);
}

void removeRtmpSessionByMedia(RtmpMedia *media, RtmpSession *session)
{
    if (!media || !media->sessions || !session)
        return;

    MUTEX_LOCK(&media->myMutex);
    FindDeleteFifoQueueTask(media->sessions, RtmpSession, session);
    MUTEX_UNLOCK(&media->myMutex);
}

static void _initVideoChannl(RtmpMedia *media, RtmpConfig *config)
{
    if (!config->createH264Stream || !config->h264_file)
        return;

    media->video = config->createH264Stream(config->h264_file);
}

RtmpMedia *createRtmpMedia(RtmpConfig *config)
{   
    if (!config)
        return NULL;

    RtmpMedia *media = CALLOC(1, RtmpMedia);
    if (!media)
        return NULL;

    _initVideoChannl(media, config);

    snprintf(media->app, sizeof(media->app), "%s", config->app);

    media->config = config;

    MUTEX_INIT(&media->myMutex);

    do {
        media->sessions = createFifiQueue();
        if (!media->sessions)
            break;

        media->scher = createTaskScheduler();
        if (!media->scher)
            break;

        media->gop = createGopCache(2);

        _startPushSessionStream(media);

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

    if (media->scher)
        destroyTaskScheduler(media->scher);

    if (media->video && media->config && media->config->destroyH264Stream)
        media->config->destroyH264Stream(media->video);

    if (media->audio && media->config && media->config->destroyAacStream)
        media->config->destroyAacStream(media->audio);

    destroyFifoQueueTask(media->sessions, RtmpSession);
    MUTEX_DESTROY(&media->myMutex);

    media->scher  = NULL;
    media->vtimer = NULL;
    media->sessions = NULL;
    
    media->audio  = NULL;
    media->video  = NULL;
    media->config = NULL;

    FREE(media);
}







