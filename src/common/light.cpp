#include "light.hpp"

/***************CLASSE ASTRATTA LUCE*******************/

glm::vec3 Light::GetDirection() const { return direction; }

glm::vec3 Light::GetColor() const { return color; }

glm::mat4 Light::GetLightMatrix() const { return light_matrix; }

void Light::SetColor(glm::vec3 new_color) { color = new_color; }

bool Light::IntersectFrustum(const AABB& obj_aabb)
{
	if (obj_aabb.isNull()) return false;

	glm::vec3 aabbMin = obj_aabb.getMin();
	glm::vec3 aabbMax = obj_aabb.getMax();

	for (const auto& plane : light_frustum_planes)
	{
		// p_vertex: il vertice della AABB che è più lontano dalla normale del piano (lato positivo della normale)
		// n_vertex: il vertice della AABB che è più vicino alla normale del piano (lato negativo della normale)
		// se anche il punto più favorevole della AABB è fuori dal piano, allora tutta l'AABB è fuori.

		glm::vec3 p_vertex = aabbMin;
		glm::vec3 n_vertex = aabbMax;

		if (plane.normal.x >= 0)
		{
			p_vertex.x = aabbMax.x;
			n_vertex.x = aabbMin.x;
		}
		else
		{
			p_vertex.x = aabbMin.x;
			n_vertex.x = aabbMax.x;
		}
		if (plane.normal.y >= 0)
		{
			p_vertex.y = aabbMax.y;
			n_vertex.y = aabbMin.y;
		}
		else
		{
			p_vertex.y = aabbMin.y;
			n_vertex.y = aabbMax.y;
		}
		if (plane.normal.z >= 0)
		{
			p_vertex.z = aabbMax.z;
			n_vertex.z = aabbMin.z;
		}
		else
		{
			p_vertex.z = aabbMin.z;
			n_vertex.z = aabbMax.z;
		}

		// Calcola la distanza del p-vertex dal piano.
		// Se questa distanza è negativa (il p-vertex è sul lato "esterno" del piano),
		// allora tutta l'AABB è completamente fuori da questo piano.
		if (glm::dot(plane.normal, p_vertex) + plane.distance < 0.0f) return false;
	}
	return true;
}

void Light::ComputeFrustumPlanes()
{
	glm::mat4 m = glm::transpose(GetLightMatrix()); // trasposta per facilitare l'estrazione

	// LEFT
	light_frustum_planes[0].normal.x = m[3][0] + m[0][0];
	light_frustum_planes[0].normal.y = m[3][1] + m[0][1];
	light_frustum_planes[0].normal.z = m[3][2] + m[0][2];
	light_frustum_planes[0].distance = m[3][3] + m[0][3];

	// RIGHT
	light_frustum_planes[1].normal.x = m[3][0] - m[0][0];
	light_frustum_planes[1].normal.y = m[3][1] - m[0][1];
	light_frustum_planes[1].normal.z = m[3][2] - m[0][2];
	light_frustum_planes[1].distance = m[3][3] - m[0][3];

	// BOTTOM
	light_frustum_planes[2].normal.x = m[3][0] + m[1][0];
	light_frustum_planes[2].normal.y = m[3][1] + m[1][1];
	light_frustum_planes[2].normal.z = m[3][2] + m[1][2];
	light_frustum_planes[2].distance = m[3][3] + m[1][3];

	// TOP
	light_frustum_planes[3].normal.x = m[3][0] - m[1][0];
	light_frustum_planes[3].normal.y = m[3][1] - m[1][1];
	light_frustum_planes[3].normal.z = m[3][2] - m[1][2];
	light_frustum_planes[3].distance = m[3][3] - m[1][3];

	// NEAR
	light_frustum_planes[4].normal.x = m[3][0] + m[2][0];
	light_frustum_planes[4].normal.y = m[3][1] + m[2][1];
	light_frustum_planes[4].normal.z = m[3][2] + m[2][2];
	light_frustum_planes[4].distance = m[3][3] + m[2][3];

	// FAR
	light_frustum_planes[5].normal.x = m[3][0] - m[2][0];
	light_frustum_planes[5].normal.y = m[3][1] - m[2][1];
	light_frustum_planes[5].normal.z = m[3][2] - m[2][2];
	light_frustum_planes[5].distance = m[3][3] - m[2][3];

	// normalizzo i piani
	for (int i = 0; i < 6; ++i)
	{
		float length = glm::length(light_frustum_planes[i].normal);
		light_frustum_planes[i].normal /= length;
		light_frustum_planes[i].distance /= length;
	}
}

void Light::Update()
{
	light_matrix = light_proj_matrix * light_view_matrix;
	ComputeFrustumPlanes();
}

/******************************************************/

/*****************LUCE DIREZIONALE*********************/

DirectionalLight::DirectionalLight() {}

DirectionalLight::DirectionalLight(glm::vec3 _direction, glm::vec3 _color)
{
	this->direction = _direction;
	this->light_view_matrix = glm::lookAt(glm::vec3(0.0f, 2000.0f, 0.0f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
	this->light_proj_matrix = glm::ortho(-500.f, 500.f, -500.f, 500.f, 1000.0f, 2010.0f);
	this->color = _color;
}

std::shared_ptr<Light> DirectionalLight::Clone() // non implementata
{
	assert(false);
	return std::shared_ptr<Light>();
}

void DirectionalLight::SetDirection(glm::vec3 new_direction) { direction = new_direction; }

void DirectionalLight::ApplyMatrix(glm::mat4 matrix)
{
	direction = glm::vec3(matrix * glm::vec4(direction, 0.f));
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

	light_proj_matrix = glm::perspective(glm::radians(120.f), 1.f, 0.01f, (float)(intensity / 30.0));
	light_view_matrix = glm::lookAt(position, position - glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	light_frustum_planes = std::vector<Plane>(6);
	Update();
}

PositionalLight::PositionalLight(const PositionalLight& positional_light): index(-1)
{
	position = positional_light.position;
	intensity = positional_light.intensity;
	color = positional_light.color;
	light_proj_matrix = positional_light.light_proj_matrix;
	light_view_matrix = positional_light.light_view_matrix;
	light_frustum_planes = positional_light.light_frustum_planes;
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

std::shared_ptr<Light> PositionalLight::Clone() { return PositionalLight::Create(*this); }

glm::vec3 PositionalLight::GetPosition() const { return position; }

int PositionalLight::GetNumberOfLights() { return positional_lights.size(); }

std::shared_ptr<PositionalLight>& PositionalLight::GetPositionalLight(int index)
{
	if (index < 0 || index >= positional_lights.size())
		xterminate("Index out of range in PositionalLight::GetPositionalLight", QUI);

	return positional_lights[index];
}

void PositionalLight::GenerateSSBO(glm::mat4 view_matrix)
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
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, positional_lights_SSBO); // id 0 for positional lights

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, positional_lights_SSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, positional_lights_data.size() * sizeof(PositionalLightSSBOData), positional_lights_data.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void PositionalLight::UpdateSSBO(glm::mat4 view_matrix)
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
	else std::cout << "Errore nella apertura di positional_lights_SSBO" << std::endl;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void PositionalLight::SetPosition(glm::vec3 newPosition)
{
	position = newPosition;
	light_view_matrix = glm::lookAt(position, position + glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	Update();
}

void PositionalLight::ApplyMatrix(glm::mat4 matrix)
{
	position = glm::vec3(matrix * glm::vec4(position, 1.f));
	light_view_matrix = glm::lookAt(position, position + glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	Update();
}

/******************************************************/

/*********************SPOTLIGHT************************/

GLuint SpotLight::spot_lights_SSBO;

std::vector<std::shared_ptr<SpotLight>> SpotLight::spot_lights;

SpotLightSSBOData::SpotLightSSBOData(glm::vec3 positionVS, glm::vec3 directionVS, float cutoff, float inner_cutoff, glm::vec3 color, glm::mat4 light_matrix):
	positionVS(glm::vec4(positionVS, 1)),
	directionVS(glm::vec4(directionVS, 0)),
	cutoff(cutoff), inner_cutoff(inner_cutoff), pad1(0), pad2(0),
	color(glm::vec4(color, 1)),
	light_matrix(light_matrix) {}

SpotLight::SpotLight() {}

SpotLight::SpotLight(glm::vec3 position, glm::vec3 direction, float cutoff, float inner_cutoff, float range, glm::vec3 color) : 
	cutoff(cutoff), inner_cutoff(inner_cutoff), range(range), index(-1)
{
	this->position = position;
	this->direction = glm::normalize(direction);
	local_position = position;
	local_direction = direction;
	light_view_matrix = glm::lookAt(position, position + direction, glm::vec3(.0f, 1.f, .0f));
	light_proj_matrix = glm::perspective(glm::radians(cutoff), 1.f, 0.1f, range/10);
	light_matrix = light_proj_matrix * light_view_matrix;
	this->color = color;
	light_frustum_planes = std::vector<Plane>(6); // 6 piani: left, right, bottom, top, near, far
	Update();
}

SpotLight::SpotLight(const SpotLight& spotlight): index(-1)
{
	position = spotlight.position;
	local_position = spotlight.local_position;
	local_direction = spotlight.local_direction;
	direction = spotlight.direction;
	cutoff = spotlight.cutoff;
	inner_cutoff = spotlight.inner_cutoff;
	light_view_matrix = spotlight.light_view_matrix;
	light_proj_matrix = spotlight.light_proj_matrix;
	light_matrix = spotlight.light_matrix;
	range = spotlight.range;
	color = spotlight.color;
	light_frustum_planes = spotlight.light_frustum_planes;
	Update();
}

std::shared_ptr<SpotLight> SpotLight::Create(glm::vec3 position, glm::vec3 direction, float cutoff, float inner_cutoff, float range, glm::vec3 color)
{
	std::shared_ptr<SpotLight> instance = std::make_shared<SpotLight>(position, direction, cutoff, inner_cutoff, range, color);
	spot_lights.push_back(instance);
	instance->index = spot_lights.size() - 1;
	return instance;
}

std::shared_ptr<SpotLight> SpotLight::Create(const SpotLight& spotlight)
{
	std::shared_ptr<SpotLight> instance = std::make_shared<SpotLight>(spotlight);
	spot_lights.push_back(instance);
	instance->index = spot_lights.size() - 1;
	return instance;
}

std::shared_ptr<Light> SpotLight::Clone() { return SpotLight::Create(*this); }

glm::vec3 SpotLight::GetPosition() const { return position; }

float SpotLight::GetCutoff() const { return cutoff; }

float SpotLight::GetInnerCutoff() const { return inner_cutoff; }

int SpotLight::GetNumberOfLights() { return spot_lights.size(); }

std::shared_ptr<SpotLight>& SpotLight::GetSpotLight(int index)
{
	if (index < 0 || index >= spot_lights.size())
	{
		xterminate("Index out of range in SpotLight::GetSpotLight(int index)", QUI);
	}
	return spot_lights[index];
}

void SpotLight::GenerateSSBO(glm::mat4 view_matrix)
{
	std::vector<SpotLightSSBOData> spotlight_data;
	for (const std::shared_ptr<SpotLight> light : spot_lights)
	{
		if (light)
			spotlight_data.push_back(SpotLightSSBOData(
				view_matrix * glm::vec4(light->position, 1.f),
				view_matrix * glm::vec4(light->direction, 0.f),
				light->cutoff,
				light->inner_cutoff,
				light->color,
				light->light_proj_matrix * light->light_view_matrix));
		else xterminate("light empty", QUI);
	}
	glGenBuffers(1, &spot_lights_SSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, spot_lights_SSBO); // id 1 for spot lights
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, spot_lights_SSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, spotlight_data.size() * sizeof(SpotLightSSBOData), spotlight_data.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void SpotLight::UpdateSSBO(glm::mat4 view_matrix)
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, spot_lights_SSBO);
	SpotLightSSBOData* mapped_lights = (SpotLightSSBOData*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);

	if (mapped_lights)
	{
		for (unsigned int i = 0; i < spot_lights.size(); i++)
		{
			mapped_lights[i].positionVS = view_matrix * glm::vec4(spot_lights[i]->position, 1.f);
			mapped_lights[i].directionVS = view_matrix * glm::vec4(spot_lights[i]->direction, 0.f);
			mapped_lights[i].cutoff = spot_lights[i]->cutoff;
			mapped_lights[i].inner_cutoff = spot_lights[i]->inner_cutoff;
			mapped_lights[i].color = glm::vec4(spot_lights[i]->color, 1.f);
			mapped_lights[i].light_matrix = spot_lights[i]->light_proj_matrix * spot_lights[i]->light_view_matrix;
		}
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}
	else std::cout << "Errore nella apertura di spotlights_SSBO" << std::endl;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void SpotLight::SetPosition(glm::vec3 new_position)
{
	position = new_position;
	light_view_matrix = glm::lookAt(position, position + direction, glm::vec3(.0f, 1.f, .0f));
	Update();
}

void SpotLight::SetDirection(glm::vec3 new_direction)
{
	direction = glm::normalize(new_direction);
	light_view_matrix = glm::lookAt(position, position + direction, glm::vec3(.0f, 1.f, .0f));
	Update();
}

void SpotLight::SetCutoff(float new_cutoff)
{
	cutoff = new_cutoff;
	light_proj_matrix = glm::perspective(glm::radians(cutoff), 1.f, 0.1f, range / 10);
	Update();
}

void SpotLight::SetInnerCutoff(float new_inner_cutoff) { inner_cutoff = new_inner_cutoff; }

/// <summary>Apply the matrix to any point and vector such as position and direction.
/// This updates the light's view matrix also.</summary>
void SpotLight::ApplyMatrix(glm::mat4 matrix)
{
	position = glm::vec3(matrix * glm::vec4(local_position, 1.f));
	direction = glm::normalize(glm::vec3(matrix * glm::vec4(local_direction, 0.f)));
	light_view_matrix = glm::lookAt(position, position + direction, glm::vec3(.0f, 1.f, .0f));
	Update();
}






/******************************************************/
