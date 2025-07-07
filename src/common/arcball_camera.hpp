#pragma once

#include <camera.hpp>
#include <stuff.h>
#include <input_manager.hpp>

class ArcballCamera : public Camera
{
private:
	glm::vec2 p0, p1;
	glm::vec3 forward_direction;
	glm::vec3 right_direction;

	float pan_sensitivity = 0.001f;
	float zoom_sensitivity = 0.1f;
	float rotation_sensitivity = 0.2f;

	bool mouse_pressed = false;
	bool scroll_pressed = false;

	bool IsMoving();
	bool IsChanged();

public:

	ArcballCamera();
	ArcballCamera(glm::vec3 eye, glm::vec3 center, glm::vec3 up);
	ArcballCamera(ArcballCamera& ac);
	~ArcballCamera();

	void MouseMove();
	void MouseLPress();
	void MouseLRelease();
	void MouseScroll();
	void MouseScrollPress();
	void MouseScrollRelease();
};