#pragma once
#include <string>

class IvTexture;

namespace cali
{
	namespace texture
	{
		IvTexture* load_texture_from_bmp(const std::string & path);

		template <typename T>
		void set_texture_safely(T* shader, const char* texture_name, IvTexture* texture)
		{
			auto* texture_uniform = shader->GetUniform(texture_name);
			if (texture_uniform)
			{
				texture_uniform->SetValue(texture);
			}
		}
	}
}