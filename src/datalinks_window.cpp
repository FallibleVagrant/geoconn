#include "datalinks_window.h"

#include "imgui.h"

datalinks_window::datalinks_window(){}

datalinks_window::~datalinks_window(){}

void datalinks_window::render(bool& trying_to_connect){
	ImGui::SetNextWindowSizeConstraints(ImVec2(500, 300), ImVec2(FLT_MAX, FLT_MAX)); // Width > 100, Height > 100

	ImGui::Begin("Datalinks", NULL, ImGuiWindowFlags_NoResize);
	ImGui::Text("stdin: NOT READING");
	ImGui::Text("/proc/net/*: NOT READING");
	ImGui::Text("Remote Sources: NONE");
	ImGui::Separator();

	ImGui::Button("Toggle reading from /proc/net/*");

	static char str0[128] = "localhost";
	ImGui::InputText("IP/Domain", str0, IM_ARRAYSIZE(str0));
	static int i0 = 40344;
	ImGui::InputInt("Port", &i0);
	if(trying_to_connect){
		ImGui::BeginDisabled();
		ImGui::Button("Connect");
		ImGui::EndDisabled();
	}
	else{
		if(ImGui::Button("Connect")){
			trying_to_connect = true;
		}
	}
	ImGui::End();
}
