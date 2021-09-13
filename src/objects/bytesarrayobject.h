#ifndef __BYTESARRAYOBJECT_H__
#define __BYTESARRAYOBJECT_H__

#include "object.h"

typedef struct _bytearrayobject
{
	ViObject_VAR_HEAD
	Vi_int8_t* ob_bytes;	// Byte array
	size_t ob_alloc;		// Amount of bytes allocated in ob_bytes
} ViByteArrayObject;

/* Type object */
extern ViTypeObject ViByteArrayType;

/* Type check macros */
#define ViByteArray_Check(self) ViObject_TypeCheck(self, &ViByteArrayType)
#define ViByteArray_CheckExact(self) Vi_IS_TYPE(self, &ViByteArrayType)

/* Convert an array of bytes to a ViBytesArrayObject */
ViObject* ViByteArrayObject_FromString(const char* bytes, size_t size);

#endif // __BYTESARRAYOBJECT_H__