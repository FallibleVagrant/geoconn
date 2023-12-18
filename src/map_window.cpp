#include "map_window.h"
#include "imgui.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

map_window::map_window(){}

map_window::~map_window(){}

// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height)
{
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
	const float ZOOM_AMOUNT_X = map_width / 240;
	const float ZOOM_AMOUNT_Y = (1 / aspect_ratio) * ZOOM_AMOUNT_X;

	if(view_x > 0 || view_x + view_width < map_width){
		view_width += ZOOM_AMOUNT_X;
		view_height += ZOOM_AMOUNT_Y;
		view_x -= ZOOM_AMOUNT_X/2;
		view_y -= ZOOM_AMOUNT_Y/2;
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

	if(view_x < map_width - view_width){
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

	if(view_y < map_height - view_height){
		view_y += SCROLL_AMOUNT_Y;
	}
	else{
		//Snap to maximum view_y if near SCROLL_AMOUNT_Y will go over,
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

	if(ImGui::IsWindowHovered()){
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
	point.x = coord.lat * 1;
	point.y = coord.lon * 1;
	return point;
}

void map_window::render_nodes(){
	struct coordinates coord;
	coord.lat = 50;
	coord.lon = 50;

	ImVec2 point = coords_to_equirectangular(coord);

	//ImGui::Image(my_tex_id, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, tint_col, border_col);
    //ImGui::Selectable("test", true, ImGuiSelectableFlags_None, ImVec2(-10, 50));
	/*ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;

	const float PAD = 10.0f;
	ImVec2 window_pos = ImGui::GetWindowPos();
    window_pos.x += PAD;
    window_pos.y += PAD;
	ImGui::SetNextWindowPos(window_pos);
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::Begin("node", NULL, window_flags);
	ImGui::Text("testtesttest");
	ImGui::End();*/

	ImVec2 window_pos = ImGui::GetWindowPos();
    window_pos.x += 9;
    window_pos.y += 25;
	//ImGui::SetNextWindowPos(window_pos);
	//ImGui::SetCursorPos(window_pos);
	//ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiChildFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
	ImGuiChildFlags child_flags = ImGuiChildFlags_AlwaysAutoResize;
	ImGui::SetCursorScreenPos(window_pos);
	//ImGui::BeginChild("scrolling", ImVec2(100, 100), false, child_flags);
	ImGui::BeginChild("name", ImVec2(-FLT_MIN, 0.0f), ImGuiWindowFlags_NoMove);
	ImGui::Image((void*)(intptr_t)node_texture, ImVec2(node_width, node_height), ImVec2(0, 0), ImVec2(1, 1));
	ImGui::EndChild();
}

void map_window::render(){
	ImGui::SetNextWindowSize(ImVec2(700, 700));
	ImGui::Begin("Map", NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	render_background();

	render_nodes();
    
	ImGui::End();
}
