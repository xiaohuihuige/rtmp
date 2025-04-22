#include "amf0.h"

//static double s_double = 1.0; // 3ff0 0000 0000 0000

typedef struct 
{
    int code;
    char error[126];
} stateMessage;

stateMessage amf_type_message[] = {
    {0, "AMF_NUMBER"},
    {1, "AMF_BOOLEAN"},
    {2, "AMF_STRING"},
    {3, "AMF_OBJECT"},
    {9, "AMF_OBJECT_END"},
    {5, "AMF_NULL"},
};

static inline void printfAMFMessage(char *message, int code)
{
    for (int i = 0; i < sizeof(amf_type_message)/sizeof(stateMessage); i++) {
        if (code == amf_type_message[i].code)
            LOG("%s %s", message, amf_type_message[i].error);
    }
}


void printfChar(uint8_t *data, int len)
{
    LOG("byte len %d", len);
    for (int i = 0; i < len; i++) {
        fprintf(stderr, "%02x ", data[i]);
        if ((i + 1) % 8 == 0 && i != 0)
            fprintf(stderr, "    ");

        if ((i+1) % 16 == 0 && i != 0)
            fprintf(stderr, " [%d]\n", i + 1);
    }
    fprintf(stderr, "\n");
}

//1 bytes
int amf_write_null(bs_t *b)
{
    if (!b) 
        return NET_FAIL;

    bs_write_u8(b, AMF_NULL);

    return 0;
}

//1 bytes
int amf_write_undefined(bs_t *b)
{
    if (!b) 
        return NET_FAIL;

    bs_write_u8(b, AMF_UNDEFINED);

    return 0;
}

//1 bytes
int amf_write_object(bs_t *b)
{
    if (!b) 
        return NET_FAIL;

    bs_write_u8(b, AMF_OBJECT);

    return 0;
}

//3 bytes
int amf_write_objectEnd(bs_t *b)
{
    if (!b) 
        return NET_FAIL;

    bs_write_u(b, 24, AMF_OBJECT_END);

    return 0;
}

//1 bytes
int amf_write_typedObject(bs_t *b)
{
    if (!b) 
        return NET_FAIL;

    bs_write_u8(b, AMF_TYPED_OBJECT);

    return 0;
}

//5 bytes
int amf_write_CMAArarry(bs_t *b)
{
    if (!b) 
        return NET_FAIL;

    bs_write_u8(b, AMF_ECMA_ARRAY);

    bs_write_u(b, 32, 0);

    return 0;
}

//2 bytes
int amf_write_boolean(bs_t *b, uint8_t value)
{
    if (!b) 
        return NET_FAIL;

    bs_write_u8(b, AMF_BOOLEAN);

    bs_write_u8(b, value == 0 ? 0 : 1);

    return 0;
}

//9 bytes
int amf_write_double(bs_t *b, double value)
{
    if (!b) 
        return NET_FAIL;

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

// 3 + length bytes
int amf_write_string(bs_t *b, const char* string, size_t length)
{
    if (!b || length > UINT32_MAX)
        return NET_FAIL;

    bs_write_u8(b, AMF_STRING);

    bs_write_u(b, 16, length);

    bs_write_bytes(b, (uint8_t *)string, length);

    return 0;
}

// 5 + length bytes
int amf_write_long_string(bs_t *b, const char* string, size_t length)
{
    if (!b || length > UINT32_MAX)
        return NET_FAIL;

    bs_write_u8(b, AMF_LONG_STRING);

    bs_write_u(b, 32, length);

    bs_write_bytes(b, (uint8_t *)string, length);
    
    return 0;
}

// 12 bytes
int amf_write_date(bs_t *b, double milliseconds, int16_t timezone)
{
    if (!b) 
        return NET_FAIL;

    amf_write_double(b, milliseconds);

    bs_write_u8(b, AMF_DATE);

    bs_write_u(b, 16, timezone);

    return 0;
}

// 5 + length + length2 bytes
int amf_write_NamedString(bs_t *b, const char* name, size_t length, const char* value, size_t length2)
{
    if (!b)
        return NET_FAIL;

    bs_write_u(b, 16, length);

    bs_write_bytes(b, (uint8_t *)name, length);

    amf_write_string(b, value, length2);

    return 0;

}

// 11 + length bytes
int amf_write_NamedDouble(bs_t *b, const char* name, size_t length, double value)
{
    if (!b)
        return NET_FAIL;

    bs_write_u(b, 16, length);

    bs_write_bytes(b, (uint8_t *)name, length);

    amf_write_double(b, value);

    return 0;
}

// 4 + length bytes
int amf_write_NamedBoolean(bs_t *b, const char* name, size_t length, uint8_t value)
{
    if (!b)
        return NET_FAIL;

    bs_write_u(b, 16, length);
    
    bs_write_bytes(b, (uint8_t *)name, length);

    amf_write_boolean(b, value);

    return 0;
}

int amf_read_double(bs_t *b, double* value)
{
    if (!b || !value)
        return -1;
  
    uint8_t* p = (uint8_t*)value;
    p[7] = bs_read_u8(b);
    p[6] = bs_read_u8(b);
    p[5] = bs_read_u8(b);
    p[4] = bs_read_u8(b);
    p[3] = bs_read_u8(b);
    p[2] = bs_read_u8(b);
    p[1] = bs_read_u8(b);
    p[0] = bs_read_u8(b);

    DBG("read double %f", *value);

    return 8;
}

int amf_read_string(bs_t *b, char *string, int size)
{
    if (!b)
        return -1;
    
    uint32_t str_size = bs_read_u(b, 16);

    if (bs_bytes_left(b) < str_size)
        return -1;

    if (string && str_size != 0) {
        bs_read_string(b, str_size, string, size);
    	DBG("read string %s, %u", string, str_size);
    }

    return str_size;
}

int amf_read_long_string(bs_t *b, char *string, int size)
{
    if (!b)
        return -1;

    uint32_t str_size = bs_read_u(b, 32);

    if (bs_bytes_left(b) < str_size)
        return -1;

    if (string) {
        bs_read_string(b, str_size, string, size);
	    DBG("read string %s, %d", string, str_size);
    }

    return str_size;
}

int amf_read_boolean(bs_t *b, uint8_t *value)
{
    if (!b || !value)
        return -1;
    
    *value = bs_read_u8(b);

    DBG("read boolean %d", *value);

    return *value;
}

int amf_read_date(bs_t *b, double *milliseconds, int16_t *timezone)
{
    if (!b || milliseconds || timezone)
        return -1;

    int size = amf_read_double(b, (double *)milliseconds);

    *timezone = bs_read_u(b, 16);

    return size + 2;
}

int amf_read_ecma_array(bs_t *b, amf_object_item* items, size_t n)
{
    if (!b || !items)
        return -1;

    bs_read_u(b, 32); 

    return amf_read_object(b, items, n);
}

int amf_read_strict_array(bs_t *b, amf_object_item* items, size_t n)
{
    if (!b || !items)
        return -1;

    uint32_t count = bs_read_u(b, 32);
    for (uint32_t i = 0; i < count && bs_bytes_left(b) > 1; i++) {

        amf_read_object_item(b, &items[i]);  
    }

    return bs_bytes_left(b);
}

int amf_read_null(bs_t *b, uint8_t *value)
{
    if (!b || !value)
        return -1;

    *value = bs_read_u8(b);

    return 1;
}

amf_object_item *_findObjectItem(amf_object_item* items, size_t n, char *string, size_t string_len)
{
    for (int i = 0; i < n; i++) {
        //LOG("compare %d, %s ; %d, %s", string_len, string, strlen(items[i].name), items[i].name);
        if (string_len == strlen(items[i].name) 
            && !memcmp(string, items[i].name, string_len))
            return &items[i];
    }

    return NULL;
}

int amf_read_object(bs_t *b, amf_object_item* items, size_t n)
{
    if (!b || !items)
        return -1;

    while (!bs_eof(b)) {
        char string[64] = {0};
        int string_len = amf_read_string(b, string, sizeof(string));
        if (string_len < 0 || 0 == strlen(string)) {
            break;
        }

        amf_object_item * find_item = _findObjectItem(items, n, string, string_len);
        if (!find_item)
            break;

        amf_read_object_item(b,  find_item);
        
        if (bs_read_ru(b, 24) == AMF_OBJECT_END)
            break;
    }

    return bs_bytes_left(b); 
}

int amf_read_object_item(bs_t *b, amf_object_item *item)
{
    if (!b ||!item)
        return -1;

    if (bs_bytes_left(b) <= 1)
        return -1;
        
    int object_type = bs_read_u8(b);

    printfAMFMessage("amf type :", object_type);

    switch (object_type)
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
            amf_read_null(b, (uint8_t *)item->value);
            break;
        case AMF_UNDEFINED:
            break;
        case AMF_ECMA_ARRAY:
            amf_read_ecma_array(b, (amf_object_item* )item->value, item->size);
            break;
        case AMF_STRICT_ARRAY:
            amf_read_strict_array(b, (amf_object_item* )item->value, item->size);
            break;
        default:
            break;
	}

    return 0; 
}

int amf_read_item(bs_t *b, amf_object_item *item, int size)
{
    if (b == NULL || bs_bytes_left(b) <= 1)
        return NET_FAIL;

    while (size > 0)
    {
        amf_read_object_item(b, item);
        size--;
        item++;
    }
       
    return NET_SUCCESS; 
}
