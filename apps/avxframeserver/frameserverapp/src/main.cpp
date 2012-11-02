// Copyright (c) 2012 Anne Aaron, Milan Stevanovic, Pradip Gajjar.  All rights reserved.
// avxanne@gmail.com, avxsynth@gmail.com, pradip.gajjar@gmail.com
// http://www.avxsynth.org


// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA, or visit
// http://www.gnu.org/copyleft/gpl.html .
//
// Linking Avisynth statically or dynamically with other modules is making a
// combined work based on Avisynth.  Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Avisynth give you
// permission to link Avisynth with independent modules that communicate with
// Avisynth solely through the interfaces defined in avisynth.h, regardless of the license
// terms of these independent modules, and to copy and distribute the
// resulting combined work under terms of your choice, provided that
// every copy of the combined work is accompanied by a complete copy of
// the source code of Avisynth (the version of Avisynth used to produce the
// combined work), being distributed under the terms of the GNU General
// Public License plus this exception.  An independent module is a module
// which is not derived from or based on Avisynth, such as 3rd-party filters,
// import and export plugins, or graphical user interfaces.auto

#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <avxlog.h>

#include "frameserverlib.h"

using namespace avxsynth;

#ifdef MODULE_NAME
#undef MODULE_NAME
#endif

#define MODULE_NAME avxSynthFrameServer

#define MAX_SCRIPT_NAME_LEN 2048

typedef enum
{
    CMD_LINE_ARG_EXECUTABLE_NAME = 0,
    CMD_LINE_ARG_INPUT_SCRIPT_PATH,
    CMD_LINE_ARG_LAUNCH_STREAM_RECIPIENT_PROCESS,
    MAX_CMD_LINE_ARGUMENTS,
    MIN_CMD_LINE_ARGUMENTS = 2
} COMMAND_LINE_ARGUMENTS;


int executeScript(const char *scriptName, bool isMPlayerLaunchRequired)
{
    int result = -1;

    if (scriptName != NULL)
    {
        result = ProcessScript(scriptName, isMPlayerLaunchRequired);
    }

    return result;
}

void usage(const char *prgName)
{
    if (prgName != NULL)
    {
        AVXLOG_NOTICE("\nUsage:\n\t%s <avxsynth script.avs> [mplayer launch flag (default=(Y)es)]\n", prgName);
    }
}

int main(int argc, char* argv[])
{
	if(MIN_CMD_LINE_ARGUMENTS > argc)
	{
        usage(argv[CMD_LINE_ARG_EXECUTABLE_NAME]);
		return -1;
	}
	
    bool isMPlayerLaunchRequired = true;
	if (MIN_CMD_LINE_ARGUMENTS < argc)
    {
        const char *mplayerLaunch = argv[CMD_LINE_ARG_LAUNCH_STREAM_RECIPIENT_PROCESS];
        if (strcmp(mplayerLaunch, "Y") != 0)
            isMPlayerLaunchRequired = false;
    }
        
    char scriptName[MAX_SCRIPT_NAME_LEN] = {0x00};
    strncpy(scriptName, argv[CMD_LINE_ARG_INPUT_SCRIPT_PATH], MAX_SCRIPT_NAME_LEN);
        
    return executeScript(scriptName, isMPlayerLaunchRequired);
}
