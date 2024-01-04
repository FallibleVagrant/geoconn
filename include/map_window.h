#ifndef MAP_WINDOW_H
#define MAP_WINDOW_H

//For loading the map. Cf.:
//https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif

#include <vector>
#include "ip_database.h"
#include <atomic>

struct coordinates;

class map_window{
	public:
		map_window(geolocation& geo);
		~map_window();

		void init();
		void render(ip_database& ip_db, unsigned int& selected_ip);
		GLuint map_texture = 0;
		int map_width, map_height;
		float view_x, view_y;
		float view_width, view_height;
		GLuint node_texture = 0;
		int node_width, node_height;
	private:
		void zoom_in(float aspect_ratio);
		void zoom_out(float aspect_ratio);

		void scroll_up_by_y(float y);
		void scroll_up(float aspect_ratio);
		void scroll_right_by_x(float x);
		void scroll_right(float aspect_ratio);
		void scroll_down_by_y(float y);
		void scroll_down(float aspect_ratio);
		void scroll_left_by_x(float x);
		void scroll_left(float aspect_ratio);

		void render_background();
		ImVec2 coords_to_equirectangular(struct coordinates coord);
		void render_nodes(std::vector<struct ip_entry>& db, unsigned int& selected_ip);

		void render_loading_screen();

		geolocation& geo;
		long ipv4db_filelength;
		long ipv6db_filelength;

		std::atomic<bool> are_textures_initialized;
};

#endif
