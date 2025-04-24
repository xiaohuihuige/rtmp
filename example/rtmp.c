#include <schedule/net-common.h>
#include "rtmp_server.h"
#include "media.h"

int main()
{

    Media *media  = createMediaChannl("app", 0, "test.h264");
    if (!media)
        return NET_FAIL;

    startRunMediaStream(media);

    //RtmpServer * rtmp = createRtmpServer(DEFAULT_IP, SERVER_PORT);

    while (1)
    {
        sleep(1);
    }
    
    //destroyRtmpServer(rtmp);
}
