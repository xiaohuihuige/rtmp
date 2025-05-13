#include <schedule/net-common.h>
#include "rtmp_server.h"
#include "rtmp_media.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>


volatile sig_atomic_t keep_running = 1;

void signal_handler(int signum) {
    switch (signum) {
        case SIGINT:
            printf("Caught SIGINT (Ctrl+C)\n");
            keep_running = 0; // 设置标志以指示程序应退出
            break;
        case SIGTERM:
            printf("Caught SIGTERM\n");
            keep_running = 0; // 设置标志以指示程序应退出
            break;
        case SIGQUIT:
            printf("Caught SIGQUIT (Ctrl+\\)\n");
            exit(0); // 处理完信号后退出程序
            break;
        case SIGHUP:
            printf("Caught SIGHUP\n");
            // 可以在这里添加特定的处理逻辑
            keep_running = 0; // 设置标志以指示程序应退出
            break;
        case SIGSEGV:
            printf("Caught SIGSEGV\n");
            // 可以在这里添加特定的处理逻辑
            keep_running = 0; // 设置标志以指示程序应退出
            break;
        case SIGPIPE:
             printf("Caught SIGPIPE\n");
            // 可以在这里添加特定的处理逻辑
            keep_running = 0; // 设置标志以指示程序应退出
            break;
        default:
            break;
    }
}
int main()
{
    struct sigaction sa;
    sa.sa_handler = signal_handler; // 设置信号处理函数
    sigemptyset(&sa.sa_mask);       // 初始化信号集
    sa.sa_flags = 0;                 // 默认标志

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGQUIT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGHUP, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    if (sigaction(SIGPIPE, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    
    RtmpServer * rtmp = createRtmpServer(DEFAULT_IP, SERVER_PORT);

    addRtmpServerMedia(rtmp, createRtmpMedia("app", "./resources/test.h264", NULL));
    addRtmpServerMedia(rtmp, createRtmpMedia("live", "./resources/test1.h264", NULL));
    addRtmpServerMedia(rtmp, createRtmpMedia("girl", "./resources/girl.h264", NULL));
    addRtmpServerMedia(rtmp, createRtmpMedia("mountain","./resources/mountain.h264", NULL));

    while (keep_running) sleep(1);

    destroyRtmpServer(rtmp);
}
