#include <freecam.hpp>


Freecam::Freecam() {}

Freecam::Freecam(glm::vec3 eye, glm::vec3 center, glm::vec3 up): Camera(eye, center, up) { }

Freecam::Freecam(Freecam& freecam): Camera(freecam)
{
	speed = freecam.speed;
	sprint_speed_multiplier = freecam.sprint_speed_multiplier;
	mouse_sensitivity = freecam.mouse_sensitivity;
}

Freecam::~Freecam() {}

void Freecam::MoveForward()
{
	_eye += _center * GetSpeed();
}
void Freecam::MoveBackward()
{
	_eye -= _center * GetSpeed();
}
void Freecam::MoveLeft()
{
	glm::vec3 right_vector = glm::cross(_center, up_direction);
	_eye -= right_vector * GetSpeed();
}
void Freecam::MoveRight()
{
	glm::vec3 right_vector = glm::cross(_center, up_direction);
	_eye += right_vector * GetSpeed();
}
void Freecam::MoveUp()
{
	_eye += up_direction * GetSpeed();
}
void Freecam::MoveDown()
{
	_eye -= up_direction * GetSpeed();
}

void Freecam::MouseMove()
{
	if (skip_movement)
	{
		input_manager::old_mouse_position = input_manager::mouse_position;
		skip_movement = false;
		return;
	}

	glm::vec2 delta = input_manager::old_mouse_position - input_manager::mouse_position;
	delta *= mouse_sensitivity / 10000;
	_center = glm::rotate(glm::rotate(_center, delta.x, glm::vec3(0, 1, 0)), delta.y, glm::cross(_center, up_direction));
	input_manager::old_mouse_position = input_manager::mouse_position;
}

float Freecam::GetSpeed()
{
	return speed / 1000 * (is_sprinting ? sprint_speed_multiplier : 1);
}