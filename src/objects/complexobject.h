#ifndef __COMPLEXOBJECT_H_
#define __COMPLEXOBJECT_H__

#include "object.h"

typedef struct _vicomplex
{
    double real;
    double imag;
} ViComplex;

typedef struct _complexobject
{
	ViObject_HEAD
	ViComplex ob_cval;
} ViComplexObject;

/* Type object */
extern ViTypeObject ViComplexType;

/* Type check macros */
#define ViComplex_Check(self) ViObject_TypeCheck(self, &ViComplexType)
#define ViComplex_CheckExact(self) Vi_IS_TYPE(self, &ViComplexType)

ViObject *ViComplexObject_FromComplex(ViComplex cval);
ViObject *ViComplexObject_FromDoubles(double real, double imag);

#endif // __COMPLEXOBJECT_H__