#include<arcball_camera.hpp>


// PRIVATE

bool ArcballCamera::IsMoving()
{
	return false;
}

bool ArcballCamera::IsChanged()
{
	return false;
}



// PUBLIC

ArcballCamera::ArcballCamera() {}

ArcballCamera::ArcballCamera(glm::vec3 eye, glm::vec3 center, glm::vec3 up): Camera(eye, center, up) {}

ArcballCamera::ArcballCamera(ArcballCamera& ac) : Camera(ac) {}

ArcballCamera::~ArcballCamera() {}

void ArcballCamera::MouseMove()
{
	if (!(mouse_pressed || scroll_pressed)) return;

	p1 = input_manager::mouse_position;
	forward_direction = glm::normalize((_center - _eye) * glm::vec3(1, 0, 1));
	right_direction = glm::normalize(glm::cross(forward_direction, up_direction));

	if (mouse_pressed)
	{
		glm::vec2 pan_vector = (p0 - p1);
		float distance = glm::length(_center - _eye);

		glm::vec3 step_vector = (right_direction * pan_vector.x * pan_sensitivity * distance) + forward_direction * -pan_vector.y * pan_sensitivity * distance;
		
		_eye += step_vector;
		_center += step_vector;
	}
	else // scroll_pressed == true
	{
		glm::vec3 pivot = _center;
		pivot.y = 0.f;
		glm::vec2 pan_vector = (p1 - p0);
		if (pan_vector == glm::vec2(0)) return;

		float _yaw = -pan_vector.x * rotation_sensitivity;
		float _pitch = -pan_vector.y * rotation_sensitivity;

		glm::mat4 rotation_matrix = glm::rotate(
			glm::rotate(glm::mat4(1.f), glm::radians(_yaw), glm::vec3(0.0f, 1.0f, 0.0f)),
			glm::radians(_pitch),
			right_direction);

		glm::mat4 transformation_matrix = 
			glm::translate(glm::mat4(1.0f),pivot) *
			rotation_matrix *
			glm::translate(glm::mat4(1.0f), -pivot);

		_eye = glm::vec3(transformation_matrix * glm::vec4(_eye, 1.f));
	}


	p0 = p1;
}

void ArcballCamera::MouseLPress()
{
	mouse_pressed = true;

	p0 = input_manager::mouse_position;
}

void ArcballCamera::MouseLRelease()
{
	mouse_pressed = false;
}

void ArcballCamera::MouseScroll()
{
	float distance = glm::length(_center - _eye);
	float zoom_step = glm::max(distance * zoom_sensitivity, 0.05f);

	if (input_manager::mouse_scroll.y > 0)
	{
		if (distance >= 0.1f)
		{
			_eye += glm::normalize(_center - _eye) * zoom_step;
		}
	}
	else
	{
		_eye += glm::normalize(_eye - _center) * zoom_step;
	}
}

void ArcballCamera::MouseScrollPress()
{
	scroll_pressed = true;

	p0 = input_manager::mouse_position;
}

void ArcballCamera::MouseScrollRelease()
{
	scroll_pressed = false;
}

