#pragma once

#include <iostream>
#include <glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <ext.hpp>
#include <GL/glew.h>

enum CameraID { TRACKBALL_ID, FREECAM_ID, CAMERAMAN1_ID, CAMERAMAN2_ID, CAMERAMAN3_ID, CAMERAMAN4_ID, CAMS_NUMBER };

class Camera
{
public:
	bool is_active = false;

	Camera();
	Camera(glm::vec3 eye, glm::vec3 center, glm::vec3 up);

	Camera(Camera& cam); // cc

	~Camera();

	glm::mat4 GetViewMatrix();
	glm::vec3 GetPosition();

	void SetParameters(glm::vec3 eye, glm::vec3 view_dir, glm::vec3 up_dir); // TODO: trovare un nome migliore
	

protected:
	glm::vec3 _eye, view_direction, up_direction;
};