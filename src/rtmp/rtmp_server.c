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

static void _printfRtmpAddr(int port)
{
    char play_ip[64] = {0};

    getHostAddrs("eth0", play_ip, sizeof(play_ip));

    LOG("play rtmp address rtmp://%s:%d/live", play_ip, port);
}

RtmpServer *createRtmpServer(const char *ip, int port)
{
    if (!ip)
        return NULL;

    RtmpServer *rtmp = CALLOC(1, RtmpServer);
    if (!rtmp )
        return NULL;

    rtmp->stream = createFifiQueue();

    rtmp->server = createTcpServer(ip, port);

    setTcpServerCallBack(rtmp->server, _createRtmpSession, _recvMessage, _destroyRtmpSession);
    
    setParentClassServer(rtmp->server, rtmp);

    _printfRtmpAddr(port);
    
    return rtmp;
}

void destroyRtmpServer(RtmpServer *rtmp)
{
    if (!rtmp)
        return;

    return destroyTcpServer(rtmp->server);
}


void addRtmpServerStream(RtmpServer *rtmp, Media *media)
{
    if (!rtmp || !media)
        return;

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