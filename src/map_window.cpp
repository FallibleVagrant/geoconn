#include "map_window.h"
#include "imgui.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

map_window::map_window(){}

map_window::~map_window(){}

// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height){
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}

void map_window::init(){
	//Load the map. Cf.:
	//https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples
	bool ret = LoadTextureFromFile("map.jpg", &map_texture, &map_width, &map_height);
	IM_ASSERT(ret);
	view_width = 700;
	view_height = 700;

	ret = LoadTextureFromFile("node.png", &node_texture, &node_width, &node_height);
	IM_ASSERT(ret);
}

void map_window::zoom_in(float aspect_ratio){
	const float ZOOM_AMOUNT_X = map_width / 80;
	const float ZOOM_AMOUNT_Y = (1 / aspect_ratio) * ZOOM_AMOUNT_X;

	if(view_width >= map_width / 20 && view_height >= map_height / 20){
		view_width -= ZOOM_AMOUNT_X;
		view_height -= ZOOM_AMOUNT_Y;
		view_x += ZOOM_AMOUNT_X/2;
		view_y += ZOOM_AMOUNT_Y/2;
	}
}

void map_window::zoom_out(float aspect_ratio){
	const float ZOOM_AMOUNT_X = map_width / 80;
	const float ZOOM_AMOUNT_Y = (1 / aspect_ratio) * ZOOM_AMOUNT_X;

	if(view_x - ZOOM_AMOUNT_X/2 > 0 || view_x + view_width + ZOOM_AMOUNT_X < map_width){
		view_width += ZOOM_AMOUNT_X;
		view_height += ZOOM_AMOUNT_Y;
		view_x -= ZOOM_AMOUNT_X/2;
		view_y -= ZOOM_AMOUNT_Y/2;
	}
	else{
		//Snap to widest view.
		view_width = map_width;
		view_y = -(view_height/2) + map_height/2;
		view_x = 0;
	}
}

void map_window::scroll_up(float aspect_ratio){
	const float SCROLL_AMOUNT_X = (map_width / 24) * (view_width / map_width);
	const float SCROLL_AMOUNT_Y = (1 / aspect_ratio) * SCROLL_AMOUNT_X;

	if(view_y >= SCROLL_AMOUNT_Y){
		view_y -= SCROLL_AMOUNT_Y;
	}
	else{
		//Snap to 0 if SCROLL_AMOUNT_Y will go under,
		//but only if view isn't large enough to go into negative view_y.
		if(view_height <= map_height){
			view_y = 0;
		}
	}
}

void map_window::scroll_right(float aspect_ratio){
	const float SCROLL_AMOUNT_X = (map_width / 24) * (view_width / map_width);
	//const float SCROLL_AMOUNT_Y = (1 / aspect_ratio) * SCROLL_AMOUNT_X;

	if(view_x + SCROLL_AMOUNT_X < map_width - view_width){
		view_x += SCROLL_AMOUNT_X;
	}
	else{
		//Snap to max view_x.
		view_x = map_width - view_width;
	}
}

void map_window::scroll_down(float aspect_ratio){
	const float SCROLL_AMOUNT_X = (map_width / 24) * (view_width / map_width);
	const float SCROLL_AMOUNT_Y = (1 / aspect_ratio) * SCROLL_AMOUNT_X;

	if(view_y + SCROLL_AMOUNT_Y < map_height - view_height){
		view_y += SCROLL_AMOUNT_Y;
	}
	else{
		//Snap to maximum view_y if SCROLL_AMOUNT_Y will go over,
		//but only if view isn't large enough to go into negative view_y.
		if(view_height <= map_height){
			view_y = map_height - view_height;
		}
	}
}

void map_window::scroll_left(float aspect_ratio){
	const float SCROLL_AMOUNT_X = (map_width / 24) * (view_width / map_width);
	//const float SCROLL_AMOUNT_Y = (1 / aspect_ratio) * SCROLL_AMOUNT_X;

	if(view_x >= SCROLL_AMOUNT_X){
		view_x -= SCROLL_AMOUNT_X;
	}
	else{
		//Snap to 0.
		view_x = 0;
	}
}

void map_window::render_background(){
	ImGuiIO& io = ImGui::GetIO();
	
    ImVec2 window_size = ImGui::GetWindowSize();
	float aspect_ratio = window_size.x / window_size.y;
	//Maintain aspect ratio upon resizing window.
	if(view_width / view_height > aspect_ratio + 0.001 || view_width / view_height < aspect_ratio - 0.001){
		view_height = (1 / aspect_ratio) * view_width;
		//Centers the view on y upon resize, since this is the only case where y can become misaligned.
		if(view_height > map_height){
			view_y = -(view_height/2) + map_height/2;
		}
	}

	if(ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows)){
		if(io.MouseWheel > 0){
			if(ImGui::IsKeyDown(ImGuiKey_LeftCtrl)){
				zoom_in(aspect_ratio);
			}
			else{
				scroll_up(aspect_ratio);
			}
		}
		if(io.MouseWheel < 0){
			if(ImGui::IsKeyDown(ImGuiKey_LeftCtrl)){
				zoom_out(aspect_ratio);
			}
			else{
				scroll_down(aspect_ratio);
			}
		}
		if(io.MouseWheelH > 0 && !ImGui::IsKeyDown(ImGuiKey_LeftCtrl)){
			scroll_right(aspect_ratio);
		}
		if(io.MouseWheelH < 0 && !ImGui::IsKeyDown(ImGuiKey_LeftCtrl)){
			scroll_left(aspect_ratio);
		}
	}

	//View of the map will always be constrained by the map_width, no exceptions.
	if(view_x < 0){
		view_x = 0;
	}
	else if(view_x > map_width - view_width){
		view_x = map_width - view_width;
	}
	//View CAN extend past the bounds of the map_height, however,
	//as the map is longer than it is high.
	if(view_y < 0 && view_height <= map_height){
		view_y = 0;
	}
	if(view_y > 0 && view_y + view_height > map_height && view_height <= map_height){
		view_y = map_height - view_height;
	}

	ImGui::Begin("test");
	ImGui::Text("view_x: %f\nview_y: %f\nview_width: %f\nview_height: %f", view_x, view_y, view_width, view_height);
	ImGui::Text("size: %dx%d", map_width, map_height);
	ImGui::End();

	ImVec2 uv0 = ImVec2(view_x / map_width, view_y / map_height);
	ImVec2 uv1 = ImVec2((view_x + view_width) / map_width, (view_y + view_height) / map_height);
	ImGui::Image((void*)(intptr_t)map_texture, ImVec2(window_size.x, window_size.y), uv0, uv1);
}

struct coordinates{
	float lat;
	float lon;
};

ImVec2 map_window::coords_to_equirectangular(struct coordinates coord){
	ImVec2 point;
	float pixels_per_latitude = map_height / 180.0f;
	float pixels_per_longitude = map_width / 360.0f;

	point.y = -(coord.lat - 90) * pixels_per_latitude;
	point.x = (coord.lon + 180.0f) * pixels_per_longitude;
	return point;
}

void map_window::render_nodes(std::vector<struct ip_entry>& db, unsigned int& selected_ip){
	/*struct coordinates coord1;
	coord1.lat = 51.0f;
	coord1.lon = -9.0f;
	struct coordinates coord2;
	coord2.lat = 0.0f;
	coord2.lon = 0.0f;
	struct coordinates coord3;
	coord3.lat = 37.5f;
	coord3.lon = -122.2f;*/

	ImGui::Begin("Debug Coords");
	std::vector<struct coordinates> coords;
	for(struct ip_entry ipe : db){
		struct coordinates c;
		c.lat = ipe.lat;
		c.lon = ipe.lon;
		coords.push_back(c);
		ImGui::Text("lat: %f", c.lat);
		ImGui::Text("lon: %f", c.lon);
	}
	ImGui::End();

	/*coords.push_back(coord1);
	coords.push_back(coord2);
	coords.push_back(coord3);*/

	/*Test lines.
	for(int i = -180; i <= 180; i++){
		struct coordinates coordi;
		coordi.lat = i / 2;
		coordi.lon = i;
		coords.push_back(coordi);
	}
	for(int i = -180; i <= 180; i++){
		struct coordinates coordi;
		coordi.lat = i / 2;
		coordi.lon = 0;
		coords.push_back(coordi);
	}
	for(int i = -180; i <= 180; i++){
		struct coordinates coordi;
		coordi.lat = 0;
		coordi.lon = i;
		coords.push_back(coordi);
	}
	*/

	for(unsigned int i = 0; i < coords.size(); i++){
		struct coordinates coord;
		coord = coords[i];

		//Point is a point on map.jpg, from zero to map_width/map_height.
		ImVec2 point = coords_to_equirectangular(coord);

		//If point is in view.
		if(point.x > view_x && point.x < view_x + view_width
				&& point.y > view_y && point.y < view_y + view_height){
			ImVec2 window_size = ImGui::GetWindowSize();
			ImVec2 window_pos = ImGui::GetWindowPos();
			//Add the title bar and misc. spacing.
			//window_pos.x += 9;
			//window_pos.y += 29;

			ImVec2 test_pos = window_pos;

			ImVec2 view_pos(point.x - view_x, point.y - view_y);
			ImVec2 view_to_window(window_size.x / view_width, window_size.y / view_height);
			window_pos.x += view_pos.x * view_to_window.x;
			window_pos.y += view_pos.y * view_to_window.y;

ImGui::Begin("Debug Node");
ImGui::Text("node: %u", i);
ImGui::Text("point: %f, %f", point.x, point.y);
ImGui::Text("view_pos: %f, %f", view_pos.x, view_pos.y);
ImGui::Text("initial window_pos: %f, %f", test_pos.x, test_pos.y);
ImGui::Text("point is drawn to: %f, %f", window_pos.x, window_pos.y);
ImGui::Text("window_size: %f, %f", window_size.x, window_size.y);
ImGui::End();

			//ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
			ImGui::SetCursorScreenPos(window_pos);
			ImGui::PushID(i);
			ImGui::BeginChild("Node", ImVec2(-FLT_MIN, 0.0f), ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			if(ImGui::IsWindowHovered()){
				selected_ip = i;
			}
			float coefficient = 1 - ((view_width * view_width * view_width) / (map_width * map_width * map_width));
			if(coefficient < 0.3){
				coefficient = 0.3;
			}
			//TODO: figure out node size.
			coefficient = 1;
			ImGui::Image((void*)(intptr_t)node_texture, ImVec2(node_width * coefficient, node_height * coefficient), ImVec2(0, 0), ImVec2(1, 1));
			ImGui::EndChild();
			ImGui::PopID();
			//ImGui::GetForegroundDrawList()->AddCircle(window_pos, 10.6f, IM_COL32(0, 255, 0, 200), 0, 10);
		}
	}
}

void map_window::render(ip_database& ip_db, unsigned int& selected_ip){
    ImGui::SetNextWindowSizeConstraints(ImVec2(500, 500), ImVec2(FLT_MAX, FLT_MAX)); // Width > 100, Height > 100
	ImGui::Begin("Map", NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	render_background();

	std::vector<struct ip_entry> db = ip_db.get_db();
	render_nodes(db, selected_ip);
    
	ImGui::End();
}
