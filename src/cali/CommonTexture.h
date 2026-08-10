#pragma once
#include <string>
#include <cstdint>

class IvTexture;

namespace cali
{
namespace texture
{
	IvTexture* load_texture_from_bmp(const std::string & path);
	IvTexture* generate_procedural_heightmap(uint64_t seed, int width = 1024, int height = 1024);
	IvTexture* generate_procedural_heightmap(const std::string& hash_str, int width = 1024, int height = 1024);

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