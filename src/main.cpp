#include "Viper.h"

static void vimain_free()
{
	ViRuntime_Finalize();
}

static void vimain_header(ViConfig *config)
{
	if (config->quiet)
		return;

	if (!config->verbose && !config->interactive)
		return;

	printf("Viper %s on %s\n", Vi_GetVersion(), PLATFORM);
}

static ViStatus vimain_init(ViArgv *args)
{
	ViStatus status;
	status = ViRuntime_Initialize();

	if (ViStatus_Exception(status))
	{
		std::cout << "Error: " << status.error_msg << std::endl;
		return status;
	}

	ViPreConfig preconfig;
	ViPreConfig_InitViperConfig(&preconfig);

	status = Vi_PreInitFromFromViArgv(&preconfig, args);
	if (ViStatus_Exception(status))
		return status;

	ViConfig config;
	ViConfig_InitViperConfig(&config);

	if (args->use_bytes_argv)
		status = ViConfig_SetArgv(&config, args->argc, args->bytes_argv);
	else
		status = ViConfig_SetArgv(&config, args->argc, args->wchar_argv);
	if (ViStatus_Exception(status))
		goto done;

	status = Vi_InitializeFromConfig(&config);
	if (ViStatus_Exception(status))
		goto done;
	status = ViStatus_Ok();
done:
	//ViConfig_Clear(&config); // this crashes???
	return status;
}

static int vimain_run_stdin()
{
	std::ifstream file("<stdin>"); // make a blank file for stdin
	int run = ViRun_FileObject(&file, "<stdin>", false);
	return (run != 0);
}

static int vimain_run_viper(int *exitcode)
{
	ViInterpreterState *interp = ViInterpreterState_GET();
	ViConfig *config = (ViConfig *)ViInterpreterState_GetConfig(interp);

	// TEMPORARY
	config->interactive = 1;

	vimain_header(config);

	if (config->run_filename)
		*exitcode = 0; // not supported yet;
	else if (config->interactive) 
		*exitcode = vimain_run_stdin();

	return 0;
}

int Vi_RunMain()
{
	int exitcode = 0;

	vimain_run_viper(&exitcode);
	vimain_free();

	return exitcode;
}

static int vimain_main(ViArgv *args)
{
	ViStatus status = vimain_init(args);

	if (ViStatus_Exception(status))
	{
		vimain_free();
		std::cout << "Error: " << status.error_msg << std::endl;
		return status.exit_code;
	}

	return Vi_RunMain();
}

int main(int argc, char **argv)
{
	ViArgv args{
		argc,
		1,
		argv,
		NULL
	};
	return vimain_main(&args);
}
