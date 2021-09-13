#ifndef __INTOBJECT_H__
#define __INTOBJECT_H__

#include "object.h"

typedef struct _intobject
{
	ViObject_VAR_HEAD
	Vi_int32_t ob_ival;
} ViIntObject;

/* Type object */
extern ViTypeObject ViIntType;

/* Type check macros */
#define ViInt_Check(self) ViObject_TypeCheck(self, &ViIntType)
#define ViInt_CheckExact(self) Vi_IS_TYPE(self, &ViIntType)

/* Cast argument to ViIntObject* type. */
#define ViInt_CAST(obj) (assert(ViInt_Check(obj)), ((ViTupleObject*)obj))

/* Convert a C++ int to a ViIntObject */
ViObject* ViIntObject_FromInt(Vi_int32_t ival);
ViObject *ViIntObject_FromString(const char *str, int base);

#endif // __INTOBJECT_H__