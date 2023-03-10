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

	/*	ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(1, 1, 1, .4));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(1, 1, 1, .5));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, .6));
		ImGui::PushStyleColor(ImGuiCol_Border,        ImVec4(0, 0, 0, .8));
		ImGui::PushStyleColor(ImGuiCol_WindowBg,      ImVec4(.8, .8, .8, 1));
		ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(0, 0, 0, 1));
		ImGui::PushStyleColor(ImGuiCol_Border,        ImVec4(0, 0, 0, 1));*/

		ImVec2 size = ImVec2(GetWindow().Width(), 400);
		float margin = 3;

		ImGui::SetNextWindowFocus();
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(size);
		ImGui::Begin("Console window", 0, 
			  ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_NoScrollbar
			| ImGuiWindowFlags_NoResize);
		
		if (lastLength != get_all_logs().size())
		{
			record = combine_all_logs();
			lastLength = get_all_logs().size();
		}

		ImGui::InputTextMultiline(
			"records", 
			(char*)record.c_str(), 
			record.size(), 
			ImVec2(size.x - margin, size.y * .9), 
			ImGuiInputTextFlags_ReadOnly
		);

		//ImGui::PushID(ImGui::GetID("records"));
		//ImGui::SetScrollHereY(1.f);
		//ImGui::PopID();

		float height = ImGui::GetFrameHeight();
		ImGui::SetCursorPos(ImVec2(margin, size.y - margin - height));

		if (ImGui::Button("Execute"))
		{
			SendCommand();
		}

		if (firstFrame)
		{
			firstFrame = false;
			ImGui::SetKeyboardFocusHere();
		}

		ImGui::SameLine();
		
		if (ImGui::InputText("cmd", input, inputSize, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			SendCommand();
		}

		ImGui::End();
		//ImGui::PopStyleColor(7);
	}

private:

	bool consoleOpen = false;
	char* input = nullptr;
	const int inputSize = 1000;

	bool firstFrame = true;
	int lastLength = 0;

	std::string record;

	void CreateInput() { input = new char[inputSize]; ClearInput(); }
	void FreeInput() { delete[] input; }
	void ClearInput() { memset(input, 0, inputSize); }
	void ToggleConsole() { consoleOpen = !consoleOpen; firstFrame = true; }

	void SendCommand()
	{
		if (input[0] != '\0')
		{
			SendToRoot(event_ConsoleCommand{ input });
			ClearInput();
		}

		firstFrame = true; // refocus input
	}
};