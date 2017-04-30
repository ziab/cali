#include "TerrainIcosahedron.h"

#include <IvUniform.h>
#include <IvResourceManager.h>
#include <IvTexture.h>
#include "CommonFileSystem.h"

#pragma warning(disable : 4996)
#include "..\..\depends\bitmap_image\bitmap_image.hpp"
#pragma warning(default : 4996)

#include <vector>

#include "World.h"

namespace Cali
{
	void TerrainIcosahedron::update(float dt)
	{
	}

	void TerrainIcosahedron::set_viewer(const IvVector3 & position)
	{
		m_viewer_position = position;
	}

	TerrainIcosahedron::TerrainIcosahedron() :
		m_viewer_position{ 0.0f, 0.0f, 0.0f },
		m_planet_center(Cali::World::c_earth_center),
		m_planet_radius(Cali::World::c_earth_radius)
	{
		std::string vertex_shader = construct_shader_path("simple.hlslv");
		std::string pixel_shader = construct_shader_path("simple.hlslf");

		m_shader = IvRenderer::mRenderer->GetResourceManager()->CreateShaderProgram(
			IvRenderer::mRenderer->GetResourceManager()->CreateVertexShaderFromFile(
				vertex_shader.c_str(), "main"),
			IvRenderer::mRenderer->GetResourceManager()->CreateFragmentShaderFromFile(
				pixel_shader.c_str(), "main"));

		if (!m_shader) throw std::exception("TerrainIcosahedron: failed to load shader program");

		m_height_map_texture = load_height_map_texture(get_executable_file_directory() + "\\bitmaps\\heightmap.bmp");
		if (!m_height_map_texture) throw("TerrainIcosahedron: failed to load height map texture");

		//m_shader->GetUniform("height_map")->SetValue(m_height_map_texture);

		m_icosahedron.set_position(m_planet_center);
		m_icosahedron.set_scale(m_planet_radius);
	}

	TerrainIcosahedron::~TerrainIcosahedron()
	{
		IvRenderer::mRenderer->GetResourceManager()->Destroy(m_shader);
	}

	struct Pixel
	{
		uint8_t red;
		uint8_t green;
		uint8_t blue;
	};

	void TerrainIcosahedron::read_height_map(
		const std::string & path,
		BufferRAIIWrapper<IvVertexBuffer, IvNPVertex>& vertices,
		size_t width,
		size_t height)
	{
		bitmap_image hmap(path);
		if (!hmap) return;

		float wstep = (float)hmap.width() / width;
		float hstep = (float)hmap.height() / height;

		const float scale = 0.04f;

		size_t index = 0;
		for (size_t vy = 0, by = 0; vy < height; ++vy)
		{
			for (size_t vx = 0, bx = 0; vx < width; ++vx)
			{
				Pixel pixel;
				hmap.get_pixel((unsigned int)bx, (unsigned int)by, pixel);

				vertices[index++].position.y = (float)pixel.blue * scale;

				bx += (size_t)wstep;
			}

			by += (size_t)hstep;
		}
	}

	IvTexture* TerrainIcosahedron::load_height_map_texture(const std::string & path)
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

	void TerrainIcosahedron::render(IvRenderer& renderer)
	{
		renderer.SetBlendFunc(kSrcAlphaBlendFunc, kOneMinusSrcAlphaBlendFunc, kAddBlendOp);
		m_icosahedron.render(renderer, m_shader);
	}

}