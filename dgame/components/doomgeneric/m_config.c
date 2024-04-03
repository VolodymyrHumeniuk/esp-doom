//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//    Configuration file interface.
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "mem_alloc.h"
#include "config.h"

#include "doomtype.h"
#include "doomkeys.h"
#include "doomfeatures.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_misc.h"

#include "z_zone.h"

// All deleted out for ESP32

void M_SetConfigFilenames(char *main_config, char *extra_config)
{
}

//
// M_SaveDefaults
//
void M_SaveDefaults (void)
{
}

//
// Save defaults to alternate filenames
//
void M_SaveDefaultsAlternate(char *main, char *extra)
{
}

//
// M_LoadDefaults
//
void M_LoadDefaults (void)
{
}


//
// Bind a variable to a given configuration file variable, by name.
//
void M_BindVariable(char *name, void *location)
{
}

// Set the value of a particular variable; an API function for other
// parts of the program to assign values to config variables by name.

boolean M_SetVariable(char *name, char *value)
{
    return true;
}

// Get the value of a variable.

int M_GetIntVariable(char *name)
{
    return 0;
}

const char *M_GetStrVariable(char *name)
{
    return NULL;
}

float M_GetFloatVariable(char *name)
{
    return 0.0f;
}

// Get the path to the default configuration dir to use, if NULL
// is passed to M_SetConfigDir.

static char *GetDefaultConfigDir(void)
{
    char *result = (char *)mem_alloc(2);
    result[0] = '.';
    result[1] = '\0';

    return result;
}

// 
// SetConfigDir:
//
// Sets the location of the configuration directory, where configuration
// files are stored - default.cfg, chocolate-doom.cfg, savegames, etc.
//

void M_SetConfigDir(char *dir)
{
}

//
// Calculate the path to the directory to use to store save games.
// Creates the directory as necessary.
//

char *M_GetSaveGameDir(char *iwadname)
{
    return NULL;
}

