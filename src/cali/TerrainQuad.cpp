#include "TerrainQuad.h"

#include <IvShaderProgram.h>
#include <IvTexture.h>
#include <IvUniform.h>

#include "World.h"
#include "CommonFileSystem.h"
#include "CommonTexture.h"

#include "DebugInfo.h"

namespace Cali
{
	TerrainQuad::TerrainQuad() :
		m_qtree({ { 0.0, 0.0 }, { 20e6, 20e6 } }),
		m_grid(c_gird_dimention, c_gird_dimention, 1.0f),
		m_viewer_position{ 0.0f, 0.0f, 0.0f },
		m_overlapping_edge_cells(c_gird_dimention / 16),
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

		m_shader->GetUniform("height_map")->SetValue(m_height_map_texture);
	}

	TerrainQuad::~TerrainQuad()
	{
	}

	IvVector3 quad_center_to_vector_on_surf(const Quad& quad)
	{
		return IvVector3{ (float)quad.center.x, 0.0f, (float)quad.center.y };
	}

	struct LevelDesc
	{
		int level;
		double area_size;
	};

	LevelDesc get_level_from_distance(double distance, const double area_size, const int max_level)
	{
		distance = abs(distance);
		LevelDesc level_desc = {};
		level_desc.level = 0;
		level_desc.area_size = area_size;
		while (level_desc.area_size > distance && level_desc.level < max_level)
		{
			level_desc.area_size /= 2.0;
			level_desc.level += 1;
		}

		return level_desc;
	}

	void TerrainQuad::update(float dt)
	{
		m_qtree.collapse();

		float height = m_viewer_position.y;

		auto level_desc = get_level_from_distance((double)height, m_qtree.width(), 22);
		auto& info = DebugInfo::get_debug_info();
		info.set_debug_string(L"lod_level", (float)level_desc.level);

		Circle circle{ { m_viewer_position.x, m_viewer_position.z }, level_desc.area_size * 1.2};
		m_qtree.divide(circle, level_desc.level);
		m_qtree.divide(circle * 2, level_desc.level - 1);
		m_qtree.divide(circle * 4, level_desc.level - 2);
		m_qtree.divide(circle * 8, level_desc.level - 3);
		m_qtree.divide(circle * 32, level_desc.level - 4);
	}

	void TerrainQuad::render(IvRenderer & renderer)
	{
		assert("TerrainQuad::render() is not supported");
	}

	static float lerp(float a, float b, float f)
	{
		return a + f * (b - a);
	}

	void TerrainQuad::render(IvRenderer & renderer, const Frustum& frustum)
	{
		auto height = m_viewer_position.y;

		float curvature = 0.0;
		if (height > 1000.0f)
		{
			curvature = lerp(0.0f, 1.0f, float(height) / (m_planet_radius / 1000.0f));
			if (curvature > 1.0f) curvature = 1.0f;
		}

		m_shader->GetUniform("planet_center")->SetValue(m_planet_center, 0);
		m_shader->GetUniform("planet_radius")->SetValue(m_planet_radius, 0);
		m_shader->GetUniform("curvature")->SetValue(curvature, 0);

		Circle circle({ m_viewer_position.x, m_viewer_position.z }, height * 32 < 100000.0f ? 100000.0f : height * 32);
		
		m_nodes_rendered_per_frame = 0;

		RenderContext render_context{ renderer, frustum };

		m_qtree.visit(circle, *this, &TerrainQuad::render_node, &render_context);

		auto& info = DebugInfo::get_debug_info();
		info.set_debug_string(L"rendered_ndoes", (float)m_nodes_rendered_per_frame);
	}

	void TerrainQuad::render_node(const TerrainQuadTree::Node& node, void* render_context_ptr)
	{
		assert(render_context_ptr != nullptr);
		const RenderContext& render_context = *reinterpret_cast<RenderContext*>(render_context_ptr);

		auto& quad = node.get_centred_quad();

		if (!render_context.frustum.contains_aligned_bounding_box(
			(float)quad.center.x, 0.0f, (float)quad.center.y,
			(float)quad.width(), 1.0f, (float)quad.width()))
		{
			return;
		}

		m_grid.set_position(quad_center_to_vector_on_surf(quad));
		float scale = (float)quad.width() / (m_grid.width() - m_grid.stride() * m_overlapping_edge_cells);

		m_grid.set_scale(IvVector3{ scale, 1.0f, scale });
		m_shader->GetUniform("modelMatrix")->SetValue(m_grid.get_transformation_matrix(), 0);
		m_shader->GetUniform("grid_stride")->SetValue(m_grid.stride() * scale, 0);
		m_shader->GetUniform("grid_cols")->SetValue((float)m_grid.cols(), 0);
		m_shader->GetUniform("grid_rows")->SetValue((float)m_grid.rows(), 0);
		m_shader->GetUniform("grid_uv_quad_size")->SetValue(
			IvVector3{ scale * 0.01f, scale * 0.01f, 0.0f },
			0);
		m_shader->GetUniform("grid_center")->SetValue(m_grid.get_position(), 0);
		m_grid.render(render_context.renderer, m_shader);

		++m_nodes_rendered_per_frame;
	}

	void TerrainQuad::set_viewer(const IvVector3 & camera_position)
	{
		m_viewer_position = camera_position;
	}
}