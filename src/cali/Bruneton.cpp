#include "Bruneton.h"

#include <IvVector3.h>
#include <IvShaderProgram.h>
#include <IvUniform.h>

#include <D3D11\IvTextureD3D11.h>
#include <D3D11\IvRendererD3D11.h>

#include "CommonFileSystem.h"

#include "shaders\BrunetonCommonDefs.h"

namespace cali
{
	constexpr int SCATTERING_TEXTURE_WIDTH =
		SCATTERING_TEXTURE_NU_SIZE * SCATTERING_TEXTURE_MU_S_SIZE;
	constexpr int SCATTERING_TEXTURE_HEIGHT = SCATTERING_TEXTURE_MU_SIZE;
	constexpr int SCATTERING_TEXTURE_DEPTH = SCATTERING_TEXTURE_R_SIZE;

	constexpr int NUM_SCATTERING_ORDERS = 4;

	void bruneton::initialize(IvRenderer& renderer)
	{
		auto& resman = *renderer.GetResourceManager();

		// quad is used as screen dimentions
        m_quad = create_quad({ 1.f, 1.f, 1.f }, false, false);

		std::string draw_quad_vs_file = construct_shader_path("draw_quad.hlslv");
		std::string transmittance_ps_file = construct_shader_path("bruneton_transmittance.hlslf");

		m_compute_transmittance_shader = resman.CreateShaderProgram(
			resman.CreateVertexShaderFromFile(
				draw_quad_vs_file.c_str(), "main"),
			resman.CreateFragmentShaderFromFile(
				transmittance_ps_file.c_str(), "main"));

		std::string irradiance_ps_file = construct_shader_path("bruneton_irradiance.hlslf");

		m_compute_direct_irradiance_shader = resman.CreateShaderProgram(
			resman.CreateVertexShaderFromFile(
				draw_quad_vs_file.c_str(), "main"),
			resman.CreateFragmentShaderFromFile(
				irradiance_ps_file.c_str(), "main"));

		std::string single_scattering_ps_file = construct_shader_path("bruneton_single_scattering.hlslf");

		m_compute_single_scattering_shader = resman.CreateShaderProgram(
			resman.CreateVertexShaderFromFile(
				draw_quad_vs_file.c_str(), "main_3d"),
			resman.CreateFragmentShaderFromFile(
				single_scattering_ps_file.c_str(), "main"));

		std::string scattering_density_ps_file = construct_shader_path("bruneton_scattering_density.hlslf");

		m_compute_scattering_density_shader = resman.CreateShaderProgram(
			resman.CreateVertexShaderFromFile(
				draw_quad_vs_file.c_str(), "main_3d"),
			resman.CreateFragmentShaderFromFile(
				scattering_density_ps_file.c_str(), "main"));

		std::string indirect_irradiance_ps_file = construct_shader_path("bruneton_indirect_irradiance.hlslf");

		m_compute_indirect_irradiance_shader = resman.CreateShaderProgram(
			resman.CreateVertexShaderFromFile(
				draw_quad_vs_file.c_str(), "main"),
			resman.CreateFragmentShaderFromFile(
				indirect_irradiance_ps_file.c_str(), "main"));

		std::string multiple_scattering_ps_file = construct_shader_path("bruneton_multiple_scattering.hlslf");

		m_compute_multiple_scattering_shader = resman.CreateShaderProgram(
			resman.CreateVertexShaderFromFile(
				draw_quad_vs_file.c_str(), "main_3d"),
			resman.CreateFragmentShaderFromFile(
				multiple_scattering_ps_file.c_str(), "main"));

		if (!m_compute_transmittance_shader || !m_compute_direct_irradiance_shader || !m_compute_single_scattering_shader ||
			!m_compute_scattering_density_shader || !m_compute_indirect_irradiance_shader || !m_compute_multiple_scattering_shader)
			throw std::exception("sky: failed to load on of the shaders!");

		m_transmittance_texture = std::unique_ptr<IvRenderTexture>(
			renderer.GetResourceManager()->CreateRenderTexture(TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT,
				IvTextureFormat::kRGBAFloat16TexFmt));

		m_delta_irradiance_texture = std::unique_ptr<IvRenderTexture>(
			renderer.GetResourceManager()->CreateRenderTexture(IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT,
				IvTextureFormat::kRGBAFloat16TexFmt));

		m_irradiance_texture = std::unique_ptr<IvRenderTexture>(
			renderer.GetResourceManager()->CreateRenderTexture(IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT,
				IvTextureFormat::kRGBAFloat16TexFmt));

		m_scattering_texture = std::unique_ptr<IvRenderTexture>(
			renderer.GetResourceManager()->CreateRenderTexture(SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT,
				SCATTERING_TEXTURE_DEPTH, IvTextureFormat::kRGBAFloat16TexFmt));

		m_delta_rayleigh_scattering_texture = std::unique_ptr<IvRenderTexture>(
			renderer.GetResourceManager()->CreateRenderTexture(SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT,
				SCATTERING_TEXTURE_DEPTH, IvTextureFormat::kRGBAFloat16TexFmt));

		m_delta_multiple_scattering_texture_ref = m_delta_rayleigh_scattering_texture.get();

		m_delta_mie_scattering_texture = std::unique_ptr<IvRenderTexture>(
			renderer.GetResourceManager()->CreateRenderTexture(SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT,
				SCATTERING_TEXTURE_DEPTH, IvTextureFormat::kRGBAFloat16TexFmt));

		m_delta_scattering_density_texture = std::unique_ptr<IvRenderTexture>(
			renderer.GetResourceManager()->CreateRenderTexture(SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT,
				SCATTERING_TEXTURE_DEPTH, IvTextureFormat::kRGBAFloat16TexFmt));
	}

	void bruneton::compute_transmittance(IvRenderer & renderer)
	{
		renderer.SetBlendFunc(kOneBlendFunc, kZeroBlendFunc, kAddBlendOp);
		renderer.SetViewPort(m_transmittance_texture->GetWidth(), m_transmittance_texture->GetHeight());
		renderer.SetRenderTarget(m_transmittance_texture.get(), true);
		m_quad.render(renderer, m_compute_transmittance_shader);
		renderer.ReleaseRenderTarget();
	}

	void bruneton::compute_direct_irradiance(IvRenderer & renderer)
	{
		renderer.SetBlendFunc(kOneBlendFunc, kZeroBlendFunc, kAddBlendOp);
		renderer.SetViewPort(m_delta_irradiance_texture->GetWidth(), m_delta_irradiance_texture->GetHeight());
		renderer.SetRenderTarget(m_delta_irradiance_texture.get(), true);
		m_compute_direct_irradiance_shader->GetUniform("transmittance_texture")->SetValue(m_transmittance_texture.get());
		m_quad.render(renderer, m_compute_direct_irradiance_shader);
		renderer.ReleaseRenderTarget();
		m_compute_direct_irradiance_shader->GetUniform("transmittance_texture")->Unbind();
	}

	void bruneton::compute_single_scattering(IvRenderer & renderer)
	{
		renderer.SetBlendFunc(kOneBlendFunc, kZeroBlendFunc, kAddBlendOp);
		renderer.SetViewPort(m_scattering_texture->GetWidth(), m_scattering_texture->GetHeight());
		m_compute_single_scattering_shader->GetUniform("transmittance_texture")->SetValue(m_transmittance_texture.get());

		for (unsigned int layer = 0; layer < m_scattering_texture->GetDepth(); ++layer)
		{
			m_delta_rayleigh_scattering_texture->Set3DSlice(layer);
			m_delta_mie_scattering_texture->Set3DSlice(layer);
			m_scattering_texture->Set3DSlice(layer);
			// Build a vector with view. Note that the order _matters_
			std::vector<IvRenderTexture*> render_textures;
			render_textures.push_back(m_delta_rayleigh_scattering_texture.get());
			render_textures.push_back(m_delta_mie_scattering_texture.get());
			render_textures.push_back(m_scattering_texture.get());

			renderer.SetRenderTargets(render_textures, true);

			m_compute_single_scattering_shader->GetUniform("layer")->SetValue((float)layer, 0);
			m_quad.render(renderer, m_compute_single_scattering_shader);
		}

		m_compute_single_scattering_shader->GetUniform("transmittance_texture")->Unbind();
		renderer.ReleaseRenderTarget();

		// Reset view
		m_delta_rayleigh_scattering_texture->Set3DSlice(0);
		m_delta_mie_scattering_texture->Set3DSlice(0);
		m_scattering_texture->Set3DSlice(0);
	}

	// Compute the scattering density, and store it in
	// delta_scattering_density_texture.
	void bruneton::compute_scattering_density(IvRenderer & renderer, size_t scattering_order)
	{
		renderer.SetBlendFunc(kOneBlendFunc, kZeroBlendFunc, kAddBlendOp);
		renderer.SetViewPort(m_delta_scattering_density_texture->GetWidth(), m_delta_scattering_density_texture->GetHeight());

		m_compute_scattering_density_shader->GetUniform("transmittance_texture")->SetValue(
			m_transmittance_texture.get());
		m_compute_scattering_density_shader->GetUniform("single_rayleigh_scattering_texture")->SetValue(
			m_delta_rayleigh_scattering_texture.get());
		m_compute_scattering_density_shader->GetUniform("single_mie_scattering_texture")->SetValue(
			m_delta_mie_scattering_texture.get());
		m_compute_scattering_density_shader->GetUniform("multiple_scattering_texture")->SetValue(
			m_delta_multiple_scattering_texture_ref);
		m_compute_scattering_density_shader->GetUniform("irradiance_texture")->SetValue(
			m_delta_irradiance_texture.get());

		m_compute_scattering_density_shader->GetUniform("scattering_order")->SetValue((float)scattering_order, 0);

		for (unsigned int layer = 0; layer < SCATTERING_TEXTURE_DEPTH; ++layer)
		{
			m_delta_scattering_density_texture->Set3DSlice(layer);
			renderer.SetRenderTarget(m_delta_scattering_density_texture.get(), true);
			m_compute_scattering_density_shader->GetUniform("layer")->SetValue((float)layer, 0);
			m_quad.render(renderer, m_compute_scattering_density_shader);
		}

		m_compute_scattering_density_shader->GetUniform("transmittance_texture")->Unbind();
		m_compute_scattering_density_shader->GetUniform("single_rayleigh_scattering_texture")->Unbind();
		m_compute_scattering_density_shader->GetUniform("single_mie_scattering_texture")->Unbind();
		m_compute_scattering_density_shader->GetUniform("multiple_scattering_texture")->Unbind();
		m_compute_scattering_density_shader->GetUniform("irradiance_texture")->Unbind();

		renderer.ReleaseRenderTarget();
	}

	// Compute the indirect irradiance, store it in delta_irradiance_texture and
	// accumulate it in irradiance_texture.
	void bruneton::compute_indirect_irradiance(IvRenderer & renderer, size_t scattering_order)
	{
		renderer.SetBlendFunc(kOneBlendFunc, kOneBlendFunc, kAddBlendOp);
		renderer.SetViewPort(m_delta_irradiance_texture->GetWidth(), m_delta_irradiance_texture->GetHeight());

		m_compute_indirect_irradiance_shader->GetUniform("single_rayleigh_scattering_texture")->SetValue(
			m_delta_rayleigh_scattering_texture.get());
		m_compute_indirect_irradiance_shader->GetUniform("single_mie_scattering_texture")->SetValue(
			m_delta_mie_scattering_texture.get());
		m_compute_indirect_irradiance_shader->GetUniform("multiple_scattering_texture")->SetValue(
			m_delta_multiple_scattering_texture_ref);

		m_compute_indirect_irradiance_shader->GetUniform("scattering_order")->SetValue((float)scattering_order, 0);

		renderer.ClearRenderTarget(m_delta_irradiance_texture.get(), IvClearBuffer::kColorClear, { 0.0f, 0.0f, 0.0f, 0.0f });

		std::vector<IvRenderTexture*> render_textures;
		render_textures.push_back(m_delta_irradiance_texture.get());
		render_textures.push_back(m_irradiance_texture.get());

		renderer.SetRenderTargets(render_textures, true);

		m_quad.render(renderer, m_compute_indirect_irradiance_shader);

		renderer.ReleaseRenderTarget();

		m_compute_indirect_irradiance_shader->GetUniform("single_rayleigh_scattering_texture")->Unbind();
		m_compute_indirect_irradiance_shader->GetUniform("single_mie_scattering_texture")->Unbind();
		m_compute_indirect_irradiance_shader->GetUniform("multiple_scattering_texture")->Unbind();
	}

	void bruneton::compute_multiple_scattering(IvRenderer & renderer, size_t scattering_order)
	{
		renderer.SetBlendFunc(kOneBlendFunc, kOneBlendFunc, kAddBlendOp);
		renderer.SetViewPort(m_scattering_texture->GetWidth(), m_scattering_texture->GetHeight());

		m_compute_multiple_scattering_shader->GetUniform("transmittance_texture")->SetValue(
			m_transmittance_texture.get());

		m_compute_multiple_scattering_shader->GetUniform("scattering_density_texture")->SetValue(
			m_delta_scattering_density_texture.get());

		for (unsigned int layer = 0; layer < m_scattering_texture->GetDepth(); ++layer)
		{
			m_delta_multiple_scattering_texture_ref->Set3DSlice(layer);
			m_scattering_texture->Set3DSlice(layer);

			renderer.ClearRenderTarget(m_delta_multiple_scattering_texture_ref, IvClearBuffer::kColorClear, { 0.0f, 0.0f, 0.0f, 0.0f });

			// Build a vector with view. Note that the order _matters_
			std::vector<IvRenderTexture*> render_textures;
			render_textures.push_back(m_delta_multiple_scattering_texture_ref);
			render_textures.push_back(m_scattering_texture.get());

			renderer.SetRenderTargets(render_textures, true);

			m_compute_multiple_scattering_shader->GetUniform("layer")->SetValue((float)layer, 0);
			m_quad.render(renderer, m_compute_multiple_scattering_shader);
		}

		renderer.ReleaseRenderTarget();

		m_compute_multiple_scattering_shader->GetUniform("transmittance_texture")->Unbind();
		m_compute_multiple_scattering_shader->GetUniform("scattering_density_texture")->Unbind();
	}

	void bruneton::compute_oders(IvRenderer & renderer)
	{
		for (size_t scattering_order = 2; scattering_order <= NUM_SCATTERING_ORDERS; ++scattering_order)
		{
			compute_scattering_density(renderer, scattering_order);
			compute_indirect_irradiance(renderer, scattering_order);
			compute_multiple_scattering(renderer, scattering_order);
		}
	}

	void bruneton::precompute(IvRenderer& renderer)
	{
		initialize(renderer);

		auto& resman = *renderer.GetResourceManager();
		auto& d3d11renderer = static_cast<IvRendererD3D11&>(renderer);

		unsigned int previous_width = renderer.GetWidth();
		unsigned int previous_height = renderer.GetHeight();

		// WARNING: the order of calls matters

		compute_transmittance(renderer);
		compute_direct_irradiance(renderer);
		compute_single_scattering(renderer);
		compute_oders(renderer);

		renderer.SetViewPort(previous_width, previous_height);
	}
}