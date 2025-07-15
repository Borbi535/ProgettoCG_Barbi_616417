#pragma once

#include <iostream>
#include <vector>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm.hpp>
#include <gtx/string_cast.hpp>
#include <ext.hpp>

extern glm::mat4 projection_matrix;
extern glm::mat4 view_matrix;

class Light
{
public:
	virtual glm::vec3 GetDirection();
	virtual glm::vec3 GetColor();
	
	virtual void SetDirection(glm::vec3 newDirection);
	virtual void SetColor(glm::vec3 newColor);

protected:
	glm::vec3 direction;
	glm::vec3 color;

	glm::mat4 cam_view_matrix;
	glm::mat4 cam_proj_matrix;
};

class DirectionalLight : public Light
{
public:
	DirectionalLight();
	DirectionalLight(glm::vec3 _direction, glm::vec3 _color = glm::vec3(255, 255, 255));

private:
};

struct PositionalLightData
{
	PositionalLightData(glm::vec3 positionVS, float intensity, glm::vec3 color);

	glm::vec4 positionVS;
	float intensity, pad0, pad1, pad2;
	glm::vec4 color;
};

class PositionalLight : public Light
{
public:
	static int number_of_lights;

	PositionalLight();
	PositionalLight(glm::vec3 position, float intensity, glm::vec3 color = glm::vec3(255, 255, 255));
	PositionalLight(const PositionalLight& positional_light);

	glm::vec3 GetPosition();
	static std::vector<PositionalLightData> GetLightsData();

	void SetPosition(glm::vec3 new_pos);

private:
	glm::vec3 position;
	float intensity;

	static std::vector<PositionalLightData> positionallight_data;
};

struct SpotLightData
{
	SpotLightData(glm::vec3 positionVS, glm::vec3 directionVS, float cutoff, float inner_cutoff, glm::vec3 color);

	glm::vec4 positionVS;
	glm::vec4 directionVS;
	float cutoff, inner_cutoff;
	float pad1, pad2;
	glm::vec4 color;
};

class SpotLight : public Light
{
public:
	static int number_of_lights;

	SpotLight();
	SpotLight(glm::vec3 position, glm::vec3 direction, float cutoff, float inner_cutoff, glm::vec3 color = glm::vec3(255, 255, 255));

	glm::vec3 GetPosition();
	float GetCutoff();
	float GetInnerCutoff();
	static std::vector<SpotLightData> GetLightsData();

	void SetPosition(glm::vec3 new_pos);
	void SetCutoff(float new_cutoff);
	void SetInnerCutoff(float new_inner_cutoff);

	static std::vector<SpotLightData> spotlight_data;
private:

	glm::vec3 position;

	float cutoff;
	float inner_cutoff;
};


