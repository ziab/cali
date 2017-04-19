#include "Camera.h"

#include <IvRendererHelp.h>
#include <IvMath.h>
#include "DebugInfo.h"
#include "Constants.h"
#include "Atomsphere.h"
#include "ConstantBuffer.h"

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(x) (void)(x)
#endif // !UNREFERENCED_PARAMETER

namespace Cali
{
	const float Camera::movement_inertia_decay = 1.2f;
	const float Camera::movement_acceleration = 240.f; // m/s ?
	const float Camera::movement_max_velocity = 60.f; // m/s
	const float Camera::roatation_sensitivity = 0.15f;
	const float Camera::rotation_inertia_decay = 0.1f;

	void Camera::next_position(float dt)
	{
		// speed decay
		if (!m_is_moving)
		{
			m_velocity /= movement_inertia_decay;
		}

		// speed limiter
		float velocity = m_velocity.Length();
		if (velocity > movement_max_velocity)
		{
			m_velocity *= movement_max_velocity / velocity;
		}

		Physical::set_position(Physical::get_position() + m_velocity * dt * m_addtional_acceleration);
		m_is_moving = false;

		m_addtional_acceleration = 1.0f;
	}

	void Camera::next_angle(float dt)
	{
		if (!m_is_rotating)
		{
			m_yaw_inertia *= (dt * rotation_inertia_decay);
			m_pitch_inertia *= (dt * rotation_inertia_decay);
		}

		m_is_rotating = false;

		Physical::yaw(m_yaw_inertia * roatation_sensitivity  * dt);
		Physical::pitch(m_pitch_inertia * roatation_sensitivity * dt);
	}

	void Camera::normalize()
	{
	}

	Camera::Camera(const IvVector3& postition, const IvVector3& direction)
	{
		Physical::set_position(postition);
		Physical::set_direction(direction, Cali::Constants::c_world_up);

		m_fov = 60.0f;
		m_near = 0.1f;
		m_far = Cali::Atmosphere::c_camera_far;

		m_velocity = { 0.f, 0.f, 0.f };
		m_yaw_inertia = 0.f;
		m_pitch_inertia = 0.f;
		m_is_moving = false;
		m_is_rotating = false;

		m_addtional_acceleration = 1.0;
	}

	void Camera::set_fov(float new_fov)
	{
		m_fov = new_fov;
	}

	void Camera::send_settings_to_renderer(IvRenderer& renderer)
	{
		// set default projection matrix
		float d = 1.0f / IvTan(m_fov / 180.0f * kPI * 0.5f);
		float Q = m_far / (m_far - m_near);
		IvMatrix44 perspective;

		perspective(0, 0) = d / ((float)renderer.GetWidth() / (float)renderer.GetHeight());
		perspective(1, 1) = d;
		perspective(2, 2) = Q;
		perspective(2, 3) = -m_near * Q;
		perspective(3, 2) = 1.0f;
		perspective(3, 3) = 0.0f;

		renderer.SetFOV(m_fov);
		renderer.SetNearPlane(m_near);
		renderer.SetFarPlane(m_far);
		renderer.SetProjectionMatrix(perspective);
	}

	void Camera::update(float dt)
	{
		////////////////////////////////////////////////////////////////////////
		// print debug info
		auto& debug_info = DebugInfo::get_debug_info();
		debug_info.set_debug_string(L"camera_velocity", m_velocity.Length() * m_addtional_acceleration);
		debug_info.set_debug_string(L"camera_x", get_position().x);
		debug_info.set_debug_string(L"camera_y", get_position().y);
		debug_info.set_debug_string(L"camera_z", get_position().z);
		////////////////////////////////////////////////////////////////////////

		next_angle(dt);
		next_position(dt);
	}

	IvMatrix44 Camera::get_view_matrix()
	{
		// build 4x4 matrix
		IvMatrix44 matrix;

		const auto& right = Physical::get_right();

		IvVector3 view_up = Physical::get_direction().Cross(right);
		view_up.Normalize();

		// now set up matrices
		// world->view rotation
		IvMatrix33 rotate;
		rotate.SetRows(right, view_up, Physical::get_direction());

		// world->view translation
		IvVector3 xlate = -(rotate * Physical::get_position());

		matrix.Rotation(rotate);
		matrix(0, 3) = xlate.x;
		matrix(1, 3) = xlate.y;
		matrix(2, 3) = xlate.z;

		return matrix;
	}

	void Camera::render(IvRenderer & renderer)
	{
		normalize();
		IvRenderer::mRenderer->SetViewMatrix(get_view_matrix());
	}

	void Camera::update_global_state(ConstantBufferWrapper<ConstantBuffer::GlobalState>& global_state)
	{
		global_state->camera_position = get_position();
	}

	void Camera::enable_speed_mode(float dt)
	{
		m_addtional_acceleration = 100.0f;
	}

	void Camera::move_forward(float dt)
	{
		normalize();
		add_velocity(get_direction() * (movement_acceleration * dt));
	}

	void Camera::move_backward(float dt)
	{
		normalize();
		add_velocity(-get_direction() * (movement_acceleration * dt));
	}

	void Camera::move_left(float dt)
	{
		normalize();
		add_velocity(-get_right() * (movement_acceleration * dt));
	}

	void Camera::move_right(float dt)
	{
		normalize();
		add_velocity(get_right() * (movement_acceleration * dt));
	}

	void Camera::pitch(float angle, float dt)
	{
		Physical::pitch(angle * roatation_sensitivity * dt);
		m_pitch_inertia = angle;
		m_is_rotating = true;
	}

	void Camera::yaw(float angle, float dt)
	{
		Physical::yaw(angle * roatation_sensitivity * dt);
		m_yaw_inertia = angle;
		m_is_rotating = true;
	}

	inline void Camera::add_velocity(const IvVector3 & velocity)
	{
		m_velocity += velocity;
		m_is_moving = true;
	}
}