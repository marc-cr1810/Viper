#ifndef __STRINGOBJECT_H__
#define __STRINGOBJECT_H__

#include "object.h"

typedef struct _stringobject
{
	ViObject_VAR_HEAD;
	size_t ob_alloc;
	char *ob_svar;
} ViStringObject;

/* Type object */
extern ViTypeObject ViStringType;

/* Type check macros */
#define ViString_Check(self) ViObject_TypeCheck(self, &ViStringType)
#define ViString_CheckExact(self) Vi_IS_TYPE(self, &ViStringType)

/* Convert an array of bytes to a ViStringObject */
ViObject *ViStringObject_FromString(const char *bytes);
ViObject *ViStringObject_FromStringAndSize(const char *bytes, Vi_size_t size);

/* API Functions */
ViObject *ViString_Concat(ViObject *a, ViObject *b);
char *ViString_ToString(ViObject *str);

#endif // __STRINGOBJECT_H__