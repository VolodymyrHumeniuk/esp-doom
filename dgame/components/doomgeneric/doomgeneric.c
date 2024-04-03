#include <stdio.h>
#include "mem_alloc.h"
#include "m_argv.h"

#include "doomgeneric.h"

uint32_t* DG_ScreenBuffer = 0;

void M_FindResponseFile(void);
void D_DoomMain(void);


void doomgeneric_Create( void* pFrameBuf )
{
	// save arguments
    myargc = NULL;
    myargv = NULL;

	M_FindResponseFile();

	DG_ScreenBuffer = (uint32_t*)pFrameBuf;//mem_alloc(DOOMGENERIC_RESX * DOOMGENERIC_RESY * 4);

	DG_Init();

	D_DoomMain();
}

