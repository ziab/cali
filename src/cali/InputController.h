#pragma once

#ifdef WIN32
#include <Windows.h>
#endif

#include <Mouse.h>
#include <Keyboard.h>

#include <functional>

namespace Cali
{
	class InputController
	{
	public:
		typedef void (t_SimpleInput)(float dt);
		typedef void (t_MouseInput)(float value, float dt);

	private:
		DirectX::Mouse m_mouse;
		DirectX::Keyboard m_keyboard;

		std::function<t_SimpleInput> m_forward;
		std::function<t_SimpleInput> m_backward;
		std::function<t_SimpleInput> m_strafe_left;
		std::function<t_SimpleInput> m_strafe_right;
		std::function<t_SimpleInput> m_wireframe;
		std::function<t_SimpleInput> m_shift;
		std::function<t_SimpleInput> m_debug_info;

		std::function<t_MouseInput> m_mouse_x;
		std::function<t_MouseInput> m_mouse_y;

		InputController(const InputController&) = delete;
		InputController& operator=(const InputController&) = delete;

	public:

		InputController();

		~InputController();

		void read_input(float dt);

		void clear();

		void forward(std::function<t_SimpleInput> callback) { m_forward = callback; }
		void backward(std::function<t_SimpleInput> callback) { m_backward = callback; }
		void strafe_left(std::function<t_SimpleInput> callback) { m_strafe_left = callback; }
		void strafe_right(std::function<t_SimpleInput> callback) { m_strafe_right = callback; }
		void wireframe(std::function<t_SimpleInput> callback) { m_wireframe = callback; }
		void shift(std::function<t_SimpleInput> callback) { m_shift = callback; }
		void debug_info(std::function<t_SimpleInput> callback) { m_debug_info = callback; }
		void mouse_x(std::function<t_MouseInput> callback) { m_mouse_x = callback; }
		void mouse_y(std::function<t_MouseInput> callback) { m_mouse_y = callback; }
	};
}
