/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "esp_err.h"
#include "esp_log.h"
#include "usb/usb_host.h"
#include "errno.h"
#include "driver/gpio.h"

#include "usb/hid_host.h"
#include "usb/hid_usage_keyboard.h"
#include "usb/hid_usage_mouse.h"
#include "main.h"
#include "hid_kbd_host.h"

static const char *TAG = "HID";
static QueueHandle_t hid_host_event_queue;

/**
 * @brief HID Host event
 * This event is used for delivering the HID Host event from callback to a task.
 */
typedef struct {
    hid_host_device_handle_t hid_device_handle;
    hid_host_driver_event_t event;
    void *arg;
} hid_host_event_queue_t;

/**
 * @brief HID Protocol string names
 */
static const char *hid_proto_name_str[] = {
    "NONE",
    "KEYBOARD",
    "MOUSE"
};

static HIDNotifySink* g_pSink = nullptr;

/**
 * @brief Key buffer scan code search.
 * @param[in] src       Pointer to source buffer where to search
 * @param[in] key       Key scancode to search
 * @param[in] length    Size of the source buffer
 */
static inline bool key_found( const uint8_t *const src, uint8_t key, unsigned int length )
{
    for( unsigned int i = 0; i < length; i++ ) {
        if (src[i] == key) {
            return true;
        }
    }
    return false;
}

static uint8_t modsState = 0; // modifiers

static void check_modifier( uint8_t mods, uint8_t bit, uint8_t code )
{
    key_event_t key_event;

    if( mods & bit ) // modifier key pressed
    {
        if( !( modsState & bit ) ) { // send event
            key_event.key_code = code;
            key_event.state = KEY_STATE_PRESSED;
            key_event.modifier = bit;
            g_pSink->OnKeyboardEventReceived( &key_event );
            modsState |= bit;  // mark bit
        }
    } else { // modifier key was not pressed was not pressed
        if( modsState & bit ) { // but if press was reistered
            key_event.key_code = code;
            key_event.state = KEY_STATE_RELEASED;
            key_event.modifier = bit;
            g_pSink->OnKeyboardEventReceived( &key_event );
            modsState &= ~bit; // remove bit
        }
    }
}

/**
 * @brief USB HID Host Keyboard Interface report callback handler
 * @param[in] data    Pointer to input report data buffer
 * @param[in] length  Length of input report data buffer
 */
static void hid_host_keyboard_report_callback( const uint8_t *const data, const int length )
{
    hid_keyboard_input_report_boot_t *kb_report = (hid_keyboard_input_report_boot_t *)data;

    if( length < sizeof( hid_keyboard_input_report_boot_t ) ) {
        return;
    }
    
    static uint8_t prev_keys[HID_KEYBOARD_KEY_MAX] = { 0 };
    key_event_t key_event;
    
    // check modifiers first. HID kbd doesn't send them as key-codes, but Doom expects them as keys that can be pressed/released
    check_modifier( kb_report->modifier.val, LEFT_CTRL,   HID_KEY_LEFT_CONTROL );
    check_modifier( kb_report->modifier.val, LEFT_SHIFT,  HID_KEY_LEFT_SHIFT );
    check_modifier( kb_report->modifier.val, LEFT_ALT,    HID_KEY_LEFT_ALT );
    //check_modifier( kb_report->modifier.val, LEFT_GUI,    HID_KEY_LEFT_GUI );
    check_modifier( kb_report->modifier.val, RIGHT_CTRL,  D_HID_KEY_RIGHT_CONTROL );
    check_modifier( kb_report->modifier.val, RIGHT_SHIFT, D_HID_KEY_RIGHT_SHIFT );
    check_modifier( kb_report->modifier.val, RIGHT_ALT,   D_HID_KEY_RIGHT_ALT );
    //check_modifier( kb_report->modifier.val, RIGHT_GUI,   D_HID_KEY_RIGHT_GUI );

    for( int i = 0; i < HID_KEYBOARD_KEY_MAX; i++ )
    {
        // key has been released verification
        if( prev_keys[i] > HID_KEY_ERROR_UNDEFINED && !key_found(kb_report->key, prev_keys[i], HID_KEYBOARD_KEY_MAX ) )
        {
            key_event.key_code = prev_keys[i];
            key_event.state = KEY_STATE_RELEASED;
            key_event.modifier = kb_report->modifier.val;
            g_pSink->OnKeyboardEventReceived( &key_event );
        }

        // key has been pressed verification
        if( kb_report->key[i] > HID_KEY_ERROR_UNDEFINED && !key_found(prev_keys, kb_report->key[i], HID_KEYBOARD_KEY_MAX ) )
        {
            key_event.key_code = kb_report->key[i];
            key_event.state = KEY_STATE_PRESSED;
            key_event.modifier = kb_report->modifier.val;
            g_pSink->OnKeyboardEventReceived( &key_event );
        }
    }

    memcpy( prev_keys, &kb_report->key, HID_KEYBOARD_KEY_MAX );
}

/**
 * @brief USB HID Host Mouse Interface report callback handler
 *
 * @param[in] data    Pointer to input report data buffer
 * @param[in] length  Length of input report data buffer
 */
static void hid_host_mouse_report_callback(const uint8_t *const data, const int length)
{
    hid_mouse_input_report_boot_t *mouse_report = (hid_mouse_input_report_boot_t *)data;

    if (length < sizeof(hid_mouse_input_report_boot_t)) {
        return;
    }

    static int x_pos = 0;
    static int y_pos = 0;

    // Calculate absolute position from displacement
    x_pos += mouse_report->x_displacement;
    y_pos += mouse_report->y_displacement;

    printf( "X: %06d\tY: %06d\t|%c|%c|\r", x_pos, y_pos, (mouse_report->buttons.button1 ? 'o' : ' '), (mouse_report->buttons.button2 ? 'o' : ' ') );
    fflush(stdout);
}

/**
 * @brief USB HID Host Generic Interface report callback handler
 *
 * 'generic' means anything else than mouse or keyboard
 *
 * @param[in] data    Pointer to input report data buffer
 * @param[in] length  Length of input report data buffer
 */
static void hid_host_generic_report_callback(const uint8_t *const data, const int length)
{
    for (int i = 0; i < length; i++) {
        printf("%02X", data[i]);
    }
    putchar('\r');
}

/**
 * @brief USB HID Host interface callback
 * @param[in] hid_device_handle  HID Device handle
 * @param[in] event              HID Host interface event
 * @param[in] arg                Pointer to arguments, does not used
 */
void hid_host_interface_callback( hid_host_device_handle_t hid_device_handle, const hid_host_interface_event_t event, void *arg )
{
    uint8_t data[64] = { 0 };
    size_t data_length = 0;
    hid_host_dev_params_t dev_params;
    esp_err_t err = hid_host_device_get_params( hid_device_handle, &dev_params );
    if( err == ESP_OK )
    {
        switch( event ) {
        case HID_HOST_INTERFACE_EVENT_INPUT_REPORT:
            hid_host_device_get_raw_input_report_data( hid_device_handle, data, 64, &data_length );

            if( HID_SUBCLASS_BOOT_INTERFACE == dev_params.sub_class ) {
                if( HID_PROTOCOL_KEYBOARD == dev_params.proto ) {
                    hid_host_keyboard_report_callback( data, data_length );
                } else if ( HID_PROTOCOL_MOUSE == dev_params.proto ) {
                    hid_host_mouse_report_callback( data, data_length );
                }
            } else {
                hid_host_generic_report_callback( data, data_length );
            }
            break;

        case HID_HOST_INTERFACE_EVENT_DISCONNECTED:
            ESP_LOGI( TAG, "HID Device, protocol '%s' DISCONNECTED", hid_proto_name_str[dev_params.proto] );
            hid_host_device_close( hid_device_handle );
            break;
        case HID_HOST_INTERFACE_EVENT_TRANSFER_ERROR:
            ESP_LOGI( TAG, "HID Device, protocol '%s' TRANSFER_ERROR", hid_proto_name_str[dev_params.proto] );
            break;
        default:
            ESP_LOGE( TAG, "HID Device, protocol '%s' Unhandled event", hid_proto_name_str[dev_params.proto] );
            break;
        }
    }
}

/**
 * @brief USB HID Host Device event
 * @param[in] hid_device_handle  HID Device handle
 * @param[in] event              HID Host Device event
 * @param[in] arg                Pointer to arguments, does not used
 */
void hid_host_device_event( hid_host_device_handle_t hid_device_handle, const hid_host_driver_event_t event, void *arg )
{
    hid_host_dev_params_t dev_params;
    esp_err_t err = hid_host_device_get_params( hid_device_handle, &dev_params );

    if( event == HID_HOST_DRIVER_EVENT_CONNECTED )
    {
        ESP_LOGI( TAG, "HID Device, protocol '%s' CONNECTED", hid_proto_name_str[dev_params.proto] );

        const hid_host_device_config_t dev_config = {
            .callback = hid_host_interface_callback,
            .callback_arg = NULL
        };

/* Some keyboards give this:
 I (1045252) HID: HID Device, protocol 'KEYBOARD' CONNECTED
 I (1045252) HID: HID Device, protocol 'NONE' CONNECTED
 E (1045252) HCD DWC: bInterval value (255) of Interrupt pipe exceeds max supported limit
 E (1045262) hid-host: hid_host_interface_claim_and_prepare_transfer(622): Unable to claim Interface
 E (1045272) hid-host: hid_host_device_open(1212): Unable to claim interface
 E (1045282) HID: hid_host_device_open() -> failed, err=1006985543err
 
 The file with source code is here: C:\esp\esp-idf\components\usb\hcd_dwc.c  (IDF 5.1.2)
 The only reading that I've found on this is here: https://github.com/espressif/esp-idf/issues/12336
 The exaplnation what is BInterval is here: https://microchip.my.site.com/s/article/What-is-bInterval
 
 Current solution: don't care, main device is still working, just ignore it :)
*/
        if( dev_params.proto == 0 ) // protocol NONE
        {
            ESP_LOGI( TAG, "IGNORING strange device" );
            return;
        }

        err = hid_host_device_open( hid_device_handle, &dev_config );
        if( err == ESP_OK )
        {
            if( HID_SUBCLASS_BOOT_INTERFACE == dev_params.sub_class )
            {
                err = hid_class_request_set_protocol( hid_device_handle, HID_REPORT_PROTOCOL_BOOT );
                if( err == ESP_OK )
                {
                    if( HID_PROTOCOL_KEYBOARD == dev_params.proto ) {
                        err = hid_class_request_set_idle( hid_device_handle, 0, 0 );
                        if( err != ESP_OK ) {
                            ESP_LOGE( TAG, "hid_class_request_set_idle() -> failed, err=%derr" );                
                        }
                    }
                } else {
                    ESP_LOGE( TAG, "hid_class_request_set_protocol() -> failed, err=%derr" );        
                }
            }

            err = hid_host_device_start( hid_device_handle );
            if( err != ESP_OK ) {
                ESP_LOGE( TAG, "hid_host_device_start() -> failed, err=%derr" );    
            }
        } else {
            ESP_LOGE( TAG, "hid_host_device_open() -> failed, err=%derr" );
        }
    }
}

/**
 * @brief Start USB Host install and handle common USB host library events while app pin not low
 *
 * @param[in] arg  Not used
 */
static void usb_monitor_task( void *arg )
{
    const usb_host_config_t host_config = {
        .skip_phy_setup = false,
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
    };

    ESP_ERROR_CHECK( usb_host_install( &host_config ) );
    xTaskNotifyGive( (TaskHandle_t)arg );

    while( true )
    {
        uint32_t event_flags;
        usb_host_lib_handle_events( portMAX_DELAY, &event_flags );

        // Release devices once all clients has deregistered
        if( event_flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS ) {
            usb_host_device_free_all();
            ESP_LOGI(TAG, "USB Event flags: NO_CLIENTS");
        }
        // All devices were removed
        if( event_flags & USB_HOST_LIB_EVENT_FLAGS_ALL_FREE ) {
            ESP_LOGI(TAG, "USB Event flags: ALL_FREE");
        }
    }
}

// Creates queue and get new event from the queue
void hid_host_task( void *pvParameters )
{
    hid_host_event_queue_t evt_queue;
    // Create queue for 10 events
    hid_host_event_queue = xQueueCreate( 10, sizeof(hid_host_event_queue_t) );

    while( true ) // Wait queue for incomming events
    {
        if( xQueueReceive( hid_host_event_queue, &evt_queue, pdMS_TO_TICKS(50)) )
        {
            hid_host_device_event( evt_queue.hid_device_handle, evt_queue.event, evt_queue.arg );
        }
    }
    // never reached
    xQueueReset( hid_host_event_queue );
    vQueueDelete( hid_host_event_queue );
    vTaskDelete( NULL );
}

// HID Host Device callback
// Puts new HID Device event to the queue
static void hid_host_device_callback(hid_host_device_handle_t hid_device_handle,
                              const hid_host_driver_event_t event,
                              void *arg)
{
    const hid_host_event_queue_t evt_queue = {
        .hid_device_handle = hid_device_handle,
        .event = event,
        .arg = arg
    };
    xQueueSend( hid_host_event_queue, &evt_queue, 0 );
}

void hid_init( HIDNotifySink* pSink )
{
    g_pSink = pSink;
    
    BaseType_t task_created;
    ESP_LOGI( TAG, "HID Host ");

    /*
    * Create usb_monitor_task to:
    * - initialize USB Host library
    * - Handle USB Host events
    */
    task_created = xTaskCreatePinnedToCore( usb_monitor_task, "usb_events", 4096, xTaskGetCurrentTaskHandle(), 2, NULL, 0 );
    assert(task_created == pdTRUE);

    // Wait for notification from usb_monitor_task to proceed
    ulTaskNotifyTake( false, 1000 );

    /*
    * HID host driver configuration
    * - create background task for handling low level event inside the HID driver
    * - provide the device callback to get new HID Device connection event
    */
    const hid_host_driver_config_t hid_host_driver_config = {
        .create_background_task = true,
        .task_priority = 5,
        .stack_size = 4096,
        .core_id = 0,
        .callback = hid_host_device_callback,
        .callback_arg = NULL
    };

    ESP_ERROR_CHECK( hid_host_install( &hid_host_driver_config ) );

    /*
    * Create HID Host task process for handle events
    * IMPORTANT: Task is necessary here while there is no possibility to interact
    * with USB device from the callback.
    */
    task_created = xTaskCreate( &hid_host_task, "hid_task", 4 * 1024, NULL, 2, NULL );
    assert( task_created == pdTRUE );
}
