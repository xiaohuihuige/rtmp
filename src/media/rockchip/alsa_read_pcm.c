
#include "alsa_read_pcm.h"
#include <alsa/control.h>

//"USB-Audio"
//getAlsaCareDriver("USB-Audio", "DOV");
int getAlsaCareDriver(const char *driver_name, const char *name, char *result, int size)
{
    int card = -1;
    snd_ctl_t *handle;
    snd_ctl_card_info_t *info;
    int err;

    // 初始化声卡信息结构
    snd_ctl_card_info_alloca(&info);

    // 遍历所有声卡
    while (snd_card_next(&card) >= 0 && card >= 0) 
    {
        char card_name[128] = {0};
        snprintf(card_name, sizeof(card_name), "hw:%d", card);

        // 打开声卡控制
        if ((err = snd_ctl_open(&handle, card_name, 0)) < 0) {
            ERR("Cannot open control for card %d: %s", card, snd_strerror(err));
            continue;
        }

        // 获取声卡信息
        if ((err = snd_ctl_card_info(handle, info)) < 0) {
            ERR("Cannot get card info for card %d: %s", card, snd_strerror(err));
            snd_ctl_close(handle);
            continue;
        }

        // 打印声卡信息
        LOG("Card %d: %s", card, snd_ctl_card_info_get_id(info));
        LOG("  Driver: %s", snd_ctl_card_info_get_driver(info));
        LOG("  Name: %s", snd_ctl_card_info_get_name(info));
        LOG("  Longname: %s", snd_ctl_card_info_get_longname(info));

        // 获取驱动名称
        const char *driver = snd_ctl_card_info_get_driver(info);
        const char *card_name1 = snd_ctl_card_info_get_id(info);

        snd_ctl_close(handle);
        if (!strcmp(driver, driver_name) && !strcmp(card_name1, name)) {
            snprintf(result, size, "plughw:%d,0", card);
            return NET_SUCCESS;
        }
    }
    return NET_FAIL;
}

//SND_PCM_FORMAT_S16_LE
snd_pcm_t *createAlsaAudio(const char *device_name, int sample_rate, int channel_count, snd_pcm_format_t format)
{
    snd_pcm_t *capture_handle = NULL;

    do {
        int32_t err = snd_pcm_open(&capture_handle, device_name, SND_PCM_STREAM_CAPTURE, 0);
        if (err)
        {
            ERR("Unable to open: %s. %s", device_name, snd_strerror(err));
            break;
        }

        snd_pcm_hw_params_t *hw_params;
        snd_pcm_hw_params_alloca(&hw_params);

        err = snd_pcm_hw_params_any(capture_handle, hw_params);
        if (err) {
            ERR("Failed to initialize hw_params: %s", snd_strerror(err));
            break;
        }

        err = snd_pcm_hw_params_set_access(capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
        if (err) {
            ERR("Failed to set access type: %s", snd_strerror(err));
            break;
        }

        err = snd_pcm_hw_params_set_format(capture_handle, hw_params, format);
        if (err) {
            ERR("Failed to set format: %s", snd_strerror(err));
            break;
        }

        // mono
        err = snd_pcm_hw_params_set_channels(capture_handle, hw_params, channel_count);
        if (err) {
            ERR("Failed to set number of channels to 2. %s",snd_strerror(err));
            break; 
        }

        int32_t dir = 0;
        err = snd_pcm_hw_params_set_rate_near(capture_handle, hw_params, &sample_rate, &dir);
        if (err) {
            ERR("Failed to set sample rate to, %d: %s", sample_rate, snd_strerror(err));
            break;
        }

        err = snd_pcm_hw_params(capture_handle, hw_params);
        if (err) {
            ERR("Failed to set hw params: %s", snd_strerror(err));
            break;
        }

        err = snd_pcm_prepare(capture_handle);
        if (err) {
            ERR("Failed to prepare for recording: %s", snd_strerror(err));
            break;
        }

        LOG("Recording started! %d", sample_rate);

        return capture_handle;
    } while(0);

    snd_pcm_close(capture_handle);

    return NULL;
}

Buffer *readAlsaAudio(snd_pcm_t *capture_handle, int channel_count, int32_t num_samples)
{
    Buffer *buffer = createBuffer(num_samples * channel_count * 2);
    if (!buffer)
        return NULL;

    int32_t count = snd_pcm_readi(capture_handle, buffer->data, num_samples);
    if (count == -EPIPE) {
        snd_pcm_prepare(capture_handle);
        return NULL;
    } else if (count < 0)
    {
        ERR("Can't read PCM device: %s", snd_strerror(count));
        return NULL;
    }
    LOG("%d", buffer->length);
    return buffer;
}