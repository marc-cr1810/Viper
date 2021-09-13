#include "listobject.h"

#include "../core/error.h"
#include "../core/vimem.h"

static int list_resize(ViListObject* self, Vi_size_t new_size)
{
	ViObject** items;
	size_t new_allocated, num_allocated_bytes;
	Vi_size_t allocated = self->allocated;

	if (allocated >= new_size && new_size >= (allocated >> 1))
	{
		assert(self->ob_items != NULL || new_size != 0);
		VAROBJECT_SET_SIZE(self, new_size);
		return 0;
	}

	/* This over-allocates proportional to the list size, making room
	 * for additional growth.  The over-allocation is mild, but is enough to give 
	 * linear-time amortized behavior over a long sequence of appends() in the 
	 * presence of a poorly-performing system realloc().
	 * Add padding to make the allocated size multiple of 4.
	 * The growth pattern is:  0, 4, 8, 16, 24, 32, 40, 52, 64, 76, ...
	 * Note: new_allocated won't overflow because the largest possible value
	 *       is VI_SIZE_T_MAX * (9 / 8) + 6 which always fits in a size_t.
	 */
	new_allocated = ((size_t)new_size + (new_size >> 3) + 6) & ~(size_t)3;
	/* Do not overallocate if the new size is closer to overallocated size
	 * than to the old size.
	 */
	if (new_size - Vi_SIZE(self) > (Vi_size_t)(new_allocated - new_size))
		new_allocated = ((size_t)new_size + 3) & ~(size_t)3;

	if (new_size == 0)
		new_allocated = 0;
	num_allocated_bytes = new_allocated * sizeof(ViObject*);
	items = (ViObject**)Mem_Realloc(self->ob_items, num_allocated_bytes);
	if (items == NULL)
	{
		ViError_SetString(ViExc_MemoryError, "out of memory");
		return -1;
	}
	self->ob_items = items;
	VAROBJECT_SET_SIZE(self, new_size);
	self->allocated = new_allocated;
	return 0;
}

static int append(ViListObject* self, ViObject* obj)
{
	Vi_size_t n = ViList_GET_SIZE(self);

	assert(obj != NULL);
	assert((size_t)n + 1 < VI_SIZE_T_MAX);
	if (list_resize(self, n + 1) < 0)
		return -1;

	ViObject_INCREF(obj);
	ViList_SET_ITEM(self, n, obj);
	return 0;
}

static inline int valid_index(Vi_size_t i, Vi_size_t limit)
{
	return (size_t)i < (size_t)limit;
}

//
//
//		Methods
//
//

static void list_dealloc(ViListObject *self)
{
	Vi_size_t i;
	// Untrack list for GC
	// begin trashcan
	if (self->ob_items != NULL)
	{
		i = Vi_SIZE(self);
		while (--i >= 0)
		{
			ViObject_DECREF(self->ob_items[i]);
		}
		Mem_Free(self->ob_items);
	}
	Vi_TYPE(self)->tp_free((ViObject *)self);

	// end trashcan
}

/* Sequence methods */

static Vi_size_t list_length(ViListObject* list)
{
	return Vi_SIZE(list);
}

static ViObject* list_concat(ViListObject* list, ViObject* obj)
{
	Vi_size_t size;
	Vi_size_t i;
	ViObject** src, ** dest;
	ViListObject* new_list;
	if (!ViList_Check(obj))
	{
		ViError_SetString(ViExc_TypeError, "can only concatenate list to list");
		return NULL;
	}

#define other_list ((ViListObject*)obj)
	assert((size_t)Vi_SIZE(list) + (size_t)Vi_SIZE(other_list) < VI_SIZE_T_MAX);
	size = Vi_SIZE(list) + Vi_SIZE(other_list);
	new_list = (ViListObject*)ViListObject_New(size);
	if (new_list == NULL)
		return NULL;

	src = list->ob_items;
	dest = new_list->ob_items;
	for (i = 0; i < Vi_SIZE(list); i++)
	{
		ViObject* ob = src[i];
		ViObject_INCREF(ob);
		dest[i] = ob;
	}

	src = other_list->ob_items;
	dest = new_list->ob_items + Vi_SIZE(list);
	for (i = 0; i < Vi_SIZE(other_list); i++)
	{
		ViObject* ob = src[i];
		ViObject_INCREF(ob);
		dest[i] = ob;
	}
	VAROBJECT_SET_SIZE(new_list, size);
	return (ViObject*)new_list;
#undef other_list
}

static ViObject* list_item(ViListObject* list, Vi_size_t i)
{
	if (valid_index(i, Vi_SIZE(list)))
	{
		ViError_SetString(ViExc_IndexError, "list index out of range");
		return NULL;
	}
	ViObject_INCREF(list->ob_items[i]);
	return list->ob_items[i];
}

static ViSequenceMethods list_sequence_methods = {
	(lenfunc)list_length,		// sq_length
	(binaryfunc)list_concat,	// sq_concat
	0,	// sq_repeat
	(sizeargfunc)list_item,		// sq_item
	0,	// sq_slice
	0,	// sq_assign_item
	0,	// sq_assign_slice
	0,	// sq_contains
	0,	// sq_inplace_concat
	0,	// sq_inplace_repeat
};

ViTypeObject ViListType = {
	VAROBJECT_HEAD_INIT(&ViListType, 0)		// base
	"list",									// tp_name
	"List object type",						// tp_doc
	sizeof(ViListObject),					// tp_size
	0,										// tp_itemsize
	TPFLAGS_DEFAULT | TPFLAGS_BASETYPE |	// tp_flags
		TPFLAGS_LIST_SUBCLASS,
	(destructor)list_dealloc,				// tp_dealloc
	0,										// tp_number_methods
	&list_sequence_methods,					// tp_sequence_methods
	0,										// tp_clear
	&ViBaseObjectType,						// tp_base
	0,										// tp_dict
	0,										// tp_new
	Mem_Free								// tp_free
};

ViObject* ViListObject_New(Vi_size_t size)
{
	ViListObject* obj;
	size_t alloc;

	obj = ViObject_NEW(ViListObject, &ViListType);
	if (obj == NULL)
		return NULL;
	if (size == 0)
	{
		obj->ob_items = NULL;
		alloc = 0;
	}
	else
	{
		alloc = size * sizeof(ViObject*);
		obj->ob_items = (ViObject**)Mem_Alloc(alloc);
	}
	VAROBJECT_SET_SIZE(obj, size);
	obj->allocated = size;
	return (ViObject*)obj;
}

int ViList_Append(ViObject* list, ViObject* new_item)
{
	if (ViList_Check(list) && (new_item != NULL))
		return append((ViListObject*)list, new_item);
	ViError_SetString(ViExc_SystemError, "Bad internal call");
	return -1;
}

int ViList_SetItem(ViObject *list, Vi_size_t i, ViObject *newitem)
{
	ViObject **p;
	if (ViList_Check(list))
	{
		ViObject_XDECREF(newitem);
		ViError_BadInternalCall();
		return -1;
	}
	if (!valid_index(i, Vi_SIZE(list)))
	{
		ViObject_XDECREF(newitem);
		ViError_SetString(ViExc_IndexError, "list assignment index out of range");
		return -1;
	}
	p = ((ViListObject *)list)->ob_items + i;
	ViObject_XSETREF(*p, newitem);
	return 0;
}
