#define GLM_ENABLE_EXPERIMENTAL // risolve questo: GLM_GTX_dual_quaternion is an experimental extension and may change in the future. Use #define GLM_ENABLE_EXPERIMENTAL before including it, if you really want to use it."

#define NANOSVG_IMPLEMENTATION	// Expands implementation
#include "nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <xerrori.hpp>
#include "debugging.h"
#include "renderable.h"
#include "shaders.h"
#include "simple_shapes.h"
#include "carousel\carousel.h"
#include "carousel\carousel_to_renderable.h"
#include "carousel\carousel_loader.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <iostream>
#include <time.h>
#include <Windows.h>
#include <algorithm>
#include <conio.h>
#include <direct.h>
#include "matrix_stack.h"
#include "intersection.h"
#include "trackball.h"
#include "camera.hpp"
#include "input_manager.hpp"
#include "texture.h"

#include <glm.hpp>
#include <ext.hpp>
#include <gtx/string_cast.hpp>

trackball tb[2];
int curr_tb;

glm::mat4 proj; /* projection matrix*/
glm::mat4 view; /* view matrix */

Camera cameras[NUM_OF_CAMS];
unsigned int camera_index = 0;

matrix_stack stack;

// paths
std::string shaders_path = "src/shaders/";
std::string textures_path = "assets/textures/";

// window
int width, height;
GLFWwindow* window;
bool fullscreen = true;
bool quit = false;
bool cursor_hidden = false;

// Input
InputManager input_manager = NULL;

// callbacks
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	switch (camera_index)
	{
	case TRACKBALL:
		tb[curr_tb].mouse_move(proj, view, xpos, ypos);
		break;
	case FREECAM:
		cameras[FREECAM].MouseLook(xpos, ypos);
		break;

	default: xtermina("Indice della cam errato (o cam non ancora implementata).", QUI);
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		//std::cout << "pos: " << xpos << ", " << ypos << std::endl;

		switch (camera_index)
		{
		case TRACKBALL:
			tb[curr_tb].mouse_press(proj, view, xpos, ypos);
			break;
		case FREECAM:
			if (cursor_hidden) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			else
			{
				//if (ypos > 18) glfwSetCursorPos(window, 860, 540);
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}

			cursor_hidden = !cursor_hidden;
			break;

		default: xtermina("Indice della cam errato (o cam non ancora implementata).", QUI);
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		switch (camera_index)
		{
		case TRACKBALL:
			tb[curr_tb].mouse_release();
			break;
		case FREECAM: break;

		default: xtermina("Indice della cam errato (o cam non ancora implementata).", QUI);
		}
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (curr_tb == 0) tb[0].mouse_scroll(xoffset, yoffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) // la uso solo aggiornare lo stato dei tasti che mi interessano
{
	input_manager.UpdateState(key, action);
}

void window_size_callback(GLFWwindow* window, int _width, int _height)
{
	width = _width;
	height = _height;
	glViewport(0, 0, width, height);
	proj = glm::perspective(glm::radians(40.f), width / float(height), 2.f, 100.f);
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
	tb[0].reset();
}

static int fps;
bool first_time_gui = true;
void gui_setup()
{
	ImGui::BeginMainMenuBar();

	ImGui::Text((std::string("FPS: ") + std::to_string(fps)).c_str());

	if (ImGui::MenuItem("Quit")) { quit = true; }

	if (ImGui::BeginMenu("Camera"))
	{
		if (ImGui::Selectable("trackball", camera_index == 0))
		{
			camera_index = 0;
			tb[0].reset();
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		if (ImGui::Selectable("freecam", camera_index == 1))
		{
			camera_index = 1;
			tb[0].reset();
			//glfwSetCursorPos(window, 860, 540);
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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
					glfwSetWindowMonitor(window, NULL, 0, 0, width, height, GLFW_DONT_CARE);
					fullscreen = false;
				}
			}

			if (ImGui::Selectable("Fullscreen", fullscreen == true))
			{
				if (!fullscreen)
				{
					glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, width, height, GLFW_DONT_CARE);
					fullscreen = true;
				}
			}

			ImGui::EndMenu();
		}

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
		ImGui::Text(std::to_string(cameras[FREECAM].GetSpeed() * tb[0].GetScalingFactor()).c_str());
		ImGui::Text(glm::to_string(cameras[FREECAM].GetPosition()).c_str());
	}

	ImGui::End();
}


void key_bindings()
{
	// callbacks
	input_manager.BindKeyCallback(GLFW_KEY_W, []() { if (camera_index == 1) cameras[FREECAM].MoveForward(tb[0].GetScalingFactor()); });
	input_manager.BindKeyCallback(GLFW_KEY_S, []() { if (camera_index == 1) cameras[FREECAM].MoveBackward(tb[0].GetScalingFactor()); });
	input_manager.BindKeyCallback(GLFW_KEY_A, []() { if (camera_index == 1) cameras[FREECAM].MoveLeft(tb[0].GetScalingFactor()); });
	input_manager.BindKeyCallback(GLFW_KEY_D, []() { if (camera_index == 1) cameras[FREECAM].MoveRight(tb[0].GetScalingFactor()); });
	input_manager.BindKeyCallback(GLFW_KEY_SPACE, []() { if (camera_index == 1) cameras[FREECAM].MoveUp(tb[0].GetScalingFactor()); });
	input_manager.BindKeyCallback(GLFW_KEY_LEFT_CONTROL, []() { if (camera_index == 1) cameras[FREECAM].MoveDown(tb[0].GetScalingFactor()); });

	input_manager.BindKeyCallback(GLFW_KEY_TAB, []() { if (camera_index == 0 || camera_index == 1) curr_tb = 1 - curr_tb; }); // VERIFICA SE SERVE

	// states
	input_manager.BindKeyState(GLFW_KEY_LEFT_SHIFT, &(cameras[FREECAM].is_sprinting));
}


void process()
{
	input_manager.ProcessInput();
}



int main(int argc, char** argv)
{
	race r;

	carousel_loader::load("assets/small_test.svg", "assets/terrain_256.png", r);

	//add 10 cars
	for (int i = 0; i < 10; ++i)
		r.add_car();

	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	width = 1920;
	height = 1080;

	/* Create a windowed mode window and its OpenGL context */
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	window = xglfwCreateWindow(width, height, "CarOusel", NULL, NULL, QUI);

	input_manager = InputManager(window);

	/* declare the callback functions on mouse events */
	if (glfwRawMouseMotionSupported()) glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);

	key_bindings();

	glfwSetErrorCallback(error_callback);

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	glewInit();

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplOpenGL3_Init();
	ImGui_ImplGlfw_InitForOpenGL(window, true);

	printout_opengl_glsl_info();

	renderable fram = shape_maker::frame();

	renderable r_cube = shape_maker::cube();

	renderable r_track;
	r_track.create();
	game_to_renderable::to_track(r, r_track);

	renderable r_terrain;
	r_terrain.create();
	game_to_renderable::to_heightfield(r, r_terrain);

	renderable r_trees;
	r_trees.create();
	game_to_renderable::to_tree(r, r_trees);

	renderable r_lamps;
	r_lamps.create();
	game_to_renderable::to_lamps(r, r_lamps);

	texture grass_texture;
	grass_texture.load(textures_path + "grass_tile.png", 0);

	shader basic_shader;
	basic_shader.create_program((shaders_path + "basic.vert").c_str(), (shaders_path + "basic.frag").c_str());
	shader texture_shader;
	texture_shader.create_program((shaders_path + "texture.vert").c_str(), (shaders_path + "texture.frag").c_str());

	/* use the program shader "program_shader" */

	/* define the viewport  */
	glViewport(0, 0, width, height);

	tb[0].reset();
	tb[0].set_center_radius(glm::vec3(0, 0, 0), 1.f);
	curr_tb = 0;

	proj = glm::perspective(glm::radians(45.f), 1.f, 1.f, 10.f);

	cameras[0] = Camera(glm::vec3(0, 1.f, 1.5), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.0, 1.f, 0.f));
	cameras[0].is_active = true;
	cameras[1] = Camera(glm::vec3(0, 1.f, 1.5), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.0, 1.f, 0.f));

	glUseProgram(basic_shader.program);
	glUniformMatrix4fv(basic_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(basic_shader["uView"], 1, GL_FALSE, &view[0][0]);

	glUseProgram(texture_shader.program);
	glUniformMatrix4fv(texture_shader["uModel"], 1, GL_FALSE, &glm::mat4(1.0)[0][0]);
	glUniformMatrix4fv(texture_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(texture_shader["uView"], 1, GL_FALSE, &view[0][0]);
	glUniform1i(texture_shader["uColorImage"], 0);

	/* create a rectangle*/
	renderable r_plane;
	shape s_plane;
	shape_maker::rectangle(s_plane, 1, 1);
	s_plane.to_renderable(r_plane);

	r.start(11, 0, 0, 600);
	r.update();

	matrix_stack stack;

	glEnable(GL_DEPTH_TEST);
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		glUseProgram(basic_shader.program);

		/* Render here */
		glClearColor(0.3f, 0.3f, 0.3f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		check_gl_errors(__LINE__, __FILE__);

		r.update();
		stack.load_identity();
		stack.push();
		stack.mult(tb[0].matrix());
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		glUniform3f(basic_shader["uColor"], -1.f, 0.6f, 0.f);
		fram.bind();
		glDrawArrays(GL_LINES, 0, 6);

		glColor3f(0, 0, 1);
		glBegin(GL_LINES);
		glVertex3f(0, 0, 0);
		glVertex3f(r.sunlight_direction().x, r.sunlight_direction().y, r.sunlight_direction().z);
		glEnd();


		float s = 1.f / r.bbox().diagonal();
		glm::vec3 c = r.bbox().center();

		stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(s)));
		stack.mult(glm::translate(glm::mat4(1.f), -c));

		glUseProgram(texture_shader.program);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, grass_texture.id);

		//stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(30, 30, 30)));
		//glUniformMatrix4fv(texture_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		//r_plane.bind();
		//glDrawElements(r_plane().mode, r_plane().count, r_plane().itype, 0);
		//glBindTexture(GL_TEXTURE_2D, 0);

		glUseProgram(texture_shader.program);
		//glUseProgram(basic_shader.program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, grass_texture.id);

		glDepthRange(0.01, 1);
		glUniformMatrix4fv(texture_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		//glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		//glUniform3f(basic_shader["uColor"], 1, 1, 1.0);
		r_terrain.bind();
		//glDrawArrays(GL_POINTS, 0, r_terrain.vn);
		glDrawElements(r_terrain().mode, r_terrain().count, r_terrain().itype, 0); // mode = GL_TRIANGLES, 
		glBindTexture(GL_TEXTURE_2D, 0);
		glDepthRange(0.0, 1);


		glUseProgram(basic_shader.program);
		for (unsigned int ic = 0; ic < r.cars().size(); ++ic) {
			stack.push();
			stack.mult(r.cars()[ic].frame);
			stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(0, 0.1, 0.0)));
			glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
			glUniform3f(basic_shader["uColor"], -1.f, 0.6f, 0.f);
			fram.bind();
			glDrawArrays(GL_LINES, 0, 6);
			stack.pop();
		}

		fram.bind();
		for (unsigned int ic = 0; ic < r.cameramen().size(); ++ic) {
			stack.push();
			stack.mult(r.cameramen()[ic].frame);
			stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(4, 4, 4)));
			glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
			glUniform3f(basic_shader["uColor"], -1.f, 0.6f, 0.f);
			glDrawArrays(GL_LINES, 0, 6);
			stack.pop();
		}
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);

		r_track.bind();
		glPointSize(3.0);
		glUniform3f(basic_shader["uColor"], 0.2f, 0.3f, 0.2f);
		glDrawArrays(GL_LINE_STRIP, 0, r_track.vn);
		glPointSize(1.0);


		r_trees.bind();
		glUniform3f(basic_shader["uColor"], 0.f, 1.0f, 0.f);
		glDrawArrays(GL_LINES, 0, r_trees.vn);


		r_lamps.bind();
		glUniform3f(basic_shader["uColor"], 1.f, 1.0f, 0.f);
		glDrawArrays(GL_LINES, 0, r_lamps.vn);




		stack.pop();
		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}
	glUseProgram(0);
	glfwTerminate();
	return 0;
}

