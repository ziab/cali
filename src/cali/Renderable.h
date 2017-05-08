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

	class CompoundRenderable
	{
		virtual void render(IvRenderer& renderer, const class Frustum& frustum) = 0;
	};

	class Physical
	{
		IvMatrix44 m_model_matrix;
		IvVector3 m_position;
		IvVector3 m_direction;
		IvVector3 m_right;
		IvVector3 m_scale;

	private:
		Physical(Physical&) = delete;
		Physical& operator= (const Physical&) = delete;

		void calculate_model_matrix();
		void normalize();

	public:
		Physical();
		virtual ~Physical() {};
		
		void set_scale(float scale);
		void set_scale(const IvVector3& scale);
		void set_position(const IvVector3& pos);
		void set_direction(const IvVector3& dir, const IvVector3& up);

		const IvVector3& get_scale() const { return m_scale; }
		const IvMatrix33& get_rotation();
		const IvVector3& get_position() const { return m_position; }
		const IvVector3& get_direction() const { return m_direction; }
		const IvVector3& get_right() const { return m_right; }

		void look_at(const IvVector3& point, const IvVector3 & up);

		const IvMatrix44& get_transformation_matrix() { return m_model_matrix; };

		void set_transformation_matrix(IvRenderer& renderer) const;

		void pitch(float angle);
		void yaw(float angle);
	};
}