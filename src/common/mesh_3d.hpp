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

class Mesh3D
{
public:
	std::vector<glm::vec3> light_local_positions;

	Mesh3D();
	Mesh3D(std::string filename, float scale = 1.f);
	Mesh3D(const Mesh3D& mesh);
	~Mesh3D();

	static void SetShader(shader& new_shader);

	void Draw(matrix_stack& stack) const;


private:

	static shader* _shader;

	tinygltf::Model model; // da levare, non serve memorizzarlo
	std::vector<renderable> object;
	box3 bbox;
	std::vector<GLuint> id_textures;
	int n_vert, n_tri;
	float scale;

	std::string filename;

	bool CreateRenderable();

	void VisitNodes(tinygltf::Node& node, glm::mat4 currT);
	void VisitMesh(tinygltf::Mesh& mesh, glm::mat4 currT);

	static std::string GetFilePathExtension(const std::string& FileName);
};