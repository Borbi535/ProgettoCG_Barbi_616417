#include "light.hpp"

/***************CLASSE ASTRATTA LUCE*******************/

glm::vec3 Light::GetDirection() { return direction; }

glm::vec3 Light::GetColor() { return color; }

void Light::SetDirection(glm::vec3 newDirection) { direction = newDirection; }

void Light::SetColor(glm::vec3 newColor) { color = newColor; }

/******************************************************/

/*****************LUCE DIREZIONALE*********************/

DirectionalLight::DirectionalLight() {}

DirectionalLight::DirectionalLight(glm::vec3 _direction, glm::vec3 _color)
{
	this->direction = _direction;
	this->view_matrix = glm::lookAt(glm::vec3(0.0f, 2000.0f, 0.0f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
	this->proj_matrix = glm::ortho(-500.f, 500.f, -500.f, 500.f, 1000.0f, 2010.0f);
	this->color = _color;
}

/******************************************************/

/*****************LUCE POSIZIONALE*********************/

int PositionalLight::number_of_lights = 0;

PositionalLight::PositionalLight() {}

PositionalLight::PositionalLight(glm::vec3 position, glm::vec3 direction, float cutoff, float inner_cutoff, glm::vec3 color) : 
	position(position), cutoff(cutoff), inner_cutoff(inner_cutoff)
{
	this->direction = direction;
	this->view_matrix = glm::lookAt(position, position + glm::vec3(.0f, -1.0f, .0f), glm::vec3(.0f, .0f, 1.f));
	this->proj_matrix = glm::perspective(glm::radians(45.f), 1.f, 5.f, 100.f);
	this->color = color;

	number_of_lights++;

	// position e direction già in view space per il calcolo della luce
	lights_data.push_back(PositionalLightData(
		view_matrix * glm::vec4(position, 1.f),
		view_matrix * glm::vec4(glm::normalize(direction), 1.f),
		cutoff, inner_cutoff, color));
}

glm::vec3 PositionalLight::GetPosition() { return position; }

float PositionalLight::GetCutoff() { return cutoff; }

float PositionalLight::GetInnerCutoff() { return inner_cutoff; }

std::vector<PositionalLightData> PositionalLight::GetLightsData() { return lights_data; }

void PositionalLight::SetPosition(glm::vec3 newPosition) { position = newPosition; }

void PositionalLight::SetCutoff(float new_max_light_angle) { cutoff = new_max_light_angle; }

void PositionalLight::SetInnerCutoff(float new_min_light_angle) { inner_cutoff = new_min_light_angle; }


std::vector<PositionalLightData> PositionalLight::lights_data;

PositionalLightData::PositionalLightData(glm::vec3 position, glm::vec3 direction, float cutoff, float inner_cutoff, glm::vec3 color) :
	position(glm::vec4(position, 1)), direction(glm::vec4(direction, 0)), cutoff(cutoff), inner_cutoff(inner_cutoff), pad1(0), pad2(0), color(glm::vec4(color, 1)) {}

/******************************************************/