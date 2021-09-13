#ifndef __TUPLEOBJECT_H__
#define __TUPLEOBJECT_H__

#include "object.h"

typedef struct _tupleobject
{
    ViObject_VAR_HEAD
    /* ob_item contains space for 'ob_size' elements.
        Items must normally not be NULL, except during construction when
        the tuple is not yet visible outside the function that builds it. */
    ViObject** ob_items;
} ViTupleObject;

/* Type object */
extern ViTypeObject ViTupleType;

/* Create a new tuple of a specified size */
ViObject* ViTupleObject_New(Vi_size_t size);

ViObject* ViTupleObject_FromArray(ViObject* const* src, Vi_size_t size);

/* Type check macros */
#define ViTuple_Check(self) ViObject_TypeCheck(self, &ViTupleType)
#define ViTuple_CheckExact(self) Vi_IS_TYPE(self, &ViTupleType)

/* Cast argument to ViTupleObject* type. */
#define ViTuple_CAST(obj) (assert(ViTuple_Check(obj)), ((ViTupleObject*)obj))

#define ViTuple_GET_SIZE(obj) Vi_SIZE(ViTuple_CAST(obj))

#define ViTuple_GET_ITEM(obj, i) (ViTuple_CAST(obj)->ob_items[i])

/* Macro, *only* to be used to fill in brand new tuples */
#define ViTuple_SET_ITEM(obj, i, v) ((void)(ViTuple_CAST(obj)->ob_items[i] = v))

#endif // __TUPLEOBJECT_H__