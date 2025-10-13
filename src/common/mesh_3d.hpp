#pragma once

#include <stb_image.h>
//#include <stb_image_write.h>

#include <tiny_gltf.h>
#include <GL/glew.h>
//#include "texture.h"
#include "xerrori.hpp"
#include "debugging.h"
#include "renderable.h"
#include "matrix_stack.h"
#include "shaders.h"
#include "aabb.h"

class Mesh3D
{
public:

	Mesh3D();
	Mesh3D(std::string filename, float scale = 1.f);
	Mesh3D(const Mesh3D& mesh);
	~Mesh3D();

	AABB GetAABB() const;

	static void SetShaders(shader* draw_shader, shader* depth_shader);

	void Draw(matrix_stack& stack) const;
	void DrawDepthMap(matrix_stack& stack) const;


private:

	static shader* draw_shader;
	static GLuint draw_uColorLocation;
	static GLuint draw_uTextureAvailableLocation;
	static GLuint draw_uModelLocation;

	static shader* depth_shader;
	static GLuint depth_uColorLocation;
	static GLuint depth_uTextureAvailableLocation;
	static GLuint depth_uModelLocation;

	tinygltf::Model model; // da levare, non serve memorizzarlo
	std::vector<renderable> object;
	box3 bbox;
	AABB aabb;
	std::vector<GLuint> id_textures;
	int n_vert, n_tri;
	float scale;

	std::string filename;

	bool CreateRenderable();

	void VisitNodes(tinygltf::Node& node, glm::mat4 currT);
	void VisitMesh(tinygltf::Mesh& mesh, glm::mat4 currT);

	static std::string GetFilePathExtension(const std::string& FileName);

	static void InitUniformLocation();
};