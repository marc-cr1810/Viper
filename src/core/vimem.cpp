#include "vimem.h"

void* Mem_Alloc(size_t size)
{
	if (size == 0)
		size = 1;
	return malloc(size);
}

void* Mem_Calloc(size_t elemCount, size_t elemSize)
{
	if (elemCount == 0 || elemCount == 0)
	{
		elemCount = 1;
		elemSize = 1;
	}
	return calloc(elemCount, elemSize);
}

void* Mem_Realloc(void* ptr, size_t new_size)
{
	if (new_size == 0)
		new_size = 1;
	return realloc(ptr, new_size);
}

void Mem_Free(void* ptr)
{
	free(ptr);
}

wchar_t *Mem_WcsDup(const wchar_t *str)
{
	assert(str != NULL);

	size_t len = wcslen(str);
	if (len > (size_t)VI_SIZE_T_MAX / sizeof(wchar_t) - 1)
	{
		return NULL;
	}

	size_t size = (len + 1) * sizeof(wchar_t);
	wchar_t *str2 = (wchar_t *)Mem_Alloc(size);
	if (str2 == NULL)
	{
		return NULL;
	}

	memcpy(str2, str, size);
	return str2;
}
