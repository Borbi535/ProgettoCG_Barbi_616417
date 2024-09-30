#pragma once

#include <iostream>
#include <map>
#include <GLFW/glfw3.h>
#include <ext/vector_float2.hpp>
#include <functional>
#include <xerrori.hpp>
#include <string>


extern GLFWwindow* main_window;

namespace input_manager
{
	struct input_key
	{
		int key_state;
		bool callback_called;
		bool hold_enabled;
		std::function<void()> callbacks_press; // mappa <game_state, callback>
		std::function<void()> callbacks_release;
		std::list<bool*> bound_bools; // mappa <game_state, puntatori a booleani da aggiornare con lo stato>

		input_key();
	};

	extern std::map<int, std::map<int, input_key>> assigned_keys_per_state; // mappa game_state -> (mappa key -> input_key)

	extern glm::vec2 mouse_position;
	extern glm::vec2 old_mouse_position;
	extern glm::vec2 mouse_scroll;
	extern bool mouse_scroll_changed;
	extern bool cursor_hidden;
	extern std::map<int, std::function<void()>> mouse_move_callbacks;
	extern std::map<int, std::function<void()>> mouse_scroll_callbacks;

	int GetKeyState(int key);

	/// <summary>Binds a key to a callback for a specific game state.</summary>
	/// <returns>True on success, False otherwise.</returns>
	bool BindKeyCallback(int key, int action, int game_state, std::function<void()> const& callback_function, bool hold_enabled = false, bool terminate_on_failure = false);
	bool BindMouseScrollCallback(int game_state, std::function<void()> callback_function, bool terminate_on_failure = false);
	bool BindMouseMoveCallback(int game_state, std::function<void()> callback_function, bool terminate_on_failure = false);

	/// <summary>Bind a boolean to the state of a key (True: pressed, False: not pressed).</summary>
	/// <returns>True on success, False otherwise.</returns>
	bool BindBoolToKey(int key, int game_state, bool* state_to_bind, bool terminate_on_failure = false);

	/// <summary>Update assigned keys' state and the bounded booleans.</summary>
	void UpdateKeyState(int key, int state);
	void UpdateMousePosition(double xpos, double ypos);
	void UpdateMouseScroll(double xoffset, double yoffset);

	/// <summary>Calls the callback for every pressed key in this frame for the current game state</summary>
	void ProcessInput();

	void ToggleMousePointer();
	void ShowMousePointer();
	void HideMousePointer();
}