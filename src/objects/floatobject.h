#ifndef __FLOATOBJECT_H__
#define __FLOATOBJECT_H__

#include "object.h"

typedef struct _floatobject
{
	ViObject_HEAD
	double ob_fval;
} ViFloatObject;

extern ViTypeObject ViFloatType;

/* Convert a C++ double to a ViFloatObject */
ViObject* ViFloatObject_FromDouble(double dval);

#endif // __FLOATOBJECT_H__