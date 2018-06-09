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

namespace cali
{
	class renderable
	{
		renderable(renderable&) = delete;
		renderable& operator= (const renderable&) = delete;

	public:
		renderable() {};
		virtual ~renderable() {};

		virtual void update(float dt) = 0;
		virtual void render(IvRenderer& renderer) = 0;
	};

	class compound_renderable
	{
		virtual void render(IvRenderer& renderer, const class frustum& frustum) = 0;
	};

	class physical
	{
		IvMatrix44 m_model_matrix;
		IvVector3 m_position;
		IvVector3 m_direction;
		IvVector3 m_right;
		IvVector3 m_scale;

	private:
		physical(physical&) = delete;
		physical& operator= (const physical&) = delete;

		void calculate_model_matrix();
		void normalize();

	public:
		physical();
		virtual ~physical() {};
		
		void set_scale(float scale);
		void set_scale(const IvVector3& scale);
		void set_position(const IvVector3& pos);
		void set_direction(const IvVector3& dir, const IvVector3& up);

		const IvVector3& get_scale() const { return m_scale; }
		IvMatrix44 get_rotation();
		const IvVector3& get_position() const { return m_position; }
		const IvVector3& get_direction() const { return m_direction; }
		const IvVector3& get_right() const { return m_right; }

		void look_at(const IvVector3& point, const IvVector3 & up);

		const IvMatrix44& get_transformation_matrix() { return m_model_matrix; };

		void set_transformation_matrix(IvRenderer& renderer) const;

		void pitch(float angle);
		void yaw(float angle);
		void rotate(const IvVector3& origin, const IvVector3& deviation);
	};
}