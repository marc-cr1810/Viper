#ifndef __VICONFIG_H__
#define __VICONFIG_H__

#include "../port.h"
#include "vistatus.h"
#include "../objects/object.h"

/* -- Global configuration variables -------*/

static int Vi_QuietFlag = 0;
static int Vi_VerboseFlag = 0;
static int Vi_EnvironmentFlag = 0;
static int Vi_IsolatedFlag = 0;
static int Vi_WriteBytecodeFlag = 0;
static int Vi_InteractiveFlag = 0;

// Forward declarations
typedef struct _config ViConfig;
typedef struct _preconfig ViPreConfig;

typedef enum _configinitenum
{
    ViConfig_INIT_COMPAT = 1,
    ViConfig_INIT_VIPER = 2,
    ViConfig_INIT_ISOLATED = 3
} ViConfigInitEnum;

/* -- ViWideStringList ---------------------*/

typedef struct
{
    /* If length is greater than zero, items must be non-NULL
       and all items strings must be non-NULL */
    Vi_size_t length;
    wchar_t **items;
} ViWideStringList;

#define ViWideStringList_INIT { 0, NULL}

int ViWideStringList_CheckConsistency(const ViWideStringList *list);
void ViWideStringList_Clear(ViWideStringList *list);
int ViWideStringList_Copy(ViWideStringList *list, const ViWideStringList *list2);
ViStatus ViWideStringList_Insert(ViWideStringList *list, Vi_size_t index, const wchar_t *item);
ViStatus ViWideStringList_Append(ViWideStringList *list, const wchar_t *item);
ViStatus ViWideStringList_Extend(ViWideStringList *list, const ViWideStringList *list2);
int ViWideStringList_Find(ViWideStringList *list, const wchar_t *item);

/* -- ViArgv -------------------------------*/

typedef struct _argv
{
    Vi_size_t argc;
    int use_bytes_argv;
    char *const *bytes_argv;
    wchar_t *const *wchar_argv;
} ViArgv;

ViStatus ViArgv_AsWstrList(const ViArgv *args, ViWideStringList *list);

/* -- ViPreCmdline -------------------------*/

typedef struct _precmdline
{
    ViWideStringList argv;
    ViWideStringList xoptions;     /* "-X value" option */
    int isolated;             /* -I option */
    int use_environment;      /* -E option */
    int dev_mode;             /* -X dev and VIPERDEVMODE */
} ViPreCmdline;

#define ViPreCmdline_INIT { ViWideStringList_INIT, ViWideStringList_INIT, -1, -1, -1 }

void ViPreCmdline_Clear(ViPreCmdline *cmdline);
ViStatus ViPreCmdline_SetArgv(ViPreCmdline *cmdline, const ViArgv *args);
ViStatus ViPreCmdline_Read(ViPreCmdline *cmdline, const ViPreConfig *preconfig);

/* -- ViPreConfig --------------------------*/

typedef struct _preconfig
{
    int config_init; // ViConfigInitEnum
    int parse_argv;
    int isolated;
    int use_environment;
    int dev_mode;
} ViPreConfig;

void ViPreConfig_InitViperConfig(ViPreConfig *preconfig);
ViStatus ViPreConfig_InitFromPreConfig(ViPreConfig *preconfig, const ViPreConfig *preconfig2);
void ViPreConfig_InitFromConfig(ViPreConfig *preconfig, const ViConfig *config);

ViStatus ViPreConfig_Read(ViPreConfig *preconfig, const ViArgv *args);
ViStatus ViPreConfig_Write(const ViPreConfig *preconfig);
void ViPreConfig_GetConfig(ViPreConfig *preconfig, const ViConfig *config);

/* -- ViConfig -----------------------------*/

typedef struct _config
{
    int config_init; // ViConfigInitEnum

    int parse_argv;
    ViWideStringList argv;

    int isolated;
    int use_environment;
    int dev_mode;
    int verbose;
    int quiet;
    int write_bytecode;

    wchar_t *run_filename;
    int interactive;
} ViConfig;

void ViConfig_InitViperConfig(ViConfig *config);

ViStatus ViConfig_SetViArgv(ViConfig *config, ViArgv *args);
ViStatus ViConfig_SetArgv(ViConfig *config, Vi_size_t argc, char *const *argv);
ViStatus ViConfig_SetArgv(ViConfig *config, Vi_size_t argc, wchar_t *const *argv);

ViStatus ViConfig_SetString(ViConfig *config, wchar_t **config_str, const wchar_t *str);

ViStatus ViConfig_Copy(ViConfig *config, const ViConfig *config2);
void ViConfig_Clear(ViConfig *config);

#endif // __VICONFIG_H__