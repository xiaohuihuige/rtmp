#include <schedule/net-common.h> 
#include "rtmp_server.h"
#include "rtmp_media.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "mpp_h264.h"
#include "aac_audio.h"
#include "h264.h"

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

void exception_handling()
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
}

RtmpConfig *createOnlieRtmpConfig(const char *app, const char *v4l2_device, const char *aac_device)
{
    RtmpConfig *config = CALLOC(1, RtmpConfig);
    if (!config)
        return NULL;

    //gop 缓存2个I帧
    config->idr_count         = 2;
    
    //名称
    config->app               = app;
    config->v4l2_device       = v4l2_device;
    config->alsa_device       = aac_device;

    config->createH264Stream  = createMppH264Media;
    config->destroyH264Stream = destroyMppH264Media;
    config->getH264Stream     = getMppH264MediaFrame;
    
    //查看支持的分辨率和v4l2输出的格式
    //v4l2-ctl --device=/dev/video0 --list-formats-ext
    config->display_height    = 640;
    config->display_width     = 480;
    config->height            = 640;
    config->width             = 480;

    config->v4l2_format       = V4L2_PIX_FMT_YUYV;
    config->fps               = 30;
    config->mpp_format        = MPP_FMT_YUV422_YUYV;

    // config->createAacStream  = createAlsaAacMedia;
    // config->destroyAacStream = destroyAlsaAacMedia;
    // config->getAacStream     = getAlsaAacMediaFrame;
    // config->u64PcmSampleRate = 16000;
    // config->u32PcmSampleBits = 16;
    // config->u32PcmChannels   = 2;

    return config;
}

RtmpConfig *createFileRtmpConfig(const char *app, const char *h264_file)
{
    RtmpConfig *config = CALLOC(1, RtmpConfig);
    if (!config)
        return NULL;

    //gop 缓存2个I帧
    config->idr_count         = 2;
    
    //名称
    config->app               = app;
    config->h264_file         = h264_file;

    config->createH264Stream  = createH264Media;
    config->destroyH264Stream = destroyH264Media;
    config->getH264Stream     = getH264MediaFrame;
    
    return config;
}

int main()
{
    exception_handling();
    
    RtmpServer *rtmp        = NULL;
    RtmpMedia *app_media    = NULL;
    RtmpMedia *live_media   = NULL;
    RtmpConfig *app_config  = NULL;
    RtmpConfig *live_config = NULL;

    rtmp = createRtmpServer(DEFAULT_IP, 1935);
    if (!rtmp)
        goto ERROR;

    app_config = createOnlieRtmpConfig("app", "/dev/video0", "plughw:2,0");
    if (!app_config)
        goto ERROR;
    
    live_config = createFileRtmpConfig("live", "./resources/mountain.h264");
    if (!live_config)
        goto ERROR;

    app_media = createRtmpMedia(app_config);
    if (!app_media)
        goto ERROR;

    live_media= createRtmpMedia(live_config);
    if (!live_media)
        goto ERROR;

    addMediaToRtmpServer(rtmp, app_media);
    addMediaToRtmpServer(rtmp, live_media);

    while (keep_running) 
        sleep(1);

ERROR:    
    destroyRtmpMedia(app_media);
    destroyRtmpMedia(live_media);
    destroyRtmpServer(rtmp);
    FREE(app_config);
    FREE(live_config);
    return EXIT_SUCCESS;
}
