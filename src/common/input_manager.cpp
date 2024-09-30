#include <input_manager.hpp>


input_manager::input_key::input_key() : key_state(GLFW_RELEASE), callback_called(false), bound_bools({}) {}


std::map<int, std::map<int, input_manager::input_key>> input_manager::assigned_keys_per_state = {};

glm::vec2 input_manager::mouse_position = glm::vec2(960, 540);
glm::vec2 input_manager::old_mouse_position = glm::vec2(960, 540);
glm::vec2 input_manager::mouse_scroll = glm::vec2(0);
bool input_manager::mouse_scroll_changed = false;
bool input_manager::cursor_hidden = false;
std::map<int, std::function<void()>> input_manager::mouse_move_callbacks = {};
std::map<int, std::function<void()>> input_manager::mouse_scroll_callbacks = {};

extern int game_state;

int input_manager::GetKeyState(int key)
{
	if (assigned_keys_per_state[game_state].count(key)) return assigned_keys_per_state[game_state][key].key_state;
	
	std::cout << "the key " << key << " is not assigned." << std::endl;
	return -1;
}


static bool AssignCallbackToMap(int key, std::function<void()>& callback, std::function<void()> const& new_callback, bool terminate_on_failure)
{
	if (callback)
	{
		std::string msg = "a callback function for the key " + std::to_string(key) + " in the game state " + std::to_string(game_state) + " is already assigned.";
		if (terminate_on_failure) xterminate(msg.c_str(), QUI);
		else std::cout << msg << std::endl;

		return false;
	}

	callback = new_callback;
	return true;
}

bool input_manager::BindKeyCallback(int key, int action, int game_state, std::function<void()> const& callback_function, bool hold_enabled, bool terminate_on_failure)
{
	switch (action)
	{
		case GLFW_PRESS:
		case GLFW_REPEAT:
			assigned_keys_per_state[game_state][key].hold_enabled = hold_enabled;
			return AssignCallbackToMap(key, assigned_keys_per_state[game_state][key].callbacks_press, callback_function, terminate_on_failure);

		case GLFW_RELEASE:
			return AssignCallbackToMap(key, assigned_keys_per_state[game_state][key].callbacks_release, callback_function, terminate_on_failure);

		default: xassert(false, "should not execute this", QUI); return false;
	}
}

bool input_manager::BindMouseScrollCallback(int game_state, std::function<void()> callback_function, bool terminate_on_failure)
{
	if (mouse_scroll_callbacks.count(game_state) > 0)
	{
		std::string msg = "a callback function for the mouse scroll in the game state " + std::to_string(game_state) + " is already assigned.";
		if (terminate_on_failure) xterminate(msg.c_str(), QUI);
		else std::cout << msg << std::endl;
		
		return false;
	}

	mouse_scroll_callbacks[game_state] = callback_function;
	return true;
}

bool input_manager::BindMouseMoveCallback(int game_state, std::function<void()> callback_function, bool terminate_on_failure)
{
	if (mouse_move_callbacks.count(game_state) > 0)
	{
		std::string msg = "a callback function for the mouse move in the game state " + std::to_string(game_state) + " is already assigned.";
		if (terminate_on_failure) xterminate(msg.c_str(), QUI);
		else std::cout << msg << std::endl;

		return false;
	}

	mouse_move_callbacks[game_state] = callback_function;
	return true;
}

bool input_manager::BindBoolToKey(int key, int game_state, bool* state_to_bind, bool terminate_on_failure)
{
	for (bool* b : assigned_keys_per_state[game_state][key].bound_bools)
	{
		if (b == state_to_bind)
		{
			std::string msg = "this boolean is already assigned to the state of the key " + std::to_string(key) + " in the game state " + std::to_string(game_state) + ".";
			if (terminate_on_failure) xterminate(msg.c_str(), QUI);
			else std::cout << msg << std::endl;
			return false;
		}
	}

	assigned_keys_per_state[game_state][key].bound_bools.push_back(state_to_bind);
	return true;
}

void input_manager::UpdateKeyState(int key, int state)
{
	if (assigned_keys_per_state[game_state].count(key))
	{
		assigned_keys_per_state[game_state][key].key_state = state;

		for (bool* b : assigned_keys_per_state[game_state][key].bound_bools) *b = state;
	}

}

void input_manager::UpdateMousePosition(double xpos, double ypos)
{
	old_mouse_position = mouse_position;
	mouse_position = glm::vec2(xpos, ypos);

	mouse_move_callbacks[game_state]();
}

void input_manager::UpdateMouseScroll(double xoffset, double yoffset)
{
	mouse_scroll_changed = true;
	mouse_scroll = glm::vec2(xoffset, yoffset);
}

void input_manager::ProcessInput()
{
	if (mouse_scroll_changed && mouse_scroll_callbacks.count(game_state) > 0)
	{
		mouse_scroll_callbacks[game_state]();
		mouse_scroll_changed = false;
	}

	for (auto& [key, assigned_key] : assigned_keys_per_state[game_state])
	{
		switch (assigned_key.key_state)
		{
			case GLFW_PRESS:
			case GLFW_REPEAT:
				if (assigned_key.callbacks_press && (assigned_key.hold_enabled || !assigned_key.callback_called))
				{
					assigned_key.callbacks_press();
					assigned_key.callback_called = true;
				}
				break;

			case GLFW_RELEASE:
				if (assigned_key.callbacks_release) assigned_key.callbacks_release(); 
				if (assigned_key.callback_called) assigned_key.callback_called = false;
				break;

			default:
				xassert(false, "this should never be executed", QUI);
		}
	}
}


void input_manager::ToggleMousePointer()
{
	if (cursor_hidden) ShowMousePointer();
	else HideMousePointer();
}

void input_manager::ShowMousePointer()
{
	glfwSetCursorPos(main_window, 960, 540);

	mouse_position = old_mouse_position = glm::vec2(960, 540);

	glfwSetInputMode(main_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	cursor_hidden = false;
}

void input_manager::HideMousePointer()
{
	glfwSetCursorPos(main_window, 960, 540);

	mouse_position = old_mouse_position = glm::vec2(960, 540);

	glfwSetInputMode(main_window, GLFW_CURSOR, cursor_hidden ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

	cursor_hidden = true;
}
