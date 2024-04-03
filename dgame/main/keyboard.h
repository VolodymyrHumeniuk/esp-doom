#pragma once
#include "main.h"

#define KEYQUEUE_SIZE 16

class Keyboard : public HIDNotifySink
{
    QueueHandle_t  m_hKBQue;

    // warning: comes from internal hid thread
    virtual void OnKeyboardEventReceived( key_event_t* pKeyEvent );

public:
    void init();

    uint16_t getKey();
};