#include "visys.h"

#include "../port.h"
#include "vimem.h"

char *ViSys_ReadLine(const char *prompt)
{
	printf("%s", prompt);
	std::string l;
	std::getline(std::cin, l);

	char *result = (char*)Mem_Alloc(l.size() + 2);
	memcpy(result, l.c_str(), l.size());
	result[l.size()] = '\n';
	result[l.size() + 1] = '\0';
	return result;
}
