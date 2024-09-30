#pragma once

#include <camera.hpp>
#include <intersection.h>
#include <input_manager.hpp>


// projection and view matrix
extern glm::mat4 projection_matrix;
extern glm::mat4 view_matrix;

// mouse position
//extern glm::vec2 mouse_position;
//extern glm::vec2 old_mouse_position;
//extern glm::vec2 mouse_scroll;

class TrackballCamera: public Camera
{
private:

	/* a bool variable that indicates if we are currently rotating the trackball*/
	bool is_trackball_dragged = false;

	/* a bool variable that indicates if the transformation has been changed since last time it was check*/
	bool changed;

	/* p0 and p1 points on the sphere */
	glm::vec3 p0, p1;

	/* matrix to transform the scene according to the trackball: rotation only*/
	glm::mat4 rotation_matrix;

	/* matrix to transform the scene according to the trackball: scaling only*/
	glm::mat4  scaling_matrix;

	/* matrix to transform the scene according to the trackball: translation only*/
	glm::mat4  translation_matrix;

	/* old trackball*/
	glm::mat4  old_tb_matrix;


	float scaling_factor = 1.0;

	/* trackball center */
	glm::vec3 center;

	/* trackball radius */
	float radius;

	/// <summary>Transform from viewport to the view reference frame.</summary>
	void Viewport_to_ray(glm::mat4 proj, double pX, double pY, glm::vec4& p0, glm::vec4& d);

	/// <summary>Handles the intersection between the position under the mouse and the sphere.</summary>
	bool Cursor_sphere_intersection(glm::mat4 proj, glm::mat4 view, glm::vec3& int_point, double xpos, double ypos);

public:
	TrackballCamera();
	TrackballCamera(glm::vec3 eye, glm::vec3 center, glm::vec3 up);
	TrackballCamera(TrackballCamera& tb);
	~TrackballCamera();

	void Reset();
	void Set_center_radius(glm::vec3 c, float r);

	void MouseMove();
	void MousePress();
	void MouseRelease();
	void MouseScroll();


	bool IsMoving();
	bool IsChanged();

	glm::mat4 GetMatrix();
	float GetScalingFactor();
};

