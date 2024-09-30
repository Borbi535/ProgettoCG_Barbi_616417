#pragma once


class view_manipulator
{

	/* matrix to transform the scene according to the trackball: rotation only */
	glm::mat4 rotation_matrix;

	/* a bool variable that indicates if we are currently rotating the trackball */
	bool is_dragged;

	/* starting position of dragging */
	float start_xpos = 0.0f, start_ypos = 0.0f;

	/* euler angles */
	float d_alpha, d_beta;
	
	/* matrix to transform the scene according to the input: translation only */
	glm::mat4 translation_matrix;

	glm::vec3 translation_values;
	float translation_factor = 0.01f;


public:
	void reset() {
		rotation_matrix = glm::mat4(1.f);
		d_alpha = d_beta = 0.0;

		translation_matrix = glm::mat4(1.f);
		translation_values = glm::vec3(0.f);
	}

	void mouse_move(double xpos, double ypos)
	{
		d_alpha += (float)(xpos - start_xpos) / 1000.f;
		d_beta += (float)(ypos - start_ypos) / 800.f;
		start_xpos = (float)xpos;
		start_ypos = (float)ypos;
		rotation_matrix = glm::rotate(glm::rotate(glm::mat4(1.f), d_alpha, glm::vec3(0, 1, 0)), d_beta, glm::vec3(1, 0, 0));
	}

	void keyboard_press(int key)
	{
		switch (key)
		{
			case GLFW_KEY_W: translation_values.z += -translation_factor; break;
			case GLFW_KEY_S: translation_values.z += translation_factor; break;
			case GLFW_KEY_A: translation_values.x += translation_factor; break;
			case GLFW_KEY_D: translation_values.x += -translation_factor; break;
			case GLFW_KEY_LEFT_CONTROL: translation_values.y += -translation_factor; break;
			case GLFW_KEY_SPACE: translation_values.y += translation_factor; break;
		}
		translation_matrix = glm::translate(glm::mat4(1.f), translation_values);
		//std::cout << "tra matrix:" << std::endl << translation_matrix << std::endl;
	}

	glm::mat4 get_rot_matrix() { return rotation_matrix; }

	glm::mat4 get_tra_matrix() { return translation_matrix; }

	glm::mat4 apply_to_view(glm::mat4 view_transformation)
	{
		glm::mat4 view_frame = inverse(view_transformation);
		glm::mat4 curr_view = view_frame;
		curr_view[3] = glm::vec4(0, 0, 0, 1);
		curr_view = rotation_matrix * curr_view;
		curr_view[3] = view_frame[3];
		curr_view = translation_matrix * curr_view;
		curr_view = inverse(curr_view);
		reset(); // aggiunto perchè tengo le view separate per adesso, e questa la aggiorno direttamente invece di ricarolcare ogni volta
		return curr_view;
	}
};
