#pragma once

//-------------------------------------------------------------------------------
//-- Dependencies ---------------------------------------------------------------
//-------------------------------------------------------------------------------

#include <IvMatrix33.h>
#include <IvVector2.h>
#include <IvVector3.h>
#include <IvVector4.h>

#include "Renderable.h"

#include "ConstantBufferWrapper.h"
#include "ConstantBuffer.h"
#include "Frustum.h"

//-------------------------------------------------------------------------------
//-- Classes --------------------------------------------------------------------
//-------------------------------------------------------------------------------

namespace Cali
{
	class Camera : public Physical, public Renderable
	{
		float m_fov;
		float m_near;
		float m_far;

		IvVector3 m_velocity;
		float m_yaw_inertia;
		float m_pitch_inertia;
		bool m_is_moving;
		bool m_is_rotating;
		
		static const float movement_inertia_decay;
		static const float movement_acceleration;
		static const float movement_max_velocity;
		static const float roatation_sensitivity;
		static const float rotation_inertia_decay;
		static const float addtional_acceleration;

		float m_addtional_acceleration;

		Frustum m_frustum;

	private:
		//////////////////////////////////////////////
		// Movable ?
		void next_position(float dt);
		void next_angle(float dt);
		//////////////////////////////////////////////
		void normalize();

		IvMatrix44 get_view_matrix();

	public:

		Camera(const IvVector3& postition, const IvVector3& direction);
		~Camera() {};

		// sends perspective settings to the renderer
		void set_fov(float new_fov);
		void send_settings_to_renderer(IvRenderer& renderer);

		virtual void update(float dt) override;
		virtual void render(IvRenderer& renderer) override;

		void update_global_state(ConstantBufferWrapper<ConstantBuffer::GlobalState>& global_state);

		void enable_speed_mode(float dt);

		const Frustum& get_frustum();

		//////////////////////////////////////////////
		// Movable ?

		void move_forward(float dt);
		void move_backward(float dt);
		void move_left(float dt);
		void move_right(float dt);

		void pitch(float angle, float dt);
		void yaw(float angle, float dt);

		void add_velocity(const IvVector3& velocity);
		//////////////////////////////////////////////
	};
}
