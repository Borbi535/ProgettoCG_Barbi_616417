#include <freecam.hpp>

Freecam::Freecam() {}

Freecam::Freecam(glm::vec3 eye, glm::vec3 center, glm::vec3 up): Camera(eye, center, up) 
{
	forward_direction = glm::normalize(_center - _eye);
}

Freecam::Freecam(Freecam& freecam): Camera(freecam)
{
	speed = freecam.speed;
	sprint_speed_multiplier = freecam.sprint_speed_multiplier;
	mouse_sensitivity = freecam.mouse_sensitivity;
	forward_direction = freecam.forward_direction;
}

Freecam::~Freecam() {}

void Freecam::MoveForward()
{
	_eye += forward_direction * GetSpeed();
}
void Freecam::MoveBackward()
{
	_eye -= forward_direction * GetSpeed();
}
void Freecam::MoveLeft()
{
	_eye -= glm::cross(forward_direction, up_direction) * GetSpeed();
}
void Freecam::MoveRight()
{
	_eye += glm::cross(forward_direction, up_direction) * GetSpeed();
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

	forward_direction = glm::rotate(
		glm::rotate(forward_direction, delta.x, glm::vec3(0, 1, 0)),
		delta.y,
		glm::normalize(glm::cross(forward_direction, up_direction))); // right direction

	input_manager::old_mouse_position = input_manager::mouse_position;
}

glm::mat4 Freecam::GetViewMatrix() const
{
	return glm::lookAt(_eye, _eye + forward_direction, up_direction);
}

float Freecam::GetSpeed() const
{
	return speed / 1000 * (is_sprinting ? sprint_speed_multiplier : 1);
}

void Freecam::SetSpeed(float speed) { this->speed = speed; }

void Freecam::SetSprintSpeedMultiplier(float multiplier) { sprint_speed_multiplier = multiplier; }
