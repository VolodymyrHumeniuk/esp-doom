#include "main.h"
#include "sd_card.h"
#include "st7793.h"
#include "doomgeneric.h"
#include "doomkeys.h"
#include "keyboard.h"
#include "soundfx.h"

#ifndef CONFIG_IDF_TARGET_ESP32S3
    #error this app requires ESP32S3 MCU
#endif

static lpcstr TAG = "MAIN";

static Keyboard*     g_pKBD = nullptr;
static ST7793Driver* g_pTFT = nullptr;
static SDCardReader  g_sdCard;
static SoundFXMixer* g_pSndFx = nullptr;
static char          g_strFPS[16] = { 0 };


extern "C" {
void DG_Init() // nothing to do
{
}

void DG_DrawFrame()
{
    g_pTFT->printText( g_strFPS, g_pTFT->width() - 48, 1, GREEN );
    g_pTFT->flip(); // frame is already in frameBuffer, present it on LCD
}

void DG_SleepMs( uint32_t ms )
{
    vTaskDelay( ms2ticks( ms ) );
}

uint32_t DG_GetTicksMs()
{
    return esp_timer_get_time() / 1000;
}

int DG_GetKey( int* pressed, unsigned char* key )
{
    uint16_t keyInfo = g_pKBD->getKey();
    if( keyInfo ) { // if non-zero, we've got a key
        *pressed = keyInfo & 0xFF00;
        *key = keyInfo & 0xFF;
        return 1;
    }
    return 0;
}

void DG_SetWindowTitle( const char* title ) // output title as log message
{
    ESP_LOGI( "DOOM", "Set Title: %s", title );
}

void DG_CacheSoundFx( int fxId, void* data, int len )
{
    g_pSndFx->cache( fxId, data, len );
}

int DG_StartSound( int sfxId, int channel, int vol, int sep )
{
    return g_pSndFx->start( sfxId, channel, vol, sep );
}

int DG_IsSoundPlaying( int channel )
{
    return g_pSndFx->is_channelActive( channel );
}

void DG_UpdateSoundParams( int channel, int vol, int sep )
{
    return g_pSndFx->update( channel, vol, sep );
}

void DG_StopSound( int channel )
{
    g_pSndFx->stop( channel );
}

// music
void* DG_RegisterSong( const char* fileName )
{
    return g_pSndFx->registerSong( fileName );
}

void DG_UnRegisterSong( void* handle )
{
    g_pSndFx->unRegisterSong( handle );
}

void DG_PlaySong( void* handle, int loop )
{
    g_pSndFx->playSong( handle, (bool)loop );
}

void DG_StopSong()
{
    g_pSndFx->stopSong();
}

void DG_PauseSong()
{
    g_pSndFx->pauseSong();
}

void DG_ResumeSong()
{
    g_pSndFx->resumeSong();
}

void DG_SetMusicVolume( int vol )
{
    g_pSndFx->setMusicVolume( vol );
}

int DG_IsMusicPlaying()
{
    return g_pSndFx->isMusicPlaying();
}

}

// DOOM engine task
void doom_task( void* pArg )
{
    uint32_t tickStart, elapsed;

    g_pTFT->init( LCD_WIDTH, LCD_HEIGHT, IDS_CLK_FREQ, 
        LCD_RD, LCD_WR, LCD_RS, LCD_CS, 
        LCD_D0, LCD_D1, LCD_D2, LCD_D3, LCD_D4, LCD_D5, LCD_D6, LCD_D7,
        LCD_RST );

    g_pTFT->clearFrameBuffer( BLACK );
    g_pTFT->flip();
    g_pTFT->rotate( 1 );
    g_pTFT->clearFrameBuffer( BLACK );
    g_pTFT->flip();

    doomgeneric_Create( g_pTFT->getFrameBuffer() );

    uint32_t freeHeap = esp_get_free_heap_size();
    uint32_t freePsram = heap_caps_get_free_size( MALLOC_CAP_SPIRAM );
    ESP_LOGI( TAG, "Free heap left: %lu bytes, free PSRAM left: %lu", freeHeap, freePsram );

    while( true )
    {
        tickStart = esp_timer_get_time();

        doomgeneric_Tick();

        elapsed = ( esp_timer_get_time() - tickStart ) / 1000; // 1 tick time in ms
        sprintf( g_strFPS, "%lu FPS", 1000 / elapsed ); // fps -- how many ticks per second
    }
}

// create all objects in PSRAM
/*void* operator new ( size_t size ) {
    return heap_caps_malloc( size, MALLOC_CAP_32BIT | MALLOC_CAP_SPIRAM );
}*/

extern "C" void app_main(void)
{
    g_pTFT = new ST7793Driver();
    
    g_pKBD = new Keyboard();
    g_pKBD->init();

    g_sdCard.init();

    DG_SleepMs( 20 );

    g_pSndFx = new SoundFXMixer();
    g_pSndFx->init();
    
    DG_SleepMs( 100 );

    xTaskCreate( &doom_task, "doom", 18*1024, nullptr, 3, nullptr );
}
