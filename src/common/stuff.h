#pragma once

extern int window_width;
extern int window_height;
extern glm::vec2 window_center;
extern glm::mat4 projection_matrix;
extern glm::mat4 view_matrix;


struct Plane
{
	glm::vec3 normal;
	float distance; // distanza dall'origine al piano lungo la normale
};

/// <summary>
/// Teorema di Pitagora, se hypotenuse = true, allora il parametro l1 deve essere l'ipotenusa
/// </summary>
inline double pythagoras(double l1, double l2, bool hypotenuse = false)
{
	assert(l1 > 0 && l2 > 0);

	if (hypotenuse)
	{
		assert(l1 > l2);

		return glm::sqrt(pow(l1, 2) - pow(l2, 2));
	}

	return glm::sqrt(pow(l1, 2) + pow(l2, 2));
}

/// <summary>
/// Formula di Erone
/// </summary>
inline double hero(double l1, double l2, double l3)
{
	assert(l1 > 0 && l2 > 0 && l3 > 0);

	double p = (l1 + l2 + l3) / 2;
	return glm::sqrt(p * (p - l1) * (p - l2) * (p - l3));
}

/// <summary>
/// Da vec2 viewport a vec3 world space
/// </summary>
inline glm::vec3 ViewportToWorldSpace(glm::vec2 p)
{
	float depth_value;
	GLint viewport_y = window_height - 1 - p.y;
	glReadPixels(p.x, viewport_y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth_value);
	return glm::unProject(
		glm::vec3(p.x, viewport_y, depth_value),
		view_matrix,
		projection_matrix,
		glm::vec4(0, 0, window_width, window_height));
}

inline glm::vec3 PerpendicularVector(glm::vec3 v1, glm::vec3 v2)
{
	assert(v1 != glm::vec3(0) && v2 != glm::vec3(0));

	glm::vec3 v1_normalized = glm::normalize(v1);
	glm::vec3 v2_normalized = glm::normalize(v2);

	return glm::normalize(glm::cross(v1_normalized, v2_normalized));
}





