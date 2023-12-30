//Note to self.
//Maybe do more visualizations?
//Instead of lat & long projected onto map,
//we can project latency and throughput onto the xy plane.
#include "geoconn.h"

#include "imgui.h"

#include "datalinks_window.h"
#include "ips_window.h"
#include "map_window.h"
#include "networking_agent.h"
#include "common.h"

FILE* error_log = stderr;

namespace geoconn{
	bool show_datalinks_window = true;
	bool show_ips_window = true;

	bool sending_request = false;

	datalinks_window datalinks_win;
	ips_window ips_win;

	networking_agent net_agent;

	geolocation geo;
	ip_database ip_db(geo);
	std::vector<struct connection> active_connections;
	map_window map_win(geo);

	unsigned int selected_ip = 0;

	void setup(){
		map_win.init();
		net_agent.init();
	}

	void renderUI(){
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
		net_agent.check_for_incoming_connections();
		net_agent.check_for_messages(ip_db, active_connections);
		ImGui::ShowDemoWindow();

		if(ImGui::BeginMainMenuBar()){
			if(ImGui::BeginMenu("Manage")){
				if(ImGui::MenuItem("Datalinks", NULL, &show_datalinks_window)){}
				if(ImGui::MenuItem("IPs", NULL, &show_ips_window)){}
				ImGui::EndMenu();
			}
			/*
			if(ImGUi::BeginMenu("Visualizer")){
				//if(ImGui::MenuItem("N/A for now. single selection or options subwindow?", NULL, )){}
				ImGui::EndMenu();
			}
			*/
			ImGui::EndMainMenuBar();
    	}

		if(show_datalinks_window){
			datalinks_win.render(sending_request, net_agent.num_data_sources());
		}

		if(show_ips_window){
			ips_win.render(ip_db, selected_ip);
		}

		if(true){
			map_win.render(ip_db, selected_ip);
		}

		if(sending_request){
			int r = net_agent.request_ips();
			if(r == -1){
				;
			}
			sending_request = false;
		}
	}
}
