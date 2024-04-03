#pragma once
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <esp_vfs_fat.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_system.h>
#include <esp_heap_caps.h>
#include <esp_timer.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <sdmmc_cmd.h>
#include <driver/sdmmc_host.h>
#include <dirent.h>
#include "driver/i2s_std.h"
#include "hid_kbd_host.h"
#include "sdkconfig.h"

typedef uint8_t     BYTE;
typedef uint8_t     byte;
typedef uint16_t    WORD;
typedef uint16_t    word;
typedef uint32_t    DWORD;
typedef uint32_t    dword;
typedef int         result;
typedef const char* lpcstr;

#define RED    0xf800
#define GREEN  0x07e0
#define BLUE   0x001f
#define BLACK  0x0000
#define WHITE  0xffff
#define GRAY   0x8c51
#define YELLOW 0xFFE0
#define CYAN   0x07FF
#define PURPLE 0xF81F

// LCD pins
#define LCD_RST GPIO_NUM_9
#define LCD_CS  GPIO_NUM_10
#define LCD_RS  GPIO_NUM_11
#define LCD_WR  GPIO_NUM_12
#define LCD_RD  GPIO_NUM_13

#define LCD_D0  GPIO_NUM_17
#define LCD_D1  GPIO_NUM_18     
#define LCD_D2  GPIO_NUM_4
#define LCD_D3  GPIO_NUM_5
#define LCD_D4  GPIO_NUM_6
#define LCD_D5  GPIO_NUM_7
#define LCD_D6  GPIO_NUM_15
#define LCD_D7  GPIO_NUM_16

#define IDS_CLK_FREQ (16 * 1000 * 1000)

// card pins
#define SD_CARD_PIN_CD    GPIO_NUM_48
#define SD_CARD_PIN_D1    GPIO_NUM_39
#define SD_CARD_PIN_D2    GPIO_NUM_40
#define SD_CARD_PIN_D3    GPIO_NUM_41
#define SD_CARD_PIN_CLK   GPIO_NUM_42
#define SD_CARD_PIN_CMD   GPIO_NUM_2
#define SD_CARD_PIN_D0    GPIO_NUM_1

// PCM5102 pins
#define I2S_SCLK_PIN      GPIO_NUM_46
#define I2S_BCLK_PIN      GPIO_NUM_14
#define I2S_DOUT_PIN      GPIO_NUM_3
#define I2S_LRCK_PIN      GPIO_NUM_8

#define ms2ticks(x)  (pdMS_TO_TICKS(x))

