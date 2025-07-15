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
	this->cam_view_matrix = glm::lookAt(glm::vec3(0.0f, 2000.0f, 0.0f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
	this->cam_proj_matrix = glm::ortho(-500.f, 500.f, -500.f, 500.f, 1000.0f, 2010.0f);
	this->color = _color;
}

/******************************************************/

/*****************LUCE POSIZIONALE*********************/

PositionalLightData::PositionalLightData(glm::vec3 positionVS, float intensity, glm::vec3 color):
	positionVS(glm::vec4(positionVS, 1.f)), intensity(intensity), color(glm::vec4(color, 1.f)) {}

std::vector<PositionalLightData> PositionalLight::positionallight_data;

int PositionalLight::number_of_lights = 0;

PositionalLight::PositionalLight() {}

PositionalLight::PositionalLight(glm::vec3 position, float intensity, glm::vec3 color): position(position), intensity(intensity)
{
	this->color = color;
	number_of_lights++;

	positionallight_data.push_back(PositionalLightData(view_matrix * glm::vec4(position, 1.f), intensity, color));
}

PositionalLight::PositionalLight(const PositionalLight& positional_light)
{
	position = positional_light.position;
	color = positional_light.color;

	number_of_lights++;
}

glm::vec3 PositionalLight::GetPosition() { return position; }

std::vector<PositionalLightData> PositionalLight::GetLightsData() { return positionallight_data; }

void PositionalLight::SetPosition(glm::vec3 newPosition) { position = newPosition; }

/******************************************************/

/*********************SPOTLIGHT************************/

std::vector<SpotLightData> SpotLight::spotlight_data;

SpotLightData::SpotLightData(glm::vec3 positionVS, glm::vec3 directionVS, float cutoff, float inner_cutoff, glm::vec3 color) :
	positionVS(glm::vec4(positionVS, 1)),
	directionVS(glm::vec4(directionVS, 0)),
	cutoff(cutoff), inner_cutoff(inner_cutoff), pad1(0), pad2(0),
	color(glm::vec4(color, 1)) {}

int SpotLight::number_of_lights = 0;

SpotLight::SpotLight() {}

SpotLight::SpotLight(glm::vec3 position, glm::vec3 direction, float cutoff, float inner_cutoff, glm::vec3 color) : 
	position(position), cutoff(cutoff), inner_cutoff(inner_cutoff)
{
	this->direction = direction;
	this->cam_view_matrix = glm::lookAt(position, position + glm::vec3(.0f, -1.0f, .0f), glm::vec3(.0f, .0f, 1.f));
	this->cam_proj_matrix = glm::perspective(glm::radians(45.f), 1.f, 5.f, 100.f);
	this->color = color;

	number_of_lights++;

	// position e direction già in view space per il calcolo della luce
	spotlight_data.push_back(SpotLightData(
		view_matrix * glm::vec4(position, 1.f),
		glm::normalize(view_matrix * glm::vec4(glm::normalize(direction), 0.f)),
		cutoff, inner_cutoff, color));
}

glm::vec3 SpotLight::GetPosition() { return position; }

float SpotLight::GetCutoff() { return cutoff; }

float SpotLight::GetInnerCutoff() { return inner_cutoff; }

std::vector<SpotLightData> SpotLight::GetLightsData() { return spotlight_data; }

void SpotLight::SetPosition(glm::vec3 newPosition) { position = newPosition; }

void SpotLight::SetCutoff(float new_max_light_angle) { cutoff = new_max_light_angle; }

void SpotLight::SetInnerCutoff(float new_min_light_angle) { inner_cutoff = new_min_light_angle; }



/******************************************************/
