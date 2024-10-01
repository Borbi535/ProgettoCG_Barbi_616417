#define GLM_ENABLE_EXPERIMENTAL // risolve questo: GLM_GTX_dual_quaternion is an experimental extension and may change in the future. Use #define GLM_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#include <glm.hpp>
#include <ext.hpp>
#include <gtx/string_cast.hpp>

#define NANOSVG_IMPLEMENTATION	// Expands implementation
#include "nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <xerrori.hpp>
#include <iostream>
#include <time.h>
#include <Windows.h>
#include <algorithm>
#include <conio.h>
#include <direct.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
//#include <stb_image.h>
//#include <stb_image_write.h>
#include "gltf_loader.h"

#include "debugging.h"
#include "renderable.h"
#include "shaders.h"
#include "simple_shapes.h"
#include "matrix_stack.h"
#include "intersection.h"
#include "input_manager.hpp"
#include "camera.hpp"
#include "trackball_camera.hpp"
#include "freecam.hpp"
#include "texture.h"

#include "carousel\carousel.h"
#include "carousel\carousel_to_renderable.h"
#include "carousel\carousel_loader.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

enum TextureID { GRASS, TRACK, TEX_IDS_NUMBER };

enum GameState { TRACKBALL_STATE, FREECAM_STATE, CAMERAMEN_STATE };

glm::mat4 projection_matrix; /* projection matrix*/
glm::mat4 view_matrix; /* view matrix */

Camera* cameras[CAMS_NUMBER];
unsigned int camera_index = 0;
TrackballCamera trackball;
Freecam freecam;

matrix_stack stack;

int camera_state = TRACKBALL_STATE;
bool game_paused = false;

// paths
std::string shaders_path = "src/shaders/";
std::string textures_path = "assets/textures/";

// window
int width, height;
GLFWwindow* main_window;
bool fullscreen = false;
bool quit = false;

// shaders
shader basic_shader;
shader texture_shader;

// textures
texture grass_texture;
texture track_texture;
bool grass_texture_disabled = false;
bool track_texture_disabled = false;

// renderables
renderable fram;
renderable r_cube;
renderable r_track;
renderable r_terrain;
renderable r_trees;
renderable r_lamps;

// models
box3 streetlamp_bbox;
std::vector<renderable> streetlamp_obj;
box3 m1abrams_bbox;
std::vector<renderable> m1abrams_obj;
box3 tree_bbox;
std::vector<renderable> tree_obj;





// callbacks
static void cursor_position_callback(GLFWwindow* main_window, double xpos, double ypos)
{
	input_manager::UpdateMousePosition(xpos, ypos);
}

void mouse_button_callback(GLFWwindow * main_window, int key, int action, int mods)
{
	input_manager::UpdateKeyState(key, action);
}

void scroll_callback(GLFWwindow * main_window, double xoffset, double yoffset)
{
	input_manager::UpdateMouseScroll(xoffset, yoffset);
}

void key_callback(GLFWwindow * main_window, int key, int scancode, int action, int mods) // la uso solo aggiornare lo stato dei tasti che mi interessano
{
	input_manager::UpdateKeyState(key, action);
}

void window_size_callback(GLFWwindow* main_window, int _width, int _height)
{
	width = _width;
	height = _height;
	glViewport(0, 0, width, height);
	projection_matrix = glm::perspective(glm::radians(40.f), width / float(height), 2.f, 100.f);
}

void error_callback(int, const char* err_str)
{
	std::cout << "GLFW Error: " << err_str << std::endl;
}

void change_camera(unsigned int i, Camera in, Camera out)
{
	camera_index = i;
	in.is_active = true;
	out.is_active = false;
	trackball.Reset();
}

void input_bindings()
{
	using namespace input_manager;
	{
		// keyboard callback bindings
		BindKeyCallback(GLFW_KEY_W, GLFW_PRESS, FREECAM_STATE, []() { if (input_manager::cursor_hidden) freecam.MoveForward(trackball.GetScalingFactor()); }, true);
		BindKeyCallback(GLFW_KEY_S, GLFW_PRESS, FREECAM_STATE, []() { if (input_manager::cursor_hidden) freecam.MoveBackward(trackball.GetScalingFactor()); }, true);
		BindKeyCallback(GLFW_KEY_A, GLFW_PRESS, FREECAM_STATE, []() { if (input_manager::cursor_hidden) freecam.MoveLeft(trackball.GetScalingFactor()); }, true);
		BindKeyCallback(GLFW_KEY_D, GLFW_PRESS, FREECAM_STATE, []() { if (input_manager::cursor_hidden) freecam.MoveRight(trackball.GetScalingFactor()); }, true);
		BindKeyCallback(GLFW_KEY_SPACE, GLFW_PRESS, FREECAM_STATE, []() { if (input_manager::cursor_hidden) freecam.MoveUp(trackball.GetScalingFactor()); }, true);
		BindKeyCallback(GLFW_KEY_LEFT_CONTROL, GLFW_PRESS, FREECAM_STATE, []() { if (input_manager::cursor_hidden) freecam.MoveDown(trackball.GetScalingFactor()); }, true);

		BindKeyCallback(GLFW_KEY_C, GLFW_PRESS, FREECAM_STATE, input_manager::ToggleMousePointer);
		BindKeyCallback(GLFW_KEY_P, GLFW_PRESS, FREECAM_STATE, []() { game_paused = !game_paused; });
		BindKeyCallback(GLFW_KEY_P, GLFW_PRESS, TRACKBALL_STATE, []() { game_paused = !game_paused; });

		// keyboard state bindings
		BindBoolToKey(GLFW_KEY_LEFT_SHIFT, FREECAM_STATE, &(freecam.is_sprinting));

		// mouse callback bindings
		BindKeyCallback(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, TRACKBALL_STATE,[]() { trackball.MousePress(); });
		BindKeyCallback(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, TRACKBALL_STATE, []() { trackball.MouseRelease(); });
		BindMouseScrollCallback(TRACKBALL_STATE, []() { trackball.MouseScroll(); });

		BindMouseMoveCallback(TRACKBALL_STATE, []() { trackball.MouseMove(); });
		BindMouseMoveCallback(FREECAM_STATE ,[]() { freecam.MouseMove(); });
	}
}

static int fps;
bool first_time_gui = true;
void gui_setup() // da sistemare, magari in un file a parte
{
	ImGui::BeginMainMenuBar();
	
	ImGui::Text((std::string("FPS: ") + std::to_string(fps)).c_str());

	if (ImGui::MenuItem("Quit")) { quit = true; }

	if (ImGui::BeginMenu("Camera"))
	{
		if (ImGui::Selectable("trackball", camera_index == 0))
		{
			camera_index = 0;
			trackball.Reset();
			input_manager::ShowMousePointer();
			camera_state = TRACKBALL_STATE;
		}
		if (ImGui::Selectable("freecam", camera_index == 1))
		{
			camera_index = 1;
			trackball.Reset();
			input_manager::HideMousePointer();
			camera_state = FREECAM_STATE;
		}

		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Video settings"))
	{
		if (ImGui::BeginMenu("Window mode"))
		{
			if (ImGui::Selectable("Windowed", fullscreen == false))
			{
				if (fullscreen)
				{
					glfwSetWindowMonitor(main_window, NULL, 0, 0, width, height, GLFW_DONT_CARE);
					fullscreen = false;
				}
			}
			
			if (ImGui::Selectable("Fullscreen", fullscreen == true))
			{
				if (!fullscreen)
				{
					glfwSetWindowMonitor(main_window, glfwGetPrimaryMonitor(), 0, 0, width, height, GLFW_DONT_CARE);
					fullscreen = true;
				}
			}

			ImGui::EndMenu();
		}

		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Hide Textures"))
	{
		ImGui::Checkbox("Grass", &grass_texture_disabled);
		ImGui::Checkbox("Track", &track_texture_disabled);

		ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();

	if (ImGui::Begin("Debug info"))
	{
		if (first_time_gui)
		{
			ImGui::SetWindowSize(ImVec2(150, 300));
			ImGui::SetWindowPos(ImVec2(1770, 18));
			first_time_gui = false;
		}
		//ImGui::Text(std::to_string(tb[0].GetScalingFactor()).c_str());
		//ImGui::Text(std::to_string(freecam.GetSpeed() * trackball.GetScalingFactor()).c_str());
		//ImGui::Text(glm::to_string(freecam.GetPosition()).c_str());
		ImGui::Text(glm::to_string(input_manager::mouse_position).c_str());
	}

	ImGui::End();
}

void startup_routine(race& r)
{
	carousel_loader::load("assets/small_test.svg", "assets/terrain_256.png", r);

	//add 10 cars
	for (int i = 0; i < 10; ++i)
		r.add_car();

	/* Initialize the library */
	xglfwInit(QUI);

	width = 1920;
	height = 1080;

	/* Create a windowed mode window and its OpenGL context */
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	main_window = xglfwCreateWindow(width, height, "CarOusel", NULL, NULL, QUI);

	/* declare the callback functions on mouse events */
	if (glfwRawMouseMotionSupported()) glfwSetInputMode(main_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

	glfwSetCursorPosCallback(main_window, cursor_position_callback);
	glfwSetMouseButtonCallback(main_window, mouse_button_callback);
	glfwSetScrollCallback(main_window, scroll_callback);
	glfwSetKeyCallback(main_window, key_callback);
	glfwSetWindowSizeCallback(main_window, window_size_callback);

	input_bindings();

	glfwSetErrorCallback(error_callback);

	/* Make the window's context current */
	glfwMakeContextCurrent(main_window);
	glewInit();

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplOpenGL3_Init();
	ImGui_ImplGlfw_InitForOpenGL(main_window, true);

	printout_opengl_glsl_info();

	// textures TODO: metterle in un dizionario	
	grass_texture.load(textures_path + "grass_tile.png", GRASS);	
	track_texture.load(textures_path + "street_tile.png", TRACK);

	// renderables
	fram = shape_maker::frame();

	r_cube = shape_maker::cube();

	r_track.create();
	game_to_renderable::to_track(r, r_track);

	r_terrain.create();
	game_to_renderable::to_heightfield(r, r_terrain);

	r_trees.create();
	game_to_renderable::to_tree(r, r_trees);

	r_lamps.create();
	game_to_renderable::to_lamps(r, r_lamps);

	// shaders
	basic_shader.create_program((shaders_path + "basic.vert").c_str(), (shaders_path + "basic.frag").c_str());
	texture_shader.create_program((shaders_path + "texture.vert").c_str(), (shaders_path + "texture.frag").c_str());

	glUseProgram(basic_shader.program);
	glUniformMatrix4fv(basic_shader["uProj"], 1, GL_FALSE, &projection_matrix[0][0]);
	glUniformMatrix4fv(basic_shader["uView"], 1, GL_FALSE, &view_matrix[0][0]);

	glUseProgram(texture_shader.program);
	glUniformMatrix4fv(texture_shader["uModel"], 1, GL_FALSE, &glm::mat4(1.0)[0][0]);
	glUniformMatrix4fv(texture_shader["uProj"], 1, GL_FALSE, &projection_matrix[0][0]);
	glUniformMatrix4fv(texture_shader["uView"], 1, GL_FALSE, &view_matrix[0][0]);

	glUseProgram(0);

	// modelli
	gltf_loader gltfL1, gltfL2, gltfL3;

	gltfL2.load_to_renderable("assets/models/decorations/street_lamp.glb", streetlamp_obj, streetlamp_bbox);
	gltfL1.load_to_renderable("assets/models/cars/troll.glb", m1abrams_obj, m1abrams_bbox);
	//gltfL1.load_to_renderable("assets/models/decorations/maple_tree.glb", tree_obj, tree_bbox);

	// cameras
	/* define the viewport  */
	glViewport(0, 0, 1920, 1080);

	trackball = TrackballCamera(glm::vec3(0.f, 1.f, 1.5f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	trackball.Set_center_radius(glm::vec3(0.f, 0.f, 0.f), 1.f);

	projection_matrix = glm::perspective(glm::radians(45.f), 16.f / 9, 0.001f, 100.f);

	cameras[TRACKBALL_ID] = &trackball;
	cameras[TRACKBALL_ID]->is_active = true;

	freecam = Freecam(glm::vec3(0.f, 1.f, 1.5f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 1.f, 0.f));
	cameras[FREECAM_ID] = &freecam;
			
	check_gl_errors(QUI, true);

	r.start(11, 0, 0, 600);
	r.update();
}

void process()
{
	input_manager::ProcessInput();
} 

void exit_routine()
{
	glUseProgram(0);
	glfwTerminate();
}

int main(int argc, char** argv)
{
	race r;
	startup_routine(r);

	matrix_stack stack;

	glEnable(GL_DEPTH_TEST);

	double last_time = glfwGetTime();
	int n_frames = 0;
	while (!glfwWindowShouldClose(main_window) && !quit)
	{
		double start_time = glfwGetTime();
		
		glClearColor(0.3f, 0.3f, 0.3f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		check_gl_errors(QUI);

		glUseProgram(basic_shader.program);

		view_matrix = cameras[camera_index]->GetViewMatrix();
		glUniformMatrix4fv(basic_shader["uView"], 1, GL_FALSE, &view_matrix[0][0]);

		if (!game_paused) r.update();

		stack.load_identity();
		stack.push();
		stack.mult(trackball.GetMatrix());
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		glUniform3f(basic_shader["uColor"], -1.f, 0.6f, 0.f);
		fram.bind();
		glDrawArrays(GL_LINES, 0, 6);

		glColor3f(0, 0, 1);
		glBegin(GL_LINES);
		glVertex3f(0, 0, 0);
		glVertex3f(r.sunlight_direction().x, r.sunlight_direction().y, r.sunlight_direction().z);
		glEnd();


		float s = 1.f/r.bbox().diagonal();
		glm::vec3 c = r.bbox().center();

		stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(s)));
		stack.mult(glm::translate(glm::mat4(1.f), -c));

		if (grass_texture_disabled)
		{
			glDepthRange(0.01, 1);
			glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
			glUniform3f(basic_shader["uColor"], 1, 1, 1.0);
			r_terrain.bind();
			glDrawArrays(GL_POINTS, 0, r_terrain.vn);
			glDepthRange(0.0, 1);
		}
		else
		{
			glUseProgram(texture_shader.program);
			glActiveTexture(GL_TEXTURE0 + GRASS);
			glBindTexture(GL_TEXTURE_2D, grass_texture.id);
			glDepthRange(0.01, 1);
			glUniformMatrix4fv(texture_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
			glUniformMatrix4fv(texture_shader["uView"], 1, GL_FALSE, &view_matrix[0][0]); // forse da spostare
			glUniform1i(texture_shader["uColorImage"], GRASS);
			r_terrain.bind();
			glDrawElements(r_terrain().mode, r_terrain().count, r_terrain().itype, 0); // mode = GL_TRIANGLES, 
			glDepthRange(0.0, 1);
			glBindTexture(GL_TEXTURE_2D, 0);

			glUseProgram(basic_shader.program);
		}

		glUseProgram(texture_shader.program);
		// macchine
		for (unsigned int ic = 0; ic < r.cars().size(); ++ic) 
		{

			stack.push();
			stack.mult(r.cars()[ic].frame);
			stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(0,0.1,0.0)));

			for (renderable obj : m1abrams_obj)
			{
				obj.bind();
				stack.push();
				stack.mult(obj.transform);

				glBindTexture(GL_TEXTURE_2D, obj.mater.base_color_texture);
				glUniformMatrix4fv(texture_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);

				glDrawElements(obj().mode, obj().count, obj().itype, 0);
				stack.pop();
			}
			
			//glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
			//glUniform3f(basic_shader["uColor"], -1.f, 0.6f, 0.f);
			//fram.bind();
			//glDrawArrays(GL_LINES, 0, 6);
			stack.pop();
		}

		glUseProgram(basic_shader.program);

		check_gl_errors(QUI);

		fram.bind();
		for (unsigned int ic = 0; ic < r.cameramen().size(); ++ic)
		{
			stack.push();
			stack.mult(r.cameramen()[ic].frame);
			stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(4, 4,4)));
			glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
			glUniform3f(basic_shader["uColor"], -1.f, 0.6f, 0.f);
			glDrawArrays(GL_LINES, 0, 6);
			stack.pop();
		}

		check_gl_errors(QUI);

		if (track_texture_disabled)
		{
			glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
			r_track.bind();
			glPointSize(3.0);
			glUniform3f(basic_shader["uColor"], 0.2f, 0.3f, 0.2f);
			glDrawArrays(GL_LINE_STRIP, 0, r_track.vn);
			glPointSize(1.0);
		}
		else
		{
			glUseProgram(texture_shader.program);
			glActiveTexture(GL_TEXTURE0 + TRACK);
			glBindTexture(GL_TEXTURE_2D, track_texture.id);
			glUniformMatrix4fv(texture_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
			glUniformMatrix4fv(texture_shader["uView"], 1, GL_FALSE, &view_matrix[0][0]); // forse da spostare
			glUniform1i(texture_shader["uColorImage"], TRACK);
			r_track.bind();
			glDrawElements(r_track().mode, r_track().count, r_track().itype, 0);
			glBindTexture(GL_TEXTURE_2D, 0);

			glUseProgram(basic_shader.program);
			glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		}

		check_gl_errors(QUI);

		// alberi
		//for (stick_object tree : r.trees())
		//{
		//	stack.push();
		//	stack.mult(glm::translate(glm::mat4(1), tree.pos));
		//	for (renderable obj : streetlamp_obj)
		//	{
		//		obj.bind();
		//		stack.push();
		//		stack.mult(obj.transform);
		//
		//		glBindTexture(GL_TEXTURE_2D, obj.mater.base_color_texture);
		//		glUniformMatrix4fv(texture_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		//
		//		glDrawElements(obj().mode, obj().count, obj().itype, 0);
		//		stack.pop();
		//	}
		//	stack.pop();
		//}

		r_trees.bind();
		glUniform3f(basic_shader["uColor"], 0.f, 1.0f, 0.f);
		glDrawArrays(GL_LINES, 0, r_trees.vn);
		

		check_gl_errors(QUI);

		// lampioni


		glUseProgram(texture_shader.program);
		for (stick_object lamp : r.lamps())
		{
			stack.push();
			stack.mult(glm::translate(glm::mat4(1), lamp.pos));
			for (renderable obj : streetlamp_obj)
			{
				obj.bind();
				stack.push();
				stack.mult(obj.transform);
		
				glBindTexture(GL_TEXTURE_2D, obj.mater.base_color_texture);
				glUniformMatrix4fv(texture_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		
				glDrawElements(obj().mode, obj().count, obj().itype, 0);
				stack.pop();
			}
			stack.pop();
		}

		check_gl_errors(QUI);


		//r_lamps.bind();
		//glUniform3f(basic_shader["uColor"], 1.f, 1.0f, 0.f);
		//glDrawArrays(GL_LINES, 0, r_lamps.vn);
		
		stack.pop();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		gui_setup();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		/* Swap front and back buffers */
		glfwSwapBuffers(main_window);

		/* Poll for and process events */
		glfwPollEvents();
		process(); // da migliorare


		double end_time = glfwGetTime();
		n_frames++;
		if (end_time - last_time >= 1.0)
		{
			fps = (double)(n_frames * 0.5 + n_frames * 0.5);
			n_frames = 0;
			last_time = end_time;
		}

		Sleep(1000.f / 64 - (end_time - start_time));
	}
	exit_routine();
	return 0;
}

 