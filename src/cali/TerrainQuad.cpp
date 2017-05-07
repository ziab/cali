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
		m_qtree({ { 0.0, 0.0 }, { 10e3, 10e3 } }),
		m_grid(c_gird_dimention, c_gird_dimention, 1.0f),
		m_viewer_position{ 0.0f, 0.0f, 0.0f },
		m_overlapping_edge_cells(4),
		m_planet_center(Cali::World::c_earth_center),
		m_planet_radius(Cali::World::c_earth_radius)
	{
		std::string vertex_shader = construct_shader_path("terrain_quad.hlslv");
		std::string pixel_shader = construct_shader_path("terrain.hlslf");

		m_shader = IvRenderer::mRenderer->GetResourceManager()->CreateShaderProgram(
			IvRenderer::mRenderer->GetResourceManager()->CreateVertexShaderFromFile(
				vertex_shader.c_str(), "main"),
			IvRenderer::mRenderer->GetResourceManager()->CreateFragmentShaderFromFile(
				pixel_shader.c_str(), "main"));

		if (!m_shader) throw std::exception("Terrain: failed to load shader program");

		m_height_map_texture = Texture::load_texture_from_bmp(get_executable_file_directory() + "\\bitmaps\\heightmap.bmp");
		if (!m_height_map_texture) throw("Terrain: failed to load height map texture");

		//m_shader->GetUniform("height_map")->SetValue(m_height_map_texture);
	}

	TerrainQuad::~TerrainQuad()
	{
	}

	IvVector3 quad_center_to_vector_on_surf(const TerrainQuadTree::Quad& quad)
	{
		return IvVector3{ (float)quad.center.x, 0.0f, (float)quad.center.y };
	}

	double quad_size(const TerrainQuadTree::Quad& quad)
	{
		return quad.half_size.x * 2.0;
	}

	void TerrainQuad::render_grid(
		IvRenderer & renderer,
		Grid & grid,
		const IvVector3& offset,
		float scale,
		float grid_scale_factor)
	{
		grid.set_current_origin(m_viewer_position + offset, { scale, 1.0f, scale });

		m_shader->GetUniform("modelMatrix")->SetValue(grid.get_transformation_matrix(), 0);
		m_shader->GetUniform("grid_stride")->SetValue(grid.stride() * scale, 0);
		m_shader->GetUniform("grid_cols")->SetValue((float)grid.cols(), 0);
		m_shader->GetUniform("grid_rows")->SetValue((float)grid.rows(), 0);
		m_shader->GetUniform("grid_camera_offset")->SetValue(offset, 0);
		m_shader->GetUniform("grid_uv_quad_size")->SetValue(
			IvVector3{ grid_scale_factor, grid_scale_factor, 0.0f },
			0);

		m_shader->GetUniform("planet_center")->SetValue(m_planet_center, 0);
		m_shader->GetUniform("planet_radius")->SetValue(m_planet_radius, 0);

		grid.render(renderer, m_shader);
	}

	void TerrainQuad::update(float dt)
	{
		m_qtree.collapse();
		TerrainQuadTree::Circle circle{ { m_viewer_position.x, m_viewer_position.z }, 300};
		m_qtree.divide(circle, 6);
	}

	void TerrainQuad::render(IvRenderer & renderer)
	{
		std::vector<const TerrainQuadTree::Node*> nodes;
		m_qtree.query_nodes(nodes);

		for (auto&& node : nodes)
		{
			auto& quad = node->get_centred_quad();
			m_grid.set_position(quad_center_to_vector_on_surf(quad));

			float scale = (float)quad_size(quad) / (m_grid.width() - m_grid.stride());
			m_grid.set_scale(IvVector3{ scale, 1.0f, scale });

			m_shader->GetUniform("modelMatrix")->SetValue(m_grid.get_transformation_matrix(), 0);
			m_grid.render(renderer, m_shader);

			/*m_aabb.set_position(quad_center_to_vector_on_surf(quad));
			m_aabb.set_scale((float)quad_size(quad));
			m_aabb.render(renderer);*/
		}
	}

	void TerrainQuad::set_viewer(const IvVector3 & camera_position)
	{
		m_viewer_position = camera_position;
	}
}