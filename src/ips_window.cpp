#include "ips_window.h"
#include "imgui.h"

#include <stdio.h>

ips_window::ips_window(){}

ips_window::~ips_window(){}

#include <arpa/inet.h>

static void* get_in_addr(struct sockaddr* sa){
	if(sa->sa_family == AF_INET){
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
#include "debug.h"
#include <stdlib.h>
void ips_window::render(ip_database& ip_db){
	std::vector<struct ip_entry> db = ip_db.get_db();

	ImGui::SetNextWindowSizeConstraints(ImVec2(400, 300), ImVec2(FLT_MAX, FLT_MAX)); // Width > 100, Height > 100

	ImGui::Begin("IPs");

	ImGui::BeginChild("left pane", ImVec2(150, 0), true);
	for(unsigned int i = 0; i < db.size(); i++){
		// FIXME: Good candidate to use ImGuiSelectableFlags_SelectOnNav
		char label[128];

		struct ip_entry ipe = db[i];
		sockaddr_storage ss = ipe.ss;
		const char* r = inet_ntop(ss.ss_family, get_in_addr((struct sockaddr*) &ss), label, 128);
		if(r == NULL){
			dbgprint("[IPS_WINDOW] Could not convert IP addr from bits to string! IP db must be corrupt.\n");
			abort();
		}

		if(ImGui::Selectable(label, selected_ip == i)){
			selected_ip = i;
		}
	}
	ImGui::EndChild();

	ImGui::End();
}
