#ifndef __PORT_H__
#define __PORT_H__

#include <assert.h>
#include <iostream>
#include <fstream>
#include <stack>
#include <string>
#include <vector>
#include <regex>

#ifndef MS_WINDOWS
#include <sys/stat.h>
#include <cmath>
#endif

#include "config.h"

#ifdef MS_WINDOWS

typedef int8_t    Vi_int8_t;
typedef int16_t   Vi_int16_t;
typedef int32_t   Vi_int32_t;
typedef int64_t   Vi_int64_t;

typedef uint8_t   Vi_uint8_t;
typedef uint16_t  Vi_uint16_t;
typedef uint32_t  Vi_uint32_t;
typedef uint64_t  Vi_uint64_t;

#else

typedef __int8_t    Vi_int8_t;
typedef __int16_t   Vi_int16_t;
typedef __int32_t   Vi_int32_t;
typedef __int64_t   Vi_int64_t;

typedef __uint8_t   Vi_uint8_t;
typedef __uint16_t  Vi_uint16_t;
typedef __uint32_t  Vi_uint32_t;
typedef __uint64_t  Vi_uint64_t;

#endif

typedef float       Vi_float_t;
typedef double      Vi_double_t;

typedef std::string Vi_string_t;

typedef intptr_t Vi_intptr_t;
typedef Vi_intptr_t Vi_size_t;
typedef Vi_size_t Vi_hash_t;

/* Largest possible value of size_t. */
#define VI_SIZE_MAX SIZE_MAX

/* Largest positive value of type Vi_size_t. */
#define VI_SIZE_T_MAX ((Vi_size_t)(((size_t)-1)>>1))
/* Smallest negative value of type Vi_size_t. */
#define VI_SIZE_T_MIN (-VI_SIZE_T_MAX-1)

#define VI_CHARMASK(c) ((unsigned char)((c) & 0xFF))

template<typename First, typename ... T>
bool ValueIsIn(First&& first, T && ... t)
{
    return ((first == t) || ...);
}

/* Below "a" is a power of 2. */
/* Round down size "n" to be a multiple of "a". */
#define Vi_SIZE_ROUND_DOWN(n, a) ((size_t)(n) & ~(size_t)((a) - 1))
/* Round up size "n" to be a multiple of "a". */
#define Vi_SIZE_ROUND_UP(n, a) (((size_t)(n) + (size_t)((a) - 1)) & ~(size_t)((a) - 1))
/* Round pointer "p" down to the closest "a"-aligned address <= "p". */
#define Vi_ALIGN_DOWN(p, a) ((void *)((uintptr_t)(p) & ~(uintptr_t)((a) - 1)))
/* Round pointer "p" up to the closest "a"-aligned address >= "p". */
#define Vi_ALIGN_UP(p, a) ((void *)(((uintptr_t)(p) + (uintptr_t)((a) - 1)) & ~(uintptr_t)((a) - 1)))
/* Check if pointer "p" is aligned to "a"-bytes boundary. */
#define Vi_IS_ALIGNED(p, a) (!((uintptr_t)(p) & (uintptr_t)((a) - 1)))

#ifndef ViAPI_FUNC
#   define ViAPI_FUNC(RTYPE) RTYPE
#endif
#ifndef ViAPI_DATA
#   define ViAPI_DATA(RTYPE) extern RTYPE
#endif

#endif // __PORT_H__