#include <schedule/net-common.h> 
#include "rtmp_server.h"
#include "rtmp_media.h"
#include "media_config.h"
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

    RtmpServer * rtmp = NULL;

    RtmpMedia *app_media = NULL;
    RtmpMedia *light_media = NULL;
    RtmpMedia *girl_media = NULL;
    RtmpMedia *mountain_media = NULL;
    RtmpMedia *poker_media = NULL;

    RtmpConfig *app_config = NULL;
    RtmpConfig *light_config = NULL;
    RtmpConfig *girl_config = NULL;
    RtmpConfig *mountain_config = NULL;
    RtmpConfig *poker_config = NULL;

    do {
        rtmp = createRtmpServer(DEFAULT_IP, 3000);
        if (!rtmp)
            break;

        app_config = createFileRtmpConfig("app",
                                        "./resources/test.h264",
                                        "./resources/suiyueruge.aac");
        if (!app_config)
            break;

        // light_config = createFileRtmpConfig("light",
        //                                 "./resources/light.h264",
        //                                 NULL);
        // if (!light_config)
        //     break;

        girl_config = createFileRtmpConfig("girl",
                                           "./resources/girl.h264",
                                           NULL);
        if (!girl_config)
            break;

        mountain_config = createFileRtmpConfig("mountain",
                                               "./resources/mountain.h264",
                                               NULL);
        if (!mountain_config)
            break;

        // poker_config = createFileRtmpConfig("poker",
        //                                     "./resources/poker_face.h264",
        //                                     "./resources/poker_face.aac");
        // if (!poker_config)
        //     break;

        app_media = createRtmpMedia(app_config);
        if (!app_media)
            break;

        // light_media = createRtmpMedia(light_config);
        // if (!light_media)
        //     break;

        girl_media = createRtmpMedia(girl_config);
        if (!girl_media)
            break;

        // poker_media = createRtmpMedia(poker_config);
        // if (!poker_media)
        //     break;

        mountain_media = createRtmpMedia(mountain_config);
        if (!mountain_media)
            break;

        addRtmpServerMedia(rtmp, app_media);
        // addRtmpServerMedia(rtmp, light_media);
        addRtmpServerMedia(rtmp, girl_media);
        // addRtmpServerMedia(rtmp, poker_media);
        addRtmpServerMedia(rtmp, mountain_media);

        while (keep_running) sleep(1);
    } while (0);

    destroyRtmpMedia(app_media);
    // destroyRtmpMedia(light_media);
    destroyRtmpMedia(girl_media);
    // destroyRtmpMedia(poker_media);
    destroyRtmpMedia(mountain_media);

    destroyRtmpServer(rtmp);

    // destroyRtmpConfig(app_config);
    // destroyRtmpConfig(light_config);
    destroyRtmpConfig(girl_config);
    // destroyRtmpConfig(poker_config);
    destroyRtmpConfig(mountain_config);

    return EXIT_SUCCESS;
}
