#include "datalinks_window.h"

#include "imgui.h"

datalinks_window::datalinks_window(){}

datalinks_window::~datalinks_window(){}

//TODO: Put this in a common header file if you use it elsewhere.
static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void datalinks_window::render(bool& sending_request, unsigned int num_data_sources){
	ImGui::SetNextWindowSizeConstraints(ImVec2(500, 300), ImVec2(FLT_MAX, FLT_MAX)); // Width > 100, Height > 100

	ImGui::Begin("Datalinks", NULL, ImGuiWindowFlags_NoResize);
	ImGui::Text("stdin: NOT IMPLEMENTED");
	ImGui::Text("/proc/net/*: NOT IMPLEMENTED");
	ImGui::Text("Remote Sources (Listening on localhost:40343): %u", num_data_sources);
	ImGui::Separator();

	ImGui::Button("Toggle reading from /proc/net/*");
	ImGui::Separator();

	ImGui::SetNextItemWidth(7.0f * ImGui::GetFontSize());

	const char* request_templates[] = { "connwatch", "custom" };
	const int num_request_templates = sizeof(request_templates) / sizeof(char*);
	const char* combo_preview_value = request_templates[template_current_idx];
	if(ImGui::BeginCombo("##Templates Combo", combo_preview_value)){
		for(int n = 0; n < IM_ARRAYSIZE(request_templates); n++){
			const bool is_selected = (template_current_idx == n);
			if(ImGui::Selectable(request_templates[n], is_selected)){
				template_current_idx = n;
			}

			//Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if(is_selected){
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	ImGui::SameLine();
	ImGui::SetNextItemWidth(8.0f * ImGui::GetFontSize());

	//Disable changing these if not set to custom.
	if(template_current_idx != num_request_templates - 1){
		ImGui::BeginDisabled();
	}

	static char str0[128] = "localhost";
	ImGui::InputText("##IP/Domain", str0, IM_ARRAYSIZE(str0));

	ImGui::SameLine();
	ImGui::SetNextItemWidth(8.0f * ImGui::GetFontSize());

	static int i0 = 40344;
	ImGui::InputInt("##Port", &i0);

	//End disable.
	if(template_current_idx != num_request_templates - 1){
		ImGui::EndDisabled();
	}

	ImGui::SameLine(); HelpMarker("Currently can't request from anywhere besides localhost, which is probably a good thing.");

	if(sending_request){
		ImGui::BeginDisabled();
		ImGui::Button("Request IPs from remote source");
		ImGui::EndDisabled();
	}
	else{
		if(ImGui::Button("Request IPs from remote source")){
			sending_request = true;
		}
	}

	ImGui::End();
}
