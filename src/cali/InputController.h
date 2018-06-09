#pragma once

#ifdef WIN32
#include <Windows.h>
#endif

#include <Mouse.h>
#include <Keyboard.h>

#include <functional>

namespace cali
{
	class input_controller
	{
	public:
		typedef void (t_simple_input)(float dt);
		typedef void (t_MouseInput)(float value, float dt);

	private:
		DirectX::Mouse m_mouse;
		DirectX::Keyboard m_keyboard;

		std::function<t_simple_input> m_forward;
		std::function<t_simple_input> m_backward;
		std::function<t_simple_input> m_strafe_left;
		std::function<t_simple_input> m_strafe_right;
		std::function<t_simple_input> m_wireframe;
		std::function<t_simple_input> m_shift;
		std::function<t_simple_input> m_reset;
		std::function<t_simple_input> m_stop;
		std::function<t_simple_input> m_debug_info;

		std::function<t_MouseInput> m_mouse_x;
		std::function<t_MouseInput> m_mouse_y;

		input_controller(const input_controller&) = delete;
		input_controller& operator=(const input_controller&) = delete;

	public:

		input_controller();

		~input_controller();

		void read_input(float dt);

		void clear();

		void forward(std::function<t_simple_input> callback) { m_forward = callback; }
		void backward(std::function<t_simple_input> callback) { m_backward = callback; }
		void strafe_left(std::function<t_simple_input> callback) { m_strafe_left = callback; }
		void strafe_right(std::function<t_simple_input> callback) { m_strafe_right = callback; }
		void wireframe(std::function<t_simple_input> callback) { m_wireframe = callback; }
		void shift(std::function<t_simple_input> callback) { m_shift = callback; }
		void reset(std::function<t_simple_input> callback) { m_reset = callback; }
		void stop(std::function<t_simple_input> callback) { m_stop = callback; }
		void debug_info(std::function<t_simple_input> callback) { m_debug_info = callback; }
		void mouse_x(std::function<t_MouseInput> callback) { m_mouse_x = callback; }
		void mouse_y(std::function<t_MouseInput> callback) { m_mouse_y = callback; }
	};
}
