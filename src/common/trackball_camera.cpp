#include <trackball_camera.hpp>

// PRIVATE

void TrackballCamera::Viewport_to_ray(glm::mat4 proj, double pX, double pY, glm::vec4& p0, glm::vec4& d) {
	GLint vp[4];
	glm::mat4 proj_inv = glm::inverse(proj);
	glGetIntegerv(GL_VIEWPORT, vp);
	glm::vec4 p1;
	p1.x = p0.x = -1.f + ((float)pX / vp[2]) * (1.f - (-1.f));
	p1.y = p0.y = -1.f + ((vp[3] - (float)pY) / vp[3]) * (1.f - (-1.f));
	p0.z = -1;
	p1.z = 1;
	p1.w = p0.w = 1.0;
	p0 = proj_inv * p0; p0 /= p0.w;
	p1 = proj_inv * p1; p1 /= p1.w;
	d = glm::normalize(p1 - p0);
}

bool TrackballCamera::Cursor_sphere_intersection(glm::mat4 proj, glm::mat4 view, glm::vec3& int_point, double xpos, double ypos) {
	glm::mat4 view_frame = glm::inverse(view);

	glm::vec4 o, d;
	Viewport_to_ray(proj, xpos, ypos, o, d);

	o = view_frame * o;
	d = view_frame * d;

	bool hit = intersection_ray::sphere(int_point, o, d, center, radius);
	if (hit)
		int_point -= center;

	/* this was left to "return true" in class.. It was a gigantic bug with almost never any consequence, except while
	click near the silohuette of the sphere.*/
	return hit;
}


// PUBLIC

TrackballCamera::TrackballCamera() {}

TrackballCamera::TrackballCamera(glm::vec3 eye, glm::vec3 center, glm::vec3 up): Camera(eye, center, up) { Reset(); old_tb_matrix = glm::mat4(1.f); }

TrackballCamera::TrackballCamera(TrackballCamera& tb): Camera(tb)
{
	scaling_factor = tb.scaling_factor;
	scaling_matrix = tb.scaling_matrix;
	rotation_matrix = tb.rotation_matrix;
	translation_matrix = tb.translation_matrix;
	old_tb_matrix = tb.old_tb_matrix;
	center = tb.center;
	radius = tb.radius;

}

TrackballCamera::~TrackballCamera() {}

void TrackballCamera::Reset() {
	scaling_factor = 1.f;
	scaling_matrix = glm::mat4(1.f);
	rotation_matrix = glm::mat4(1.f);
	translation_matrix = glm::mat4(1.f);
}

void TrackballCamera::Set_center_radius(glm::vec3 c, float r) {
	old_tb_matrix = this->GetMatrix();
	Reset();
	center = c;
	radius = r;
	translation_matrix = glm::translate(glm::mat4(1.f), center);
}

void TrackballCamera::MouseMove()
{
	if (!is_trackball_dragged)
		return;

	if (Cursor_sphere_intersection(projection_matrix, view_matrix, p1, input_manager::mouse_position.x, input_manager::mouse_position.y))
	{
		glm::vec3 rotation_vector = glm::cross(glm::normalize(p0), glm::normalize(p1));

		/* avoid near null rotation axis*/
		if (glm::length(rotation_vector) > 0.01)
		{
			float alpha = glm::asin(glm::length(rotation_vector));
			glm::mat4 delta_rot = glm::rotate(glm::mat4(1.f), alpha, rotation_vector);
			rotation_matrix = delta_rot * rotation_matrix;

			/*p1 becomes the p0 value for the next movement */
			p0 = p1;
		}
	}
}

void TrackballCamera::MousePress()
{
	glm::vec3 int_point;
	if (Cursor_sphere_intersection(projection_matrix, view_matrix, int_point, input_manager::mouse_position.x, input_manager::mouse_position.y))
	{
		p0 = int_point;
		is_trackball_dragged = true;
	}
}

void TrackballCamera::MouseRelease()
{
	is_trackball_dragged = false;
}

void TrackballCamera::MouseScroll()
{
	scaling_factor *= (float)((input_manager::mouse_scroll.y > 0) ? 1.1 : 0.97);
	scaling_matrix = glm::scale(glm::mat4(1.f), glm::vec3(scaling_factor));
}

glm::mat4 TrackballCamera::GetMatrix() { return translation_matrix * scaling_matrix * rotation_matrix * glm::inverse(translation_matrix) * old_tb_matrix; }

bool TrackballCamera::IsMoving() { return is_trackball_dragged; }

//bool TrackballCamera::IsChanged() {
//	if (changed || is_trackball_dragged) {
//		changed = false;
//		return true;
//	}
//	return false;
//}

float TrackballCamera::GetScalingFactor() { return scaling_factor; }