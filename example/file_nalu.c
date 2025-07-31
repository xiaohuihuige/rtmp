#include <schedule/net-common.h> 
#include "rtmp_server.h"
#include "rtmp_media.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "h264_nal.h"

int main()
{
    FILE *file_fp = fopen("./resources/poker_face.h264", "rb+");
    if (!file_fp)
        return NET_FAIL;
    int i = 8;
    while (i--)
        find_file_nal_unit(file_fp);
    fclose(file_fp);
}
