#include "app/InputEventHandler.h"

InputEventHandler::InputEventHandler()
    : m_queue (nullptr)
{}

InputEventHandler::InputEventHandler(EventQueue* queue)
    : m_queue (queue)
{
    auto bus = queue->GetBus();
    
    bus->Attach<event_Key>(this);
    bus->Attach<event_Mouse>(this);
    bus->Attach<event_Controller>(this);
}

void InputEventHandler::on(event_Key& e)
{
    if (e.repeat == 0)
    {
        HandleInputMapping(GetInputCode(e.keycode), e.state ? 1.f : 0.f);
    }
}

void InputEventHandler::on(event_Mouse& e)
{
    switch (e.mousecode)
    {
        case MOUSE_LEFT:
        case MOUSE_MIDDLE:
        case MOUSE_RIGHT:
        case MOUSE_X1:
        case MOUSE_X2:
        {
            HandleInputMapping(GetInputCode(e.mousecode),
                   e.button_left
                || e.button_middle
                || e.button_right
                || e.button_x1
                || e.button_x2
            );

            break;
        }

        case MOUSE_VEL_POS:
        {
            vec2 pos = Input::MapToViewport(e.screen_x, e.screen_y);

            HandleInputMapping(GetInputCode(MOUSE_POS_X), pos.x);
            HandleInputMapping(GetInputCode(MOUSE_POS_Y), pos.y);
            HandleInputMapping(GetInputCode(MOUSE_VEL_X), e.vel_x);
            HandleInputMapping(GetInputCode(MOUSE_VEL_Y), e.vel_y);

            break;
        }
            
        case MOUSE_VEL_WHEEL:
        {
            HandleInputMapping(GetInputCode(MOUSE_VEL_WHEEL_X), e.vel_x);
            HandleInputMapping(GetInputCode(MOUSE_VEL_WHEEL_Y), e.vel_y);

            break;
        }
            
        default:
        {
            assert(false && "a mouse event without a code was sent");
            break;
        }
    }
}

void InputEventHandler::on(event_Controller& e)
{
    HandleInputMapping(GetInputCode(e.input), e.value);
}

void InputEventHandler::HandleInputMapping(int code, float state)
{
    Input::SetState(code, state);

    InputName input = Input::GetMapping(code);

    if (input.size() > 0)
    {
        event_Input e;
        e.name = input;
        e.axis = Input::GetAxis(input);

        m_queue->Send(e);
    }
}
