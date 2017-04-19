#pragma once

//-------------------------------------------------------------------------------
//-- Dependencies ---------------------------------------------------------------
//-------------------------------------------------------------------------------

#include <IvMatrix33.h>
#include <IvVector2.h>
#include <IvVector3.h>
#include <IvRenderer.h>

#include <IvMatrix44.h>

//-------------------------------------------------------------------------------
//-- Classes --------------------------------------------------------------------
//-------------------------------------------------------------------------------

namespace Cali
{
	class Renderable
	{
		Renderable(Renderable&) = delete;
		Renderable& operator= (const Renderable&) = delete;

	public:
		Renderable() {};
		virtual ~Renderable() {};

		virtual void update(float dt) = 0;
		virtual void render(IvRenderer& renderer) = 0;
	};

	class Physical : public Renderable
	{
		IvMatrix44 m_model_matrix;
		IvVector3 m_position;
		IvVector3 m_direction;
		IvVector3 m_right;
		float m_scale;

	private:
		Physical(Physical&) = delete;
		Physical& operator= (const Physical&) = delete;

		void calculate_model_matrix();
		void normalize();

	public:
		Physical();
		virtual ~Physical() {};
		
		void set_scale(float scale);
		void set_position(const IvVector3& pos);
		void set_direction(const IvVector3& dir, const IvVector3& up);

		float get_scale() { return m_scale; }
		IvMatrix33 get_rotation();
		IvVector3& get_position() { return m_position; }
		IvVector3& get_direction() { return m_direction; }
		IvVector3& get_right() { return m_right; }

		void look_at(const IvVector3& point, const IvVector3 & up);

		const IvMatrix44& get_transformation_matrix() { return m_model_matrix; };

		void set_transformation_matrix(IvRenderer& renderer);

		void pitch(float angle);
		void yaw(float angle);
	};
}