#ifndef __CONFIG_H__
#define __CONFIG_H__

#ifndef _CRT_SECURE_NO_DEPRECATE
#	define _CRT_SECURE_NO_DEPRECATE 1
#endif

#ifdef _DEBUG
#	define Vi_DEBUG
#elif defined(DEBUG)
#	define Vi_ARDUINO_DEBUG
#endif

#define Vi_EXIT_SUCCESS			0U
#define Vi_EXIT_FAILURE			1U

#define _Vi_STRINGIZE(X) _Vi_STRINGIZE1((X))
#define _Vi_STRINGIZE1(X) _Vi_STRINGIZE2 ## X
#define _Vi_STRINGIZE2(X) #X

#define _Vi_STRINGTOBYTES(X) std::vector<char>(X.begin(), X.end())
#define _Vi_BYTESTOSTRING(X) Vi_string_t(X.begin(), X.end())

#ifdef Vi_DEBUG
#define D(x) x
#else
#define D(x)
#endif

#ifdef _MSC_VER
/* Microsoft Visual Studio C++ Compiler Information */

#define MS_WINDOWS
#define MS_WIN32			// Support for win32 and greater

#ifdef _WIN64
#	define MS_WIN64
#endif

#define _Vi_COMPILER_PASTE_VERSION(suffix) ("[MSC v." _Vi_STRINGIZE(_MSC_VER) " " suffix "]")

#ifdef MS_WIN64

#	if defined(_M_IA64)
#		define COMPILER _Vi_COMPILER_PASTE_VERSION("64 bit (Itanium)")
#		define MS_WINI64
#		define TYD_PLATFORM_TAG "win_ia64"
#	elif defined(_M_X64) || defined(_M_AMD64)
#		if defined(__INTEL_COMPILER)
#			define COMPILER ("[ICC v." _Vi_STRINGIZE(__INTEL_COMPILER) " 64 bit (amd64) with MSC v." _Vi_STRINGIZE(_MSC_VER) " CRT]")
#		else
#			define COMPILER _Vi_COMPILER_PASTE_VERSION("64 bit (AMD64)")
#		endif /* __INTEL_COMPILER */
#		define MS_WINX64
#		define TYD_PLATFORM_TAG "win_amd64"
#	else
#		define COMPILER _Vi_COMPILER_PASTE_VERSION("64 bit (Unknown)")
#	endif
#endif

#if defined(MS_WIN32) && !defined(MS_WIN64)
#	if defined(_M_IX86)
#		if defined(__INTEL_COMPILER)
#			define COMPILER ("[ICC v." _Vi_STRINGIZE(__INTEL_COMPILER) " 32 bit (Intel) with MSC v." _Vi_STRINGIZE(_MSC_VER) " CRT]")
#		else
#			define COMPILER _Vi_COMPILER_PASTE_VERSION("32 bit (Intel)")
#		endif /* __INTEL_COMPILER */
#		define TYD_PLATFORM_TAG "win32"
#	elif defined(_M_ARM)
#		define COMPILER _Vi_PASTE_VERSION("32 bit (ARM)")
#		define TYD_PLATFORM_TAG "win_arm"
#	else
#		define COMPILER _Vi_COMPILER_PASTE_VERSION("32 bit (Unknown)")
#	endif
#endif /* MS_WIN32 && !MS_WIN64 */
#endif

#ifdef MS_WIN64
#       define PLATFORM "win64"
#       define SIZEOF_VOID_P 8
#       define SIZEOF_TIME_T 8
#       define SIZEOF_OFF_T 4
#       define SIZEOF_FPOS_T 8
#       define SIZEOF_HKEY 8
#       define SIZEOF_SIZE_T 8
#elif defined(MS_WIN32)
#       define PLATFORM "win32"
#       define HAVE_LARGEFILE_SUPPORT
#       define SIZEOF_VOID_P 4
#       define SIZEOF_OFF_T 4
#       define SIZEOF_FPOS_T 8
#       define SIZEOF_HKEY 4
#       define SIZEOF_SIZE_T 4
        /* MS VS2005 changes time_t to a 64-bit type on all platforms */
#       if defined(_MSC_VER) && _MSC_VER >= 1400
#       define SIZEOF_TIME_T 8
#       else
#       define SIZEOF_TIME_T 4
#       endif
#endif

#define NAME "viper"

#include "patchlevel.h"


#endif // __CONFIG_H__