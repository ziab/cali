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
	const float camera::movement_inertia_decay = 1.6f;
	const float camera::movement_acceleration = 300.f; // m/s ?
	const float camera::movement_max_velocity = 300.f; // m/s
	const float camera::roatation_sensitivity = 0.15f;
	const float camera::rotation_inertia_decay = 0.2f;
	const float camera::addtional_acceleration = 25.0f;

	IvVector3 camera::get_gravity() const
	{
		IvVector3 g = get_position() - world::c_earth_center;
		g.Normalize();
		return g;
	}

	void camera::align_to_gravity()
	{
		IvVector3 new_gravity = get_gravity();
		IvVector3 old_gravity = m_last_gravity;
		if (old_gravity.Length() < 0.5f) { m_last_gravity = new_gravity; return; }
		// rotation from old_gravity to new_gravity
		IvVector3 axis = old_gravity.Cross(new_gravity);
		float axis_len = axis.Length();
		if (axis_len < 1e-6f) { m_last_gravity = new_gravity; return; }
		axis.Normalize();
		float dot = Dot(old_gravity, new_gravity);
		if (dot > 1.f) dot = 1.f;
		if (dot < -1.f) dot = -1.f;
		float angle = acosf(dot);
		if (fabsf(angle) < 1e-6f) { m_last_gravity = new_gravity; return; }
		IvMatrix33 rot;
		rot.Rotation(axis, angle);
		IvVector3 dir = get_direction() * rot;
		IvVector3 right = get_right() * rot;
		dir.Normalize();
		right.Normalize();
		// re-orthogonalize slightly: ensure right is orthogonal to new_gravity and dir
		// keep dir, recompute right = new_gravity.Cross(dir)
		IvVector3 new_right = new_gravity.Cross(dir);
		if (new_right.Length() < 1e-6f) {
			// looking straight up/down, keep previous right rotated
			new_right = right;
		}
		new_right.Normalize();
		// keep dir as is, but ensure orthonormal with new_right/new_gravity via set_direction which computes right as up.Cross(dir)
		// To preserve pitch, we keep dir; right will be recomputed from gravity and dir via set_direction.
		// Use set_direction with dir and new_gravity – this preserves dir's vertical component but recomputes right.
		// However set_direction uses right = up.Cross(dir), which for pitched dir will still be correct (right horizontal)
		physical::set_direction(dir, new_gravity);
		m_last_gravity = new_gravity;
	}

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

		align_to_gravity();
	}

	void camera::next_angle(float dt)
	{
		if (!m_is_rotating)
		{
			m_yaw_inertia *= (dt * rotation_inertia_decay);
			m_pitch_inertia *= (dt * rotation_inertia_decay);
		}

		m_is_rotating = false;

		// yaw around gravity, pitch around local right
		if (fabsf(m_yaw_inertia) > 1e-6f)
		{
			IvVector3 gravity = get_gravity();
			IvMatrix33 rot;
			rot.Rotation(gravity, -m_yaw_inertia * roatation_sensitivity * dt);
			IvVector3 dir = get_direction() * rot;
			IvVector3 right = get_right() * rot;
			physical::set_direction(dir, gravity);
			// set_direction recomputes right, but we want to keep rotated right; so directly set via set_direction's recomputed value is fine
		}
		if (fabsf(m_pitch_inertia) > 1e-6f)
		{
			IvVector3 right = get_right();
			IvMatrix33 rot;
			rot.Rotation(right, -m_pitch_inertia * roatation_sensitivity * dt);
			IvVector3 dir = get_direction() * rot;
			IvVector3 gravity = get_gravity();
			physical::set_direction(dir, gravity);
		}
	}

	void camera::normalize()
	{
		align_to_gravity();
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
		m_last_gravity = get_gravity();
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
		add_velocity(get_direction() * (movement_acceleration * dt));
	}

	void camera::move_backward(float dt)
	{
		add_velocity(-get_direction() * (movement_acceleration * dt));
	}

	void camera::move_left(float dt)
	{
		add_velocity(-get_right() * (movement_acceleration * dt));
	}

	void camera::move_right(float dt)
	{
		add_velocity(get_right() * (movement_acceleration * dt));
	}

	void camera::pitch(float angle, float dt)
	{
		IvVector3 right = get_right();
		IvMatrix33 rot;
		rot.Rotation(right, -angle * roatation_sensitivity * dt);
		IvVector3 dir = get_direction() * rot;
		IvVector3 gravity = get_gravity();
		physical::set_direction(dir, gravity);
		m_pitch_inertia = angle;
		m_is_rotating = true;
	}

	void camera::yaw(float angle, float dt)
	{
		IvVector3 gravity = get_gravity();
		IvMatrix33 rot;
		rot.Rotation(gravity, -angle * roatation_sensitivity * dt);
		IvVector3 dir = get_direction() * rot;
		physical::set_direction(dir, gravity);
		m_yaw_inertia = angle;
		m_is_rotating = true;
	}

	inline void camera::add_velocity(const IvVector3 & velocity)
	{
		m_velocity += velocity;
		m_is_moving = true;
	}
}
