#define GLM_ENABLE_EXPERIMENTAL

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <algorithm>
#include <conio.h>
#include <direct.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include "..\common\debugging.h"

#include "..\common\renderable.h"
#include "..\common\shaders.h"
#include "..\common\simple_shapes.h"
#include "..\common\matrix_stack.h"
#include "..\common\intersection.h"
#include "..\common\trackball.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#include "..\common\gltf_loader.h"
#include "..\common\texture.h"


/* projection matrix*/
glm::mat4 proj;

/* view matrix load*/
glm::mat4 view;

// paths
std::string shaders_path = "src/shaders/";
std::string textures_path = "assets/textures/";


int main(void)
{
	/* Initialize the library */
	if (!glfwInit())
		return -1;

	GLFWwindow* window;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(1000, 800, "test", NULL, NULL);
	if (!window)
	{
		check_gl_errors(__LINE__, __FILE__);
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	glewInit();

	printout_opengl_glsl_info();

	texture grass_texture;
	grass_texture.load(textures_path + "grass_tile.png", 0);

	shader texture_shader;
	texture_shader.create_program((shaders_path + "texture.vert").c_str(), (shaders_path + "texture.frag").c_str());


	/* create a rectangle*/
	renderable r_plane;
	shape s_plane;
	shape_maker::rectangle(s_plane, 1, 1);
	s_plane.to_renderable(r_plane);

	/* Transformation to setup the point of view on the scene */
	proj = glm::perspective(glm::radians(40.f), 1000 / float(800), 2.f, 20.f);
	view = glm::lookAt(glm::vec3(0, 3, 4.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

	glUseProgram(texture_shader.program);
	glUniformMatrix4fv(texture_shader["uModel"], 1, GL_FALSE, &glm::mat4(1.0)[0][0]);
	glUniformMatrix4fv(texture_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(texture_shader["uView"], 1, GL_FALSE, &view[0][0]);
	glUniform1i(texture_shader["uColorImage"], 0);

	glUseProgram(0);
	check_gl_errors(__LINE__, __FILE__, true);

	matrix_stack stack;

	glViewport(0, 0, 1000, 800);

	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClearColor(0.8f, 0.8f, 0.9f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		stack.push();

		glUseProgram(texture_shader.program);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, grass_texture.id);

		glUniformMatrix4fv(texture_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		r_plane.bind();
		glDrawElements(r_plane().mode, r_plane().count, r_plane().itype, 0);
		glUseProgram(0);

		check_gl_errors(__LINE__, __FILE__, true);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

}