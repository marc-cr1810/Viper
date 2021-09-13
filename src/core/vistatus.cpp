#include "vistatus.h"

#ifdef _MSC_VER
/* Visual Studio 2015> doesn't implement C99 __func__ in C */
#  define ViStatus_GET_FUNC() __FUNCTION__
#else
#  define ViStatus_GET_FUNC() __func__
#endif

#define ViStatus_OK() { ViStatus::ViStatus_TYPE_OK, }

#define ViStatus_ERROR(ERROR_MSG) \
	{ \
		ViStatus::ViStatus_TYPE_ERROR, \
		ViStatus_GET_FUNC(), \
		(ERROR_MSG), \
	}

#define ViStatus_NO_MEMORY() ViStatus_ERROR("memory allocation failed")

#define ViStatus_EXIT(EXIT_CODE) \
	{ \
		ViStatus::ViStatus_TYPE_EXIT, \
		"", \
		"", \
		(EXIT_CODE) \
	}

#define ViStatus_IS_ERROR(status) (status.type == ViStatus::ViStatus_TYPE_ERROR)
#define ViStatus_IS_EXIT(status) (status.type == ViStatus::ViStatus_TYPE_EXIT)
#define ViStatus_EXCEPTION(status) (status.type != ViStatus::ViStatus_TYPE_OK)

#define ViStatus_UPDATE_FUNC(status) do { status.func = ViStatus_GET_FUNC(); } while (0)

ViStatus ViStatus_Ok() 
{
	ViStatus status = ViStatus_OK();
	return status; 
}

ViStatus ViStatus_Error(const char* error_msg)
{
	assert(error_msg != NULL);
	ViStatus status = ViStatus_ERROR(error_msg);
	return status;
}

ViStatus ViStatus_NoMemory() 
{ 
	return ViStatus_Error("memory allocation failed"); 
}

ViStatus ViStatus_Exit(int exit_code)
{
	ViStatus status = ViStatus_EXIT(exit_code);
	return status;
}

bool ViStatus_IsError(ViStatus status)
{
	return ViStatus_IS_ERROR(status);
}

bool ViStatus_IsExit(ViStatus status)
{
	return ViStatus_IS_EXIT(status);
}

bool ViStatus_Exception(ViStatus status)
{
	return ViStatus_EXCEPTION(status);
}
