#include "viconfig.h"

#include "viperrun.h"
#include "runtime.h"
#include "vimem.h"

#include "../objects/listobject.h"
#include "../objects/stringobject.h"

#define DECODE_LOCALE_ERR(NAME, LEN) \
    (((LEN) == -2) \
     ? ViStatus_Error("cannot decode " NAME) \
     : ViStatus_NoMemory())

/* -- ViWideStringList ---------------------*/

int ViWideStringList_CheckConsistency(const ViWideStringList *list)
{
	assert(list->length >= 0);
	if (list->length != 0)
	{
		assert(list->items != NULL);
	}
	for (Vi_size_t i = 0; i < list->length; i++)
	{
		assert(list->items[i] != NULL);
	}
	return 1;
}

void ViWideStringList_Clear(ViWideStringList *list)
{
	assert(ViWideStringList_CheckConsistency(list));
	for (Vi_size_t i = 0; i < list->length; i++)
	{
		Mem_Free(list->items[i]);
	}
	Mem_Free(list->items);
	list->length = 0;
	list->items = NULL;
}

int ViWideStringList_Copy(ViWideStringList *list, const ViWideStringList *list2)
{
	assert(ViWideStringList_CheckConsistency(list));
	assert(ViWideStringList_CheckConsistency(list2));

	if (list2->length == 0)
	{
		ViWideStringList_Clear(list);
		return 0;
	}

	ViWideStringList copy = ViWideStringList_INIT;

	size_t size = list2->length * sizeof(list2->items[0]);
	copy.items = (wchar_t **)Mem_Alloc(size);
	if (copy.items == NULL)
	{
		return -1;
	}

	for (Vi_size_t i = 0; i < list2->length; i++)
	{
		wchar_t *item = Mem_WcsDup(list2->items[i]);
		if (item == NULL)
		{
			ViWideStringList_Clear(&copy);
			return -1;
		}
		copy.items[i] = item;
		copy.length = i + 1;
	}

	ViWideStringList_Clear(list);
	*list = copy;
	return 0;
}

ViStatus ViWideStringList_Insert(ViWideStringList *list, Vi_size_t index, const wchar_t *item)
{
	Vi_size_t len = list->length;
	if (len == VI_SIZE_T_MAX)
	{
		/* length+1 would overflow */
		return ViStatus_NoMemory();
	}
	if (index < 0)
	{
		return ViStatus_Error("ViWideStringList_Insert index must be >= 0");
	}
	if (index > len)
	{
		index = len;
	}

	wchar_t *item2 = Mem_WcsDup(item);
	if (item2 == NULL)
	{
		return ViStatus_NoMemory();
	}

	size_t size = (len + 1) * sizeof(list->items[0]);
	wchar_t **items2 = (wchar_t **)Mem_Realloc(list->items, size);
	if (items2 == NULL)
	{
		Mem_Free(item2);
		return ViStatus_NoMemory();
	}

	if (index < len)
	{
		memmove(&items2[index + 1],
				&items2[index],
				(len - index) * sizeof(items2[0]));
	}

	items2[index] = item2;
	list->items = items2;
	list->length++;
	return ViStatus_Ok();
}

ViStatus ViWideStringList_Append(ViWideStringList *list, const wchar_t *item)
{
	return ViWideStringList_Insert(list, list->length, item);
}

ViStatus ViWideStringList_Extend(ViWideStringList *list, const ViWideStringList *list2)
{
	for (Vi_size_t i = 0; i < list2->length; i++)
	{
		ViStatus status = ViWideStringList_Append(list, list2->items[i]);
		if (ViStatus_Exception(status))
		{
			return status;
		}
	}
	return ViStatus_Ok();
}

int ViWideStringList_Find(ViWideStringList *list, const wchar_t *item)
{
	for (Vi_size_t i = 0; i < list->length; i++)
	{
		if (wcscmp(list->items[i], item) == 0)
		{
			return 1;
		}
	}
	return 0;
}

/* -- ViArgv -------------------------------*/

ViStatus ViArgv_AsWstrList(const ViArgv *args, ViWideStringList *list)
{
	ViWideStringList wargv = ViWideStringList_INIT;
	if (args->use_bytes_argv)
	{
		size_t size = sizeof(wchar_t *) * args->argc;
		wargv.items = (wchar_t **)Mem_Alloc(size);
		if (wargv.items == NULL)
		{
			return ViStatus_NoMemory();
		}

		for (Vi_size_t i = 0; i < args->argc; i++)
		{
			size_t len;
			wchar_t *arg = (wchar_t *)malloc(sizeof(wchar_t));
			len = mbstowcs(arg, args->bytes_argv[i], mbstowcs(NULL, args->bytes_argv[i], 0));
			if (arg == NULL)
			{
				ViWideStringList_Clear(&wargv);
				return DECODE_LOCALE_ERR("command line arguments",
										 (Vi_size_t)len);
			}
			wargv.items[i] = arg;
			wargv.length++;
		}

		ViWideStringList_Clear(list);
		*list = wargv;
	}
	else
	{
		wargv.length = args->argc;
		wargv.items = (wchar_t **)args->wchar_argv;
		if (ViWideStringList_Copy(list, &wargv) < 0)
		{
			return ViStatus_NoMemory();
		}
	}
	return ViStatus_Ok();
}

/* -- ViPreCmdline -------------------------*/

static void precmdline_get_preconfig(ViPreCmdline *cmdline, const ViPreConfig *preconfig)
{
#define COPY_ATTR(ATTR) \
    if (preconfig->ATTR != -1) { \
        cmdline->ATTR = preconfig->ATTR; \
    }

	COPY_ATTR(isolated);
	COPY_ATTR(use_environment);
	COPY_ATTR(dev_mode);

#undef COPY_ATTR
}

static void precmdline_set_preconfig(const ViPreCmdline *cmdline, ViPreConfig *preconfig)
{
#define COPY_ATTR(ATTR) \
    preconfig->ATTR = cmdline->ATTR

	COPY_ATTR(isolated);
	COPY_ATTR(use_environment);
	COPY_ATTR(dev_mode);

#undef COPY_ATTR
}

/* Parse the command line arguments */
static ViStatus precmdline_parse_cmdline(ViPreCmdline *cmdline)
{
	// For future with reading cmd args

	return ViStatus_Ok();
}

void ViPreCmdline_Clear(ViPreCmdline *cmdline)
{
	ViWideStringList_Clear(&cmdline->argv);
	ViWideStringList_Clear(&cmdline->xoptions);
}

ViStatus ViPreCmdline_SetArgv(ViPreCmdline *cmdline, const ViArgv *args)
{
	return ViArgv_AsWstrList(args, &cmdline->argv);
}

ViStatus ViPreCmdline_Read(ViPreCmdline *cmdline, const ViPreConfig *preconfig)
{
	precmdline_get_preconfig(cmdline, preconfig);

	if (preconfig->parse_argv)
	{
		ViStatus status = precmdline_parse_cmdline(cmdline);
		if (ViStatus_Exception(status))
			return status;
	}
	/* isolated, use_environment */
	if (cmdline->isolated < 0)
		cmdline->isolated = 0;
	if (cmdline->isolated > 0)
		cmdline->use_environment = 0;
	if (cmdline->use_environment < 0)
		cmdline->use_environment = 0;

	assert(cmdline->use_environment >= 0);
	assert(cmdline->isolated >= 0);
	assert(cmdline->dev_mode >= 0);
	return ViStatus_Ok();
}

/* -- ViPreConfig --------------------------*/

static void preconfig_copy(ViPreConfig *preconfig, const ViPreConfig *preconfig2)
{
#define COPY_ATTR(ATTR) preconfig->ATTR = preconfig2->ATTR

	COPY_ATTR(config_init);
	COPY_ATTR(parse_argv);

#undef COPY_ATTR
}

static void preconfig_get_global_vars(ViPreConfig *preconfig)
{
	/* Viper and Isolated configuration ignore global variables */
	if (preconfig->config_init != static_cast<int>(ViConfig_INIT_COMPAT))
		return;

#define COPY_FLAG(ATTR, VALUE) \
    if (preconfig->ATTR < 0) { \
        preconfig->ATTR = VALUE; \
    }

	COPY_FLAG(isolated, Vi_IsolatedFlag);
	COPY_FLAG(use_environment, Vi_EnvironmentFlag);

#undef COPY_FLAG
}

static void preconfig_set_global_vars(const ViPreConfig *preconfig)
{
#define COPY_FLAG(ATTR, VAR) \
    if (preconfig->ATTR >= 0) { \
        VAR = preconfig->ATTR; \
    }

	COPY_FLAG(isolated, Vi_IsolatedFlag);
	COPY_FLAG(use_environment, Vi_EnvironmentFlag);

#undef COPY_FLAG
}

static ViStatus preconfig_read(ViPreConfig *preconfig, ViPreCmdline *cmdline)
{
	ViStatus status;
	status = ViPreCmdline_Read(cmdline, preconfig);
	if (ViStatus_Exception(status))
		return status;

	return ViStatus_Ok();
}

static void preconfig_init_defaults(ViPreConfig *preconfig)
{
	memset(preconfig, 0, sizeof(*preconfig));

	preconfig->config_init = static_cast<int>(ViConfig_INIT_COMPAT);
	preconfig->parse_argv = 0;
	preconfig->isolated = 0;
	preconfig->use_environment = 0;
	preconfig->dev_mode = 0;
}

void ViPreConfig_InitViperConfig(ViPreConfig *preconfig)
{
	preconfig_init_defaults(preconfig);

	preconfig->config_init = static_cast<int>(ViConfig_INIT_VIPER);
	preconfig->parse_argv = 1;
}

ViStatus ViPreConfig_InitFromPreConfig(ViPreConfig *preconfig, const ViPreConfig *preconfig2)
{
	ViPreConfig_InitViperConfig(preconfig);
	preconfig_copy(preconfig, preconfig2);
	return ViStatus_Ok();
}

void ViPreConfig_InitFromConfig(ViPreConfig *preconfig, const ViConfig *config)
{
	ViConfigInitEnum config_init = (ViConfigInitEnum)config->config_init;
	switch (config_init)
	{
	case ViConfig_INIT_VIPER:
		ViPreConfig_InitViperConfig(preconfig);
		break;
	default:
		preconfig_init_defaults(preconfig);
		break;
	}

	ViPreConfig_GetConfig(preconfig, config);
}

ViStatus ViPreConfig_Read(ViPreConfig *preconfig, const ViArgv *args)
{
	ViStatus status;

	status = ViRuntime_Initialize();
	if (ViStatus_Exception(status))
		return status;

	preconfig_get_global_vars(preconfig);

	ViPreCmdline cmdline = ViPreCmdline_INIT;
	if (args)
	{
		status = ViPreCmdline_SetArgv(&cmdline, args);
		if (ViStatus_Exception(status))
			goto done;
	}
	status = preconfig_read(preconfig, &cmdline);
	if (ViStatus_Exception(status))
		goto done;

	status = ViStatus_Ok();
done:
	ViPreCmdline_Clear(&cmdline);
	return status;
}

ViStatus ViPreConfig_Write(const ViPreConfig *preconfig)
{
	ViPreConfig config;

	ViStatus status = ViPreConfig_InitFromPreConfig(&config, preconfig);
	if (ViStatus_Exception(status))
		return status;

	if (ViRuntime.core_initialized)
		return ViStatus_Ok();

	preconfig_set_global_vars(&config);

	/* Write the new pre-configuration into _ViRuntime */
	preconfig_copy(&ViRuntime.preconfig, &config);

	return ViStatus_Ok();
}

void ViPreConfig_GetConfig(ViPreConfig *preconfig, const ViConfig *config)
{
#define COPY_ATTR(ATTR) \
    if (config->ATTR != -1) { \
        preconfig->ATTR = config->ATTR; \
    }

	COPY_ATTR(parse_argv);
	COPY_ATTR(isolated);
	COPY_ATTR(use_environment);
	COPY_ATTR(dev_mode);

#undef COPY_ATTR
}

/* -- ViConfig -----------------------------*/

static void config_init_defaults(ViConfig *config)
{
	memset(config, 0, sizeof(*config));

	config->config_init = static_cast<int>(ViConfig_INIT_COMPAT);
	config->parse_argv = 0;
	config->isolated = 0;
	config->use_environment = 0;
	config->dev_mode = 0;
	config->verbose = 0;
	config->quiet = 0;
	config->write_bytecode = 0;
	config->interactive = 0;
}

void ViConfig_InitViperConfig(ViConfig *config)
{
	config_init_defaults(config);

	config->config_init = static_cast<int>(ViConfig_INIT_VIPER);
	config->parse_argv = 1;
}

ViStatus ViConfig_Copy(ViConfig *config, const ViConfig *config2)
{
	ViStatus status;
	ViConfig_Clear(config);

#define COPY_ATTR(ATTR) config->ATTR = config2->ATTR
#define COPY_WSTR_ATTR(ATTR) \
    do { \
        status = ViConfig_SetString(config, &config->ATTR, config2->ATTR); \
        if (ViStatus_Exception(status)) { \
            return status; \
        } \
    } while (0)
#define COPY_WSTRLIST(LIST) \
    do { \
        if (ViWideStringList_Copy(&config->LIST, &config2->LIST) < 0) { \
            return ViStatus_NoMemory(); \
        } \
    } while (0)

	COPY_ATTR(config_init);
	COPY_ATTR(isolated);
	COPY_ATTR(use_environment);
	COPY_ATTR(dev_mode);

	COPY_ATTR(parse_argv);
	COPY_WSTRLIST(argv);
	COPY_ATTR(interactive);
	COPY_ATTR(write_bytecode);
	COPY_ATTR(verbose);
	COPY_ATTR(quiet);
	COPY_WSTR_ATTR(run_filename);

#undef COPY_ATTR
#undef COPY_WSTR_ATTR
#undef COPY_WSTRLIST

	return ViStatus_Ok();
}

ViStatus ViConfig_SetString(ViConfig *config, wchar_t **config_str, const wchar_t *str)
{
	ViStatus status = Vi_PreInitFromConfig(config, NULL);
	if (ViStatus_Exception(status))
	{
		return status;
	}

	wchar_t *str2;
	if (str != NULL)
	{
		str2 = Mem_WcsDup(str);
		if (str2 == NULL)
		{
			return ViStatus_NoMemory();
		}
	}
	else
	{
		str2 = NULL;
	}
	Mem_Free(*config_str);
	*config_str = str2;
	return ViStatus_Ok();
}

// Free all memory in config (wchar & ViWideStringList)
void ViConfig_Clear(ViConfig *config)
{
#define CLEAR(ATTR) \
    do { \
        Mem_Free(ATTR); \
        ATTR = NULL; \
    } while (0)

	ViWideStringList_Clear(&config->argv);
	CLEAR(config->run_filename);

#undef CLEAR
}

ViStatus ViConfig_SetViArgv(ViConfig *config, ViArgv *args)
{
	ViStatus status = Vi_PreInitFromConfig(config, args);
	if (ViStatus_Exception(status))
		return status;
	return ViArgv_AsWstrList(args, &config->argv);
}

ViStatus ViConfig_SetArgv(ViConfig *config, Vi_size_t argc, char *const *argv)
{
	ViArgv args = {
		argc,
		1,
		argv,
		NULL
	};
	return ViConfig_SetViArgv(config, &args);
}

ViStatus ViConfig_SetArgv(ViConfig *config, Vi_size_t argc, wchar_t *const *argv)
{
	ViArgv args = {
		argc,
		0,
		NULL,
		argv
	};
	return ViConfig_SetViArgv(config, &args);
}
