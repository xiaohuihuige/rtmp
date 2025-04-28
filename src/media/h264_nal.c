
#include "h264_nal.h"

int rbsp_to_nal(const uint8_t* rbsp_buf, const int* rbsp_size, uint8_t* nal_buf, int* nal_size)
{
    int i;
    int j     = 1;
    int count = 0;

    if (*nal_size > 0) { nal_buf[0] = 0x00; } // zero out first byte since we start writing from second byte

    for ( i = 0; i < *rbsp_size ; )
    {
        if ( j >= *nal_size ) 
        {
            // error, not enough space
            return -1;
        }

        if ( ( count == 2 ) && !(rbsp_buf[i] & 0xFC) ) // HACK 0xFC
        {
            nal_buf[j] = 0x03;
            j++;
            count = 0;
            continue;
        }
        nal_buf[j] = rbsp_buf[i];
        if ( rbsp_buf[i] == 0x00 )
        {
            count++;
        }
        else
        {
            count = 0;
        }
        i++;
        j++;
    }

    *nal_size = j;
    return j;
}

int nal_to_rbsp(const uint8_t* nal_buf, int* nal_size, uint8_t* rbsp_buf, int* rbsp_size)
{
    int i;
    int j     = 0;
    int count = 0;
  
    for( i = 0; i < *nal_size; i++ )
    { 
        // in NAL unit, 0x000000, 0x000001 or 0x000002 shall not occur at any byte-aligned position
        if( ( count == 2 ) && ( nal_buf[i] < 0x03) ) 
        {
            return -1;
        }

        if( ( count == 2 ) && ( nal_buf[i] == 0x03) )
        {
            // check the 4th byte after 0x000003, except when cabac_zero_word is used, in which case the last three bytes of this NAL unit must be 0x000003
            if((i < *nal_size - 1) && (nal_buf[i+1] > 0x03))
            {
                return -1;
            }

            // if cabac_zero_word is used, the final byte of this NAL unit(0x03) is discarded, and the last two bytes of RBSP must be 0x0000
            if(i == *nal_size - 1)
            {
                break;
            }

            i++;
            count = 0;
        }

        if ( j >= *rbsp_size ) 
        {
            // error, not enough space
            return -1;
        }

        rbsp_buf[j] = nal_buf[i];
        if(nal_buf[i] == 0x00)
        {
            count++;
        }
        else
        {
            count = 0;
        }
        j++;
    }

    *nal_size = i;
    *rbsp_size = j;
    return j;
}

int find_nal_unit(uint8_t *buf, int size, int *nal_start, int *nal_end)
{
    int i;
    // find start
    *nal_start = 0;
    *nal_end = 0;

    i = 0;
    while ( //( next_bits( 24 ) != 0x000001 && next_bits( 32 ) != 0x00000001 )
        (buf[i] != 0 || buf[i + 1] != 0 || buf[i + 2] != 0x01) &&
        (buf[i] != 0 || buf[i + 1] != 0 || buf[i + 2] != 0 || buf[i + 3] != 0x01))
    {
        i++; // skip leading zero
        if (i + 4 >= size)
        {
            return 0;
        } // did not find nal start
    }

    if (buf[i] != 0 || buf[i + 1] != 0 || buf[i + 2] != 0x01) // ( next_bits( 24 ) != 0x000001 )
    {
        i++;
    }

    if (buf[i] != 0 || buf[i + 1] != 0 || buf[i + 2] != 0x01)
    { /* error, should never happen */
        return 0;
    }
    i += 3;
    *nal_start = i;

    while ( //( next_bits( 24 ) != 0x000000 && next_bits( 24 ) != 0x000001 )
        (buf[i] != 0 || buf[i + 1] != 0 || buf[i + 2] != 0) &&
        (buf[i] != 0 || buf[i + 1] != 0 || buf[i + 2] != 0x01))
    {
        i++;
        // FIXME the next line fails when reading a nal that ends exactly at the end of the data
        if (i + 3 >= size)
        {
            *nal_end = size;
            return -1;
        } // did not find nal end, stream ended first
    }

    *nal_end = i;
    return (*nal_end - *nal_start);
}