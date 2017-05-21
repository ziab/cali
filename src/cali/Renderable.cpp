#include "Renderable.h"
#include "Constants.h"

#include <IvMath.h>
#include <IvMatrix44.h>

namespace Cali
{
	// A helper function which just works
	inline void direction_vector_to_rotation_mat(const IvVector3 & dir, const IvVector3 & world_up, IvMatrix33 & mat)
	{
		auto right = dir.Cross(world_up);
		right.Normalize();

		auto up = dir.Cross(right);
		up.Normalize();

		mat(0, 0) = right.x;
		mat(0, 1) = dir.x;
		mat(0, 2) = up.x;

		mat(1, 0) = right.y;
		mat(1, 1) = dir.y;
		mat(1, 2) = up.y;

		mat(2, 0) = right.z;
		mat(2, 1) = dir.z;
		mat(2, 2) = up.z;
	}

	void Physical::calculate_model_matrix()
	{
		normalize();

		auto up = m_direction.Cross(m_right);
		up.Normalize();

		m_model_matrix(0, 0) = m_right.x;
		m_model_matrix(0, 1) = up.x;
		m_model_matrix(0, 2) = m_direction.x;

		m_model_matrix(1, 0) = m_right.y;
		m_model_matrix(1, 1) = up.y;
		m_model_matrix(1, 2) = m_direction.y;

		m_model_matrix(2, 0) = m_right.z;
		m_model_matrix(2, 1) = up.z;
		m_model_matrix(2, 2) = m_direction.z;

		m_model_matrix(0, 3) = m_position.x;
		m_model_matrix(1, 3) = m_position.y;
		m_model_matrix(2, 3) = m_position.z;

		IvMatrix44 scale_matrix;

		scale_matrix(0, 0) = m_scale.x;
		scale_matrix(1, 1) = m_scale.y;
		scale_matrix(2, 2) = m_scale.z;

		m_model_matrix = m_model_matrix * scale_matrix;
	}

	void Physical::normalize()
	{
		m_direction.Normalize();
		m_right.Normalize();
	}

	Physical::Physical()
	{
		m_model_matrix.Identity();
		m_position = { 0.f, 0.f, 0.f };
		m_direction = { 0.f, 0.f, 1.f };
		m_right = { 1.0, 0.0, 0.0 };
		m_scale = { 1.0f, 1.0f, 1.0f };
		calculate_model_matrix();
	}

	void Physical::set_scale(float scale)
	{
		m_scale = { scale, scale, scale };
		calculate_model_matrix();
	}

	void Physical::set_scale(const IvVector3 & scale)
	{
		m_scale = scale;
		calculate_model_matrix();
	}

	void Physical::set_position(const IvVector3 & pos)
	{
		m_position = pos;
		calculate_model_matrix();
	}

	void Physical::set_direction(const IvVector3& dir, const IvVector3& up)
	{
		IvVector3 direction = dir;

		// TODO: fix this
		if (direction == up)
		{
			direction.x += kEpsilon * 1000.f;
			direction.y += kEpsilon * 1000.f;
			direction.z += kEpsilon * 1000.f;
			direction.Normalize();
		}

		m_direction = direction;
		m_right = up.Cross(m_direction);

		calculate_model_matrix();
	}

	IvMatrix44 Physical::get_rotation()
	{
		auto up = m_direction.Cross(m_right);

		IvMatrix44 matrix;
		matrix(0, 0) = m_right.x;
		matrix(0, 1) = up.x;
		matrix(0, 2) = m_direction.x;

		matrix(1, 0) = m_right.y;
		matrix(1, 1) = up.y;
		matrix(1, 2) = m_direction.y;

		matrix(2, 0) = m_right.z;
		matrix(2, 1) = up.z;
		matrix(2, 2) = m_direction.z;

		return matrix;
	}

	void Physical::look_at(const IvVector3 & point, const IvVector3 & up)
	{
		auto direction = point - m_position;
		set_direction(direction, up);
	}

	void Physical::set_transformation_matrix(IvRenderer & renderer) const
	{
		renderer.SetWorldMatrix(m_model_matrix);
	}

	void Physical::pitch(float angle)
	{
		if (angle == 0.f) return;

		IvMatrix33 rotation_matrix;
		rotation_matrix.Rotation(m_right, -angle);

		m_direction = m_direction * rotation_matrix;
		
		calculate_model_matrix();
	}

	void Physical::yaw(float angle)
	{
		if (angle == 0.f) return;

		IvMatrix33 rotation_matrix;
		rotation_matrix.Rotation(Cali::Constants::c_world_up, -angle);

		m_direction = m_direction * rotation_matrix;
		m_right = m_right * rotation_matrix;

		calculate_model_matrix();
	}

	void Physical::rotate(const IvVector3 & from, const IvVector3 & to)
	{
		if (from == to)
		{
			return;
		}

		auto axis = to.Cross(from);
		float angle = asin(axis.Length());
		axis.Normalize();

		IvMatrix33 rotation_matrix;
		rotation_matrix.Rotation(axis, angle);

		m_direction = m_direction * rotation_matrix;
		m_right = m_right * rotation_matrix;

		calculate_model_matrix();

	}
}