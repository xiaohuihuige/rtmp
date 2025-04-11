#ifndef _amf0_h_
#define _amf0_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <schedule/net-common.h>

#define AMF_OBJECT_ITEM_VALUE(v, amf_type, amf_name, amf_value, amf_size) {v.type=amf_type; v.name=amf_name; v.value=amf_value; v.size=amf_size;}

enum rtmp_encoding_amf_t
{
	RTMP_ENCODING_AMF_0		= 0,
	RTMP_ENCODING_AMF_3		= 3,
};

typedef enum 
{
	AMF_NUMBER = 0x00,
	AMF_BOOLEAN,
	AMF_STRING,
	AMF_OBJECT,
	AMF_MOVIECLIP,
	AMF_NULL,
	AMF_UNDEFINED,
	AMF_REFERENCE,
	AMF_ECMA_ARRAY,
	AMF_OBJECT_END,
	AMF_STRICT_ARRAY,
	AMF_DATE,
	AMF_LONG_STRING,
	AMF_UNSUPPORTED,
	AMF_RECORDSET,
	AMF_XML_DOCUMENT,
	AMF_TYPED_OBJECT,
	AMF_AVMPLUS_OBJECT,
} AMFDataType;

typedef struct
{
	AMFDataType type;
	const char* name;
	void* value;
	size_t size;
} amf_object_item;


int amf_write_null(bs_t *b);
int amf_write_undefined(bs_t *b);
int amf_write_object(bs_t *b);
int amf_write_objectEnd(bs_t *b);
int amf_write_typedObject(bs_t *b);
int amf_write_CMAArarry(bs_t *b);

int amf_write_boolean(bs_t *b, uint8_t value);
int amf_write_double(bs_t *b, double value);
int amf_write_string(bs_t *b, const char* string, size_t length);
int amf_write_date(bs_t *b, double milliseconds, int16_t timezone);

int amf_write_NamedString(bs_t *b, const char* name, size_t length, const char* value, size_t length2);
int amf_write_NamedDouble(bs_t *b, const char* name, size_t length, double value);
int amf_write_NamedBoolean(bs_t *b, const char* name, size_t length, uint8_t value);

int amf_read_null(bs_t *b, uint8_t *value);
int amf_read_double(bs_t *b, double* value);
int amf_read_string(bs_t *b, char *string, int size);
void amf_read_object_item(bs_t *b, amf_object_item *item);
int amf_read_object(bs_t *b, amf_object_item* items, size_t n);
int amf_read_item(bs_t *b, amf_object_item *item, int size);

#ifdef __cplusplus
}
#endif

#endif /* !_amf0_h_ */
