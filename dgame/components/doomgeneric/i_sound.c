//
// Copyright(C) 1993-1996 Id Software, Inc.
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
// DESCRIPTION:  none
//

#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "doomfeatures.h"
#include "doomtype.h"
#include "i_sound.h"
#include "i_video.h"
#include "m_argv.h"
#include "m_config.h"
#include "z_zone.h"
#include "w_wad.h"
#include "m_misc.h"
#include "doomgeneric.h"
//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//
void I_InitSound(boolean use_sfx_prefix)
{  
    boolean nosound, nosfx, nomusic;

    //!
    // @vanilla
    //
    // Disable all sound output.
    //
    nosound = M_CheckParm("-nosound") > 0;

    //!
    // @vanilla
    //
    // Disable sound effects. 
    //
    nosfx = M_CheckParm("-nosfx") > 0;

    //!
    // @vanilla
    //
    // Disable music.
    //
    nomusic = M_CheckParm("-nomusic") > 0;

    // Initialize the sound and music subsystems.
    if( !nosound && !screensaver_mode )
    {
        // This is kind of a hack. If native MIDI is enabled, set up
        // the TIMIDITY_CFG environment variable here before SDL_mixer
        // is opened.
        if( !nosfx )
        {
            //InitSfxModule(use_sfx_prefix);
        }

        if( !nomusic )
        {
            //InitMusicModule();
        }
    }
}

void I_ShutdownSound(void)
{
}

int I_GetSfxLumpNum( sfxinfo_t *sfxinfo )
{
    return sfxinfo->lumpnum;
}

void I_UpdateSound(void)
{
}

// It seems that this is volume difference between channels
static void CheckVolumeSeparation( int *vol, int *sep )
{
    if( *sep < 0 ) {
        *sep = 0;
    } else if ( *sep > 254 ) {
        *sep = 254;
    }

    if( *vol < 0 ) {
        *vol = 0;
    } else if ( *vol > 127 ) {
        *vol = 127;
    }
}

void I_UpdateSoundParams( int channel, int vol, int sep )
{
    CheckVolumeSeparation( &vol, &sep );
    //sound_module->UpdateSoundParams(channel, vol, sep);
}

int I_StartSound( sfxinfo_t *sfxinfo, int channel, int vol, int sep )
{
    CheckVolumeSeparation( &vol, &sep );
    return DG_StartSound( sfxinfo->fxID, channel, vol, sep );
}

void I_StopSound( int channel )
{
}

boolean I_SoundIsPlaying( int channel )
{
    return false;
}

void I_PrecacheSounds(sfxinfo_t *sounds, int num_sounds)
{
    char nameBuf[16];

    printf( "I_PrecacheSounds numSounds: %d\n", num_sounds );
    for( int i = 0; i < num_sounds; i++ )
    {
        sfxinfo_t* snd = &sounds[i];
        M_snprintf( nameBuf, sizeof(nameBuf), "DS%s", snd->name );

        int lmpIdx = W_CheckNumForName( nameBuf );
        if( lmpIdx < 0 ) // the effect with id 0 is a dummy
            continue;   // not present in this wad

        snd->lumpnum = lmpIdx;
        snd->driver_data = W_CacheLumpNum( snd->lumpnum, PU_SOUND );
        int len = W_LumpLength( snd->lumpnum );

        snd->fxID = i; // assign the index of sound, it corresponds to fx id
        printf( "-- sound [%d] info: %s, lumpNum: %d\n", i, snd->name, snd->lumpnum );

        DG_CacheSoundFx( i, snd->driver_data, len );
    }
}

void I_InitMusic(void)
{
}

void I_ShutdownMusic(void)
{
}

void I_SetMusicVolume(int volume)
{
}

void I_PauseSong(void)
{
}

void I_ResumeSong(void)
{
}

void* I_RegisterSong(void *data, int len)
{
    return NULL;
}

void I_UnRegisterSong(void *handle)
{
}

void I_PlaySong(void *handle, boolean looping)
{
}

void I_StopSong(void)
{
}

boolean I_MusicIsPlaying(void)
{
    return false;
}

void I_BindSoundVariables(void)
{
}

