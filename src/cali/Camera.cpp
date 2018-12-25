#include "Camera.h"

#include <IvRendererHelp.h>
#include <IvMath.h>
#include "DebugInfo.h"
#include "Constants.h"
#include "World.h"
#include "ConstantBuffer.h"

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(x) (void)(x)
#endif // !UNREFERENCED_PARAMETER

namespace cali
{
	const float camera::movement_inertia_decay = 1.2f;
	const float camera::movement_acceleration = 100.f; // m/s ?
	const float camera::movement_max_velocity = 30.f; // m/s
	const float camera::roatation_sensitivity = 0.15f;
	const float camera::rotation_inertia_decay = 0.2f;
	const float camera::addtional_acceleration = 1000.0f;

	void camera::next_position(float dt)
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

		physical::set_position(physical::get_position() + m_velocity * dt * m_addtional_acceleration);
		m_is_moving = false;

		m_addtional_acceleration = 1.0f;
	}

	void camera::next_angle(float dt)
	{
		if (!m_is_rotating)
		{
			m_yaw_inertia *= (dt * rotation_inertia_decay);
			m_pitch_inertia *= (dt * rotation_inertia_decay);
		}

		m_is_rotating = false;

		physical::yaw(m_yaw_inertia * roatation_sensitivity  * dt);
		physical::pitch(m_pitch_inertia * roatation_sensitivity * dt);
	}

	void camera::normalize()
	{
	}

	camera::camera(const IvVector3& postition, const IvVector3& direction)
	{
		physical::set_position(postition);
		physical::set_direction(direction, cali::constants::c_world_up);

		m_fov = 60.0f;
		m_near = 0.1f;
		m_far = cali::world::c_camera_far;

		m_velocity = { 0.f, 0.f, 0.f };
		m_yaw_inertia = 0.f;
		m_pitch_inertia = 0.f;
		m_is_moving = false;
		m_is_rotating = false;

		m_addtional_acceleration = 1.0;
	}

	void camera::set_fov(float new_fov)
	{
		m_fov = new_fov;
	}

	void camera::send_settings_to_renderer(IvRenderer& renderer)
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

	void camera::update(float dt)
	{
		////////////////////////////////////////////////////////////////////////
		// print debug info
		auto& debug_info = debug_info::get_debug_info();
		debug_info.set_debug_string(L"camera_velocity", m_velocity.Length() * m_addtional_acceleration);
		debug_info.set_debug_string(L"camera_x", get_position().x);
		debug_info.set_debug_string(L"camera_y", get_position().y);
		debug_info.set_debug_string(L"camera_z", get_position().z);
		////////////////////////////////////////////////////////////////////////

		next_angle(dt);
		next_position(dt);
	}

	IvMatrix44 camera::get_view_matrix()
	{
		// build 4x4 matrix
		IvMatrix44 matrix;

		const auto& right = physical::get_right();

		IvVector3 view_up = physical::get_direction().Cross(right);
		view_up.Normalize();

		// now set up matrices
		// world->view rotation
		IvMatrix33 rotate;
		rotate.SetRows(right, view_up, physical::get_direction());

		// world->view translation
		IvVector3 xlate = -(rotate * physical::get_position());

		matrix.Rotation(rotate);
		matrix(0, 3) = xlate.x;
		matrix(1, 3) = xlate.y;
		matrix(2, 3) = xlate.z;

		return matrix;
	}

	void camera::render(IvRenderer & renderer)
	{
		normalize();
		auto view_matrix = get_view_matrix();
		m_frustum.construct_frustum(renderer.GetProjectionMatrix(), view_matrix);
		IvRenderer::mRenderer->SetViewMatrixAndViewPosition(view_matrix, get_position());
	}

	void camera::update_global_state(constant_buffer_wrapper<constant_buffer::GlobalState>& global_state)
	{
		global_state->camera_position = get_position();
	}

	void camera::enable_speed_mode(float dt)
	{
		m_addtional_acceleration = addtional_acceleration;
	}

	const frustum & camera::get_frustum()
	{
		return m_frustum;
	}

	void camera::move_forward(float dt)
	{
		normalize();
		add_velocity(get_direction() * (movement_acceleration * dt));
	}

	void camera::move_backward(float dt)
	{
		normalize();
		add_velocity(-get_direction() * (movement_acceleration * dt));
	}

	void camera::move_left(float dt)
	{
		normalize();
		add_velocity(-get_right() * (movement_acceleration * dt));
	}

	void camera::move_right(float dt)
	{
		normalize();
		add_velocity(get_right() * (movement_acceleration * dt));
	}

	void camera::pitch(float angle, float dt)
	{
		physical::pitch(angle * roatation_sensitivity * dt);
		m_pitch_inertia = angle;
		m_is_rotating = true;
	}

	void camera::yaw(float angle, float dt)
	{
		physical::yaw(angle * roatation_sensitivity * dt);
		m_yaw_inertia = angle;
		m_is_rotating = true;
	}

	inline void camera::add_velocity(const IvVector3 & velocity)
	{
		m_velocity += velocity;
		m_is_moving = true;
	}
}