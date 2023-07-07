#pragma once

#include "Event.h"
#include "Input.h"

class InputEventHandler
{
public:
    InputEventHandler();

    // This has to be created with new, as this attaches events in the constructor
    // if copied, these will point to destroyed stack memory
    InputEventHandler(InputMap* map, EventQueue* queue);
    
    void on(event_Key& e);
    void on(event_Mouse& e);
    void on(event_Controller& e);

    void TickFrame();

private:
    void HandleInputMapping(int code, float state);
    
private:
    EventQueue* m_queue;
    InputMap* m_map;
    int m_frame = 0;
};
