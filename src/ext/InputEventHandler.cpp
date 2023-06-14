#include "ext/InputEventHandler.h"

InputEventHandler::InputEventHandler()
    : m_map   (nullptr)
    , m_queue (nullptr)
{}

InputEventHandler::InputEventHandler(InputMap* map, EventQueue* queue)
    : m_map   (map)
    , m_queue (queue)
{
    queue->bus->Attach<event_Key>(this);
    queue->bus->Attach<event_Mouse>(this);
    queue->bus->Attach<event_Controller>(this);
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
            vec2 pos = m_map->MapToViewport(e.screen_x, e.screen_y);

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

void InputEventHandler::TickFrame()
{
    m_frame += 1;
    m_map->SetActiveFrame(m_frame);
}

void InputEventHandler::HandleInputMapping(int code, float state)
{
    m_map->SetState(code, state);

    const auto& input = m_map->GetMapping(code);

    for (const InputName& input : m_map->GetMapping(code))
    {
        event_Input e;
        e.name = input;
        e.axis = m_map->GetAxis(input);

        m_queue->Send(e);
    }
}
