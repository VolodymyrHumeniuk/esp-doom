#include "main.h"
#include "soundfx.h"

static lpcstr TAG = "SFX";

SoundEffect::SoundEffect( uint8_t id, uint8_t* data, uint32_t len )
{
    fxId    = id;  // id from sfx table
    pcmData = data;
    dataLen = len;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
SoundFXMixer::SoundFXMixer()
{
    memset( m_mixBuffer, 0, MIX_BUF_LEN * sizeof(int16_t) );
    m_nextChannel = 0;

    for( int i = 0; i < 256; i++ ) // calc the gain separation table for right channel
    {
        m_sepGainTable[i] = (float)i / 255.0f;
    }

    for( int i = 0; i < 128; i++ ) // calc the volume table
    {
        m_volumeTable[i] = (float)i / 127.0f;
    }
}

void SoundFXMixer::init()
{
    i2s_chan_config_t tx_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG( I2S_NUM_AUTO, I2S_ROLE_MASTER );
    esp_err_t err = i2s_new_channel( &tx_chan_cfg, &m_txChannel, nullptr );
    if( err != ESP_OK )
    {
        ESP_LOGE( TAG, "i2s_new_channel failed, err=%d", err );
        return;
    }

    //i2s_chan_info_t chInfo;
    //i2s_channel_get_info( m_txChannel, &chInfo );
    //ESP_LOGI( TAG, "Channel dir: %d", chInfo.dir ); // should be 2

    i2s_std_config_t tx_std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG( SAMPLE_RATE ),
        .slot_cfg = I2S_STD_PCM_SLOT_DEFAULT_CONFIG( I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO ),
        .gpio_cfg = {
            .mclk = I2S_SCLK_PIN,    // SCLK master clock
            .bclk = I2S_BCLK_PIN,    // bit clock comes from mcu
            .ws   = I2S_LRCK_PIN,    // ws is LRCK
            .dout = I2S_DOUT_PIN,    // output data pin
            .din  = I2S_GPIO_UNUSED, // only output is used
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };

    err = i2s_channel_init_std_mode( m_txChannel, &tx_std_cfg );
    if( err != ESP_OK )
    {
        ESP_LOGE( TAG, "i2s_channel_init_std_mode failed, err=%d", err );
        return;
    }

    m_hSndQue = xQueueCreate( 10, sizeof(int) );

    xTaskCreate( soundfx_task, "soundfx_task", 4096, this, 3, nullptr );

    ESP_LOGI( TAG, "init ok" );
}

void SoundFXMixer::cache( int fxId, void* data, int len )
{
    m_effects[fxId] = SoundEffect( fxId, (uint8_t*)data, (uint32_t)len );
}

int SoundFXMixer::start( int sfxId, int channel, int vol, int sep )
{
    uint8_t id  = sfxId;    // there are 109 effects in doom, so 1 byte should be ok
    uint8_t ch  = channel;  // only 8 channels, engine stop channel first before new sound
    uint8_t vl  = vol;      // to get real volume from lookup table
    uint8_t sp  = sep;      // to get channel gain separation from lookup table

    uint32_t update = sp << 24 | vl << 16 | ch << 8 | id;
    
    //ESP_LOGI( TAG, "start: sfxId: %d, ch: %d", sfxId, channel );

    // send command to internal thread to play this fx
    xQueueSendToBack( m_hSndQue, &update, 0 ); // do not wait...

    return channel; // return channel to the engine
}

void SoundFXMixer::stop( int channel )
{
    //ESP_LOGI( TAG, "stop: ch: %d", channel );
    m_channels[channel].stop(); // just stop in place, don't send message 
}

void SoundFXMixer::update( int channel, int vol, int sep )
{
    m_channels[channel].update( (uint8_t)vol, (uint8_t)sep );
}

void SoundFXMixer::play_fx()
{
    uint32_t update = 0;
    size_t   bytesSend = 0;

    mix();
    i2s_channel_preload_data( m_txChannel, m_mixBuffer, sizeof(m_mixBuffer), &bytesSend );

    esp_err_t err = i2s_channel_enable( m_txChannel ); // enable i2s channel but still no data to play
    if( err != ESP_OK ) {
        ESP_LOGE( TAG, "i2s_channel_enable error=%d", err );
    }

    while( true )
    {
        if( xQueueReceive( m_hSndQue, &update, 0 ) ) // do not wait 
        {
            uint8_t fxId = update & 0xFF;
            uint8_t ch   = ( update >> 8 ) & 0xFF;
            uint8_t vol  = ( update >> 16 ) & 0xFF;
            uint8_t sep  = ( update >> 24 ) & 0xFF;

            SoundEffect* pFX = &m_effects[fxId];
            m_channels[ch].play( pFX, sep, vol ); // play effect
        }

        mix();

        err = i2s_channel_write( m_txChannel, m_mixBuffer, sizeof(m_mixBuffer), &bytesSend, 1000 );
        if( err != ESP_OK ) {
            ESP_LOGE( TAG, "i2s_channel_write error=%d", err );
        }
    }
}

void SoundFXMixer::mix() // mix active channels if any
{
    if( m_playMusic )
    {
        // read wave file from SD card
        int readBytes = m_wavReader.getPcmData( m_inputBuf, sizeof( m_inputBuf ) );
        if( readBytes <= 0 && m_loopMusic )
        {
            //ESP_LOGI( TAG, "rewinding..." );
            m_wavReader.rewind();
            readBytes = m_wavReader.getPcmData( m_inputBuf, sizeof( m_inputBuf ) );
        }
    } else { // zero input music buffer
        memset( m_inputBuf, 0, sizeof( m_inputBuf ) );
    }

    // mix effects into buffer with music
    for( int k = 0, j = 0; k < MIX_BUF_LEN; k += 2, j++ )
    {
        float musSmp = (float)m_inputBuf[j] / 32767.0f;

        musSmp *= m_musicGain;  // gain correction, this should probaly be linked to music volume variable

        float sL = musSmp - 0.0012345f; // make small diff in left
        float sR = musSmp + 0.0012345f; // and in right :)

        float eL = 0.0f; // effect right and left channel
        float eR = 0.0f;

        for( int i = 0; i < NUM_SFX_CHANNELS; i++ ) // if no active channels, then just music will be sent to driver
        {
            if( m_channels[i].isActive() ) // mix sound from active channels
            {
                float smp = (float)m_channels[i].getSmaple() / 32767.0f; // convert sample to float

                float gainR = m_sepGainTable[ m_channels[i].getGainSep() ]; // get gain for right channel
                float gainL = 1.0f - gainR;                                 // gain for left channel

                float chVol = m_volumeTable[ m_channels[i].getVolume() ];

                eL += smp * gainL * chVol; // mix all active effects with defined gain and volume
                eR += smp * gainR * chVol;
            }
        }

        sL += eL * 0.35f; // gain correction, this should probably be linked to effects level variable
        sR += eR * 0.35f;

        m_limiter.stereoLimit( sL, sR ); // apply limiter

        m_mixBuffer[k  ] = (int16_t)( sL * 32768.0f ); // convert samples back to 16 bit integers
        m_mixBuffer[k+1] = (int16_t)( sR * 32768.0f );
    }
}

void* SoundFXMixer::registerSong( const char* fileName )
{
    char buf[32];
    sprintf( buf, "/sd/doom_mus/%s.wav", fileName );
    result ret = m_wavReader.loadWave( buf );
    ESP_LOGI( TAG, "Open wav file %s ret=%d, dataLen=%d", buf, ret, m_wavReader.getDataLength() );
    return (void*)12345; // return some handle. Engine uses only 1 song at a time.
}

void SoundFXMixer::unRegisterSong( void* handle )
{
    m_wavReader.close();
}

void SoundFXMixer::playSong( void* handle, bool loop )
{
    m_loopMusic = loop;
    m_playMusic = true;
}

void SoundFXMixer::stopSong()
{
    m_playMusic = false;
}

void SoundFXMixer::pauseSong()
{
    m_playMusic = false;
}

void SoundFXMixer::resumeSong()
{
    m_playMusic = true;
}

void SoundFXMixer::setMusicVolume( int vol )
{
    // use the same volume table as for sfx, range 0..127
    m_musicGain = m_volumeTable[vol];
}

int SoundFXMixer::isMusicPlaying()
{
    return (int)m_playMusic;
}
