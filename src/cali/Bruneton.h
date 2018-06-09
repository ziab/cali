#pragma once

#include "Renderable.h"
#include "Model.h"
#include "IvRenderTexture.h"

namespace cali
{
	class Bruneton
	{
		cali::model<kTNPFormat, IvTNPVertex> m_quad;

		std::unique_ptr<IvRenderTexture> m_transmittance_texture;
		std::unique_ptr<IvRenderTexture> m_delta_irradiance_texture;
		std::unique_ptr<IvRenderTexture> m_irradiance_texture; // TODO: shold be filled with zeroes?; aka delta_irradiance_texture
		std::unique_ptr<IvRenderTexture> m_scattering_texture;
		std::unique_ptr<IvRenderTexture> m_delta_rayleigh_scattering_texture; // aka single_rayleigh_scattering_texture and delta_multiple_scattering_texture
		std::unique_ptr<IvRenderTexture> m_delta_mie_scattering_texture; // aka single_mie_scattering_texture
		std::unique_ptr<IvRenderTexture> m_delta_scattering_density_texture;
		IvRenderTexture* m_delta_multiple_scattering_texture_ref;

		IvShaderProgram* m_compute_transmittance_shader;
		IvShaderProgram* m_compute_direct_irradiance_shader;
		IvShaderProgram* m_compute_single_scattering_shader;
		IvShaderProgram* m_compute_scattering_density_shader;
		IvShaderProgram* m_compute_indirect_irradiance_shader;
		IvShaderProgram* m_compute_multiple_scattering_shader;

		void compute_transmittance(IvRenderer& renderer);
		void compute_direct_irradiance(IvRenderer& renderer);
		void compute_single_scattering(IvRenderer& renderer);
		void compute_oders(IvRenderer& renderer);
		void compute_scattering_density(IvRenderer& renderer, size_t scattering_order);
		void compute_indirect_irradiance(IvRenderer& renderer, size_t scattering_order);
		void compute_multiple_scattering(IvRenderer& renderer, size_t scattering_order);

		void initialize(IvRenderer& renderer);

		Bruneton(const Bruneton&) = delete;
		Bruneton& operator=(const Bruneton&) = delete;

	public:
		Bruneton() :
			m_compute_transmittance_shader(nullptr),
			m_compute_direct_irradiance_shader(nullptr),
			m_compute_single_scattering_shader(nullptr),
			m_compute_scattering_density_shader(nullptr),
			m_compute_indirect_irradiance_shader(nullptr),
			m_compute_multiple_scattering_shader(nullptr),
			m_delta_multiple_scattering_texture_ref(nullptr)
		{
		}

		~Bruneton()
		{
		}

		IvRenderTexture* get_transmittance_texture() { return m_transmittance_texture.get(); }
		IvRenderTexture* get_scattering_texture() { return m_scattering_texture.get(); }
		IvRenderTexture* get_irradiance_texture() { return m_irradiance_texture.get(); }

		void precompute(IvRenderer& renderer);
	};
}