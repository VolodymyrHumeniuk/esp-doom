#pragma once
#include "main.h"

#define MAX_PATH 256

class SDCardReader
{
    sdmmc_card_t *m_pCard = nullptr;

    lpcstr mount_point = "/sd";
    volatile int cardPinState;

    static void card_detect_isr( void* pArg ) {
        ((SDCardReader*)pArg)->cardPinState = gpio_get_level( SD_CARD_PIN_CD );
    }

    static void task_card_detect( void* pArg ) {
        ((SDCardReader*)pArg)->card_detect();
    }

    bool isCardInserted() const {
        return gpio_get_level( SD_CARD_PIN_CD ) == 0 ? true : false;
    }

    void card_detect()
    {
        int currentPinState = cardPinState;
        ESP_LOGI( __FUNCTION__, "Card monitor task started\n" );
        if( m_pCard ) {// test
            //list_dir( "/sd/" );
        }

        while( true )
        {
            cardPinState = gpio_get_level( SD_CARD_PIN_CD );
            if( currentPinState != cardPinState )
            {
                currentPinState = cardPinState;
                if( currentPinState == 0 ) {
                    ESP_LOGI( __FUNCTION__, "Card inserted\n" );
                    if( !m_pCard ) {
                        mount_fs();
                    }
                    if( m_pCard ) {
                        //list_dir( "/sd/" );
                    }
                } else {
                    ESP_LOGI( __FUNCTION__, "Card removed\n" );
                    unmount_fs();
                }
            }

            vTaskDelay( ms2ticks( 100 ) );
        }
    }

    void list_dir( const char* path )
    {
        char path_buf[MAX_PATH];

        DIR* root = opendir( path );
        if( root )
        {
            ESP_LOGI( __FUNCTION__, "Entering directory: %s\n", path );
            
            // Iterate over all files / folders and fetch their names and sizes
            struct dirent *entry = NULL;

            while( ( entry = readdir( root ) ) != NULL )
            {
                if( entry->d_type == DT_DIR )
                {
                    ESP_LOGI( __FUNCTION__, "  DIR: %s\n", entry->d_name );
                    strcpy( path_buf, path );
                    strcat( path_buf, entry->d_name );
                    strcat( path_buf, "/" );
                    list_dir( path_buf );
                } else {
                    struct stat fileStat;
                    strcpy( path_buf, path );
                    strcat( path_buf, entry->d_name );
                    if( stat( path_buf, &fileStat ) == -1 ) {
                        ESP_LOGI( __FUNCTION__, "Failed to stat %s : %s\n", path_buf, entry->d_name );
                        continue;
                    }
                    ESP_LOGI( __FUNCTION__, "  FILE: %s  size: %ld\n", entry->d_name, fileStat.st_size );
                }
            }
            closedir( root );
        } else {
            ESP_LOGI( __FUNCTION__, "Failed to open dir: %s\n", path );
        }
    }

    void test_file_read_speed( const char* path, uint32_t chunkLen )
    {
        uint8_t* buf = (uint8_t*)malloc( chunkLen );
        if( !buf ) {
            ESP_LOGI( __FUNCTION__, "Memory allocation error!\n" );
            return;
        }

        FILE* fp = fopen( path, "r" );
        if( fp )
        {
            struct stat fileStat;
            stat( path, &fileStat );
            uint32_t len = fileStat.st_size;
            uint32_t flen = len;
            uint32_t start = esp_timer_get_time();
            uint32_t readAvg = 0;
            while( len )
            {
                size_t toRead = len;
                if( toRead > chunkLen ) {
                    toRead = chunkLen;
                }
                uint32_t readStart = esp_timer_get_time();
                fread( buf, 1, toRead, fp );
                uint32_t readEnd = esp_timer_get_time();
                readAvg = ( readAvg + (readEnd - readStart) ) / 2;
                len -= toRead;
            }

            uint32_t end = esp_timer_get_time() - start;
            ESP_LOGI( __FUNCTION__, "%ld bytes read for %.2f ms (%ld mcs) chunk size: %ld, readAvg=%ld mcs, %.2f\n", 
                            flen, (float)end / 1000.0f, end, chunkLen, readAvg, (float)readAvg/(float)chunkLen );
            fclose( fp );
        } else {
            ESP_LOGI( __FUNCTION__, "Failed to open file for reading\n" );
        }

        free( buf );
        ESP_LOGI( __FUNCTION__, "----------------------------------------------------\n" );
    }

    void mount_fs()
    {
        // Options for mounting the filesystem.
        esp_vfs_fat_sdmmc_mount_config_t mount_config = {
            .format_if_mount_failed = false,
            .max_files = 5,
            .allocation_unit_size = 16 * 1024
        };

        // By default, SD card frequency is initialized to SDMMC_FREQ_DEFAULT (20MHz)
        // For setting a specific frequency, use host.max_freq_khz (range 400kHz - 40MHz for SDMMC)
        // Example: for fixed frequency of 10MHz, use host.max_freq_khz = 10000;
        sdmmc_host_t host = SDMMC_HOST_DEFAULT();
        host.max_freq_khz = SDMMC_FREQ_HIGHSPEED; // try to init in high speed mode
        // This initializes the slot without card detect (CD) and write protect (WP) signals.
        // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
        sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
        slot_config.width = 4;

        slot_config.clk = SD_CARD_PIN_CLK;
        slot_config.cmd = SD_CARD_PIN_CMD;
        slot_config.d0  = SD_CARD_PIN_D0;
        slot_config.d1  = SD_CARD_PIN_D1;
        slot_config.d2  = SD_CARD_PIN_D2;
        slot_config.d3  = SD_CARD_PIN_D3;
    
        gpio_set_pull_mode( SD_CARD_PIN_CMD, GPIO_PULLUP_ONLY );   // CMD, needed in 4- and 1- line modes
        gpio_set_pull_mode( SD_CARD_PIN_D0, GPIO_PULLUP_ONLY );   // D0, needed in 4- and 1-line modes
        gpio_set_pull_mode( SD_CARD_PIN_D1, GPIO_PULLUP_ONLY );   // D1, needed in 4-line mode only
        gpio_set_pull_mode( SD_CARD_PIN_D2, GPIO_PULLUP_ONLY );   // D2, needed in 4-line mode only
        gpio_set_pull_mode( SD_CARD_PIN_D3, GPIO_PULLUP_ONLY );   // D3, needed in 4- and 1-line modes

        // Enable internal pullups on enabled pins. The internal pullups
        // are insufficient however, please make sure 10k external pullups are
        // connected on the bus.
        slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

        ESP_LOGI( __FUNCTION__, "Mounting filesystem\n" );
        
        esp_err_t ret = esp_vfs_fat_sdmmc_mount( mount_point, &host, &slot_config, &mount_config, &m_pCard );
        if( ret != ESP_OK )
        {
            ESP_LOGI( __FUNCTION__, "Failed to initialize SDCard (%s)\n", esp_err_to_name( ret ) );
            m_pCard = nullptr;
        } else {
            ESP_LOGI( __FUNCTION__, "SD card Filesystem mounted\n" );
            // Card has been initialized, print its properties
            sdmmc_card_print_info( stdout, m_pCard );
        }
    }

    void unmount_fs()
    {
        if( m_pCard ) { // unmount partition and disable SDMMC peripheral
            esp_vfs_fat_sdcard_unmount( mount_point, m_pCard );
            m_pCard = nullptr;
            ESP_LOGI( __FUNCTION__, "SDCard unmounted\n" );
        }
    }

public:
    void init()
    {
        cardPinState = gpio_get_level( SD_CARD_PIN_CD ); // get state of the pin
        if( cardPinState == 0 ) { // card inserted
            mount_fs(); // try to mount fs
        }

        // register card-detect interrupt, no matter was it initialized or not
        gpio_install_isr_service( 0 );

        gpio_config_t gpio_conf;
        gpio_conf.intr_type = GPIO_INTR_ANYEDGE;
        gpio_conf.mode = GPIO_MODE_INPUT;
        gpio_conf.pin_bit_mask = ( 1ULL << SD_CARD_PIN_CD );
        gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        gpio_conf.pull_up_en = GPIO_PULLUP_ENABLE;
        gpio_config( &gpio_conf );

        gpio_isr_handler_add( SD_CARD_PIN_CD, &card_detect_isr, this );

        xTaskCreate( &task_card_detect, "SDCard", 4096, this, 2, NULL );
    }
};