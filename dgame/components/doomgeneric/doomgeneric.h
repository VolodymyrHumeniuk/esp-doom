#ifndef DOOM_GENERIC
#define DOOM_GENERIC

#include <stdlib.h>
#include <stdint.h>

//#define DOOMGENERIC_RESX 640
//#define DOOMGENERIC_RESY 400

#define DOOMGENERIC_RESX 400
#define DOOMGENERIC_RESY 240

extern uint32_t* DG_ScreenBuffer;

#ifdef __cplusplus
extern "C" {
#endif

void doomgeneric_Create( void* pFrameBuf );
void doomgeneric_Tick();


//Implement below functions for your platform
void DG_Init();
void DG_DrawFrame();
void DG_SleepMs(uint32_t ms);
uint32_t DG_GetTicksMs();
int DG_GetKey(int* pressed, unsigned char* key);
void DG_SetWindowTitle(const char * title);

// interface to sound fx
void  DG_CacheSoundFx( int fxId, void* data, int len );
int   DG_StartSound( int sfxId, int channel, int vol, int sep );
void  DG_StopSound( int channel );
void  DG_UpdateSoundParams( int channel, int vol, int sep );
int   DG_IsSoundPlaying( int channel );

void* DG_RegisterSong( const char* fileName );
void  DG_UnRegisterSong( void* handle );
void  DG_PlaySong( void* handle, int loop );
void  DG_StopSong();
void  DG_PauseSong();
void  DG_ResumeSong();
void  DG_SetMusicVolume( int vol );
int   DG_IsMusicPlaying();

#ifdef __cplusplus
} // extern "C" {
#endif

#endif //DOOM_GENERIC
