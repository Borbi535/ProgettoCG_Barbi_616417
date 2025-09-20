#pragma once

#include<carousel/carousel.h>
#include<mesh_3d.hpp>
#include<light.hpp>
#include<aabb.h>

class Body3D
{
public:
	Body3D(glm::mat4 model_matrix, const Mesh3D& model);
	Body3D(const Body3D& body3d);

	void AddLight(std::shared_ptr<Light> light);

	void Rotate(glm::vec3 axis, float angle);
		
	void Draw(matrix_stack& stack);
	void UpdateLights();

	glm::mat4 GetModelMatrix() const;

	void SetPosition(glm::vec3 position);
	void SetScale(float scale);
	void SetModelMatrix(glm::mat4 model_matrix);

private:
	glm::mat4 _model_matrix;

	const Mesh3D& _mesh;
	std::vector<std::shared_ptr<Light>> lights;
};