//Note to self.
//Maybe do more visualizations?
//Instead of lat & long projected onto map,
//we can project latency and throughput onto the xy plane.
#include "geoconn.h"

#include "imgui.h"

#include "datalinks_window.h"
#include "ips_window.h"
#include "map_window.h"
#include "networking_receiver.h"
#include "common.h"

namespace geoconn{
	bool show_datalinks_window = true;
	bool show_ips_window = true;

	bool trying_to_connect = false;

	datalinks_window datalinks_win;
	ips_window ips_win;
	map_window map_win;

	networking_receiver net_receiver;

	ip_database ip_db;
	std::vector<struct connection> active_connections;

	unsigned int selected_ip = 0;

	void setup(){
		map_win.init();
	}

	void renderUI(){
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
		net_receiver.check_for_messages(ip_db, active_connections);
		ImGui::ShowDemoWindow();

		if(ImGui::BeginMainMenuBar()){
			if(ImGui::BeginMenu("Manage")){
				if(ImGui::MenuItem("Datalinks", NULL, &show_datalinks_window)){}
				if(ImGui::MenuItem("IPs", NULL, &show_ips_window)){}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
    	}

		if(show_datalinks_window){
			datalinks_win.render(trying_to_connect);
		}

		if(show_ips_window){
			ips_win.render(ip_db, selected_ip);
		}

		if(true){
			map_win.render(ip_db, selected_ip);
		}

		if(trying_to_connect){
			int r = net_receiver.init();
			if(r == -1){
				;
			}
			trying_to_connect = false;
		}
	}
}
