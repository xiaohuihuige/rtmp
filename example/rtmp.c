#include <schedule/net-common.h>
#include "rtmp_server.h"
#include "media.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void signal_handler(int signum) {
    LOG("Caught signal %d\n", signum);
    exit(0); // 处理完信号后退出程序
}

int main()
{
    struct sigaction sa;
    sa.sa_handler = signal_handler; // 设置信号处理函数
    sigemptyset(&sa.sa_mask);       // 初始化信号集
    sa.sa_flags = 0;                 // 默认标志

    // 注册信号处理
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    RtmpServer * rtmp = createRtmpServer(DEFAULT_IP, SERVER_PORT);

    while (1)
    {
        sleep(1);
    }
    
    destroyRtmpServer(rtmp);
}
