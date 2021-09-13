#ifndef __BOOLOBJECT_H__
#define __BOOLOBJECT_H__

#include "../port.h"
#include "object.h"

typedef struct _intobject ViIntObject;

/* Type object */
ViAPI_DATA(ViTypeObject) ViBoolType;

#define ViBool_Check(x) Vi_IS_TYPE(x, &ViBoolType)

/* Vi_True and ViFalse are the only two bools in existence
Dont forget to apply ViObject_INCREF() when returning either one! */

/* Do not use directly */
ViAPI_DATA(ViIntObject) ViTrueStruct, ViFalseStruct;

/* Use these macros */
#define Vi_True ((ViObject *) &ViTrueStruct)
#define Vi_False ((ViObject *) &ViFalseStruct)

/* Macros for returning Vi_True and Vi_False */
#define Vi_RETURN_TRUE return ViObject_NEWREF(Vi_True);
#define Vi_RETURN_FALSE return ViObject_NEWREF(Vi_False);

/* Function to return bool from a int */
ViAPI_FUNC(ViObject *)ViBool_FromLong(int b);

#endif // __BOOLOBJECT_H__