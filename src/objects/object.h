#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "../port.h"
#include "../core/vimem.h"

typedef struct _viidentifier
{
    const char* string;
    Vi_size_t index;
} ViIdentifier;

#define Vi_SATIC_STRING_INIT(value) { value, -1 }
#define Vi_STATIC_STRING(varname, value) static ViIdentifier varname = Vi_SATIC_STRING_INIT(value)
#define Vi_IDENTIFIER(varname) Vi_STATIC_STRING(ViId_##varname, #varname)

struct _typeobject;

/* ViObject_HEAD defines the initial segment of every ViObject. */
#define ViObject_HEAD ViObject ob_base;

#define ViObject_HEAD_INIT(type) \
	{1, type},

#define VAROBJECT_HEAD_INIT(type, size) \
	{ViObject_HEAD_INIT(type) size},

/* OBJECT_VAR_HEAD defines the initial segment of all variable-size
 * container objects.  These end with a declaration of an array with 1
 * element, but enough space is malloc'ed so that the array actually
 * has room for ob_size elements.  Note that ob_size is an element count,
 * not necessarily a byte count.
 */
#define ViObject_VAR_HEAD ViVarObject ob_base;

typedef struct _object
{
	intptr_t ob_refcount;
	_typeobject* ob_type;
} ViObject;

/* Cast argument to ViObject* type. */
#define ViObject_CAST(op) ((ViObject*)(op))
#define ViObject_CAST_CONST(op) ((const ViObject*)(op))

typedef struct _varobject
{
	ViObject ob_base;
	intptr_t ob_size;
} ViVarObject;

/* Cast argument to ViVarObject* type. */
#define ViVarObject_CAST(op) ((ViVarObject*)(op))
#define ViVarObject_CAST_CONST(op) ((const ViVarObject*)(op))

/*
Type objects contain a string containing the type name (to help somewhat
in debugging), the allocation parameters (see Object_New() and
Object_NewVar()),
and methods for accessing objects of the type.  Methods are optional, a
nil pointer meaning that particular kind of access is not available for
this type.  The DECREF() macro uses the tp_dealloc method without
checking for a nil pointer; it should always be implemented except if
the implementation can guarantee that the reference count will never
reach zero (e.g., for statically allocated type objects).

NB: the methods for certain type groups are now contained in separate
method blocks.
*/

typedef ViObject* (*unaryfunc)(ViObject*);
typedef ViObject* (*binaryfunc)(ViObject*, ViObject*);
typedef ViObject* (*ternaryfunc)(ViObject*, ViObject*, ViObject*);
typedef int (*inquiry)(ViObject*);
typedef Vi_size_t(*lenfunc)(ViObject*);
typedef ViObject* (*sizeargfunc)(ViObject*, Vi_size_t);
typedef ViObject* (*sizesizeargfunc)(ViObject*, Vi_size_t, Vi_size_t);
typedef int(*sizeobjargproc)(ViObject*, Vi_size_t, ViObject*);
typedef int(*sizesizeobjargproc)(ViObject*, Vi_size_t, Vi_size_t, ViObject*);
typedef int(*objobjargproc)(ViObject*, ViObject*, ViObject*);

typedef int (*objobjproc)(ViObject*, ViObject*);
typedef int (*visitproc)(ViObject*, void*);
typedef int (*traverseproc)(ViObject*, visitproc, void*);

typedef void (*freefunc)(void*);
typedef void (*destructor)(ViObject*);
typedef ViObject* (*getattrfunc)(ViObject*, char*);
typedef ViObject* (*getattrofunc)(ViObject*, ViObject*);
typedef int (*setattrfunc)(ViObject*, char*, ViObject*);
typedef int (*setattrofunc)(ViObject*, ViObject*, ViObject*);
typedef ViObject* (*reprfunc)(ViObject*);
typedef Vi_hash_t(*hashfunc)(ViObject*);
typedef ViObject* (*richcmpfunc) (ViObject*, ViObject*, int);
typedef ViObject* (*getiterfunc) (ViObject*);
typedef ViObject* (*iternextfunc) (ViObject*);
typedef ViObject* (*descrgetfunc) (ViObject*, ViObject*, ViObject*);
typedef int (*descrsetfunc) (ViObject*, ViObject*, ViObject*);
typedef int (*initproc)(ViObject*, ViObject*, ViObject*);
typedef ViObject* (*newfunc)(_typeobject*, ViObject*, ViObject*);
typedef ViObject* (*allocfunc)(_typeobject*, Vi_size_t);

typedef void (*freefunc)(void*);

typedef struct {
    /* Number implementations must check *both*
       arguments for proper type and implement the necessary conversions
       in the slot functions themselves. */

    binaryfunc nb_add;
    binaryfunc nb_subtract;
    binaryfunc nb_multiply;
    binaryfunc nb_remainder;
    binaryfunc nb_divmod;
    ternaryfunc nb_power;
    unaryfunc nb_negative;
    unaryfunc nb_positive;
    unaryfunc nb_absolute;
    inquiry nb_bool;
    unaryfunc nb_invert;
    binaryfunc nb_lshift;
    binaryfunc nb_rshift;
    binaryfunc nb_and;
    binaryfunc nb_xor;
    binaryfunc nb_or;
    unaryfunc nb_int;
    unaryfunc nb_float;

    binaryfunc nb_inplace_add;
    binaryfunc nb_inplace_subtract;
    binaryfunc nb_inplace_multiply;
    binaryfunc nb_inplace_remainder;
    ternaryfunc nb_inplace_power;
    binaryfunc nb_inplace_lshift;
    binaryfunc nb_inplace_rshift;
    binaryfunc nb_inplace_and;
    binaryfunc nb_inplace_xor;
    binaryfunc nb_inplace_or;

    binaryfunc nb_floor_divide;
    binaryfunc nb_true_divide;
    binaryfunc nb_inplace_floor_divide;
    binaryfunc nb_inplace_true_divide;

    unaryfunc nb_index;

    binaryfunc nb_matrix_multiply;
    binaryfunc nb_inplace_matrix_multiply;
} ViNumberMethods;

typedef struct {
    lenfunc sq_length;
    binaryfunc sq_concat;
    sizeargfunc sq_repeat;
    sizeargfunc sq_item;
    void* was_sq_slice;
    sizeobjargproc sq_assign_item;
    void* was_sq_assign_slice;
    objobjproc sq_contains;

    binaryfunc sq_inplace_concat;
    sizeargfunc sq_inplace_repeat;
} ViSequenceMethods;

typedef struct _typeobject
{
	ViObject_VAR_HEAD
	const char* tp_name;		 // ViObject type name
	const char* tp_doc;			 // Documentation string
    size_t tp_size;
    size_t tp_itemsize; // For memory allocation
	unsigned long tp_flags;		 // Flags for specific features and behaviour

    /* Methods to implement standard operations */
    destructor tp_dealloc;

    /* Methods for standard classes */
    ViNumberMethods* tp_number_methods;
    ViSequenceMethods* tp_sequence_methods;

    inquiry tp_clear; // Delete references to contained objects

    // Strong reference on a heap type, borrowed reference on a static type
    struct _typeobject *tp_base;
    ViObject *tp_dict;
    newfunc tp_new;
	freefunc tp_free; // Low-level free memory routine
} ViTypeObject;

#define Vi_TYPE(ob)             (ViObject_CAST(ob)->ob_type)

#define Vi_SIZE(ob)             (ViVarObject_CAST(ob)->ob_size)

static inline int _Vi_IS_TYPE(const ViObject* ob, const ViTypeObject* type) {
	return Vi_TYPE(ob) == type;
}
#define Vi_IS_TYPE(ob, type) _Vi_IS_TYPE(ViObject_CAST_CONST(ob), type)

/* Generic type check */
int ViType_IsSubtype(ViTypeObject* a, ViTypeObject* b);
#define ViObject_TypeCheck(ob, tp) \
    (Vi_IS_TYPE(ob, tp) || ViType_IsSubtype(Vi_TYPE(ob), (tp)))

extern ViTypeObject ViBaseType; // built-in 'type'
extern ViTypeObject ViBaseObjectType; // built-in 'object'

int ViType_Ready(ViTypeObject *type);

/* Create a new reference to an object */
void ObjectNewRef(ViObject* obj);

static inline void ObjectSetType(ViObject* obj, ViTypeObject* type)
{
	obj->ob_type = type;
}
#define ViObject_SET_TYPE(obj, type) ObjectSetType(ViObject_CAST(obj), type)

static inline void VarObjectSetSize(ViVarObject* obj, intptr_t size)
{
	obj->ob_size = size;
}
#define VAROBJECT_SET_SIZE(obj, size) VarObjectSetSize(ViVarObject_CAST(obj), size)

static inline void ObjectInit(ViObject* obj, ViTypeObject* type)
{
	ObjectSetType(obj, type);
	ObjectNewRef(obj);
}

/*
 * ViObject reference count modification
 *
 * Objects keep track of the amount of references to them they have.
 * If an objects reference count reaches 0, it is free'd from memory.
*/

void ObjectDealloc(ViObject *obj);
#define ViObject_DEALLOC(obj) ObjectDealloc(obj)

/* Increase object reference count */
static inline void ObjectIncRef(ViObject* obj)
{
	obj->ob_refcount++;
}
#define ViObject_INCREF(obj) ObjectIncRef(ViObject_CAST(obj))

/* Decrease object reference count */
static inline void ObjectDecRef(ViObject* obj)
{
	obj->ob_refcount--;
	if (obj->ob_refcount == 0)
		// TODO: Change to dealloc
		obj->ob_type->tp_free(obj);
}
#define ViObject_DECREF(obj) ObjectDecRef(ViObject_CAST(obj))

/* Increase object reference count incase object pointer can be NULL */
static inline void ObjectXIncRef(ViObject* obj)
{
	if (obj != NULL)
		ObjectIncRef(obj);
}
#define ViObject_XINCREF(obj) ObjectXIncRef(ViObject_CAST(obj))

/* Decrease object reference count incase object pointer can be NULL */
static inline void ObjectXDecRef(ViObject* obj)
{
	if (obj != NULL)
		ObjectDecRef(obj);
}
#define ViObject_XDECREF(obj) ObjectXDecRef(ViObject_CAST(obj))

// Create a new strong reference to an object:
// increment the reference count of the object and return the object.
static ViObject *Object_NewRef(ViObject *obj);
#define ViObject_NEWREF(obj) Object_NewRef(ViObject_CAST(obj));

// Similar to Py_NewRef(), but the object can be NULL.
static ViObject *Object_XNewRef(ViObject *obj);
#define ViObject_XNEWREF(obj) Object_XNewRef(ViObject_CAST(obj));

/* Create a new object */
ViObject* Object_New(ViTypeObject* type);
#define ViObject_NEW(type, typedef) (type *)Object_New(typedef)

#define ViObject_CLEAR(obj)                     \
    do {                                        \
        ViObject *vi_tmp = ViObject_CAST(obj);  \
        if (vi_tmp != NULL) {                   \
            (obj) = NULL;                       \
            ViObject_DECREF(vi_tmp);              \
        }                                       \
    } while (0)

/* Safely decref `obj` and set `obj` to `obj2`.
 *
 * As in case of ViObject_CLEAR "the obvious" code can be deadly:
 *
 *     OBJECT_DECREF(obj);
 *     obj = obj2;
 *
 * The safe way is:
 *
 *      PiObject_SETREF(obj, obj2);
 *
 * That arranges to set `obj` to `obj2` before decref'ing, so that any code
 * triggered as a side-effect of `obj` getting torn down no longer believes
 * `op` points to a valid object.
 *
 * ViObject_XSETREF is a variant of ViObject_SETREF that uses ViObject_XDECREF instead of
 * Py_DECREF.
 */

#define ViObject_SETREF(obj, obj2)              \
    do {                                        \
        ViObject *vi_tmp = ViObject_CAST(obj);  \
        (obj) = (obj2);                         \
        ViObject_DECREF(vi_tmp);                  \
    } while (0)
#define ViObject_XSETREF(obj, obj2)             \
    do {                                        \
        ViObject *vi_tmp = ViObject_CAST(obj);  \
        (obj) = (obj2);                         \
        ViObject_XDECREF(vi_tmp);                 \
    } while (0)

 /*
 ViNullStruct is an object of undefined type which can be used in contexts
 where NULL (nil) is not suitable (since NULL often means 'error').

 Don't forget to apply ViObject_NEWREF() when returning this value!!!
 */
extern ViObject ViNullStruct; // Do not use directly
#define Vi_Null (&ViNullStruct)

/* Macro for returning Vi_Null from a function */
#define Vi_RETURN_NULL return ViObject_NEWREF(VI_Null)

/*
 *	Type object flags
*/

/* Set if the type object is dynamically allocated */
#define TPFLAGS_HEAPTYPE (1UL << 9)

/* Set if the type allows subclassing */
#define TPFLAGS_BASETYPE (1UL << 10)

/* Set if the type is 'ready' -- fully initialized */
#define TPFLAGS_READY (1UL << 12)

/* Set while the type is being 'readied', to prevent recursive ready calls */
#define TPFLAGS_READYING (1UL << 13)

/* Objects support garbage collection */
#define TPFLAGS_HAVE_GC (1UL << 14)

/* These flags are used to determine if a type is a subclass. */
#define TPFLAGS_LONG_SUBCLASS        (1UL << 24)
#define TPFLAGS_LIST_SUBCLASS        (1UL << 25)
#define TPFLAGS_TUPLE_SUBCLASS       (1UL << 26)
#define TPFLAGS_BYTES_SUBCLASS       (1UL << 27)
#define TPFLAGS_UNICODE_SUBCLASS     (1UL << 28)
#define TPFLAGS_DICT_SUBCLASS        (1UL << 29)
#define TPFLAGS_BASE_EXC_SUBCLASS    (1UL << 30)
#define TPFLAGS_TYPE_SUBCLASS        (1UL << 31)

#define TPFLAGS_DEFAULT 0

static inline int ViType_HasFeature(ViTypeObject* type, unsigned long feature)
{
    return ((type->tp_flags & feature) != 0);
}

#define ViType_FastSubClass(type, flag) ViType_HasFeature(type, flag)

static inline int Type_Check(ViObject *obj)
{
    return ViType_FastSubClass(Vi_TYPE(obj), TPFLAGS_TYPE_SUBCLASS);
}
#define ViType_Check(obj) Type_Check(ViObject_CAST(obj))

static inline int Type_CheckExact(ViObject *obj)
{
    return Vi_IS_TYPE(obj, &ViBaseType);
}
#define ViType_CheckExact(obj) Type_CheckExact(ViObject_CAST(obj))

#endif // __OBJECT_H__