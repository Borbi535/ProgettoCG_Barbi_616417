#include<body_3d.hpp>



Body3D::Body3D(glm::mat4 model_matrix, const Mesh3D& model) : _model_matrix(model_matrix), _mesh(model) {}

Body3D::Body3D(const Body3D& body3d): _model_matrix(body3d._model_matrix), _mesh(body3d._mesh)
{
	for (const auto& light : body3d.lights)
	{
		if (light)
			lights.push_back(light->Clone());
		else
			lights.push_back(nullptr);
	}
}

void Body3D::AddLight(std::shared_ptr<Light> light)
{
	std::shared_ptr<Light> light_ = light;
	lights.push_back(light_);
	light_->UpdatePointsAndVectors(_model_matrix);
}

void Body3D::Rotate(glm::vec3 axis, float angle)
{
	_model_matrix = glm::rotate(_model_matrix, glm::radians(angle), axis);
}

void Body3D::Draw(matrix_stack& stack)
{
	stack.push();
	stack.mult(_model_matrix);
	_mesh.Draw(stack);
	stack.pop();
}

void Body3D::UpdateLights()
{
	for (const std::shared_ptr<Light> light : lights)
	{
		light->UpdatePointsAndVectors(_model_matrix);
	}
}

glm::mat4 Body3D::GetModelMatrix() const { return _model_matrix; }

void Body3D::SetPosition(glm::vec3 position)
{
	_model_matrix = glm::translate(glm::mat4(1.f), position) * _model_matrix;
}

void Body3D::SetScale(float scale)
{
	_model_matrix = glm::scale(glm::mat4(1.f), glm::vec3(scale)) * _model_matrix;
}

void Body3D::SetModelMatrix(glm::mat4 model_matrix) { _model_matrix = model_matrix; }
