#ifndef __VIPERRUN_H__
#define __VIPERRUN_H__

#include "runtime.h"
#include "vistatus.h"
#include "viconfig.h"
#include "error.h"

#include "../port.h"
#include "../objects/object.h"

ViStatus Vi_PreInitialize(const ViPreConfig *src_config);
ViStatus Vi_PreInitFromFromViArgv(const ViPreConfig *src_config, const ViArgv *args);
ViStatus Vi_PreInitFromConfig(const ViConfig *config, const ViArgv *args);
ViStatus Vi_InitializeFromConfig(const ViConfig *config);

void Vi_Exit(int status);

ViStatus ViRuntime_Initialize();
int ViRuntime_Finalize();

int Vi_FileIsInteractive(std::ifstream* fp, ViObject* filename);

int ViRun_FileObject(std::ifstream *fp, const char *filename, bool close);
int ViRun_FileObject(std::ifstream* fp, ViObject* filename, bool close);
int ViRun_InteractiveObject(std::ifstream* fp, ViObject* filename);
int ViRun_InteractiveLoop(std::ifstream* fp, ViObject* filename);
int ViRun_SimpleFileObject(std::ifstream* fp, ViObject* filename, bool close);

#endif // __VIPERRUN_H__