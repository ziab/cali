#include "InputController.h"
#include <IvAdditionalInputHandler.h>

HWND g_hwnd = NULL;

static void CALLBACK additional_input_handler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	g_hwnd = hWnd;

	switch (message)
	{
	case WM_ACTIVATEAPP:
		DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
		DirectX::Mouse::ProcessMessage(message, wParam, lParam);
		break;
	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
		DirectX::Mouse::ProcessMessage(message, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
		break;
	}
}

namespace cali
{
	input_controller::input_controller()
	{
		// register the global addtitional wnd proc
		SetAdditionalInputHandler(additional_input_handler);
	}

	input_controller::~input_controller()
	{
	}

	template <typename CallbackType, typename... Args>
	void invoke_callback(const std::function<CallbackType>& callback, Args... args)
	{
		if (callback) callback(args...);
	}

	void input_controller::read_input(float dt)
	{
		using namespace DirectX;

		if (m_keyboard.IsConnected())
		{
			auto& state = m_keyboard.GetState();

			if (state.W) invoke_callback(m_forward, dt);
			if (state.S) invoke_callback(m_backward, dt);
			if (state.A) invoke_callback(m_strafe_left, dt);
			if (state.D) invoke_callback(m_strafe_right, dt);
			if (state.P) invoke_callback(m_wireframe, dt);
			if (state.LeftShift) invoke_callback(m_shift, dt);
			if (state.F10) invoke_callback(m_debug_info, dt);
			if (state.F8) invoke_callback(m_reset, dt);
			if (state.F7) invoke_callback(m_stop, dt);

			if (state.R) invoke_callback(m_mouse_y, -5.0f, dt);
			if (state.F) invoke_callback(m_mouse_y, 5.0f, dt);
			if (state.Q) invoke_callback(m_mouse_x, -5.0f, dt);
			if (state.E) invoke_callback(m_mouse_x, 5.0f, dt);
		}

		if (m_mouse.IsConnected())
		{
			auto& state = m_mouse.GetState();

			if (state.leftButton)
			{
				m_mouse.SetWindow(g_hwnd);

				if (m_mouse.GetState().positionMode == Mouse::MODE_ABSOLUTE)
				{
					m_mouse.SetMode(Mouse::MODE_RELATIVE);
				}
				else
				{
					m_mouse.SetMode(Mouse::MODE_ABSOLUTE);
				}
			}

			if (state.positionMode == Mouse::MODE_ABSOLUTE)
			{
				// do nothing
			}
			else if (state.positionMode == Mouse::MODE_RELATIVE)
			{
				if (state.x) invoke_callback(m_mouse_x, (float)state.x, dt);
				if (state.y) invoke_callback(m_mouse_y, (float)state.y, dt);
			}
		}
	}

	void input_controller::clear()
	{
		m_forward = nullptr;
		m_backward = nullptr;
		m_strafe_left = nullptr;
		m_strafe_right = nullptr;
		m_wireframe = nullptr;
		m_mouse_x = nullptr;
		m_mouse_y = nullptr;
	}
}
