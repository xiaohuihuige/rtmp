#include "rtmp_server.h"
#include "rtmp_session.h"
#include "util.h"

static void _recvMessage(void *seesion, Buffer *buffer)
{
    return recvRtmpSession((RtmpSession *)seesion, buffer);
}

static void *_createRtmpSession(Seesion *conn)
{
    return createRtmpSession(conn);
}

static void _destroyRtmpSession(void *seesion)
{
    return destroyRtmpSession((RtmpSession *)seesion);
}

RtmpServer *createRtmpServer(const char *ip, int port)
{
    if (!ip)
        return NULL;

    RtmpServer *rtmp = CALLOC(1, RtmpServer);
    if (!rtmp)
        return NULL;

    MUTEX_INIT(&rtmp->myMutex);

    do {
        rtmp->stream = createFifiQueue();
        if (!rtmp->stream)
            break;

        rtmp->server = createTcpServer(ip, port);
        if (!rtmp->server) 
            break;

        setTcpServerCallBack(rtmp->server, _createRtmpSession, _recvMessage, _destroyRtmpSession);
        
        setParentClassServer(rtmp->server, rtmp);

        return rtmp;

    } while (0);

    destroyRtmpServer(rtmp);

    return NULL;
}

void destroyRtmpServer(RtmpServer *rtmp)
{
    if (!rtmp)
        return;

    destroyTcpServer(rtmp->server);

    MUTEX_LOCK(&rtmp->myMutex);
    destroyFifoQueueTask(rtmp->stream, RtmpMedia);
    MUTEX_UNLOCK(&rtmp->myMutex);

    MUTEX_DESTROY(&rtmp->myMutex);
    
    rtmp->server = NULL;
    rtmp->stream = NULL;
    FREE(rtmp);
}

void addRtmpServerMedia(RtmpServer *rtmp, RtmpMedia *media)
{
    if (!rtmp || !media)
        return;

    printfRtmpAddr(rtmp->server->port, media->app);

    MUTEX_LOCK(&rtmp->myMutex);
    enqueue(rtmp->stream, media);
    MUTEX_UNLOCK(&rtmp->myMutex);
}

RtmpMedia *findRtmpServerMedia(RtmpServer *rtmp, const char *app)
{
    if (!rtmp || !app)
        return NULL;

    MUTEX_LOCK(&rtmp->myMutex);
    FifoQueue *task_node = NULL;
    FifoQueue *temp_node = NULL;
    list_for_each_entry_safe(task_node, temp_node, &rtmp->stream->list, list) {
        RtmpMedia *media = (RtmpMedia *)task_node->task;
        if (!strncmp(media->app, app, strlen(app)) ) 
        {
            MUTEX_UNLOCK(&rtmp->myMutex);
            return media;
        }
    }
    MUTEX_UNLOCK(&rtmp->myMutex);
    return NULL;
}