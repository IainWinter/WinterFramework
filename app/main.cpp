#include "EngineLoop.h"

struct MainSystem : SystemBase
{
    void UI()
    {
        ImGui::Begin("App");
        ImGui::Text("This is a test");
        ImGui::End();
    }
};

struct App : EngineLoop
{
    void _Init()
    {
        Window& window = m_app.GetModule<Window>();
        window.SetTitle("Bluefusion Radar");
        window.Resize(1280, 720);

        LevelManager::CurrentLevel()->AddSystem(MainSystem());
    }
};

void setup()
{
    RunEngineLoop<App>();
}