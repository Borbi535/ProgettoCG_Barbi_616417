#include "camera.hpp"


Camera::Camera()
{
	_eye = glm::vec3(1);
	view_direction = glm::vec3(0);
	up_direction = glm::vec3(0, 1, 0);

}

Camera::Camera(glm::vec3 eye, glm::vec3 center, glm::vec3 up): _eye(eye), view_direction(center), up_direction(up) { }

Camera::Camera(Camera& cam)
{
	_eye = cam._eye;
	view_direction = cam.view_direction;
	up_direction = cam.up_direction;
}

Camera::~Camera() { }

glm::mat4 Camera::GetViewMatrix()
{
	return glm::lookAt(_eye, view_direction == glm::vec3(0) ? view_direction : _eye + view_direction, up_direction);
}

glm::vec3 Camera::GetPosition() { return _eye; }

void Camera::SetParameters(glm::vec3 eye, glm::vec3 view_dir, glm::vec3 up_dir)
{
	_eye = eye; view_direction = view_dir; up_direction = up_dir;
}



