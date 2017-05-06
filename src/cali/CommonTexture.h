#pragma once
#include <string>

class IvTexture;

namespace Cali
{
	namespace Texture
	{
		IvTexture* load_texture_from_bmp(const std::string & path);
	}
}