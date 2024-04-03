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
#include <driver/i2s_std.h>
#include <esp_vfs_fat.h>
#include <sdmmc_cmd.h>
#include <driver/sdmmc_host.h>
#include <dirent.h>
#include "ai_thinker_es8388.h"
#include "sdkconfig.h"

typedef uint8_t     BYTE;
typedef uint8_t     byte;
typedef uint16_t    WORD;
typedef uint16_t    word;
typedef uint32_t    DWORD;
typedef uint32_t    dword;
typedef int         result;
typedef const char* lpcstr;

#define ms2ticks(x)  (pdMS_TO_TICKS(x))

#define I2S_SCLK_PIN      GPIO_NUM_18 // GPIO_NUM_46
#define I2S_BCLK_PIN      GPIO_NUM_5  // GPIO_NUM_14
#define I2S_DIN_PIN       GPIO_NUM_13 // GPIO_NUM_3
#define I2S_LRCK_PIN      GPIO_NUM_15  // GPIO_NUM_8
