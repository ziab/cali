#include "CommonTexture.h"
#include "Procedural.h"
#include <IvTexture.h>
#include <IvUniform.h>
#include <IvResourceManager.h>
#include <IvRenderer.h>

#pragma warning(disable : 4996)
#include "..\..\depends\bitmap_image\bitmap_image.hpp"
#pragma warning(default : 4996)

namespace cali
{
	IvTexture * texture::load_texture_from_bmp(const std::string & path)
	{
		auto& renderer = *IvRenderer::mRenderer;
		auto& resman = *renderer.GetResourceManager();

		bitmap_image hmap(path);
		if (!hmap) return nullptr;

		IvTexture* texture = resman.CreateTexture(kRGB24TexFmt, hmap.width(), hmap.height(), hmap.data(), kDefaultUsage);

		if (!texture) return nullptr;

		texture->SetAddressingU(kClampTexAddr);
		texture->SetAddressingV(kClampTexAddr);
		texture->SetMagFiltering(kBilerpTexMagFilter);
		texture->SetMinFiltering(kBilerpTexMinFilter);

		return texture;
	}

	IvTexture* texture::generate_procedural_heightmap(uint64_t seed, int width, int height)
	{
		return proc::generate_heightmap_texture(seed, width, height);
	}

	IvTexture* texture::generate_procedural_heightmap(const std::string& hash_str, int width, int height)
	{
		return proc::generate_heightmap_texture(hash_str, width, height);
	}

	IvTexture* texture::generate_planet_heightmap(uint64_t seed, int width, int height)
	{
		return proc::generate_planet_heightmap(seed, width, height);
	}

	IvTexture* texture::generate_planet_heightmap(const std::string& hash_str, int width, int height)
	{
		return proc::generate_planet_heightmap(hash_str, width, height);
	}
}