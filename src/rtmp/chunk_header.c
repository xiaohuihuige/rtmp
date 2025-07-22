#include "chunk_header.h"
#include "type.h"

#define CHUNK_HEADER 14

static int _wirteChunkHeader(bs_t *b, HeaderChunk *header)
{
    assert(b || header);

    bs_write_u(b, 2, header->fmt);
    if (header->csid >= 64 + 255) {
        bs_write_u(b, 6, 1);
        bs_write_u(b, 16, header->csid);
    }else if (header->csid >= 64) {
        bs_write_u(b, 6, 0);
        bs_write_u(b, 8, header->csid);
    }else {
        bs_write_u(b, 6, header->csid);
    }

    if (header->fmt == RTMP_CHUNK_TYPE_0)
    {
        bs_write_u(b, 24, header->timestamp >= 0xFFFFFF ? 0xFFFFFF : header->timestamp);
        bs_write_u(b, 24, header->length);
        bs_write_u(b, 8, header->type_id);
        bs_write_u(b, 32, header->stream_id);
    }
    else if (header->fmt == RTMP_CHUNK_TYPE_1)
    {
        bs_write_u(b, 24, header->timestamp >= 0xFFFFFF ? 0xFFFFFF : header->timestamp);
        bs_write_u(b, 24, header->length);
        bs_write_u(b, 8, header->type_id);
    }
    else if (header->fmt == RTMP_CHUNK_TYPE_2)
    {
        bs_write_u(b, 24, header->timestamp >= 0xFFFFFF ? 0xFFFFFF : header->timestamp);
    }
    return bs_pos(b);
}

int writeChunkHeader(bs_t *b, HeaderChunk *header)
{
    assert(b || header);

    return _wirteChunkHeader(b, header);
}

static int _readBasicHeader(bs_t *b, HeaderChunk *header)
{
    assert(b || header);

    header->fmt = bs_read_u(b, 2);
    header->csid = bs_read_u(b, 6);
    if (header->csid == 0)
    {
        header->csid = bs_read_u8(b) + 64;
    }
    else if (header->csid == 1)
    {
        header->csid = bs_read_u(b, 16) + 64 + 255;
    }
    return bs_overrun(b);
} 

static int _readMessageHeader(bs_t *b, HeaderChunk *header)
{
    assert(b || header);

    if (header->fmt == RTMP_CHUNK_TYPE_3)
        return NET_SUCCESS;

    if (header->fmt == RTMP_CHUNK_TYPE_0)
    {
        header->timestamp = bs_read_u(b, 24);
        header->length = bs_read_u(b, 24);
        header->type_id = bs_read_u(b, 8);
        header->stream_id = bs_read_u(b, 32);
        if (header->timestamp >= 0xffffff)
        {
            header->timestamp = bs_read_u(b, 32);
        }
    }
    else if (header->fmt == RTMP_CHUNK_TYPE_1)
    {
        header->timestamp = bs_read_u(b, 24);
        header->length = bs_read_u(b, 24);
        header->type_id = bs_read_u(b, 8);
        if (header->timestamp >= 0xffffff)
        {
            header->timestamp = bs_read_u(b, 32);
        }
    }
    else if (header->fmt == RTMP_CHUNK_TYPE_2)
    {
        header->timestamp = bs_read_u(b, 24);
        if (header->timestamp >= 0xffffff)
        {
            header->timestamp = bs_read_u(b, 32);
        }
    }

    return bs_overrun(b);
}

void _showChunkHeader(HeaderChunk *header)
{
    if (!header)
        return;

    LOG("fmt %d, csid %d, timestemp %d, header_len %d, length %d, type %d, stream %d", 
            header->fmt, header->csid, header->timestamp, header->header_len,
            header->length, header->type_id, header->stream_id);
}


static int _readHeaderChunk(bs_t *b, HeaderChunk *header)
{
    if (!b || !header)
        return NET_FAIL;

    _readBasicHeader(b, header);

    _readMessageHeader(b, header);

    header->header_len = bs_pos(b);

    _showChunkHeader(header);

    return bs_overrun(b);
}

int readHeaderChunk(Buffer *buffer, HeaderChunk *header)
{
    if (!buffer || !header)
        return NET_FAIL;

    bs_t *b = bs_new(buffer->data + buffer->index,  buffer->length - buffer->index);
    if (!b)
        return NET_FAIL;

    LOG("buffer->length - buffer->index %d", buffer->length - buffer->index);

    int overturn = _readHeaderChunk(b, header);

    FREE(b);

    return overturn;
}