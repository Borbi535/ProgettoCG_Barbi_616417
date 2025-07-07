#pragma once

#include <camera.hpp>
#include <input_manager.hpp>


class Freecam : public Camera
{
public:
	bool is_sprinting = false;

	Freecam();

	Freecam(glm::vec3 eye, glm::vec3 center, glm::vec3 up);

	Freecam(Freecam& freecam);

	~Freecam();

	void MoveForward();
	void MoveBackward();
	void MoveLeft();
	void MoveRight();
	void MoveUp();
	void MoveDown();

	void MouseMove();



	float GetSpeed();

private:

	float speed = 10.f;
	float sprint_speed_multiplier = 1.5f;

	float mouse_sensitivity = 5;

	bool skip_movement = false; // per non fare scatti quando il cursore viene spostato dal programma
};