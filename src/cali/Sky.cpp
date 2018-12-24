#include "Sky.h"

#include <IvVector3.h>
#include <IvShaderProgram.h>
#include <IvUniform.h>

#include <D3D11\IvTextureD3D11.h>
#include <D3D11\IvRendererD3D11.h>

#include <ScreenGrab.h>

#include <vector>
#include <string>

#include "CommonFileSystem.h"
#include "CommonTexture.h"
#include "World.h"

namespace cali
{
	void sky::create_sky_box(const IvVector3& size)
	{
		create_box(m_sky_box, size, false, false);
	}

	sky::sky(Bruneton& bruneton) :
		m_bruneton(bruneton)
	{
		std::string vertex_shader_file = construct_shader_path("sky.hlslv");
		std::string pixel_shader_file = construct_shader_path("sky_precomp.hlslf");

		auto& renderer = *IvRenderer::mRenderer;
		auto& resman = *renderer.GetResourceManager();

		create_sky_box(cali::world::c_sky_box_size);
        set_position(cali::world::c_earth_center);

		m_sky_shader = resman.CreateShaderProgram(
			resman.CreateVertexShaderFromFile(
				vertex_shader_file.c_str(), "vs_sky"),
			resman.CreateFragmentShaderFromFile(
				pixel_shader_file.c_str(), "ps_sky"));

		m_sky_shader->GetUniform("transmittance_texture")->SetValue(m_bruneton.get_transmittance_texture());
		m_sky_shader->GetUniform("scattering_texture")->SetValue(m_bruneton.get_scattering_texture());
		texture::set_texture_safely(m_sky_shader, "irradiance_texture", m_bruneton.get_irradiance_texture());
	}

	sky::~sky()
	{
		auto& renderer = *IvRenderer::mRenderer;
		renderer.GetResourceManager()->Destroy(m_sky_shader);
	}

	void sky::update(float dt)
	{
	}

	void sky::render(IvRenderer & renderer)
	{
		physical::set_transformation_matrix(renderer);
		m_sky_box.render(renderer, m_sky_shader);
	}
}