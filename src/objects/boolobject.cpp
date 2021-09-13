#include "boolobject.h"

#include "intobject.h"
#include "stringobject.h"

static ViObject *true_str = NULL;
static ViObject *false_str = NULL;

static void bool_dealloc(ViObject *self)
{
	Vi_TYPE(self)->tp_free(self);
}

static ViObject *bool_repr(ViObject *self)
{
	ViObject *s;

	if (self == Vi_True)
		s = true_str ? true_str : (true_str = ViStringObject_FromString("True"));
	else
		s = false_str ? false_str : (false_str = ViStringObject_FromString("False"));
	ViObject_XINCREF(s);
	return s;
}

static ViObject *bool_new(ViTypeObject *type, ViObject *args, ViObject *kwds)
{
	ViObject *x = Vi_False;

	// take in args

	return x;
}

/* Arithmetic operations redefined to return bool if both args are bool. */

static ViObject *bool_and(ViObject *a, ViObject *b)
{
	if (!ViBool_Check(a) || !ViBool_Check(b))
		return ViIntType.tp_number_methods->nb_and(a, b);
	return ViBool_FromLong((a == Vi_True) & (b == Vi_True));
}

static ViObject *bool_or(ViObject *a, ViObject *b)
{
	if (!ViBool_Check(a) || !ViBool_Check(b))
		return ViIntType.tp_number_methods->nb_or(a, b);
	return ViBool_FromLong((a == Vi_True) | (b == Vi_True));
}

static ViObject *bool_xor(ViObject *a, ViObject *b)
{
	if (!ViBool_Check(a) || !ViBool_Check(b))
		return ViIntType.tp_number_methods->nb_xor(a, b);
	return ViBool_FromLong((a == Vi_True) ^ (b == Vi_True));
}

static ViNumberMethods bool_as_number = {
    0,                          // nb_add
    0,                          // nb_subtract
    0,                          // nb_multiply
    0,                          // nb_remainder
    0,                          // nb_divmod
    0,                          // nb_power
    0,                          // nb_negative
    0,                          // nb_positive
    0,                          // nb_absolute
    0,                          // nb_bool
    0,                          // nb_invert
    0,                          // nb_lshift
    0,                          // nb_rshift
    bool_and,                   // nb_and
    bool_xor,                   // nb_xor
    bool_or,                    // nb_or
    0,                          // nb_int
    0,                          // nb_reserved
    0,                          // nb_float
    0,                          // nb_inplace_add
    0,                          // nb_inplace_subtract
    0,                          // nb_inplace_multiply
    0,                          // nb_inplace_remainder
    0,                          // nb_inplace_power
    0,                          // nb_inplace_lshift
    0,                          // nb_inplace_rshift
    0,                          // nb_inplace_and
    0,                          // nb_inplace_xor
    0,                          // nb_inplace_or
    0,                          // nb_floor_divide
    0,                          // nb_true_divide
    0,                          // nb_inplace_floor_divide
    0,                          // nb_inplace_true_divide
    0,                          // nb_index
};

/* The type object for bool.  Note that this cannot be subclassed! */

ViTypeObject ViBoolType = {
	VAROBJECT_HEAD_INIT(&ViBoolType, 0)		// base
	"bool",									// tp_name
	"Bool object type",						// tp_doc
	sizeof(ViObject),						// tp_size
	0,										// tp_itemsize
	TPFLAGS_DEFAULT | TPFLAGS_BASETYPE,		// tp_flags
	(destructor)bool_dealloc,				// tp_dealloc
	0,										// tp_number_methods
	0,										// tp_sequence_methods
	0,										// tp_clear
	0,										// tp_base
	0,										// tp_dict
	bool_new,								// tp_new
	Mem_Free								// tp_free
};

/* The objects representing bool values False and True */

ViIntObject ViFalseStruct = {
	VAROBJECT_HEAD_INIT(&ViBoolType, 0)
	{ 0 }
};

ViIntObject ViTrueStruct = {
	VAROBJECT_HEAD_INIT(&ViBoolType, 0)
	{ 1 }
};

ViObject *ViBool_FromLong(int b)
{
	ViObject *result;

	if (b)
		result = Vi_True;
	else
		result = Vi_False;
	ViObject_INCREF(result);
	return result;
}
