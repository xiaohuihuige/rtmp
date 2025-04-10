#include "amf0.h"

//static double s_double = 1.0; // 3ff0 0000 0000 0000

int amf_write_null(bs_t *b)
{
    if (bs_bytes_left(b) < 1 || b == NULL)
    {
        ERR("args error");
        return -1;
    }

    bs_write_u8(b, AMF_NULL);

    return 0;
}

int amf_write_undefined(bs_t *b)
{
    if (bs_bytes_left(b) < 1 || b == NULL)
    {
        ERR("args error");
        return -1;
    }

    bs_write_u8(b, AMF_UNDEFINED);

    return 0;
}

int amf_write_object(bs_t *b)
{
    if (bs_bytes_left(b) < 1 || b == NULL)
    {
        ERR("args error");
        return -1;
    }

    bs_write_u8(b, AMF_OBJECT);

    return 0;
}

int amf_write_objectEnd(bs_t *b)
{
    if (bs_bytes_left(b) < 1 || b == NULL)
    {
        ERR("args error");
        return -1;
    }

    bs_write_u(b, 24, AMF_OBJECT_END);

    return 0;
}

int amf_write_typedObject(bs_t *b)
{
    if (bs_bytes_left(b) < 1 || b == NULL)
    {
        ERR("args error");
        return -1;
    }

    bs_write_u8(b, AMF_TYPED_OBJECT);

    return 0;
}

int amf_write_CMAArarry(bs_t *b)
{
    if (bs_bytes_left(b) < 1 || b == NULL)
    {
        ERR("args error");
        return -1;
    }

    bs_write_u8(b, AMF_ECMA_ARRAY);
    bs_write_u(b, 32, 0);
    return 0;
}

int amf_write_boolean(bs_t *b, uint8_t value)
{
    if (bs_bytes_left(b) < 2 || b == NULL)
    {
        ERR("args error");
        return -1;
    }

    bs_write_u8(b, AMF_BOOLEAN);
    bs_write_u8(b, value == 0 ? 0 : 1);

    return 0;
}

int amf_write_double(bs_t *b, double value)
{
    if (bs_bytes_left(b) < 9 || b == NULL)
    {
        ERR("args error");
        return -1;
    }

    uint8_t* p = (uint8_t*)&value;

    bs_write_u8(b, AMF_NUMBER);

    bs_write_u8(b, p[7]);
    bs_write_u8(b, p[6]);
    bs_write_u8(b, p[5]);
    bs_write_u8(b, p[4]);
    bs_write_u8(b, p[3]);
    bs_write_u8(b, p[2]);
    bs_write_u8(b, p[1]);
    bs_write_u8(b, p[0]);

    return 0;
}

int amf_write_string(bs_t *b, const char* string, size_t length)
{
    if (length > UINT32_MAX || b == NULL)
    {    
        ERR("args error");
        return -1;
    }

    if (bs_bytes_left(b) < 1 + (length < 65536 ? 2 : 4) + length)
    {
        ERR("args error");
        return -1;
    }

    if (length < 65536)
    {
        bs_write_u8(b, AMF_STRING);
        bs_write_u(b, 16, length);
        bs_write_bytes(b, (uint8_t *)string, length);
    } else 
    {
        bs_write_u8(b, AMF_LONG_STRING);
        bs_write_u(b, 32, length);
        bs_write_bytes(b, (uint8_t *)string, length);
    }

    return 0;
}

int amf_write_date(bs_t *b, double milliseconds, int16_t timezone)
{
    if (bs_bytes_left(b) < 11 || b == NULL)
    {
        ERR("args error");
        return -1;
    }

    amf_write_double(b, milliseconds);
    bs_write_u8(b, AMF_DATE);
    bs_write_u(b, 16, timezone);

    return 0;
}

int amf_write_NamedString(bs_t *b, const char* name, size_t length, const char* value, size_t length2)
{
    if (bs_bytes_left(b) < length + 2 + length2 + 3 || b == NULL)
    {
        ERR("args error");
        return -1;
    }

    bs_write_u(b, 16, length);
    bs_write_bytes(b, (uint8_t *)name, length);

    amf_write_string(b, value, length2);

    return 0;
}

int amf_write_NamedDouble(bs_t *b, const char* name, size_t length, double value)
{
    if (bs_bytes_left(b) < length + 2 + 8 + 1|| b == NULL)
    {
        ERR("args error");
        return -1;
    }

    bs_write_u(b, 16, length);
    bs_write_bytes(b, (uint8_t *)name, length);

    amf_write_double(b, value);

    return 0;
}

int amf_write_NamedBoolean(bs_t *b, const char* name, size_t length, uint8_t value)
{
    if (bs_bytes_left(b) < length + 2 + 2 || b == NULL)
    {
        ERR("args error");
        return -1;
    }

    bs_write_u(b, 16, length);
    bs_write_bytes(b, (uint8_t *)name, length);

    amf_write_boolean(b, value);

    return 0;
}

int amf_read_double(bs_t *b, double* value)
{
    if (bs_bytes_left(b) < 8 || value == NULL)
    {
        return -1;
    }

    uint8_t* p = (uint8_t*)value;
    p[7] = bs_read_u8(b);
    p[6] = bs_read_u8(b);
    p[5] = bs_read_u8(b);
    p[4] = bs_read_u8(b);
    p[3] = bs_read_u8(b);
    p[2] = bs_read_u8(b);
    p[1] = bs_read_u8(b);
    p[0] = bs_read_u8(b);
    return 8;
}

int amf_read_string(bs_t *b, char *string, int size)
{
    if (bs_bytes_left(b) < 1 || string == NULL)
    {   

        ERR("read string error %d", bs_bytes_left(b));
        return -1;
    }

    uint32_t str_size = bs_read_u(b, 16);
    if (str_size == 0)
        return 0;

    if (bs_bytes_left(b) < str_size)
    {
        ERR("bs bytes left %d, %d", str_size, bs_bytes_left(b));
        return -1;
    }
    
    uint32_t stip = 0;

    for (int i = 0; i < str_size; i++)
    {
        if (b->p[i] > 127 || b->p[i] < 0)
        {
            memmove(b->p + i, b->p + i + 1, str_size - i);
            stip++;
        }
    }

    bs_read_string(b, str_size, string, size);

    if (stip > 0)
    {
        bs_read_u(b, 8 * stip);
    }
    return str_size;
}

int amf_read_long_string(bs_t *b, char *string, int size)
{
    if (bs_bytes_left(b) < 4 || string == NULL)
    {
        return -1;
    }

    uint32_t str_size = bs_read_u(b, 32);
    if (bs_bytes_left(b) < str_size)
    {
        return -1;
    }

    uint32_t stip = 0;

    for (int i = 0; i < str_size; i++)
    {
        if (b->p[i] > 127 || b->p[i] < 0)
        {
            memmove(b->p + i, b->p + i + 1, str_size - i);
            stip++;
        }
    }

    bs_read_string(b, str_size, string, size);

    if (stip > 0)
    {
        bs_read_u(b, 8 * stip);
    }

    return str_size;
}

int amf_read_boolean(bs_t *b, uint8_t *value)
{
    if (b == NULL || value == NULL || bs_bytes_left(b) < 1)
    {
        ERR("%p,%p,%d",b, value, bs_bytes_left(b));
        return -1;
    }

    *value = bs_read_u8(b);
    return 1;
}

int amf_read_date(bs_t *b, double *milliseconds, int16_t *timezone)
{
    if (b == NULL || milliseconds == NULL 
        || timezone == NULL || bs_bytes_left(b) < 10)
        return -1;

    int size = amf_read_double(b, (double *)milliseconds);

    *timezone = bs_read_u(b, 16);

    return size + 2;
}

int amf_read_ecma_array(bs_t *b, amf_object_item* items, size_t n)
{
    if (b == NULL || items == NULL || bs_bytes_left(b) < 4)
        return -1;

    bs_read_u(b, 32); 

    return amf_read_object(b, items, n);
}

int amf_read_strict_array(bs_t *b, amf_object_item* items, size_t n)
{
    if (b == NULL || items == NULL || bs_bytes_left(b) < 4)
        return -1; 

    uint32_t count = bs_read_u(b, 32);
    for (uint32_t i = 0; i < count && bs_bytes_left(b) > 1; i++)
    {
        //i < n
        amf_read_object_item(b, bs_read_u8(b), &items[i]);  
    }

    return bs_bytes_left(b);
}

int amf_read_null(bs_t *b, uint8_t *value)
{
    if (bs_bytes_left(b) < 1 || b == NULL || value == NULL)
        return -1;

    *value = bs_read_u8(b);

    return 0;
}

int amf_read_object(bs_t *b, amf_object_item* items, size_t n)
{
    if (b == NULL || items == NULL)
        return -1;

    while (bs_bytes_left(b) > 2)
    {
        char string[64] = {0};
        int str_size = amf_read_string(b, string, sizeof(string));
        
        int i = 0;
        for (i = 0; i < n; i++)
        {
            if (str_size == strlen(items[i].name) 
                && 0 == memcmp(string, items[i].name, str_size))
            {
                break;
            }
        }
  
        if (-1 == amf_read_object_item(b, bs_read_u8(b), &items[i]))
        {
            ERR("invalid object");
            break;
        }
    
        if (bs_read_ru(b, 24) == AMF_OBJECT_END)
        {
            LOG("AMF_OBJECT_END");
            break;
        }
    }

    return bs_bytes_left(b); 
}

int amf_read_object_item(bs_t *b, AMFDataType type, amf_object_item *item)
{
    if (b == NULL || bs_bytes_left(b) <= 1)
        return -1;

    int code = -1;
    switch (type)
	{
        case AMF_BOOLEAN:
            code = amf_read_boolean(b, (uint8_t *)item->value);
            break;
        case AMF_NUMBER: 
            code = amf_read_double(b, (double *)item->value);
            break;
        case AMF_STRING:
            code = amf_read_string(b, (char *)item->value, item->size);
            break;
        case AMF_LONG_STRING:
            code = amf_read_long_string(b, (char *)item->value, item->size);
            break;
        case AMF_DATE:
            code = amf_read_date(b, (double *)item->value, (int16_t *)(item->value + sizeof(double)));
            break;
        case AMF_OBJECT:
            code = amf_read_object(b, (amf_object_item* )item->value, item->size);
            break;
        case AMF_NULL: 
            //code = amf_read_null(b, (uint8_t *)item->value);
            break;
        case AMF_UNDEFINED:
            return -1;
            break;
        case AMF_ECMA_ARRAY:
            code = amf_read_ecma_array(b, (amf_object_item* )item->value, item->size);
            break;
        case AMF_STRICT_ARRAY:
            code = amf_read_strict_array(b, (amf_object_item* )item->value, item->size);
            break;
        default:
            //assert(0);
            return -1;
	}

    return code; 
}

int amf_read_item(bs_t *b, amf_object_item *item, int size)
{
    if (b == NULL || bs_bytes_left(b) <= 1)
        return NET_FAIL;

    while (size > 0)
    {
        amf_read_object_item(b, bs_read_u8(b), item);
        size--;
        item++;
    }
       
    return NET_SUCCESS; 
}


void readAMFItem(bs_t *b, amf_object_item *item)
{
    AMFDataType type = bs_read_u8(b);
    switch (type)
	{
        case AMF_BOOLEAN:
            amf_read_boolean(b, (uint8_t *)item->value);
            break;
        case AMF_NUMBER: 
            amf_read_double(b, (double *)item->value);
            break;
        case AMF_STRING:
            amf_read_string(b, (char *)item->value, item->size);
            break;
        case AMF_LONG_STRING:
            amf_read_long_string(b, (char *)item->value, item->size);
            break;
        case AMF_DATE:
            amf_read_date(b, (double *)item->value, (int16_t *)(item->value + sizeof(double)));
            break;
        case AMF_OBJECT:
            amf_read_object(b, (amf_object_item* )item->value, item->size);
            break;
        case AMF_NULL: 
            //code = amf_read_null(b, (uint8_t *)item->value);
            break;
        case AMF_UNDEFINED:
            return -1;
            break;
        case AMF_ECMA_ARRAY:
            amf_read_ecma_array(b, (amf_object_item* )item->value, item->size);
            break;
        case AMF_STRICT_ARRAY:
            amf_read_strict_array(b, (amf_object_item* )item->value, item->size);
            break;
        default:
            //assert(0);
            return -1;
	}
}