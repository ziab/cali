#include "TerrainIcosahedron.h"

#include <IvUniform.h>
#include <IvResourceManager.h>
#include <IvTexture.h>
#include "CommonFileSystem.h"
#include "CommonTexture.h"

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

		m_height_map_texture = Texture::load_texture_from_bmp(get_executable_file_directory() + "\\bitmaps\\heightmap.bmp");
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

	void TerrainIcosahedron::render(IvRenderer& renderer)
	{
		renderer.SetBlendFunc(kSrcAlphaBlendFunc, kOneMinusSrcAlphaBlendFunc, kAddBlendOp);
		m_icosahedron.render(renderer, m_shader);
	}

}