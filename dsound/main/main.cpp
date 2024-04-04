#include "main.h"
#include "es8388.h"
//#include "sd_card.h"

static lpcstr TAG = "MAIN";

#define SAMPLE_RATE     11025

// master MCU sends every time the following number of samples:
#define I2S_BUF_SIZE          ( ( SAMPLE_RATE / 100 ) * 2 * 2 )
// we are going to read every time only half of them
#define I2S_HALPH_BUF_SIZE    ( ( SAMPLE_RATE / 100 ) * 2 )

static i2s_chan_handle_t  rx_chan;        // I2S rx channel handler
static i2s_chan_handle_t  tx_chan;        // I2S tx channel handler
static int16_t            mixBuf[I2S_HALPH_BUF_SIZE]; // 2 channels 16 bit PCM

static void i2s_task( void *args )
{
    ESP_ERROR_CHECK( i2s_channel_enable( rx_chan ) );
    ESP_ERROR_CHECK( i2s_channel_enable( tx_chan ) );

    while( true )
    {
        if( i2s_channel_read( rx_chan, mixBuf, I2S_HALPH_BUF_SIZE * sizeof(int16_t), nullptr, portMAX_DELAY ) == ESP_OK )
        {
            //ESP_LOGI( TAG, "PCM readTask: %d bytes\n", readBytes );
            i2s_channel_write( tx_chan, mixBuf, I2S_HALPH_BUF_SIZE * sizeof(int16_t), nullptr, portMAX_DELAY );
        }
    }
}

static void i2s_init_std_simplex(void)
{
    // init TX channel -- send audio to on-board codec
    i2s_chan_config_t tx_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG( I2S_NUM_0, I2S_ROLE_MASTER );
    ESP_ERROR_CHECK( i2s_new_channel( &tx_chan_cfg, &tx_chan, NULL ) );

    i2s_std_config_t tx_std_cfg = {
    .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG( SAMPLE_RATE ),
    .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG( I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO ),
    .gpio_cfg = {
        .mclk = (gpio_num_t)PIN_I2S_AUDIO_KIT_MCLK,  // some codecs may require mclk signal, this example doesn't need it
        .bclk = (gpio_num_t)PIN_I2S_AUDIO_KIT_BCK,
        .ws   = (gpio_num_t)PIN_I2S_AUDIO_KIT_WS,
        .dout = (gpio_num_t)PIN_I2S_AUDIO_KIT_DATA_OUT,
        .din  = I2S_GPIO_UNUSED, //(gpio_num_t)PIN_I2S_AUDIO_KIT_DATA_IN,
        .invert_flags = {
            .mclk_inv = false,
            .bclk_inv = false,
            .ws_inv   = false,
            },
        },
    };

    ESP_ERROR_CHECK( i2s_channel_init_std_mode( tx_chan, &tx_std_cfg ) );

    // init RX channel -- get data from master MCU
    i2s_chan_config_t rx_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG( I2S_NUM_1, I2S_ROLE_SLAVE );
    ESP_ERROR_CHECK( i2s_new_channel( &rx_chan_cfg, NULL, &rx_chan ) );

    i2s_std_config_t rx_std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG( SAMPLE_RATE ),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG( I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO ),
        .gpio_cfg = {
            .mclk = I2S_SCLK_PIN,    // some codecs may require mclk signal, this example doesn't need it
            .bclk = I2S_BCLK_PIN,
            .ws   = I2S_LRCK_PIN,
            .dout = I2S_GPIO_UNUSED,
            .din  = I2S_DIN_PIN,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };

    ESP_ERROR_CHECK( i2s_channel_init_std_mode( rx_chan, &rx_std_cfg ) );
}

static bool init_codec()
{
    gpio_config_t pin_conf = { // config PA pin
        .pin_bit_mask = 1 << PA_ENABLE_GPIO,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    
    gpio_config( &pin_conf );

    audio_hal_codec_config_t codecCfg;
    codecCfg.adc_input  = AUDIO_HAL_ADC_INPUT_LINE1;    // this doesn't matter, input channel is ignored
    codecCfg.dac_output = AUDIO_HAL_DAC_OUTPUT_ALL;     // output to headphones and to line out
    codecCfg.codec_mode = AUDIO_HAL_CODEC_MODE_DECODE;  // output only
    codecCfg.i2s_iface  = {
        .mode           = AUDIO_HAL_MODE_MASTER,
        .fmt            = AUDIO_HAL_I2S_NORMAL,         // normal I2S format
        .samples        = AUDIO_HAL_11K_SAMPLES,        // doom is all 11025 Hz
        .bits           = AUDIO_HAL_BIT_LENGTH_16BITS   // doom is 8-bit per sample, but coded doesn't support it
    };

    esp_err_t err = es8388_init( &codecCfg );
    if( err != ESP_OK ) {
        ESP_LOGE( TAG, "es8388_init err=%d", err );
        return false;
    }

    err = es8388_config_i2s( codecCfg.codec_mode, &codecCfg.i2s_iface );
    if( err != ESP_OK ) {
        ESP_LOGE( TAG, "es8388_config_i2s err=%d", err );
        return false;
    }

    // the default volume is 20%
    err = es8388_set_voice_volume( 10 );
    if( err != ESP_OK ) {
        ESP_LOGE( TAG, "es8388_set_voice_volume err=%d", err );
        return false;
    }

    // this starts the codec
    err = es8388_ctrl_state( codecCfg.codec_mode, AUDIO_HAL_CTRL_START );
    if( err != ESP_OK ) {
        ESP_LOGE( TAG, "es8388_ctrl_state err=%d", err );
        return false;
    }

    es8388_pa_power( true ); // activate 21 pin, this will enable output to build-in power amp

    ESP_LOGI( TAG, "ES8388 codec init ok" );
    return true;
}

extern "C" {

void app_main(void)
{
    vTaskDelay( ms2ticks(5) ); // wait for normal pins state
 
    // To slow on this board to be usefull...
    //SDCardReader sdCard;
    //sdCard.init( false );
    
    if( !init_codec() )
    {
        return;
    }

    i2s_init_std_simplex();

    xTaskCreatePinnedToCore( i2s_task,  "i2s_task", 4096, NULL, 10, NULL, 0 );
}

}