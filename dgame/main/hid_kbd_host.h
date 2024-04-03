
#pragma once

// Key event
enum key_state {
    KEY_STATE_RELEASED = 0,
    KEY_STATE_PRESSED  = 1,
};

typedef struct {
    key_state   state;
    uint8_t     modifier;
    uint8_t     key_code;
} key_event_t;

// modifiers bits
#define LEFT_CTRL    ( 1 << 0 )
#define LEFT_SHIFT   ( 1 << 1 )
#define LEFT_ALT     ( 1 << 2 )
#define LEFT_GUI     ( 1 << 3 )
#define RIGHT_CTRL   ( 1 << 4 )
#define RIGHT_SHIFT  ( 1 << 5 )
#define RIGHT_ALT    ( 1 << 6 )
#define RIGHT_GUI    ( 1 << 7 )

// This is non-standard modifier codes, just for Doom:
#define D_HID_KEY_RIGHT_CONTROL 0xE4
#define D_HID_KEY_RIGHT_SHIFT   0xE5
#define D_HID_KEY_RIGHT_ALT     0xE6
#define D_HID_KEY_RIGHT_GUI     0xE7


class HIDNotifySink
{
public:
    virtual void OnKeyboardEventReceived( key_event_t* pKeyEvent ) = 0;
};

void hid_init( HIDNotifySink* pSink );
