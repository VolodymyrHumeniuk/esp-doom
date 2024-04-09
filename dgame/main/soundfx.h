// I've noticed that there is no simple way of converting 8-bit mono DOOM data into audiable sound.
// Taking samples as integers and converting them to 16-bit produces results that not pleasent as well.
// I've also tried some audio mixing code found in doom-fb with, without success.
// This code below is based on my own invetigation and understanding.

#pragma once

#include "sounds.h"
#include "wave_reader.h"
#include "limiter.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// sound effect in cache
class SoundEffect
{
public:
    uint8_t   fxId;    // id from sfx table
    uint8_t*  pcmData; // pointer to pcm data U8, the data is stored by engine in zone allocator
    uint32_t  dataLen; // length of the data in samples, that in this case == to bytes

    SoundEffect() {
        fxId = 0;
        pcmData = nullptr;
        dataLen = 0;
    }
    SoundEffect( uint8_t id, uint8_t* data, uint32_t len );
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// SFXChannel
class SFXChannel
{
    SoundEffect*  pEffect = nullptr; // effect that is currently played on this channel
    uint32_t      posIdx  = 0;       // position in data buffer
    uint8_t       gainSep;           // gain separation between channels
    uint8_t       volume;            // channel volume (both left and right channels)

public:
    void play( SoundEffect* sfx, uint8_t sep, uint8_t vol ) {
        gainSep = sep;
        volume  = vol;
        pEffect = sfx;
        posIdx  = 0;
    }

    void stop() {
        pEffect = nullptr;
    }

    void update( uint8_t vol, uint8_t sep ) {
        volume  = vol;
        gainSep = sep;
    }

    inline bool isActive() const {
        return pEffect != nullptr;
    }

    inline int16_t getSmaple() {
        uint8_t smp = pEffect->pcmData[posIdx];
        if( ++posIdx >= pEffect->dataLen ) { // automatically stop channel
            pEffect = nullptr;
            posIdx  = 0;
        }
        return ((int)smp << 8) - 32640; // convert to 16 bit signed
    }

    inline uint8_t getSfxID() const {
        return pEffect->fxId;
    }

    inline uint8_t getGainSep() const {
        return gainSep;
    }

    inline uint8_t getVolume() const {
        return volume;
    }
};

#define NUM_SFX_CHANNELS 8
#define SAMPLE_RATE      11025
// input buffer for song sontains mono data
#define INPUT_BUF_LEN    ( ( SAMPLE_RATE / 100 ) * 2 )
// mix output buffer contains 16 bit PCM stereo
#define MIX_BUF_LEN      ( ( SAMPLE_RATE / 100 ) * 2 * 2 )

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

    // As I understand, sepatation is just the gain difference between left and right channel for the specified efffect.
    // The range is 1..255, so sep 1 means that effect is entirely in the left channel, 255 - in the right channel
    // 127 - means effect has equal gain in both channels, i.e. - centered.
    // This table holds the gain coeffiecients for the right channel (from left to right).
    // As for the left channel, the gain will be 1 - m_sepGainTable[sep];
    float               m_sepGainTable[256];

    // The volume table. It seems that all this comes from midi controlls, as volume represented as 0..127.
    // This table holds coefficients for the effect volume in channel
    float               m_volumeTable[128];

    int16_t             m_mixBuffer[MIX_BUF_LEN];   // mix buffer of 16-bit pcm data in stereo
    int16_t             m_inputBuf[INPUT_BUF_LEN];  // read buffer of 16-bit pcm data in mono for music

    WaveReader          m_wavReader;         // read wav file from SD card
    bool                m_loopMusic = false; // play music looped
    float               m_musicGain = 1.0f;
    bool                m_playMusic = false;

    Limiter             m_limiter; // to prevent clipping of doom music and sounds

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
    void update( int channel, int vol, int sep );
    bool is_channelActive( int channel ) const {
        return m_channels[channel].isActive();
    }

    // music
    void* registerSong( const char* fileName );
    void  unRegisterSong( void* handle );
    void  playSong( void* handle, bool loop );
    void  stopSong();
    void  pauseSong();
    void  resumeSong();
    void  setMusicVolume( int vol );
    int   isMusicPlaying();

};