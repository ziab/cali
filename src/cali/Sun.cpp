#include "Sun.h"

#include <IvVector3.h>
#include <IvShaderProgram.h>
#include <IvUniform.h>

#include "CommonFileSystem.h"
#include "World.h"
#include "ConstantBuffer.h"
#include "Constants.h"
#include <IvMath.h>

namespace cali
{
	void sun::create_sun_rect(const IvVector3 & size)
	{
        create_quad(m_sun_rect, size, true, false);
	}

	sun::sun()
	{
		create_sun_rect({ 1.f, 1.f, 1.f });
		physical::set_position({ 0.f, 1000.f, -cali::world::c_horizon_distance / 10 });
		physical::set_scale(cali::world::c_horizon_distance / 300);

		std::string vertex_shader_file = construct_shader_path("sun.hlslv");
		std::string pixel_shader_file = construct_shader_path("sun.hlslf");

		m_shader = IvRenderer::mRenderer->GetResourceManager()->CreateShaderProgram(
			IvRenderer::mRenderer->GetResourceManager()->CreateVertexShaderFromFile(
				vertex_shader_file.c_str(), "vs_sun"),
			IvRenderer::mRenderer->GetResourceManager()->CreateFragmentShaderFromFile(
				pixel_shader_file.c_str(), "ps_sun"));
	}

	sun::~sun()
	{
	}

	void sun::update(float dt)
	{
		IvMatrix33 mat;
		mat.Rotation({ 1.f, 0.f, 0.f }, -0.05f * dt);

		set_position(get_position() * mat);
	}

	void sun::render(IvRenderer & renderer)
	{
		set_transformation_matrix(renderer);
		renderer.SetBlendFunc(kOneBlendFunc, kOneMinusSrcAlphaBlendFunc, kAddBlendOp);
		m_sun_rect.render(renderer, m_shader);
	}

	template <typename T>
	T lerp(T a, T b, T f)
	{
		return a + f * (b - a);
	}

	void sun::update_global_state(constant_buffer_wrapper<constant_buffer::GlobalState>& global_state)
	{
		global_state->sun_position = get_position();
		// O_o
		global_state->sun_intensity = IvVector3(1000000000000000000000.0f, 1000000000000000000000.0f, 1000000000000000000000.0f);
		global_state->sun_attenuation = IvVector3(0.0f, 0.0f, 0.0000125f);
		global_state->sun_color = world::sun_zenith_color;

	}
}
