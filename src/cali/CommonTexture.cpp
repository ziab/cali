#include "CommonTexture.h"
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
}