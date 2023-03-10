#pragma once

#include "Event.h"
#include "app/InputMap.h"

class InputEventHandler
{
private:
    EventQueue* m_queue;

public:
    InputEventHandler();

    // This has to be created with new, as this attaches events in the constructor
    // if copied, these will point to destroyed stack memory
    InputEventHandler(EventQueue* queue);
    
    void on(event_Key& e);
    void on(event_Mouse& e);
    void on(event_Controller& e);

private:
    void HandleInputMapping(int code, float state);
};
