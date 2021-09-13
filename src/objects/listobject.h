#ifndef __LISTOBJECT_H__
#define __LISTOBJECT_H__

#include "object.h"

typedef struct _listobject
{
    ViObject_VAR_HEAD
    /* Vector of pointers to list elements.  list[0] is ob_item[0], etc. */
    ViObject** ob_items;
    /* ob_item contains space for 'allocated' elements.
     * The number currently in use is ob_size.
     * Invariants:
     *     0 <= ob_size <= allocated
     *     len(list) == ob_size
     *     ob_item == NULL implies ob_size == allocated == 0
     * list.sort() temporarily sets allocated to -1 to detect mutations.
     *
     * Items must normally not be NULL, except during construction when
     * the list is not yet visible outside the function that builds it.
     */
    Vi_size_t allocated;
} ViListObject;

/* Type object */
extern ViTypeObject ViListType;

/* Type check macros */
#define ViList_Check(self) ViObject_TypeCheck(self, &ViListType)
#define ViList_CheckExact(self) Vi_IS_TYPE(self, &ViListType)

ViObject* ViListObject_New(Vi_size_t size);

/* API Functions */
int ViList_Append(ViObject* list, ViObject* new_item);
int ViList_SetItem(ViObject *list, Vi_size_t i, ViObject *newitem);

#define ViList_CAST(obj) (assert(ViList_Check(obj)), ((ViListObject*)obj))

#define ViList_GET_ITEM(obj, i)     (ViList_CAST(obj)->ob_items[i])
#define ViList_SET_ITEM(obj, i, v)  ((void)(ViList_CAST(obj)->ob_items[i] = (v)))
#define ViList_GET_SIZE(obj)        Vi_SIZE(ViList_CAST(obj))

#endif // __LISTOBJECT_H__