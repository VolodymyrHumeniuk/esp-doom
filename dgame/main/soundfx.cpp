#include "main.h"
#include "soundfx.h"

static lpcstr TAG = "SFX";

SoundEffect::SoundEffect( int8_t* data, uint32_t len )
{
    pcmData = data;
    dataLen = len;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
SoundFXMixer::SoundFXMixer()
{
    memset( m_mixBuffer, 0, MIX_BUF_LEN * sizeof(int16_t) );
    m_nextChannel = 0;

    result ret = m_wavReader.loadWave( "/sd/doom_mus/runnin.wav" );
    ESP_LOGI( TAG, "Open wav file ret=%d, dataLen=%d", ret, m_wavReader.getDataLength() );
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
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG( TX_SAMPLE_RATE ),
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
    m_effects[fxId] = SoundEffect( (int8_t*)data, (uint32_t)len );
}

int SoundFXMixer::start( int sfxId, int channel, int vol, int sep )
{
    uint8_t id  = sfxId & 0xFF;  // there are 109 effects in doom, so 1 byte should be ok
    uint8_t ch  = m_nextChannel; //channel & 0xFF; // channels only 8
    uint8_t cmd = 1;
    uint32_t update = cmd << 16 | ch << 8 | id;
    
    if( ++m_nextChannel >= NUM_SFX_CHANNELS ) // try to utilize all 8 channels, effects are very short
        m_nextChannel = 0;                    // so this will help every fx to be played from start to finish

    //ESP_LOGI( TAG, "start: sfxId: %d, ch: %d", sfxId, channel );

    // send command to internal thread to play this fx
    xQueueSendToBack( m_hSndQue, &update, 0 ); // do not wait...

    return channel; // return channel to engine
}

void SoundFXMixer::stop( int channel )
{
    uint32_t update = ( channel & 0xFF ) << 8;
    ESP_LOGI( TAG, "stop: ch: %d", channel );
    xQueueSendToBack( m_hSndQue, &update, 0 ); // do not wait...
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
        if( xQueueReceive( m_hSndQue, &update, ms2ticks( 1 ) ) ) // wait 5 ms
        {
            uint8_t fxId = update & 0xFF;
            uint8_t ch   = ( update >> 8 ) & 0xFF;
            uint8_t cmd  = ( update >> 16 ) & 0xFF;

            if( cmd ) // play fx
            {
                SoundEffect* pFX = &m_effects[fxId];
                m_channels[ch].play( pFX );
                //ESP_LOGI( TAG, "play: sfxId: %d, ch: %d", fxId, ch );
            } else { // stop fx
                m_channels[ch].stop();
            }
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
    int readBytes = m_wavReader.getPcmData( m_inputBuf, sizeof( m_inputBuf ) );
    if( readBytes <= 0 )
    {
        ESP_LOGI( TAG, "read PCM: %d bytes, rewinding...", readBytes );
        m_wavReader.rewind();
        readBytes = m_wavReader.getPcmData( m_inputBuf, sizeof( m_inputBuf ) );
    }

    // mix effects into buffer with music
    for( int k = 0, j = 0; k < MIX_BUF_LEN; k += 2, j++ )
    {
        float musSmp = (float)m_inputBuf[j] / 32767.0f;

        musSmp *= 0.25f; // gain correction

        float sL = musSmp - 0.00123f;
        float sR = musSmp + 0.00123f;

        m_mixBuffer[k  ] = (int16_t)( sL * 32768.0f );
        m_mixBuffer[k+1] = (int16_t)( sR * 32768.0f );

        /*int smp = 0; // mix on 32 bit signed int
        int div = 0;
        for( int i = 0; i < NUM_SFX_CHANNELS; i++ ) // if no active channels, then silence will be sent to driver
        {
            if( m_channels[i].isActive() )
            {
                smp += m_channels[i].getSmaple();
                div++;
            }
        }

        m_mixBuffer[k] /= 4; // gain correction ...

        if( div ) {
            m_mixBuffer[k] += (int16_t)( ( smp << 6 ) / div ); // make sample ~16 bit
        }*/
    }
}
