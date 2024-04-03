#pragma once

#include "sounds.h"
#include "wave_reader.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// sound effect in cache
class SoundEffect
{
public:
    int8_t*  pcmData; // pointer to pcm data, the data is stored by engine in zone allocator
    uint32_t dataLen; // length of the data in samples, that in this case == to bytes

    SoundEffect() {
        pcmData = nullptr;
        dataLen = 0;
    }
    SoundEffect( int8_t* data, uint32_t len );
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// SFXChannel
class SFXChannel
{
    SoundEffect*  pEffect = nullptr; // effect that is currently played on this channel
    uint32_t      posIdx  = 0;       // position in data buffer
public:
    void play( SoundEffect* sfx ) {
        pEffect = sfx;
        posIdx  = 0;
    }

    void stop() {
        pEffect = nullptr;
        posIdx  = 0;
    }

    inline bool isActive() {
        return pEffect != nullptr;
    }

    inline int8_t getSmaple() {
        int8_t smp = pEffect->pcmData[posIdx];
        if( ++posIdx >= pEffect->dataLen ) { // automatically stop channel
            pEffect = nullptr;
            posIdx  = 0;
        }
        return smp;
    }
};

#define NUM_SFX_CHANNELS 8
#define SAMPLE_RATE      11025
#define MIX_BUF_LEN      ( ( SAMPLE_RATE / 100 ) * 2 )
#define TX_SAMPLE_RATE   11050

////////////////////////////////////////////////////////////////////////////////////////////////////
// SoundFX mixer class
// Doom uses 8 sound fx channels, so we can have up to 8 pcm streams to mix into one buffer
// and output the result
class SoundFXMixer
{
    QueueHandle_t       m_hSndQue;      // queue for internal thread
    i2s_chan_handle_t   m_txChannel;    // I2S output channel

    SoundEffect         m_effects[NUMSFX]; // index corresponds to sfxenum_t

    SFXChannel          m_channels[NUM_SFX_CHANNELS]; // channels to mix
    uint8_t             m_nextChannel;

    int16_t             m_mixBuffer[MIX_BUF_LEN]; // mix buffer of 16-bit pcm data in stereo
    int16_t             m_inputBuf[MIX_BUF_LEN]; // read buffer of 16-bit pcm data in mono

    WaveReader          m_wavReader;

    static void soundfx_task( void* pArg ) {
        ((SoundFXMixer*)pArg)->play_fx();
    }

    void play_fx(); // task to play sound effects

    void mix(); // mix active channels if any

    inline int16_t softClip( int smp ) {
        int s = smp < 0 ? -smp : smp;
        while( s > 32767 ) {
            s = s >> 1;
        }
        return smp < 0 ? (int16_t)-s : (int16_t)s;
    }

public:
    SoundFXMixer();

    void init();
    void cache( int fxId, void* data, int len );
    int  start( int sfxId, int channel, int vol, int sep );
    void stop( int channel );
};