#pragma once

#include "Event.h"
#include "app/InputMap.h"

class InputEventHandler
{
private:
    EventQueue* m_queue;
public:
    InputEventHandler();
    InputEventHandler(EventQueue* queue);
    
    void on(event_Key& e);
    void on(event_Mouse& e);
    void on(event_Controller& e);

private:
    void HandleInputMapping(int code, float state);
};
