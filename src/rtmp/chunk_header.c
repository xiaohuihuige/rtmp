#include "chunk_header.h"
#include "type.h"

static void _readBasicHeader(bs_t *b, HeaderChunk *header)
{
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
} 

static void _readMessageHeader(bs_t *b, HeaderChunk *header)
{
    if (header->fmt == RTMP_CHUNK_TYPE_3)
        return;

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
}

static void _showChunkHeader(HeaderChunk *header)
{
    LOG("fmt %d, csid %d, timestemp %d, header_len %d, length %d, type %d, stream %d", 
            header->fmt, header->csid, header->timestamp, header->header_len,
            header->length, header->type_id, header->stream_id);
}

int readHeaderChunk(Buffer *buffer, HeaderChunk *header)
{
    bs_t *b = bs_new(buffer->data, buffer->length);
    if (!b)
        return NET_FAIL;

    _readBasicHeader(b, header);
    _readMessageHeader(b, header);

    header->header_len = bs_pos(b);

    FREE(b);

    _showChunkHeader(header);

    return header->header_len;
}