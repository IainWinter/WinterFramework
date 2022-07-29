#pragma once

#include "app/System.h"

struct System_Console : System<System_Console>
{
	void Init()
	{
		CreateInput();
	}

	void Dnit()
	{
		FreeInput();
	}

	void OnAttach()
	{
		Attach<event_Key>();
	}

	void on(event_Key& e)
	{
		if (e.key == '`' && e.state)
		{
			ToggleConsole();
		}
	}

	void UI()
	{
		if (!consoleOpen) return;

		ImVec2 size = ImVec2(GetWindow().Width(), 400);
		float margin = 3;

		ImGui::SetNextWindowFocus();
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(size);
		ImGui::Begin("Console window", 0, ImGuiWindowFlags_NoTitleBar);
		
		float height = ImGui::GetFrameHeight();
		ImGui::SetCursorPos(ImVec2(margin, size.y - margin - height));

		if (ImGui::InputText("Input", input, inputSize, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			SendCommand();
		}

		ImGui::SameLine();

		if (ImGui::Button("Execute"))
		{
			SendCommand();
		}

		ImGui::End();
	}

private:

	bool consoleOpen = false;
	char* input = nullptr;
	const int inputSize = 1000;

	void CreateInput() { input = new char[inputSize]; ClearInput(); }
	void FreeInput() { delete[] input; }
	void ClearInput() { memset(input, 0, inputSize); }
	void ToggleConsole() { consoleOpen = !consoleOpen; }

	void SendCommand()
	{
		SendToRoot(event_ConsoleCommand{ input });
		ClearInput();
	}
};