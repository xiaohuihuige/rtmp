#include "media.h"
#include "type.h"
#include "send_chunk.h"

static Buffer *_readMediaFile(const char *file_path)
{
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

Media *createMediaChannl(const char *app, int stream_type, const char *file_path)
{
    Media *media = CALLOC(1, Media);
    if (!media)
        return NULL;
    
    snprintf(media->app, sizeof(media->app), "%s", app);
    media->type = stream_type;
    media->func.createMedia = NULL;
    media->func.destroyMedia = NULL;
    media->func.getMediaFrame = NULL;
    media->func.media = NULL;

    Buffer * buffer = _readMediaFile(file_path);
    if (!buffer) 
        goto error;

    if (initStreamType(stream_type, &media->func))
        goto error;

    if (media->func.createMedia) {
        media->func.media = media->func.createMedia(buffer);
        if (!media->func.media)
            goto error;
    }

    FREE(buffer);

    return media;

error:
    if (media->func.destroyMedia)
        media->func.destroyMedia(media->func.media);

    FREE(buffer);
    FREE(media);
    return NULL;
}

void destroyMediaChannl(Media *media)
{
    if (!media)
        return;

    if (media->func.destroyMedia)
        media->func.destroyMedia(media->func.media);

    FREE(media);
}


Buffer *getMediaFrame(Media *media, int index)
{
    if (!media || index < 0)
        return NULL;

    if (media->func.getMediaFrame && media->func.media)
        return media->func.getMediaFrame(media->func.media, index);

    return NULL;
}

Buffer *getMediaInfo(Media *media)
{
    if (!media)
        return NULL;
    return media->func.getMediaInfo(media->func.media);
}

void *getMediaConfig(Media *media)
{
    if (!media)
        return NULL;
    return media->func.getMediaConfig(media->func.media);
}