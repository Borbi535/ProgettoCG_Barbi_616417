#pragma once

#include <iostream>
#include <vector>
#include <xerrori.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm.hpp>
#include <gtx/string_cast.hpp>
#include <ext.hpp>
#include <GL/glew.h>
#include <shaders.h>
#include <aabb.h>
#include <stuff.h>


class Light
{
public:
	virtual std::shared_ptr<Light> Clone() = 0;

	template<class T>
	static void CallUpdateLightsData(GLuint positional_lights_SSBO)
	{
		T::UpdateLightsData(positional_lights_SSBO);
	}

	virtual glm::vec3 GetPosition() const = 0;
	virtual glm::vec3 GetDirection() const;
	virtual glm::vec3 GetColor() const;
	
	virtual void SetDirection(glm::vec3 new_direction) = 0;
	virtual void SetColor(glm::vec3 new_color);
	virtual void SetPosition(glm::vec3 new_pos) = 0;
	virtual void UpdatePointsAndVectors(glm::mat4 matrix) = 0;

protected:
	glm::vec3 direction;
	glm::vec3 color;

	glm::mat4 light_view_matrix;
	glm::mat4 light_proj_matrix;
};

class DirectionalLight : public Light
{
public:
	DirectionalLight();
	DirectionalLight(glm::vec3 _direction, glm::vec3 _color = glm::vec3(255, 255, 255));
	std::shared_ptr<Light> Clone() override;

	glm::vec3 GetPosition() const override 
	{ xterminate("You're trying to get the position of a directional light!", QUI); return glm::vec3(0); }

	void SetPosition(glm::vec3 new_pos) override
	{ xterminate("You're trying to set the position of a directional light!", QUI); }
	void SetDirection(glm::vec3 new_direction) override;

	void UpdatePointsAndVectors(glm::mat4 matrix) override;

private:
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

	glm::vec3 GetPosition() const override;

	void UpdatePointsAndVectors(glm::mat4 matrix) override;

	static int GetNumberOfLights();

	static void GenerateSSBO(glm::mat4 view_matrix);
	static void UpdateSSBO(glm::mat4 view_matrix);

	void SetPosition(glm::vec3 new_pos) override;
	void SetDirection(glm::vec3 new_direction) override
	{ xterminate("You're trying to set the direction of a positional light!", QUI); }

private:
	glm::vec3 position;
	float intensity;

	static GLuint positional_lights_SSBO;
	int index;
	static std::vector<std::shared_ptr<PositionalLight>> positional_lights;
};

struct SpotLightSSBOData
{
	SpotLightSSBOData(glm::vec3 positionVS, glm::vec3 directionVS, float cutoff, float inner_cutoff, glm::vec3 color, glm::mat4 light_matrix);

	glm::vec4 positionVS;
	glm::vec4 directionVS;
	float cutoff, inner_cutoff;
	float pad1, pad2;
	glm::vec4 color;
	glm::mat4 light_matrix;
};

class SpotLight : public Light
{
public:
	SpotLight();
	SpotLight(glm::vec3 position, glm::vec3 direction, float cutoff, float inner_cutoff, float range, glm::vec3 color = glm::vec3(255, 255, 255));
	SpotLight(const SpotLight& spotlight);
	static std::shared_ptr<SpotLight> Create(glm::vec3 position, glm::vec3 direction, float cutoff, float inner_cutoff, float range, glm::vec3 color = glm::vec3(255, 255, 255));
	static std::shared_ptr<SpotLight> Create(const SpotLight& spotlight);

	std::shared_ptr<Light> Clone() override;

	glm::vec3 GetPosition() const override;
	float GetCutoff() const;
	float GetInnerCutoff() const;
	glm::mat4 GetLightMatrix() const;

	static int GetNumberOfLights();
	static std::vector<std::shared_ptr<SpotLight>>& GetSpotLights();
	static glm::mat4 GetLightMatrix(int index);

	static void GenerateSSBO(glm::mat4 view_matrix);
	static void UpdateSSBO(glm::mat4 view_matrix);

	void SetPosition(glm::vec3 new_pos);
	void SetDirection(glm::vec3 new_direction);
	void SetCutoff(float new_cutoff);
	void SetInnerCutoff(float new_inner_cutoff);

	void UpdatePointsAndVectors(glm::mat4 matrix) override;

private:
	
	glm::vec3 position;
	glm::vec3 local_position, local_direction;
	float cutoff, inner_cutoff, range;

	static GLuint spot_lights_SSBO;
	int index;
	static std::vector<std::shared_ptr<SpotLight>> spot_lights;

	std::vector<Plane> extract_frustum_planes() const;

	bool intersects_frustum(const std::vector<Plane>& frustum_planes, const AABB& obj_aabb);
};
