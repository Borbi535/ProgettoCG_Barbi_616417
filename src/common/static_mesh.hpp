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

class StaticMesh
{
public:

	StaticMesh();
	StaticMesh(std::string filename);
	StaticMesh(const StaticMesh& mesh);
	~StaticMesh();

	static void SetShader(shader& new_shader);

	/// <summary>Draw the mesh.</summary>
	void Draw(matrix_stack& stack, float scale = 1.f);


private:

	static shader* _shader;

	tinygltf::Model model; // da levare, non serve memorizzarlo
	std::vector<renderable> object;
	box3 bbox;
	std::vector<GLuint> id_textures;
	int n_vert, n_tri;

	std::string filename;

	bool CreateRenderable();

	void VisitNodes(tinygltf::Node& node, glm::mat4 currT);
	void VisitMesh(tinygltf::Mesh& mesh, glm::mat4 currT);

	static std::string GetFilePathExtension(const std::string& FileName);
};