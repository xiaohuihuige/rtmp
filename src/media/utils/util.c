#include "util.h"
#include "h264_nal.h"

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


Buffer *findTypeNaluBuffer(uint8_t *data, int length, int type)
{
    if (!data || length <= 0)
        return NULL;

    int frame_length  = 0;
    int frame_type = 0;
    int index = 0;
    uint8_t *nalu_start = NULL;

    while (index < length) 
    {
        int nal_start = 0, nal_end = 0;
        int resp = find_nal_unit(data + index, length - index, &nal_start, &nal_end);
        if (resp <= 0)
        {
            if (data[index] == 0 
                &&  data[index + 1] == 0 
                &&  data[index + 2] == 0 
                &&  data[index + 3] == 1)
            {
                nalu_start = data + index + 4;
                frame_type = (*nalu_start) & 0x1F;
                frame_length = length - (index + 4);
                if (type == frame_type)
                    return createFrameBuffer(nalu_start, frame_length, frame_type, 0);
            }
            break;
        } 

        nalu_start = data + index + nal_start;
        frame_length = nal_end - nal_start;
        frame_type = (*nalu_start) & 0x1F;
        if (type == frame_type)
            return createFrameBuffer(nalu_start, frame_length, frame_type, 0);

        index += nal_end;
    }

    return NULL;
}

Buffer *findFrameNaluBuffer(uint8_t *data, int length)
{
    if (!data || length <= 0)
        return NULL;

    //int frame_length  = 0;
    int frame_type = 0;
    int index = 0;
    uint8_t *nalu_start = NULL;

    while (index < length) 
    {
        int nal_start = 0, nal_end = 0;
        int resp = find_nal_unit(data + index, length - index, &nal_start, &nal_end);
        if (resp <= 0)
        {
            if (data[index] == 0 
                &&  data[index + 1] == 0 
                &&  data[index + 2] == 0 
                &&  data[index + 3] == 1)
            {
                nalu_start = data + index + 4;
                frame_type = (*nalu_start) & 0x1F;
                //frame_length = length - (index + 4);
                if (NAL_UNIT_TYPE_SPS != frame_type  && 
                    NAL_UNIT_TYPE_PPS != frame_type  && 
                    NAL_UNIT_TYPE_SEI != frame_type)
                    return createFrameBuffer(nalu_start, nal_end - nal_start, frame_type, 0);
            }
            break;
        } 

        nalu_start = data + index + nal_start;
        //frame_length = nal_end - nal_start;
        frame_type = (*nalu_start) & 0x1F;
        if (NAL_UNIT_TYPE_SPS != frame_type  && 
            NAL_UNIT_TYPE_PPS != frame_type  && 
            NAL_UNIT_TYPE_SEI != frame_type)
            return createFrameBuffer(nalu_start, nal_end - nal_start, frame_type, 0);

        index += nal_end;
    }

    return NULL;
}