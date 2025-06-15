#include "util.h"

Buffer *readMediaFile(const char *file_path)
{
    if (!file_path)
        return NULL;
        
    FILE *fp = fopen(file_path, "rb+");
    if (!fp)
        return NULL;

    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    Buffer *buffer = createBuffer(fileSize);
    if (!buffer) {
        fclose(fp);
        return NULL;
    }

    fread(buffer->data, 1, fileSize, fp);

    fclose(fp);

    return buffer;
}

void printfRtmpAddr(int port, const char *app)
{
    char play_ip[64] = {0};

    getHostAddrs(play_ip, sizeof(play_ip));

    LOG("play rtmp address 【rtmp://%s:%d/%s】", play_ip, port, app);
}

uint32_t calculateTimeStamp(double *fractional_part, int fps, int sample_number)
{
    if (!fps)
        return 0;

    double timestamp =  (double)(1000 * sample_number) / fps;

    uint32_t integer_part = (uint32_t)timestamp; // 整数部分

    *fractional_part = *fractional_part + (timestamp - integer_part);

    if (*fractional_part > 1.0) {
        integer_part++;
        (*fractional_part)--;
    }

    return integer_part;
}