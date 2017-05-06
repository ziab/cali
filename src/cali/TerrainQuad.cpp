#include "TerrainQuad.h"

#include <IvShaderProgram.h>
#include <IvTexture.h>
#include <IvUniform.h>

#include "World.h"
#include "CommonFileSystem.h"
#include "CommonTexture.h"

namespace Cali
{
	TerrainQuad::TerrainQuad() :
		m_qtree({ { 0.0, 0.0 }, { 0.5, 0.5 } }),
		m_hd_grid(c_hd_gird_dimention, c_hd_gird_dimention, 1.0f),
		m_ld_grid(c_hd_gird_dimention / 2, c_hd_gird_dimention / 2, 2.0f),
		m_viewer_position{ 0.0f, 0.0f, 0.0f },
		m_overlapping_edge_cells(c_hd_gird_dimention / 32),
		m_planet_center(Cali::World::c_earth_center),
		m_planet_radius(Cali::World::c_earth_radius)
	{
		std::string vertex_shader = construct_shader_path("terrain.hlslv");
		std::string pixel_shader = construct_shader_path("terrain.hlslf");

		m_shader = IvRenderer::mRenderer->GetResourceManager()->CreateShaderProgram(
			IvRenderer::mRenderer->GetResourceManager()->CreateVertexShaderFromFile(
				vertex_shader.c_str(), "main"),
			IvRenderer::mRenderer->GetResourceManager()->CreateFragmentShaderFromFile(
				pixel_shader.c_str(), "main"));

		if (!m_shader) throw std::exception("Terrain: failed to load shader program");

		m_height_map_texture = Texture::load_texture_from_bmp(get_executable_file_directory() + "\\bitmaps\\heightmap.bmp");
		if (!m_height_map_texture) throw("Terrain: failed to load height map texture");

		m_shader->GetUniform("height_map")->SetValue(m_height_map_texture);

		m_qtree.divide({ 0.2, 0.2 }, 10);
	}

	TerrainQuad::~TerrainQuad()
	{
	}

	void TerrainQuad::update(float dt)
	{
	}

	IvVector3 quad_center_to_vector_on_surf(const TerrainQuadTree::Quad& quad)
	{
		return IvVector3{ (float)quad.center.x, 0.0f, (float)quad.center.y };
	}

	IvVector3 quad_size_to_vector(const TerrainQuadTree::Quad& quad, float scale_factor)
	{
		return IvVector3{ (float)quad.half_size.x * 2.0f * scale_factor, 5.0f, (float)quad.half_size.y * 2.0f * scale_factor };
	}

	void TerrainQuad::render(IvRenderer & renderer)
	{
		std::vector<const TerrainQuadTree::Node*> nodes;
		m_qtree.query_nodes(nodes);

		for (auto&& node : nodes)
		{
			auto& quad = node->get_centred_quad();
			m_aabb.set_position(quad_center_to_vector_on_surf(quad) * 1000.0f);
			m_aabb.set_scale(quad_size_to_vector(quad, 1000.f));
			m_aabb.render(renderer);
		}
	}

	void TerrainQuad::set_viewer(const IvVector3 & camera_position)
	{
	}
}