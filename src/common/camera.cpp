#include "camera.hpp"


Camera::Camera()
{
	_eye = glm::vec3(1);
	_center = glm::vec3(0,0,0);
	up_direction = glm::vec3(0, 1, 0);

	original_eye = _eye;
	original_center = _center;
	original_up_direction = up_direction;
}

Camera::Camera(glm::vec3 eye, glm::vec3 center, glm::vec3 up): _eye(eye), up_direction(up)
{
	_center = center;

	original_eye = eye;
	original_center = center;
	original_up_direction = up;
}

Camera::Camera(Camera& cam)
{
	_eye = cam._eye;
	_center = cam._center;
	up_direction = cam.up_direction;
	
	original_eye = cam.original_eye;
	original_center = cam.original_center;
	original_up_direction = cam.original_up_direction;
}

Camera::~Camera() { }

glm::mat4 Camera::GetViewMatrix() const
{
	// return glm::lookAt(_eye, view_direction == glm::vec3(0) ? view_direction : _eye + view_direction, up_direction); // ???
	return glm::lookAt(_eye, _center, up_direction);
}

glm::vec3 Camera::GetPosition() const { return _eye; }

void Camera::SetParameters(glm::vec3 eye, glm::vec3 center, glm::vec3 up_dir)
{
	_eye = eye; _center = center; up_direction = up_dir;
}

void Camera::ResetParameters()
{
	SetParameters(original_eye, original_center, original_up_direction);
}



