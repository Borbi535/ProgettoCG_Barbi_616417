#pragma once

#include <vector>
#include <glm.hpp>
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

	glm::mat4 view_matrix;
	glm::mat4 proj_matrix;
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
	PositionalLightData(glm::vec3 position, glm::vec3 direction, float cutoff, float inner_cutoff, glm::vec3 color);

	glm::vec4 position;
	glm::vec4 direction;
	float cutoff, inner_cutoff;
	float pad1, pad2;
	glm::vec4 color;
};

class PositionalLight : public Light
{
public:
	static int number_of_lights;

	PositionalLight();
	PositionalLight(glm::vec3 position, glm::vec3 direction, float cutoff, float inner_cutoff, glm::vec3 color = glm::vec3(255, 255, 255));

	glm::vec3 GetPosition();
	float GetCutoff();
	float GetInnerCutoff();
	static std::vector<PositionalLightData> GetLightsData();

	void SetPosition(glm::vec3 newPosition);
	void SetCutoff(float new_cutoff);
	void SetInnerCutoff(float new_inner_cutoff);
	static std::vector<PositionalLightData> lights_data;
private:

	glm::vec3 position;

	float cutoff;
	float inner_cutoff;
};

