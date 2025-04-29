#include "rtmp_server.h"
#include "rtmp_session.h"

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

static void _printfRtmpAddr(int port, const char *app)
{
    char play_ip[64] = {0};

    getHostAddrs(play_ip, sizeof(play_ip));

    LOG("play rtmp address 【rtmp://%s:%d/%s】", play_ip, port, app);
}

static void _closeMeidaStream(RtmpServer *rtmp)
{
    FifoQueue *task_node = NULL;
    FifoQueue *temp_node = NULL;

    list_for_each_entry_safe(task_node, temp_node, &rtmp->stream->list, list)
    {
        if (task_node->task)
            destroyMediaChannl((Media *)task_node->task);
        task_node->task = NULL;

        deleteFifoQueueTask(task_node, Seesion);
    }
    FREE(rtmp->stream);  
}

RtmpServer *createRtmpServer(const char *ip, int port)
{
    if (!ip)
        return NULL;

    RtmpServer *rtmp = CALLOC(1, RtmpServer);
    if (!rtmp )
        return NULL;

    rtmp->stream = createFifiQueue();
    if (!rtmp->stream) {
        FREE(rtmp);
        return NULL;
    }

    rtmp->server = createTcpServer(ip, port);
    if (!rtmp->server) {
        _closeMeidaStream(rtmp);
        FREE(rtmp);
        return NULL;
    }

    setTcpServerCallBack(rtmp->server, _createRtmpSession, _recvMessage, _destroyRtmpSession);
    
    setParentClassServer(rtmp->server, rtmp);

    return rtmp;
}

void destroyRtmpServer(RtmpServer *rtmp)
{
    if (!rtmp)
        return;

    destroyTcpServer(rtmp->server);

    if (rtmp->stream)
        _closeMeidaStream(rtmp);

    FREE(rtmp);
}

void addRtmpServerStream(RtmpServer *rtmp, Media *media)
{
    if (!rtmp || !media)
        return;

    _printfRtmpAddr(rtmp->server->port, media->app);

    enqueue(rtmp->stream, media); 
}

Media *findRtmpServerStream(RtmpServer *rtmp, const char *app)
{
    if (!rtmp || !app)
        return NULL;

    FifoQueue *task_node = NULL;
    FifoQueue *temp_node = NULL;
    list_for_each_entry_safe(task_node, temp_node, &rtmp->stream->list, list) {
        Media *media = (Media *)task_node->task;
        if (!strncmp(media->app, app, strlen(app)) ) 
            return media;
    }
    return NULL;
}