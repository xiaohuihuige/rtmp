#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/select.h>
#include <sys/mman.h>
#include "v4l2_capture.h"


static int camera_source_ioctl(int fd, int req, void* arg)
{
    struct timespec poll_time;
    int ret;

    while ((ret = ioctl(fd, req, arg))) {
        if (ret == -1 && (EINTR != errno && EAGAIN != errno)) {
            // mpp_err("ret = %d, errno %d", ret, errno);
            break;
        }
        // 10 milliseconds
        poll_time.tv_sec = 0;
        poll_time.tv_nsec = 10000000;
        nanosleep(&poll_time, NULL);
    }

    return ret;
}

//V4L2_PIX_FMT_YUV422P
//V4L2_PIX_FMT_MJPEG
//V4L2_PIX_FMT_MPEG
//V4L2_PIX_FMT_YUYV
//V4L2_PIX_FMT_MJPEG
V4l2Capture *createV4l2Capture(const char *dev_name, int bufcnt, int width, int height, uint32_t format, int fps)
{
    V4l2Capture *v4l2 = CALLOC(1, V4l2Capture);
    if (!v4l2)
        return NULL;

    struct v4l2_capability     cap;
    struct v4l2_format         vfmt;
    struct v4l2_requestbuffers req;
    enum   v4l2_buf_type       type;
    struct v4l2_streamparm     param;

    v4l2->fd = open(dev_name, O_RDWR, 0);
    if (v4l2->fd < 0)
        return NULL;

    v4l2->bufcnt = bufcnt;

    do {
        // Determine if fd is a V4L2 Device
        if (0 != camera_source_ioctl(v4l2->fd, VIDIOC_QUERYCAP, &cap)) {
            ERR("Not v4l2 compatible");
            break;
        }

        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) 
            && !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)) {
            ERR("Capture not supported");
            break;
        }

        if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
            ERR("Streaming IO Not Supported");
            break;
        }

        // Preserve original settings as set by v4l2-ctl for example
        vfmt = (struct v4l2_format) {0};
        vfmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
            vfmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

        vfmt.fmt.pix.width = width;
        vfmt.fmt.pix.height = height;

        {
            struct v4l2_fmtdesc fmtdesc;

            fmtdesc.index = 0;
            fmtdesc.type = vfmt.type;
            while (!camera_source_ioctl(v4l2->fd, VIDIOC_ENUM_FMT, &fmtdesc)) {
                LOG("fmt name: [%s]", fmtdesc.description);
                LOG("fmt pixelformat: '%c%c%c%c', description = '%s'", fmtdesc.pixelformat & 0xFF,
                        (fmtdesc.pixelformat >> 8) & 0xFF, (fmtdesc.pixelformat >> 16) & 0xFF,
                        (fmtdesc.pixelformat >> 24) & 0xFF, fmtdesc.description);
                fmtdesc.index++;
            }
        }

        vfmt.fmt.pix.pixelformat = format;
        type = vfmt.type;
        v4l2->type = vfmt.type;

        if (-1 == camera_source_ioctl(v4l2->fd, VIDIOC_S_FMT, &vfmt)) {
            ERR("VIDIOC_S_FMT");
            break;
        }

        if (-1 == camera_source_ioctl(v4l2->fd, VIDIOC_G_FMT, &vfmt)) {
            ERR("VIDIOC_G_FMT");
            break;
        }

        memset(&param, 0, sizeof(param));
        param.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        param.parm.capture.timeperframe.numerator = 1;
        param.parm.capture.timeperframe.denominator = fps;
        param.parm.capture.capturemode = 1;
        if (-1 == camera_source_ioctl(v4l2->fd, VIDIOC_S_PARM, &param)) {
            ERR("VIDIOC_S_PARMT");
            break;
        }

        if (-1 == camera_source_ioctl(v4l2->fd, VIDIOC_G_PARM, &param)) {
            ERR("VIDIOC_G_PARM");
            break;
        }

        LOG("width %d height %d", vfmt.fmt.pix.width, vfmt.fmt.pix.height);
        LOG("fmt pixelformat: '%c%c%c%c'", vfmt.fmt.pix.pixelformat & 0xFF,
                        (vfmt.fmt.pix.pixelformat >> 8) & 0xFF, (vfmt.fmt.pix.pixelformat >> 16) & 0xFF,
                        (vfmt.fmt.pix.pixelformat >> 24) & 0xFF);

        // Request memory-mapped buffers
        req = (struct v4l2_requestbuffers) {0};
        req.count  = v4l2->bufcnt;
        req.type   = type;
        req.memory = V4L2_MEMORY_MMAP;
        if (-1 == camera_source_ioctl(v4l2->fd, VIDIOC_REQBUFS, &req)) {
            ERR("Device does not support mmap");
            break;
        }

        if (req.count != v4l2->bufcnt) {
            ERR("Device buffer count mismatch");
            break;
        }

        // Queue buffers
        for (int i = 0; i < v4l2->bufcnt; i++) {
            // Query buffer
            v4l2->buf.index = i;
            v4l2->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            v4l2->buf.memory = V4L2_MEMORY_MMAP;
            if (-1 == camera_source_ioctl(v4l2->fd, VIDIOC_QUERYBUF, &v4l2->buf)) {
                ERR("VIDIOC_QUERYBUF");
                break;
            }

            // mmap buffer
            v4l2->mmap_buffer[i].length = v4l2->buf.length;
            v4l2->mmap_buffer[i].start = (unsigned char *)mmap(0, v4l2->buf.length,
                PROT_READ | PROT_WRITE, MAP_SHARED, v4l2->fd, v4l2->buf.m.offset);
            if (v4l2->mmap_buffer[i].start == MAP_FAILED) {
                ERR("mmap %d, %s", i, strerror(errno));
                break;
            }

            if (-1 == camera_source_ioctl(v4l2->fd, VIDIOC_QBUF, &v4l2->buf)) {
                ERR("VIDIOC_QBUF");
                break;
            }
        }

        // Start capturing
        if (-1 == camera_source_ioctl(v4l2->fd, VIDIOC_STREAMON, &type)) {
            ERR("ERROR: VIDIOC_STREAMON");
            break;
        }
        LOG("success open v4l2");
        return v4l2;
    } while (0);

    destroyV4l2Capture(v4l2);

    return NULL;
}

void destroyV4l2Capture(V4l2Capture *v4l2)
{
    if (!v4l2)
        return;

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == camera_source_ioctl(v4l2->fd, VIDIOC_STREAMOFF, &type)) {
        ERR("VIDIOC_STREAMOFF failed %s", strerror(errno));
        return;
    }

    // Release the resource
    for (int i = 0; i < v4l2->bufcnt; i++) {
        munmap(v4l2->mmap_buffer[i].start, v4l2->mmap_buffer[i].length);
    }

    close(v4l2->fd);
    FREE(v4l2);
    LOG("Camera release done.");
}

Buffer *getV4l2Frame(V4l2Capture *v4l2)
{
    fd_set fds;
    struct timeval tv;

    // 将fd加入fds集合
    FD_ZERO(&fds);
    FD_SET(v4l2->fd, &fds);
    
    /* Timeout. */
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    // 监测是否有数据，最多等待5s
    int r = select(v4l2->fd + 1, &fds, NULL, NULL, &tv);
    if (r == -1) {
        LOG("select err");
        return NULL;
    } else if (r == 0) {
        LOG("select timeout");
        return NULL;
    }

    if (-1 == camera_source_ioctl(v4l2->fd, VIDIOC_DQBUF, &v4l2->buf)) {
        ERR("VIDIOC_DQBUF failed %s", strerror(errno));
        return NULL;
    }
    Buffer *buffer = createFrameBuffer(v4l2->mmap_buffer[v4l2->buf.index].start, v4l2->buf.bytesused, 0, 0);
    if (!buffer)
        return NULL;

    // Re-queue buffer
    // 将处理完毕的视频帧重新放回驱动程序的队列中，以供下一次捕获
    if (-1 == camera_source_ioctl(v4l2->fd, VIDIOC_QBUF, &v4l2->buf)) {
        ERR("VIDIOC_QBUF failed %s", strerror(errno));
        FREE(buffer);
        return NULL;
    }

    return buffer;  
}