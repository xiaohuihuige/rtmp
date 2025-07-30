#ifndef __ALSA_READ_H__
#define __ALSA_READ_H__

#include <schedule/net-common.h>
#include "type.h"

AudioMedia *createAlsaAacMedia(RtmpConfig *config);
Buffer *getAlsaAacMediaFrame(AudioMedia *media);
void destroyAlsaAacMedia(AudioMedia *media);

#endif