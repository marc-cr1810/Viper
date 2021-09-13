#ifndef __THREAD_H__
#define __THREAD_H__

#include "../port.h"
#include "../objects/object.h"

struct _interpreterstate; // Defined in "interpreter.h"

typedef struct _vithreadstate
{
    _interpreterstate* interp; // The interpreter the thread is owned by

    /* The exception currently being raised */
    ViObject* curr_exc_type;
    ViObject* curr_exc_value;
} ViThreadState;

ViThreadState* ViThreadState_New(_interpreterstate* interp);

#endif // __THREAD_H__