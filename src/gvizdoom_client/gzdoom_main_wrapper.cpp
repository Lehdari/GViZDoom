//
// Project: GViZDoom
// File: gzdoom_main_wrapper.cpp
//
// Copyright (c) 2022 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

// TODO this file is to be removed once SDL stuff is successfully migrated out of gvizdoom

// HEADER FILES ------------------------------------------------------------

#include <SDL.h>
#include <unistd.h>
#include <signal.h>
#include <new>
#include <sys/param.h>
#include <locale.h>
#include <sys/stat.h>
#include <sys/utsname.h>

#include "engineerrors.h"
#include "m_argv.h"
#include "c_console.h"
#include "version.h"
#include "cmdlib.h"
#include "engineerrors.h"
#include "i_system.h"
#include "i_interface.h"
#include "printf.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

extern "C" int cc_install_handlers(int, char**, int, int*, const char*, int(*)(char*, char*));

// #ifdef __APPLE__
// void Mac_I_FatalError(const char* errortext);
// #endif

#ifdef __linux__
void Linux_I_FatalError(const char* errortext);
#endif

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------
int GameMain();

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// The command line arguments.
FArgs *Args;

// PRIVATE DATA DEFINITIONS ------------------------------------------------


// CODE --------------------------------------------------------------------



static int GetCrashInfo (char *buffer, char *end)
{
    if (sysCallbacks.CrashInfo) sysCallbacks.CrashInfo(buffer, end - buffer, "\n");
    return strlen(buffer);
}

void I_DetectOS()
{
    FString operatingSystem;

    const char *paths[] = {"/etc/os-release", "/usr/lib/os-release"};

    for (const char *path : paths)
    {
        struct stat dummy;

        if (stat(path, &dummy) != 0)
            continue;

        char cmdline[256];
        snprintf(cmdline, sizeof cmdline, ". %s && echo ${PRETTY_NAME}", path);

        FILE *proc = popen(cmdline, "r");

        if (proc == nullptr)
            continue;

        char distribution[256] = {};
        fread(distribution, sizeof distribution - 1, 1, proc);

        const size_t length = strlen(distribution);

        if (length > 1)
        {
            distribution[length - 1] = '\0';
            operatingSystem = distribution;
        }

        pclose(proc);
        break;
    }

    utsname unameInfo;

    if (uname(&unameInfo) == 0)
    {
        const char* const separator = operatingSystem.Len() > 0 ? ", " : "";
        operatingSystem.AppendFormat("%s%s %s on %s", separator, unameInfo.sysname, unameInfo.release, unameInfo.machine);
    }

    if (operatingSystem.Len() > 0)
        Printf("OS: %s\n", operatingSystem.GetChars());
}

void I_StartupJoysticks();

int gzdoom_main_wrapper(int argc, char **argv)
{
    printf("Hello world!\n");

// #if !defined (__APPLE__)
    {
        int s[4] = { SIGSEGV, SIGILL, SIGFPE, SIGBUS };
        cc_install_handlers(argc, argv, 4, s, GAMENAMELOWERCASE "-crash.log", GetCrashInfo);
    }
// #endif // !__APPLE__

    printf(GAMENAME" %s - %s - SDL version\nCompiled on %s\n",
        GetVersionString(), GetGitTime(), __DATE__);

    seteuid (getuid ());
    // Set LC_NUMERIC environment variable in case some library decides to
    // clear the setlocale call at least this will be correct.
    // Note that the LANG environment variable is overridden by LC_*
    setenv ("LC_NUMERIC", "C", 1);

    setlocale (LC_ALL, "C");

    if (SDL_Init (0) < 0)
    {
        fprintf (stderr, "Could not initialize SDL:\n%s\n", SDL_GetError());
        return -1;
    }

    printf("\n");

    Args = new FArgs(argc, argv);

    printf("[ELJAS] %d args\n", argc);
    for (int i = 0; i < argc; ++i)
    {
        printf("[ELJAS] argc: %s\n", argv[i]);
    }
    printf("\n");

    // ELJAS: this piece of code gets the program path
    {
        // Should we even be doing anything with progdir on Unix systems?
        char program[PATH_MAX];
        if (realpath (argv[0], program) == NULL)
        {
            strcpy (program, argv[0]);
        }
        char *slash = strrchr(program, '/');
        if (slash != NULL)
        {
            *(slash + 1) = '\0';
            progdir = program;
        }
        else
        {
            progdir = "./";
        }
        printf("[ELJAS] program directory: %s\n", progdir.GetChars());
    }

    // ELJAS: let's not use joysticks
#define NO_SDL_JOYSTICK
    I_StartupJoysticks();

    const int result = GameMain();

    SDL_Quit();

    return result;
}
