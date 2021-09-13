#include "viperrun.h"

#include "errorcode.h"

#include "viarena.h"
#include "../parser/vigen.h"
#include "../objects/stringobject.h"

ViRuntimeState ViRuntime = { 0 };

static ViStatus vicore_create_interpreter(ViRuntimeState *runtime, const ViConfig *config, ViThreadState **tstate_p)
{
	ViInterpreterState *interp = ViInterpreter_New();
	if (interp == NULL)
		return ViStatus_Error("failed to initialize interpreter");

	ViStatus status = ViConfig_Copy(&interp->config, config);
	if (ViStatus_Exception(status))
		return status;

	ViThreadState *tstate = ViThreadState_New(interp);
	if (tstate == NULL)
		return ViStatus_Error("can't make first thread");

	*tstate_p = tstate;
	return ViStatus_Ok();
}

static ViStatus viinit_config(ViRuntimeState *runtime, const ViConfig *config, ViThreadState **tstate_p)
{
	ViStatus status;
	ViThreadState *tstate;
	status = vicore_create_interpreter(runtime, config, &tstate);
	if (ViStatus_Exception(status))
		return status;
	*tstate_p = tstate;

	runtime->core_initialized = 1;
	return ViStatus_Ok();
}

static ViStatus viinit_core(ViRuntimeState *runtime, const ViConfig *src_config, ViThreadState **tstate_p)
{
	ViStatus status;

	status = Vi_PreInitFromConfig(src_config, NULL);
	if (ViStatus_Exception(status))
		return status;

	ViConfig config;
	ViConfig_InitViperConfig(&config);

	status = ViConfig_Copy(&config, src_config);
	if (ViStatus_Exception(status))
		goto done;

	// Read config here when cmdline stuff is done

	if (!runtime->core_initialized)
		status = viinit_config(runtime, &config, tstate_p);
	// Will have a reconfigure function
	if (ViStatus_Exception(status))
		goto done;
done:
	ViConfig_Clear(&config);
	return status;
}

ViStatus Vi_PreInitialize(const ViPreConfig *src_config)
{
	return Vi_PreInitFromFromViArgv(src_config, NULL);
}

ViStatus Vi_PreInitFromFromViArgv(const ViPreConfig *src_config, const ViArgv *args)
{
	ViStatus status;

	if (src_config == NULL)
		return ViStatus_Error("preinitialization config is NULL");

	status = ViRuntime_Initialize();
	if (ViStatus_Exception(status))
		return status;

	ViRuntimeState *runtime = &ViRuntime;

	if (runtime->preinitialized)
		return ViStatus_Ok();

	runtime->preinitializing = 1;

	ViPreConfig config;

	status = ViPreConfig_InitFromPreConfig(&config, src_config);
	if (ViStatus_Exception(status))
		return status;

	// Commandline stuff we are not doing yet
	// skip for now
	/*status = ViPreConfig_Read(&config, args);
	if (ViStatus_Exception(status))
		return status;

	status = ViPreConfig_Write(&config);
	if (ViStatus_Exception(status))
		return status;*/

	runtime->preinitializing = 0;
	runtime->preinitialized = 1;

	return status;
}

ViStatus Vi_PreInitFromConfig(const ViConfig *config, const ViArgv *args)
{
	assert(config != NULL);

	ViStatus status = ViRuntime_Initialize();
	if (ViStatus_Exception(status))
		return status;

	ViRuntimeState *runtime = &ViRuntime;

	if (runtime->preinitialized)
		return ViStatus_Ok();

	ViPreConfig preconfig;
	ViPreConfig_InitFromConfig(&preconfig, config);

	if (!config->parse_argv)
		return Vi_PreInitialize(&preconfig);
	if (args == NULL)
	{
		ViArgv config_args;
		config_args.use_bytes_argv = 0;
		config_args.argc = config->argv.length;
		config_args.wchar_argv = config->argv.items;
		return Vi_PreInitFromFromViArgv(&preconfig, &config_args);
	}
	else
	{
		return Vi_PreInitFromFromViArgv(&preconfig, args);
	}
}

ViStatus Vi_InitializeFromConfig(const ViConfig *config)
{
	if (config == NULL)
		return ViStatus_Error("initialization config is NULL");

	ViStatus status = ViRuntime_Initialize();
	if (ViStatus_Exception(status))
		return status;

	ViRuntimeState *runtime = &ViRuntime;

	ViThreadState *tstate = NULL;
	status = viinit_core(runtime, config, &tstate);
	if (ViStatus_Exception(status))
		return status;

	return ViStatus_Ok();
}

void Vi_Exit(int status)
{
	ViRuntime_Finalize();
	exit(status);
}

ViStatus ViRuntime_Initialize()
{
	if (ViRuntime.initialized)
		return ViStatus_Ok();
	
	return ViRuntimeState_Init(ViRuntime);
}

int ViRuntime_Finalize()
{
	int status = 0;

	if (!ViRuntime.initialized)
		return status;

	ViRuntimeState_Finalize(ViRuntime);
	return status;
}

int Vi_FileIsInteractive(std::ifstream* fp, ViObject* filename)
{
	if (ViString_Check(filename))
	{
		return (filename == NULL) ||
			(strcmp(((ViStringObject*)filename)->ob_svar, "<stdin>")) ||
			(strcmp(((ViStringObject*)filename)->ob_svar, "???"));
	}
	ViError_SetString(ViExc_SystemError, "Bad internal call");
	return -1;
}

int ViRun_FileObject(std::ifstream *fp, const char *filename, bool close)
{
	ViObject *filename_obj;
	if (filename != NULL)
	{
		filename_obj = ViStringObject_FromString(filename);
		if (filename_obj == NULL)
		{
			ViError_Print();
			return -1;
		}
	}
	else
		filename_obj = NULL;

	int result = ViRun_FileObject(fp, filename_obj, close);
	ViObject_XDECREF(filename_obj);
	return result;
}

int ViRun_FileObject(std::ifstream* fp, ViObject* filename, bool close)
{
	bool decref_filename = false;
	if (filename == NULL)
	{
		filename = ViStringObject_FromString("???");
		if (filename == NULL)
		{
			ViError_SetString(ViExc_SystemError, "Filename is NULL");
			ViError_Print();
			return -1;
		}
		decref_filename = true;
	}

	int result = 0;
	if (Vi_FileIsInteractive(fp, filename))
	{
		result = ViRun_InteractiveLoop(fp, filename);
		if (close)
			fp->close();
	}
	else
	{
		result = ViRun_SimpleFileObject(fp, filename, close);
	}

	if (decref_filename)
		ViObject_DECREF(filename);
	return result;
}

int ViRun_InteractiveObject(std::ifstream* fp, ViObject* filename)
{
	const char* ps1 = NULL, * ps2 = NULL;
	int error_code = 0;
	mod_type mod;
	ViArena *arena;

	if (Vi_FileIsInteractive(fp, filename))
	{
		ps1 = ">>> ";
		ps2 = "... ";
	}

	arena = ViArena_New();
	if (arena == NULL)
		return -1;

	mod = ViParser_ASTFromFileObject(fp, filename, PARSER_MODE_SINGLE_INPUT, ps1, ps2, &error_code, arena);
	if (mod == NULL)
	{
		ViArena_Free(arena);
		if (error_code == E_EOF)
		{
			ViError_Clear();
			return E_EOF;
		}
		return -1;
	}

	ViArena_Free(arena);
	return 0;
}

int ViRun_InteractiveLoop(std::ifstream* fp, ViObject* filename)
{
	int error = 0;
	int ret = 0;
	do
	{
		ret = ViRun_InteractiveObject(fp, filename);
		if (ret == -1 && ViError_Occurred())
		{
			ViError_Print();
		}
	} while (ret != E_EOF);
	return error;
}

int ViRun_SimpleFileObject(std::ifstream* fp, ViObject* filename, bool close)
{
	// Currently not supported
	return -1;
}
