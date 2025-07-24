#include "light.hpp"

/***************CLASSE ASTRATTA LUCE*******************/

glm::vec3 Light::GetPosition() { return position; }

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

std::shared_ptr<Light> DirectionalLight::Clone() // non implementata
{
	assert(false);
	return std::shared_ptr<Light>();
}

/******************************************************/

/*****************LUCE POSIZIONALE*********************/

PositionalLightSSBOData::PositionalLightSSBOData(glm::vec3 positionVS, float intensity, glm::vec3 color):
	positionVS(glm::vec4(positionVS, 1.f)), intensity(intensity), color(glm::vec4(color, 1.f)) { pad0 = pad1 = pad2 = 0; }

GLuint PositionalLight::positional_lights_SSBO;

std::vector<std::shared_ptr<PositionalLight>> PositionalLight::positional_lights;

PositionalLight::PositionalLight() {}

PositionalLight::PositionalLight(glm::vec3 position, float intensity, glm::vec3 color):
	position(position), intensity(intensity), index(-1)
{
	this->color = color;
}

PositionalLight::PositionalLight(const PositionalLight& positional_light): index(-1)
{
	position = positional_light.position;
	intensity = positional_light.intensity;
	color = positional_light.color;
}

std::shared_ptr<PositionalLight> PositionalLight::Create(glm::vec3 position, float intensity, glm::vec3 color)
{
	std::shared_ptr<PositionalLight> instance = std::make_shared<PositionalLight>(position, intensity, color);
	positional_lights.push_back(instance);
	instance->index = positional_lights.size() - 1;
	return instance;
}

std::shared_ptr<PositionalLight> PositionalLight::Create(const PositionalLight& positional_light)
{
	std::shared_ptr<PositionalLight> instance = std::make_shared<PositionalLight>(positional_light);
	positional_lights.push_back(instance);
	instance->index = positional_lights.size() - 1;
	return instance;
}

std::shared_ptr<Light> PositionalLight::Clone()
{
	return PositionalLight::Create(*this);
}

glm::vec3 PositionalLight::GetPosition() { return position; }

int PositionalLight::GetNumberOfLights() { return positional_lights.size(); }

void PositionalLight::GenerateSSBO()
{
	std::vector<PositionalLightSSBOData> positional_lights_data;
	for (const std::shared_ptr<PositionalLight> light : positional_lights)
	{
		if (light)
			positional_lights_data.push_back(PositionalLightSSBOData(
				view_matrix * glm::vec4(light->position, 1.f),
				light->intensity,
				light->color));
		else xterminate("light empty", QUI);
	}

	glGenBuffers(1, &positional_lights_SSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, positional_lights_SSBO);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, positional_lights_SSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, positional_lights_data.size() * sizeof(PositionalLightSSBOData), positional_lights_data.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void PositionalLight::UpdateSSBO()
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, positional_lights_SSBO);
	PositionalLightSSBOData* mapped_lights = (PositionalLightSSBOData*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);

	if (mapped_lights)
	{
		for (unsigned int i = 0; i < positional_lights.size(); i++)
		{
			mapped_lights[i].positionVS = view_matrix * glm::vec4(positional_lights[i]->position, 1.f);
			mapped_lights[i].intensity = positional_lights[i]->intensity;
			mapped_lights[i].color = glm::vec4(positional_lights[i]->color, 1.f);
		}

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}
	else xterminate("Errore nella apertura di positional_lights_SSBO", QUI);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void PositionalLight::SetPosition(glm::vec3 newPosition) { position = newPosition; }

/******************************************************/

/*********************SPOTLIGHT************************/

std::vector<SpotLightSSBOData> SpotLight::spotlight_data;

SpotLightSSBOData::SpotLightSSBOData(glm::vec3 positionVS, glm::vec3 directionVS, float cutoff, float inner_cutoff, glm::vec3 color):
	positionVS(glm::vec4(positionVS, 1)),
	directionVS(glm::vec4(directionVS, 0)),
	cutoff(cutoff), inner_cutoff(inner_cutoff), pad1(0), pad2(0),
	color(glm::vec4(color, 1)) {}

SpotLight::SpotLight() {}

SpotLight::SpotLight(glm::vec3 position, glm::vec3 direction, float cutoff, float inner_cutoff, glm::vec3 color) : 
	position(position), cutoff(cutoff), inner_cutoff(inner_cutoff)
{
	this->direction = direction;
	this->cam_view_matrix = glm::lookAt(position, position + glm::vec3(.0f, -1.0f, .0f), glm::vec3(.0f, .0f, 1.f));
	this->cam_proj_matrix = glm::perspective(glm::radians(45.f), 1.f, 5.f, 100.f);
	this->color = color;

	spotlight_data.push_back(SpotLightSSBOData(
		view_matrix * glm::vec4(position, 1.f),
		view_matrix * glm::vec4(direction, 0.f),
		cutoff, inner_cutoff,
		color));
	index = spotlight_data.size() - 1;
}

std::shared_ptr<Light> SpotLight::Clone()
{
	return std::make_shared<SpotLight>(SpotLight(position, direction, cutoff, inner_cutoff, color));
}

glm::vec3 SpotLight::GetPosition() { return position; }

float SpotLight::GetCutoff() { return cutoff; }

float SpotLight::GetInnerCutoff() { return inner_cutoff; }

std::vector<SpotLightSSBOData> SpotLight::GetLightsData() { return spotlight_data; }

int SpotLight::GetNumberOfLights() { return spotlight_data.size(); }

void SpotLight::UpdateLightsData(GLuint spotlights_SSBO)
{
	if (spotlight_data.empty()) return;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, spotlights_SSBO);
	SpotLightSSBOData* mapped_lights = (SpotLightSSBOData*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
	if (mapped_lights)
	{
		for (unsigned int i = 0; i < spotlight_data.size(); i++)
		{
			spotlight_data[i].positionVS = view_matrix * spotlight_data[i].positionVS;
			spotlight_data[i].directionVS = view_matrix * spotlight_data[i].directionVS;

			mapped_lights[i].positionVS = spotlight_data[i].positionVS;
			mapped_lights[i].directionVS = spotlight_data[i].directionVS;
			mapped_lights[i].cutoff = spotlight_data[i].cutoff;
			mapped_lights[i].inner_cutoff = spotlight_data[i].inner_cutoff;
			mapped_lights[i].color = spotlight_data[i].color;
		}
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}
	else xterminate("Errore nella apertura di spotlights_SSBO", QUI);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void SpotLight::SetPosition(glm::vec3 newPosition)
{
	position = newPosition;
	spotlight_data[index].positionVS = view_matrix * glm::vec4(newPosition, 1.f);
}

void SpotLight::SetDirection(glm::vec3 new_direction)
{
	direction = new_direction;
	spotlight_data[index].directionVS = view_matrix * glm::vec4(new_direction, 0.f);
}

void SpotLight::SetCutoff(float new_cutoff)
{
	cutoff = new_cutoff;
	spotlight_data[index].cutoff = new_cutoff;
}

void SpotLight::SetInnerCutoff(float new_inner_cutoff)
{
	inner_cutoff = new_inner_cutoff;
	spotlight_data[index].inner_cutoff = new_inner_cutoff;
}



/******************************************************/
