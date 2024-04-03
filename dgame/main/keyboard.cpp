#include "main.h"
#include "keyboard.h"
#include "doomkeys.h"

static const lpcstr TAG = "KBD";

// Hid keyboard codes map to doom key codes
static const uint8_t g_KMAP[] = {
    0,    // HID_KEY_NO_PRESS                                                        = 0x00,
    0,    // HID_KEY_ROLLOVER                                                        = 0x01,
    0,    // HID_KEY_POST_FAIL                                                       = 0x02,
    0,    // HID_KEY_ERROR_UNDEFINED                                                 = 0x03,
    'a',  // HID_KEY_A                                                               = 0x04,
    'b',  // HID_KEY_B                                                               = 0x05,
    'c',  // HID_KEY_C                                                               = 0x06,
    'd',  // HID_KEY_D                                                               = 0x07,
    'e',  // HID_KEY_E                                                               = 0x08,
    'f',  // HID_KEY_F                                                               = 0x09,
    'g',  // HID_KEY_G                                                               = 0x0A,
    'h',  // HID_KEY_H                                                               = 0x0B,
    'i',  // HID_KEY_I                                                               = 0x0C,
    'j',  // HID_KEY_J                                                               = 0x0D,
    'k',  // HID_KEY_K                                                               = 0x0E,
    'l',  // HID_KEY_L                                                               = 0x0F,
    'm',  // HID_KEY_M                                                               = 0x10,
    'n',  // HID_KEY_N                                                               = 0x11,
    'o',  // HID_KEY_O                                                               = 0x12,
    'p',  // HID_KEY_P                                                               = 0x13,
    'q',  // HID_KEY_Q                                                               = 0x14,
    'r',  // HID_KEY_R                                                               = 0x15,
    's',  // HID_KEY_S                                                               = 0x16,
    't',  // HID_KEY_T                                                               = 0x17,
    'u',  // HID_KEY_U                                                               = 0x18,
    'v',  // HID_KEY_V                                                               = 0x19,
    'w',  // HID_KEY_W                                                               = 0x1A,
    KEY_STRAFE_R ,  // 'x',  // HID_KEY_X                                                               = 0x1B,
    'y',  // HID_KEY_Y                                                               = 0x1C,
    KEY_STRAFE_L,   // 'z',  // HID_KEY_Z                                                               = 0x1D,
    '1',  // HID_KEY_1                                                               = 0x1E,
    '2',  // HID_KEY_2                                                               = 0x1F,
    '3',  // HID_KEY_3                                                               = 0x20,
    '4',  // HID_KEY_4                                                               = 0x21,
    '5',  // HID_KEY_5                                                               = 0x22,
    '6',  // HID_KEY_6                                                               = 0x23,
    '7',  // HID_KEY_7                                                               = 0x24,
    '8',  // HID_KEY_8                                                               = 0x25,
    '9',  // HID_KEY_9                                                               = 0x26,
    '0',  // HID_KEY_0                                                               = 0x27,
    KEY_ENTER,      // HID_KEY_ENTER                                                 = 0x28,
    KEY_ESCAPE,     // HID_KEY_ESC                                                   = 0x29,
    KEY_DEL,        // HID_KEY_DEL                                                   = 0x2A,
    KEY_TAB,        // HID_KEY_TAB                                                   = 0x2B,
    KEY_FIRE,       // HID_KEY_SPACE                                                 = 0x2C,
    KEY_MINUS,      // HID_KEY_MINUS                                                 = 0x2D,
    KEY_EQUALS,     // HID_KEY_EQUAL                                                 = 0x2E,
    0,              // HID_KEY_OPEN_BRACKET                                          = 0x2F,
    0,              // HID_KEY_CLOSE_BRACKET                                         = 0x30,
    0,              // HID_KEY_BACK_SLASH                                            = 0x31,
    0,              // HID_KEY_SHARP                                                 = 0x32,
    0,              // HID_KEY_COLON                                                 = 0x33,
    0,              // HID_KEY_QUOTE                                                 = 0x34,
    0,              // HID_KEY_TILDE                                                 = 0x35,
    0,              // HID_KEY_LESS                                                  = 0x36,
    0,              // HID_KEY_GREATER                                               = 0x37,
    0,              // HID_KEY_SLASH                                                 = 0x38,
    KEY_CAPSLOCK,   // HID_KEY_CAPS_LOCK                                             = 0x39,
    KEY_F1,         // HID_KEY_F1                                                    = 0x3A,
    KEY_F2,         // HID_KEY_F2                                                    = 0x3B,
    KEY_F3,         // HID_KEY_F3                                                    = 0x3C,
    KEY_F4,         // HID_KEY_F4                                                    = 0x3D,
    KEY_F5,         // HID_KEY_F5                                                    = 0x3E,
    KEY_F6,         // HID_KEY_F6                                                    = 0x3F,
    KEY_F7,         // HID_KEY_F7                                                    = 0x40,
    KEY_F8,         // HID_KEY_F8                                                    = 0x41,
    KEY_F9,         // HID_KEY_F9                                                    = 0x42,
    KEY_F10,        // HID_KEY_F10                                                   = 0x43,
    KEY_F11,        // HID_KEY_F11                                                   = 0x44,
    KEY_F12,        // HID_KEY_F12                                                   = 0x45,
    KEY_PRTSCR,     // HID_KEY_PRINT_SCREEN                                          = 0x46,
    KEY_SCRLCK,     // HID_KEY_SCROLL_LOCK                                           = 0x47,
    KEY_PAUSE,      // HID_KEY_PAUSE                                                 = 0x48,
    KEY_INS,        // HID_KEY_INSERT                                                = 0x49,
    KEY_HOME,       // HID_KEY_HOME                                                  = 0x4A,
    KEY_PGUP,       // HID_KEY_PAGEUP                                                = 0x4B,
    KEY_DEL,        // HID_KEY_DELETE                                                = 0x4C,
    KEY_END,        // HID_KEY_END                                                   = 0x4D,
    KEY_PGDN,       // HID_KEY_PAGEDOWN                                              = 0x4E,
    KEY_RIGHTARROW, // HID_KEY_RIGHT   no idea why, but they are inverted in doom    = 0x4F,
    KEY_LEFTARROW,  // HID_KEY_LEFT    no idea why, but they are inverted in doom    = 0x50,
    KEY_DOWNARROW,  // HID_KEY_DOWN    no idea why, but they are inverted in doom    = 0x51,
    KEY_UPARROW,    // HID_KEY_UP      no idea why, but they are inverted in doom    = 0x52,
    KEY_NUMLOCK,    // HID_KEY_NUM_LOCK                                              = 0x53,
    '/',            // HID_KEY_KEYPAD_DIV                                            = 0x54,
    '*',            // HID_KEY_KEYPAD_MUL                                            = 0x55,
    '-',            // HID_KEY_KEYPAD_SUB                                            = 0x56,
    '+',            // HID_KEY_KEYPAD_ADD                                            = 0x57,
    KEY_ENTER,      // HID_KEY_KEYPAD_ENTER                                          = 0x58,
    KEY_END,        // HID_KEY_KEYPAD_1                                              = 0x59,
    KEY_DOWNARROW,  // HID_KEY_KEYPAD_2                                              = 0x5A,
    KEY_PGDN,       // HID_KEY_KEYPAD_3                                              = 0x5B,
    KEY_LEFTARROW,  // HID_KEY_KEYPAD_4                                              = 0x5C,
    KEY_DOWNARROW,  // HID_KEY_KEYPAD_5                                              = 0x5D,
    KEY_RIGHTARROW, // HID_KEY_KEYPAD_6                                              = 0x5E,
    KEY_HOME,       // HID_KEY_KEYPAD_7                                              = 0x5F,
    KEY_UPARROW,    // HID_KEY_KEYPAD_8                                              = 0x60,
    KEY_PGUP,       // HID_KEY_KEYPAD_9                                              = 0x61,
    0,              // HID_KEY_KEYPAD_0                                              = 0x62,
    0,              // HID_KEY_KEYPAD_DELETE                                         = 0x63,
    0,              // HID_KEY_KEYPAD_SLASH                                          = 0x64,
    0,              // HID_KEY_APPLICATION                                           = 0x65,
    0,              // HID_KEY_POWER                                                 = 0x66,
    KEY_EQUALS,     // HID_KEY_KEYPAD_EQUAL                                          = 0x67,
    0,              // HID_KEY_F13                                                   = 0x68,
    0,              // HID_KEY_F14                                                   = 0x69,
    0,              // HID_KEY_F15                                                   = 0x6A,
    0,              // HID_KEY_F16                                                   = 0x6B,
    0,              // HID_KEY_F17                                                   = 0x6C,
    0,              // HID_KEY_F18                                                   = 0x6D,
    0,              // HID_KEY_F19                                                   = 0x6E,
    0,              // HID_KEY_F20                                                   = 0x6F,
    0,              // HID_KEY_F21                                                   = 0x70,
    0,              // HID_KEY_F22                                                   = 0x71,
    0,              // HID_KEY_F23                                                   = 0x72,
    0,              // HID_KEY_F24                                                   = 0x73,
    KEY_ENTER,      // HID_KEY_EXECUTE                                               = 0x74,
    0,              // HID_KEY_HELP                                                  = 0x75,
    0,              // HID_KEY_MENU                                                  = 0x76,
    0,              // HID_KEY_SELECT                                                = 0x77,
    0,              // HID_KEY_STOP                                                  = 0x78,
    0,              // HID_KEY_AGAIN                                                 = 0x79,
    0,              // HID_KEY_UNDO                                                  = 0x7A,
    0,              // HID_KEY_CUT                                                   = 0x7B,
    0,              // HID_KEY_COPY                                                  = 0x7C,
    0,              // HID_KEY_PASTE                                                 = 0x7D,
    0,              // HID_KEY_FIND                                                  = 0x7E,
    0,              // HID_KEY_MUTE                                                  = 0x7F,
    0,              // HID_KEY_VOLUME_UP                                             = 0x80,
    0,              // HID_KEY_VOLUME_DOWN                                           = 0x81,
    0,              // HID_KEY_LOCKING_CAPS_LOCK                                     = 0x82,
    0,              // HID_KEY_LOCKING_NUM_LOCK                                      = 0x83,
    0,              // HID_KEY_LOCKING_SCROLL_LOCK                                   = 0x84,
    0,              // HID_KEY_KEYPAD_COMMA                                          = 0x85,
    0,              // HID_KEY_KEYPAD_EQUAL_SIGN                                     = 0x86,
    0,              // HID_KEY_INTERNATIONAL_1                                       = 0x87,
    0,              // HID_KEY_INTERNATIONAL_2                                       = 0x88,
    0,              // HID_KEY_INTERNATIONAL_3                                       = 0x89,
    0,              // HID_KEY_INTERNATIONAL_4                                       = 0x8A,
    0,              // HID_KEY_INTERNATIONAL_5                                       = 0x8B,
    0,              // HID_KEY_INTERNATIONAL_6                                       = 0x8C,
    0,              // HID_KEY_INTERNATIONAL_7                                       = 0x8D,
    0,              // HID_KEY_INTERNATIONAL_8                                       = 0x8E,
    0,              // HID_KEY_INTERNATIONAL_9                                       = 0x8F,
    0,              // HID_KEY_LANG_1                                                = 0x90,
    0,              // HID_KEY_LANG_2                                                = 0x91,
    0,              // HID_KEY_LANG_3                                                = 0x92,
    0,              // HID_KEY_LANG_4                                                = 0x93,
    0,              // HID_KEY_LANG_5                                                = 0x94,
    0,              // HID_KEY_LANG_6                                                = 0x95,
    0,              // HID_KEY_LANG_7                                                = 0x96,
    0,              // HID_KEY_LANG_8                                                = 0x97,
    0,              // HID_KEY_LANG_9                                                = 0x98,
    0,              // HID_KEY_ALTERNATE_ERASE                                       = 0x99,
    0,              // HID_KEY_SYSREQ                                                = 0x9A,
    KEY_BACKSPACE, // HID_KEY_CANCEL                                                 = 0x9B,
    0, // HID_KEY_CLEAR                                                                 = 0x9C,
    0, // HID_KEY_PRIOR                                                                 = 0x9D,
    KEY_ENTER, // HID_KEY_RETURN                                                                = 0x9E,
    0, // HID_KEY_SEPARATOR                                                             = 0x9F,
    0, // HID_KEY_OUT                                                                   = 0xA0,
    0, // HID_KEY_OPER                                                                  = 0xA1,
    0, // HID_KEY_CLEAR_AGAIN                                                           = 0xA2,
    0, // HID_KEY_CRSEL                                                                 = 0xA3,
    0, // HID_KEY_EXSEL                                                                 = 0xA4,
    0, // --                                                                              0xA5,
    0, // --                                                                              0xA6,
    0, // --                                                                              0xA7,
    0, // --                                                                              0xA8,
    0, // --                                                                              0xA9,
    0, // --                                                                              0xAA,
    0, // --                                                                              0xAB,
    0, // --                                                                              0xAC,
    0, // --                                                                              0xAD,
    0, // --                                                                              0xAE,
    0, // --                                                                              0xAF,
    0, // HID_KEY_KEYPAD_00                                                             = 0xB0,
    0, // HID_KEY_KEYPAD_000                                                            = 0xB1,
    0, // HID_KEY_THOUSANDS_SEPARATOR                                                   = 0xB2,
    0, // HID_KEY_DECIMAL_SEPARATOR                                                     = 0xB3,
    0, // HID_KEY_CURRENCY_UNIT                                                         = 0xB4,
    0, // HID_KEY_CURRENCY_SUB_UNIT                                                     = 0xB5,
    0, // HID_KEY_KEYPAD_OPEN_PARENTHESIS                                               = 0xB6,
    0, // HID_KEY_KEYPAD_CLOSE_PARENTHESIS                                              = 0xB7,
    0, // HID_KEY_KEYPAD_OPEN_BRACE                                                     = 0xB8,
    0, // HID_KEY_KEYPAD_CLOSE_BRACE                                                    = 0xB9,
    0, // HID_KEY_KEYPAD_TAB                                                            = 0xBA,
    KEY_BACKSPACE, // HID_KEY_KEYPAD_BACKSPACE                                          = 0xBB,
    0, // HID_KEY_KEYPAD_A                                                              = 0xBC,
    0, // HID_KEY_KEYPAD_B                                                              = 0xBD,
    0, // HID_KEY_KEYPAD_C                                                              = 0xBE,
    0, // HID_KEY_KEYPAD_D                                                              = 0xBF,
    0, // HID_KEY_KEYPAD_E                                                              = 0xC0,
    0, // HID_KEY_KEYPAD_F                                                              = 0xC1,
    0, // HID_KEY_KEYPAD_XOR                                                            = 0xC2,
    0, // HID_KEY_KEYPAD_CARET                                                          = 0xC3,
    0, // HID_KEY_KEYPAD_PERCENT                                                        = 0xC4,
    0, // HID_KEY_KEYPAD_LESSER                                                         = 0xC5,
    0, // HID_KEY_KEYPAD_GREATER                                                        = 0xC6,
    0, // HID_KEY_KEYPAD_AND                                                            = 0xC7,
    0, // HID_KEY_KEYPAD_LOGICAL_AND                                                    = 0xC8,
    0, // HID_KEY_KEYPAD_OR                                                             = 0xC9,
    0, // HID_KEY_KEYPAD_LOGICAL_OR                                                     = 0xCA,
    0, // HID_KEY_KEYPAD_COLON                                                          = 0xCB,
    0, // HID_KEY_KEYPAD_SHARP                                                          = 0xCC,
    0, // HID_KEY_KEYPAD_SPACE                                                          = 0xCD,
    0, // HID_KEY_KEYPAD_AT                                                             = 0xCE,
    0, // HID_KEY_KEYPAD_BANG                                                           = 0xCF,
    0, // HID_KEY_KEYPAD_MEMORY_STORE                                                   = 0xD0,
    0, // HID_KEY_KEYPAD_MEMORY_RECALL                                                  = 0xD1,
    0, // HID_KEY_KEYPAD_MEMORY_CLEAD                                                   = 0xD2,
    0, // HID_KEY_KEYPAD_MEMORY_ADD                                                     = 0xD3,
    0, // HID_KEY_KEYPAD_MEMORY_SUBSTRACT                                               = 0xD4,
    0, // HID_KEY_KEYPAD_MEMORY_MULTIPLY                                                = 0xD5,
    0, // HID_KEY_KEYPAD_MEMORY_DIVIDE                                                  = 0xD6,
    0, // HID_KEY_KEYPAD_SIGN                                                           = 0xD7,
    0, // HID_KEY_KEYPAD_CLEAR                                                          = 0xD8,
    0, // HID_KEY_KEYPAD_CLEAR_ENTRY                                                    = 0xD9,
    0, // HID_KEY_KEYPAD_BINARY                                                         = 0xDA,
    0, // HID_KEY_KEYPAD_OCTAL                                                          = 0xDB,
    0, // HID_KEY_KEYPAD_DECIMAL                                                        = 0xDC,
    0, // HID_KEY_KEYPAD_HEXADECIMAL                                                    = 0xDD,
    0, // --                                                                              0xDE,
    0, // --                                                                              0xDF,
    KEY_USE,     //KEY_RCTRL,  // HID_KEY_LEFT_CONTROL  (also HID_KEY_RIGHT_CONTROL in std spec)       = 0xE0,
    KEY_RSHIFT, // HID_KEY_LEFT_SHIFT    (also HID_KEY_RIGHT_SHIFT   in std spec)       = 0xE1,
    KEY_FIRE,   // KEY_LALT,   // HID_KEY_LEFT_ALT      (also HID_KEY_RIGHT_ALT     in std spec)       = 0xE2,
    0,          // HID_KEY_LEFT_GUI      (also HID_KEY_RIGHT_GUI     in std spec)       = 0xE3,
    KEY_RCTRL,  // D_HID_KEY_RIGHT_CONTROL  (custom defs for Doom)                        0xE4,
    KEY_RSHIFT, // D_HID_KEY_RIGHT_SHIFT    (custom defs for Doom)                        0xE5,
    KEY_RALT,   // D_HID_KEY_RIGHT_ALT      (custom defs for Doom)                        0xE6,
    0, // D_HID_KEY_RIGHT_GUI      (custom defs for Doom)                                 0xE7,
    0, // --                                                                              0xE8,
    0, // --                                                                              0xE9,
    0, // --                                                                              0xEA,
    0, // --                                                                              0xEB,
    0, // --                                                                              0xEC,
    0, // --                                                                              0xED,
    0, // --                                                                              0xEE,
    0, // --                                                                              0xEF,
    0, // --                                                                              0xF0,
    0, // --                                                                              0xF1,
    0, // --                                                                              0xF2,
    0, // --                                                                              0xF3,
    0, // --                                                                              0xF4,
    0, // --                                                                              0xF5,
    0, // --                                                                              0xF6,
    0, // --                                                                              0xF7,
    0, // --                                                                              0xF8,
    0, // --                                                                              0xF9,
    0, // --                                                                              0xFA,
    0, // --                                                                              0xFB,
    0, // --                                                                              0xFC,
    0, // --                                                                              0xFD,
    0, // --                                                                              0xFE,
    0, // --                                                                              0xFF,
};

// warning: comes from internal hid thread
void Keyboard::OnKeyboardEventReceived( key_event_t* pKeyEvent )
{
    //ESP_LOGI( TAG, "Key: 0x%02X, state: %d, mod: %u", pKeyEvent->key_code, pKeyEvent->state, pKeyEvent->modifier );

    uint8_t key = g_KMAP[pKeyEvent->key_code];
    if( key > 0 )
    {
        uint16_t keyInfo = pKeyEvent->state << 8 | key;
        xQueueSendToBack( m_hKBQue, &keyInfo, 0 ); // insert key into que, do not wait
    }
}

uint16_t Keyboard::getKey()
{
    uint16_t key = 0;
    xQueueReceive( m_hKBQue, &key, 0 ); // do not wait, just grab key if it is already in que
    return key;
}

void Keyboard::init()
{
    m_hKBQue = xQueueCreate( KEYQUEUE_SIZE, sizeof(uint16_t) );

    hid_init( this );
}