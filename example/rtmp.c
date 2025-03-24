#include <schedule/net-common.h>
#include "rtmp_server.h"

int main()
{

    RtmpServer * rtmp = createRtmpServer(DEFAULT_IP, SERVER_PORT);

    while (1)
    {
        sleep(1);
    }
    
    destroyRtmpServer(rtmp);
}
