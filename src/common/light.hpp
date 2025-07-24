#pragma once

#include <iostream>
#include <vector>
#include <xerrori.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm.hpp>
#include <gtx/string_cast.hpp>
#include <ext.hpp>
#include <GL/glew.h>

extern glm::mat4 projection_matrix;
extern glm::mat4 view_matrix;

class Light
{
public:
	virtual std::shared_ptr<Light> Clone() = 0;

	template<class T>
	static void CallUpdateLightsData(GLuint positional_lights_SSBO)
	{
		T::UpdateLightsData(positional_lights_SSBO);
	}

	virtual glm::vec3 GetPosition();
	virtual glm::vec3 GetDirection();
	virtual glm::vec3 GetColor();
	
	virtual void SetDirection(glm::vec3 newDirection);
	virtual void SetColor(glm::vec3 newColor);
	virtual void SetPosition(glm::vec3 new_pos) = 0;

protected:
	glm::vec3 position;
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
	std::shared_ptr<Light> Clone() override;

private:
	void SetPosition(glm::vec3 new_pos) override {;};
};

struct PositionalLightSSBOData
{
	PositionalLightSSBOData(glm::vec3 positionVS, float intensity, glm::vec3 color);

	glm::vec4 positionVS;
	float intensity, pad0, pad1, pad2;
	glm::vec4 color;
};

class PositionalLight : public Light, std::enable_shared_from_this<PositionalLight>
{
public:
	PositionalLight();
	PositionalLight(glm::vec3 position, float intensity, glm::vec3 color = glm::vec3(255, 255, 255));
	PositionalLight(const PositionalLight& positional_light);
	static std::shared_ptr<PositionalLight> Create(glm::vec3 position, float intensity, glm::vec3 color = glm::vec3(255, 255, 255));
	static std::shared_ptr<PositionalLight> Create(const PositionalLight& positional_light);

	std::shared_ptr<Light> Clone() override;

	glm::vec3 GetPosition() override;

	static int GetNumberOfLights();

	static void GenerateSSBO();
	static void UpdateSSBO();

	void SetPosition(glm::vec3 new_pos) override;

private:
	glm::vec3 position;
	float intensity;

	static GLuint positional_lights_SSBO;
	int index;
	static std::vector<std::shared_ptr<PositionalLight>> positional_lights;
};

struct SpotLightSSBOData
{
	SpotLightSSBOData(glm::vec3 positionVS, glm::vec3 directionVS, float cutoff, float inner_cutoff, glm::vec3 color);

	glm::vec4 positionVS;
	glm::vec4 directionVS;
	float cutoff, inner_cutoff;
	float pad1, pad2;
	glm::vec4 color;
};

class SpotLight : public Light
{
public:
	SpotLight();
	SpotLight(glm::vec3 position, glm::vec3 direction, float cutoff, float inner_cutoff, glm::vec3 color = glm::vec3(255, 255, 255));

	std::shared_ptr<Light> Clone() override;

	glm::vec3 GetPosition();
	float GetCutoff();
	float GetInnerCutoff();

	static std::vector<SpotLightSSBOData> GetLightsData();
	static int GetNumberOfLights();

	/// <summary>Updates the SSBO</summary>
	static void UpdateLightsData(GLuint positional_lights_SSBO);

	void SetPosition(glm::vec3 new_pos);
	void SetDirection(glm::vec3 new_direction);
	void SetCutoff(float new_cutoff);
	void SetInnerCutoff(float new_inner_cutoff);

private:

	glm::vec3 position;
	float cutoff, inner_cutoff;

	int index;
	static std::vector<SpotLightSSBOData> spotlight_data;
};
