#include "rtmp_media.h"
#include "send_chunk.h"
#include "rtmp_server.h"

static int _sendVideoFrameTimer(void *args)
{
    assert(args);

    FifoQueue *task_node = NULL;
    Buffer *frame = NULL;
    RtmpMedia *media = (RtmpMedia *)args;

    if (media->config->getH264Stream)
        frame = media->config->getH264Stream(media->video);

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

static VideoMedia *_initVideoChannl(RtmpMedia *media, RtmpConfig *config)
{
    if (!config->createH264Stream || !config->h264_file)
        return NULL;

    return config->createH264Stream(config->h264_file);
}

RtmpMedia *createRtmpMedia(RtmpConfig *config)
{   
    if (!config)
        return NULL;

    RtmpMedia *media = CALLOC(1, RtmpMedia);
    if (!media)
        return NULL;

    MUTEX_INIT(&media->myMutex);

    media->video = _initVideoChannl(media, config);
    media->app = config->app;
    media->config = config;

    do {
        media->sessions = createFifiQueue();
        if (!media->sessions)
            break;

        media->scher = createTaskScheduler();
        if (!media->scher)
            break;

        media->gop = createGopCache(2);
        if (!media->gop)
            break;

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

    if (media->vtimer) {
        deleteTimerTask(media->vtimer);
        media->vtimer = NULL;
    }

    if (media->scher) {
        destroyTaskScheduler(media->scher);
        media->scher  = NULL;
    }

    if (media->video && media->config && media->config->destroyH264Stream) {
        media->config->destroyH264Stream(media->video);
        media->video  = NULL;
    }

    if (media->audio && media->config && media->config->destroyAacStream) {
        media->config->destroyAacStream(media->audio);
        media->audio  = NULL;
    }

    if (!media->sessions) {
        destroyFifoQueueTask(media->sessions, RtmpSession);
        media->sessions = NULL;
    }

    if (media->gop) {
        destroyGopCache(media->gop);
        media->gop = NULL;
    }

    MUTEX_DESTROY(&media->myMutex);

    media->config = NULL;

    FREE(media);
}







