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
#include <time.h>
#include <Windows.h>
#include <algorithm>
#include <conio.h>
#include <direct.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_INCLUDE_STB_IMAGE

#include "mesh_3d.hpp"
#include "body_3d.hpp"
#include "heightmap.hpp"

#include "renderable.h"
#include "debugging.h"
#include "graphical_debugging.hpp"
#include "shaders.h"
#include "simple_shapes.h"
#include "matrix_stack.h"
#include "intersection.h"
#include "input_manager.hpp"
#include "camera.hpp"
#include "arcball_camera.hpp"
#include "freecam.hpp"
#include "texture.h"
#include "light.hpp"
#include "shadowmap.hpp"

#include "carousel\carousel.h"
#include "carousel\carousel_to_renderable.h"
#include "carousel\carousel_loader.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

enum TextureID { GRASS, TRACK, SPOTLIGHT_SHADOWMAPS, TEX_IDS_NUMBER };

// range di id per le texture delle shadowmap, per non interferire con le texture normali
enum ShadowMapID { SUN_SHADOWMAP = TEX_IDS_NUMBER + 1,
	POSITIONAL_LIGHT_SHADOWMAP_MIN = TEX_IDS_NUMBER + 2,
	POSITIONAL_LIGHT_SHADOWMAP_MAX = POSITIONAL_LIGHT_SHADOWMAP_MIN + 256,
	SPOTLIGHT_SHADOWMAP_MIN = POSITIONAL_LIGHT_SHADOWMAP_MAX + 1,
	SPOTLIGHT_SHADOWMAP_MAX = SPOTLIGHT_SHADOWMAP_MIN + 256,
};

enum GameState { ARCBALL_STATE, FREECAM_STATE, CAMERAMEN_STATE };

shader basic_shader;
shader texture_shader;
shader depth_shader;
shader debug_shader;

glm::mat4 projection_matrix;
glm::mat4 view_matrix;

Camera* cameras[CAMS_NUMBER];
unsigned int camera_index = 0;
ArcballCamera arcball_camera;
Freecam freecam;

matrix_stack stack;

DirectionalLight sun_light; // da spostare nel main, 
//da riguardare qua sotto
float ambient_color[3] = { 0.15f,0.15f,0.15f };
float diffuse_color[3] = { 0.5f,0.1f,0.2f };
float specular_color[3] = { 0.5f,0.1f,0.2f };
float shininess = 1.0f;

float sun_intensity = 1;

int camera_state = ARCBALL_STATE;
bool game_paused = false;

// paths
std::string shaders_path = "src/shaders/";
std::string textures_path = "assets/textures/";

// window
int window_width, window_height;
glm::vec2 window_center;
GLFWwindow* main_window;
bool fullscreen = false;
bool quit = false;

// debug
bool show_debug_shapes = false;


// callbacks
static void cursor_position_callback(GLFWwindow* main_window, double xpos, double ypos)
{
	if (ImGui::GetIO().WantCaptureMouse) return;

	input_manager::UpdateMousePosition(xpos, ypos);
}

void mouse_button_callback(GLFWwindow * main_window, int key, int action, int mods)
{
	if (ImGui::GetIO().WantCaptureMouse) return;

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
	window_width = _width;
	window_height = _height;
	glViewport(0, 0, window_width, window_height);
	projection_matrix = glm::perspective(glm::radians(40.f), window_width / float(window_height), 2.f, 100.f);
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
}

void input_bindings()
{
	using namespace input_manager;
	{
		// keyboard callback bindings
		BindKeyCallback(GLFW_KEY_W, GLFW_PRESS, FREECAM_STATE, []() { if (cursor_hidden) freecam.MoveForward(); }, true);
		BindKeyCallback(GLFW_KEY_S, GLFW_PRESS, FREECAM_STATE, []() { if (cursor_hidden) freecam.MoveBackward(); }, true);
		BindKeyCallback(GLFW_KEY_A, GLFW_PRESS, FREECAM_STATE, []() { if (cursor_hidden) freecam.MoveLeft(); }, true);
		BindKeyCallback(GLFW_KEY_D, GLFW_PRESS, FREECAM_STATE, []() { if (cursor_hidden) freecam.MoveRight(); }, true);
		BindKeyCallback(GLFW_KEY_SPACE, GLFW_PRESS, FREECAM_STATE, []() { if (cursor_hidden) freecam.MoveUp(); }, true);
		BindKeyCallback(GLFW_KEY_LEFT_CONTROL, GLFW_PRESS, FREECAM_STATE, []() { if (cursor_hidden) freecam.MoveDown(); }, true);

		BindKeyCallback(GLFW_KEY_C, GLFW_PRESS, FREECAM_STATE, ToggleMousePointer);
		BindKeyCallback(GLFW_KEY_P, GLFW_PRESS, FREECAM_STATE, []() { game_paused = !game_paused; });
		BindKeyCallback(GLFW_KEY_P, GLFW_PRESS, ARCBALL_STATE, []() { game_paused = !game_paused; });

		// keyboard state bindings
		BindBoolToKey(GLFW_KEY_LEFT_SHIFT, FREECAM_STATE, &(freecam.is_sprinting));

		// mouse callback bindings
		BindKeyCallback(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, ARCBALL_STATE,[]() { arcball_camera.MouseLPress(); });
		BindKeyCallback(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, ARCBALL_STATE, []() { arcball_camera.MouseLRelease(); });
		BindKeyCallback(GLFW_MOUSE_BUTTON_MIDDLE, GLFW_PRESS, ARCBALL_STATE, []() { arcball_camera.MouseScrollPress(); });
		BindKeyCallback(GLFW_MOUSE_BUTTON_MIDDLE, GLFW_RELEASE, ARCBALL_STATE, []() { arcball_camera.MouseScrollRelease(); });
		BindMouseScrollCallback(ARCBALL_STATE, []() { arcball_camera.MouseScroll(); });

		BindMouseMoveCallback(ARCBALL_STATE, []() { arcball_camera.MouseMove(); });
		BindMouseMoveCallback(FREECAM_STATE ,[]() { freecam.MouseMove(); });
	}
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
		if (ImGui::Selectable("arcball", camera_index == 0))
		{
			camera_index = 0;
			input_manager::ShowMousePointer();
			camera_state = ARCBALL_STATE;
		}
		if (ImGui::Selectable("freecam", camera_index == 1))
		{
			camera_index = 1;
			input_manager::HideMousePointer();
			camera_state = FREECAM_STATE;
		}

		if (ImGui::Button("Reset posizione cam")) cameras[camera_index]->ResetParameters();

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
					glfwSetWindowMonitor(main_window, NULL, 0, 0, window_width, window_height, GLFW_DONT_CARE);
					fullscreen = false;
				}
			}
			
			if (ImGui::Selectable("Fullscreen", fullscreen == true))
			{
				if (!fullscreen)
				{
					glfwSetWindowMonitor(main_window, glfwGetPrimaryMonitor(), 0, 0, window_width, window_height, GLFW_DONT_CARE);
					fullscreen = true;
				}
			}

			ImGui::EndMenu();
		}

		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Material"))
	{
		ImGuiColorEditFlags misc_flags = ImGuiColorEditFlags_NoOptions;
		ImGui::ColorEdit3("amb color", (float*)&ambient_color, misc_flags);
		ImGui::ColorEdit3("diff color", (float*)&diffuse_color, misc_flags);
		ImGui::ColorEdit3("spec color", (float*)&specular_color, misc_flags);
		ImGui::SliderFloat("shininess", &shininess, 1.f, 100.f);
		ImGui::EndMenu(); 
	}

	if (ImGui::BeginMenu("Lights settings"))
	{
		if (ImGui::SliderFloat("Value", &sun_intensity, 0.0f, 2.0f))
		{
			glUniform1f(texture_shader["uSunIntensity"], sun_intensity);
		}

		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Debug"))
	{
		ImGui::Checkbox("show debug shapes", &show_debug_shapes);

		ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();

	if (ImGui::Begin("Debug info"))
	{
		if (first_time_gui)
		{
			ImGui::SetWindowSize(ImVec2(150, 300));
			ImGui::SetWindowPos(ImVec2(1700, 18));
			first_time_gui = false;
		}
		ImGui::Text(glm::to_string(view_matrix * glm::vec4(cameras[camera_index]->GetPosition(), 1.f)).c_str());
	}

	ImGui::End();
}

void draw_terrain(renderable& terrain, texture& texture, TextureID id, matrix_stack& stack)
{

	glActiveTexture(GL_TEXTURE0 + id);
	glUniform1i(texture_shader["uTextureAvailable"], terrain.mater.use_texture);
	glBindTexture(GL_TEXTURE_2D, texture.id);
	if (id == GRASS) glDepthRange(0.03, 1);
	glUniformMatrix4fv(texture_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
	glUniform1i(texture_shader["uColorImage"], id);
	terrain.bind();
	glDrawElements(terrain().mode, terrain().count, terrain().itype, 0);
	if (id == GRASS) glDepthRange(0.0, 1);
	glBindTexture(GL_TEXTURE_2D, 0);
	check_gl_errors(QUI);
}

void draw_terrain_depthmap(renderable& terrain, TextureID id, matrix_stack& stack)
{
	if (id == GRASS) glDepthRange(0.03, 1);
	glUniformMatrix4fv(depth_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
	terrain.bind();
	glDrawElements(terrain().mode, terrain().count, terrain().itype, 0);
	if (id == GRASS) glDepthRange(0.0, 1);
	check_gl_errors(QUI);
}

void draw_3dbodies(std::vector<std::shared_ptr<Body3D>>& bodies, matrix_stack& stack)
{
	for (std::shared_ptr<Body3D> body : bodies)
	{
			body->Draw(stack);
	}
	check_gl_errors(QUI);
}

void process()
{
	input_manager::ProcessInput();
} 

void exit_routine()
{
	;
}

int main(int argc, char** argv)
{
	srand(time(NULL));

	race r;
	
	carousel_loader::load("assets/small_test.svg", "assets/terrain_256.png",r);
	
	//add 10 cars
	for (int i = 0; i < 10; ++i)		
		r.add_car();

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	window_width = 1920;
	window_height = 1080;
	window_center = glm::vec2(window_width / 2, window_height / 2);

	/* Create a windowed mode window and its OpenGL context */
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	main_window = xglfwCreateWindow(window_width, window_height, "CarOusel", NULL, NULL, QUI);

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

	/* define the viewport  */
	glViewport(0, 0, window_width, window_height);

	int shadowmap_sizex = 1024, shadowmap_sizey = 1024;

	projection_matrix = glm::perspective(glm::radians(45.f), 16.f / 9, 0.001f, 100.f);

	cameras[ARCBALL_ID] = &arcball_camera;
	cameras[ARCBALL_ID]->is_active = true;

	arcball_camera = ArcballCamera(glm::vec3(0.f, 1.f, 1.5f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	freecam = Freecam(glm::vec3(0.f, 1.f, 1.5f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	cameras[FREECAM_ID] = &freecam;

	view_matrix = cameras[camera_index]->GetViewMatrix();


	glm::mat4 scene_coordinates_matrix =
		glm::scale(glm::mat4(1.f), glm::vec3(1.f / r.bbox().diagonal())) *
		glm::translate(glm::mat4(1.f), -(r.bbox().center()));

	texture grass_texture;
	grass_texture.load(textures_path + "grass_tile.png", GRASS);
	texture track_texture;
	track_texture.load(textures_path + "street_tile.png", TRACK);

	renderable fram = shape_maker::frame();

	renderable r_cube = shape_maker::cube();

	//r.apply_matrix_to_track(scene_coordinates_matrix);
	renderable r_track;
	r_track.create();
	game_to_renderable::to_track(r, r_track);

	renderable r_terrain;
	r_terrain.create();
	height_map terrain_heightmap;
	terrain_heightmap.set_bounding_rect(r.ter().rect_xz[0], r.ter().rect_xz[1], r.ter().rect_xz[2], r.ter().rect_xz[3]);
	terrain_heightmap.load_from_file("assets/terrain_256.png");
	//terrain_heightmap.apply_matrix(scene_coordinates_matrix);
	terrain_heightmap.to_renderable(r_terrain);
		
	renderable r_trees;
	r_trees.create();
	game_to_renderable::to_tree(r, r_trees);

	renderable r_lamps;
	r_lamps.create();
	game_to_renderable::to_lamps(r, r_lamps);

	r.start(11, 0, 0, 600);
	r.update();

	// shaders
	basic_shader.create_program((shaders_path + "basic.vert").c_str(), (shaders_path + "basic.frag").c_str());
	texture_shader.create_program((shaders_path + "texture.vert").c_str(), (shaders_path + "texture.frag").c_str());
	Mesh3D::SetShader(texture_shader);
	depth_shader.create_program((shaders_path + "depth.vert").c_str(), (shaders_path + "depth.frag").c_str());
	debug_shader.create_program((shaders_path + "debug.vert").c_str(), (shaders_path + "debug.frag").c_str());
	
	// Models
	Mesh3D streetlamp_model("assets/models/decorations/street_lamp.glb");
	Mesh3D tree_model("assets/models/decorations/tree_3d_model_fir_spruce_pine.glb", .4f);
	Mesh3D cameraman_model("assets/models/cameraman/sphere_drone_future.glb");
		
	std::vector<Mesh3D> car_models;
	car_models.push_back(Mesh3D("assets/models/cars/bumper_car.glb", .8f));
	car_models.push_back(Mesh3D("assets/models/cars/bugatti_type_35.glb", .2f));
	
	// 3D Bodies
	std::vector<std::shared_ptr<Body3D>> tree_bodies;
	for (stick_object tree : r.trees())
		tree_bodies.push_back(std::make_shared<Body3D>(scene_coordinates_matrix * glm::translate(glm::mat4(1.f), tree.pos), tree_model));

	std::vector<std::shared_ptr<Body3D>> streetlamp_bodies;
	for (stick_object lamp : r.lamps())
	{
		streetlamp_bodies.push_back(std::make_shared<Body3D>(scene_coordinates_matrix * glm::translate(glm::mat4(1.f), lamp.pos), streetlamp_model));
		streetlamp_bodies.back()->AddLight(PositionalLight::Create(glm::vec3(0, 2.9f, 0), 1.f, glm::vec3(3.f)));
	}

	std::vector<std::shared_ptr<Body3D>> cameraman_bodies;
	for (cameraman cam : r.cameramen())
	{
		cameraman_bodies.push_back(std::make_shared<Body3D>(scene_coordinates_matrix * cam.frame, cameraman_model));
	}

	std::vector<std::shared_ptr<Body3D>> car_bodies;
	for (unsigned int i = 0; i < r.cars().size(); ++i)
	{
		car_bodies.push_back(std::make_shared<Body3D>(
			scene_coordinates_matrix * r.cars()[i].frame,
			car_models[rand() % car_models.size()]));
		car_bodies.back()->AddLight(SpotLight::Create(glm::vec3(0, 0, -1.f), glm::vec3(0, 0, -1.f), 30.f, 5.f, 20.f, glm::vec3(3.f)));
	}

	// SSBOs
	PositionalLight::GenerateSSBO(view_matrix);
	SpotLight::GenerateSSBO(view_matrix);

	// shaders
	glUseProgram(basic_shader.program);
	glUniformMatrix4fv(basic_shader["uProj"], 1, GL_FALSE, &projection_matrix[0][0]);
	glUniformMatrix4fv(basic_shader["uView"], 1, GL_FALSE, &view_matrix[0][0]);

	glUseProgram(texture_shader.program);
	glUniformMatrix4fv(texture_shader["uModel"], 1, GL_FALSE, &glm::mat4(1.0)[0][0]);
	glUniformMatrix4fv(texture_shader["uProj"], 1, GL_FALSE, &projection_matrix[0][0]);
	glUniformMatrix4fv(texture_shader["uView"], 1, GL_FALSE, &view_matrix[0][0]);

	glUniform2i(texture_shader["uShadowMapSize"], shadowmap_sizex, shadowmap_sizey);
	glUniform1i(texture_shader["uSpotLightsShadowMapArray"], SPOTLIGHT_SHADOWMAPS);
	glUniform1i(texture_shader["uNumPositionalLights"], PositionalLight::GetNumberOfLights());
	glUniform1i(texture_shader["uNumSpotLights"], SpotLight::GetNumberOfLights());
	glUniform1f(texture_shader["uSunIntensity"], sun_intensity);

	glUseProgram(depth_shader.program);
	glUniformMatrix4fv(depth_shader["uModel"], 1, GL_FALSE, &glm::mat4(1.0)[0][0]);
	glUniformMatrix4fv(depth_shader["uLightMatrix"], 1, GL_FALSE, &glm::mat4(1.0)[0][0]);
	glUniform1f(depth_shader["uPlaneApprox"], 0.5f);

	glUseProgram(debug_shader.program);
	glUniformMatrix4fv(debug_shader["uModel"], 1, GL_FALSE, &glm::mat4(1.0)[0][0]);
	glUniformMatrix4fv(debug_shader["uProj"], 1, GL_FALSE, &projection_matrix[0][0]);
	glUniformMatrix4fv(debug_shader["uView"], 1, GL_FALSE, &view_matrix[0][0]);

	glUseProgram(0);
	check_gl_errors(QUI, true);
	
	// frame buffer objects for shadow maps
	// nota: ho usato shared_ptr perchè non ho scritto un copy constructor per frame_buffer_object
	// e non voglio che vengano distrutti quando escono dallo scope (il distruttore chiama glDeleteFramebuffers)
	std::vector<std::shared_ptr<frame_buffer_object>> spotlights_fbos;
	for (int i = 0; i < SpotLight::GetNumberOfLights(); i++)
		spotlights_fbos.push_back(std::make_shared<frame_buffer_object>(shadowmap_sizex, shadowmap_sizey, true));

	// texture array for shadow maps
	shadowmap_texture_array spotlights_shadowmaps(shadowmap_sizex, shadowmap_sizey);
	
	sun_light = DirectionalLight(r.sunlight_direction(), glm::vec3(3.f));
	
	matrix_stack stack;

	glEnable(GL_DEPTH_TEST);

	double last_time = glfwGetTime();
	int n_frames = 0;
	while (!glfwWindowShouldClose(main_window) && !quit)
	{
		double start_time = glfwGetTime();
		
		glClearColor(0.3f, 0.3f, 0.3f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		//check_gl_errors(QUI);

		view_matrix = cameras[camera_index]->GetViewMatrix();

		// Update
		if (!game_paused)
		{
			r.update();

			PositionalLight::UpdateSSBO(view_matrix);

			for (unsigned int i = 0; i < r.cars().size(); i++)
			{
				car_bodies[i]->SetModelMatrix(scene_coordinates_matrix * r.cars()[i].frame);
				car_bodies[i]->UpdateLights();
			}

			for (unsigned int i = 0; i < r.cameramen().size(); i++)
			{
				cameraman_bodies[i]->SetModelMatrix(scene_coordinates_matrix * r.cameramen()[i].frame);
			}

			SpotLight::UpdateSSBO(view_matrix);
		}

		sun_light.SetDirection(r.sunlight_direction());

		glUseProgram(texture_shader.program);
		glUniformMatrix4fv(texture_shader["uView"], 1, GL_FALSE, &view_matrix[0][0]);
		glUniform3f(texture_shader["uAmbientColor"], ambient_color[0], ambient_color[1], ambient_color[2]);
		glUniform3f(texture_shader["uDiffuseColor"], diffuse_color[0], diffuse_color[1], diffuse_color[2]);
		glUniform3f(texture_shader["uSpecularColor"], specular_color[0], specular_color[1], specular_color[2]);
		glUniform3f(texture_shader["uSunColor"], sun_light.GetColor().x, sun_light.GetColor().y, sun_light.GetColor().z);
		glUniform1f(texture_shader["uShininess"], shininess);
		glUniform3f(texture_shader["uLDir"], sun_light.GetDirection().x, sun_light.GetDirection().y, sun_light.GetDirection().z);
		
		
		stack.load_identity();
		stack.push();
		fram.bind();
		glDrawArrays(GL_LINES, 0, 6);

		glColor3f(0, 0, 1);
		glBegin(GL_LINES);
		glVertex3f(0, 0, 0);
		glVertex3f(r.sunlight_direction().x, r.sunlight_direction().y, r.sunlight_direction().z);
		glEnd();

		//depth shader
		glUseProgram(depth_shader.program);
		Mesh3D::SetShader(depth_shader);
		glViewport(0, 0, shadowmap_sizex, shadowmap_sizey);


		//check_gl_errors(QUI);

		for (int i = 0; i < SpotLight::GetNumberOfLights(); i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, spotlights_fbos[i]->id_fbo);
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

			glUniformMatrix4fv(depth_shader["uLightMatrix"], 1, GL_FALSE, &(SpotLight::GetLightMatrix(i)[0][0]));
			
			stack.push();
			stack.mult(scene_coordinates_matrix);
			draw_terrain_depthmap(r_terrain, GRASS, stack);
			draw_terrain_depthmap(r_track, TRACK, stack);
			stack.pop();

			draw_3dbodies(car_bodies, stack);
			draw_3dbodies(tree_bodies, stack);
			draw_3dbodies(streetlamp_bodies, stack);
			draw_3dbodies(cameraman_bodies, stack);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//check_gl_errors(QUI);

		glUseProgram(texture_shader.program);
		Mesh3D::SetShader(texture_shader);
		glViewport(0, 0, window_width, window_height);

		glActiveTexture(GL_TEXTURE0 + SPOTLIGHT_SHADOWMAPS);
		spotlights_shadowmaps.update(spotlights_fbos);
		glBindTexture(GL_TEXTURE_2D_ARRAY, spotlights_shadowmaps.id_tex);

		stack.push();
		stack.mult(scene_coordinates_matrix);
		draw_terrain(r_terrain, grass_texture, GRASS, stack);
		draw_terrain(r_track, track_texture, TRACK, stack);
		stack.pop();

		draw_3dbodies(tree_bodies, stack);
		draw_3dbodies(streetlamp_bodies, stack);
		draw_3dbodies(cameraman_bodies, stack);
		draw_3dbodies(car_bodies, stack);
				
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

		//Sleep(1000.f / 60 - (end_time - start_time));
	}
	
	glUseProgram(0);
	glfwTerminate();
	return 0;
}

 