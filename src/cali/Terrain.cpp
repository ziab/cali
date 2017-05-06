#include "Terrain.h"

#include <IvUniform.h>
#include <IvResourceManager.h>
#include <IvTexture.h>
#include "CommonFileSystem.h"
#include "CommonTexture.h"

#include <vector>

#include "World.h"
#include "DebugInfo.h"

namespace Cali
{
	float lerp(float a, float b, float f)
	{
		return a + f * (b - a);
	}

	double get_scale_from_distance(double distance, double min_distance)
	{
		double scale = 1.0;

		while (distance > min_distance)
		{
			distance /= 2.0;
			scale *= 2.0;
		}

		return scale;
	}

	Terrain::RenderLevelParamerters Terrain::calculate_render_level_parameters(
		IvRenderer& renderer, const IvVector3 & position, const IvVector3 & planet_center, double radius)
	{
		////////////////////////////////////////////////////////////////////
		// A very hacky implementation
		////////////////////////////////////////////////////////////////////

		RenderLevelParamerters params = {};
		params.initial_scale = 1.0f;
		params.curvature = 0.0f;
		double distance_from_surface = abs((double)(position - planet_center).Length() - radius);

		params.initial_level_grid = &m_hd_grid;

		params.initial_scale = (float)get_scale_from_distance(distance_from_surface, m_hd_grid.width());

		auto& info = Cali::DebugInfo::get_debug_info();
		info.set_debug_string(L"initial_scale", params.initial_scale);

		params.max_level = size_t(abs(floor(lerp(6.0f, 1.0f, (float)distance_from_surface / (m_planet_radius / 10.0f)))));
		if (params.max_level > 5) params.max_level = 5;

		if (distance_from_surface > m_ld_grid.width())
		{
			params.curvature = lerp(0.0f, 1.0f, float(distance_from_surface - m_ld_grid.width()) / (m_planet_radius / 1000.0f));
			if (params.curvature > 1.0f) params.curvature = 1.0f;
		}

		return params;
	}

	void Terrain::update(float dt)
	{
	}

	void Terrain::set_viewer(const IvVector3 & position)
	{
		m_viewer_position = position;
	}

	Terrain::Terrain() :
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
	}

	Terrain::~Terrain()
	{
		IvRenderer::mRenderer->GetResourceManager()->Destroy(m_shader);
	}

	struct Pixel
	{
		uint8_t red;
		uint8_t green;
		uint8_t blue;
	};

	void Terrain::render_level(IvRenderer & renderer,
		Grid & level_grid,
		const IvVector3& offset,
		float scale,
		float grid_scale_factor)
	{
		level_grid.set_current_origin(m_viewer_position + offset, { scale, 1.0f, scale } );

		m_shader->GetUniform("modelMatrix")->SetValue(level_grid.get_transformation_matrix(), 0);
		m_shader->GetUniform("grid_stride")->SetValue(level_grid.stride() * scale, 0);
		m_shader->GetUniform("grid_cols")->SetValue((float)level_grid.cols(), 0);
		m_shader->GetUniform("grid_rows")->SetValue((float)level_grid.rows(), 0);
		m_shader->GetUniform("grid_camera_offset")->SetValue(offset, 0);
		m_shader->GetUniform("grid_uv_quad_size")->SetValue(
		    IvVector3{ grid_scale_factor, grid_scale_factor, 0.0f},
			0);

		m_shader->GetUniform("planet_center")->SetValue(m_planet_center, 0);
		m_shader->GetUniform("planet_radius")->SetValue(m_planet_radius, 0);

		level_grid.set_transformation_matrix(renderer);
		level_grid.render(renderer, m_shader);
	}

	void Terrain::render_levels(
		IvRenderer& renderer,
		Grid& level_grid,
		size_t level,
		size_t max_level,
		float offset_from_viewer,
		float grid_scale_factor,
		float grid_uv_scale_factor)
	{
		if (level > max_level) return;

		if (level == 0)
		{
			render_level(renderer, level_grid, { 0.0f, 0.0f, 0.0f }, grid_scale_factor, grid_uv_scale_factor);
			
			render_levels(
				renderer,
				m_ld_grid,
				level + 1, 
				max_level, 
				(level_grid.width() - level_grid.stride() * m_overlapping_edge_cells) * grid_scale_factor,
				grid_scale_factor, 
				grid_uv_scale_factor);
		}
		else
		{
			render_level(renderer, level_grid, { 0.0,                 0.0, offset_from_viewer  }, grid_scale_factor, grid_uv_scale_factor);
			render_level(renderer, level_grid, { offset_from_viewer,  0.0, offset_from_viewer  }, grid_scale_factor, grid_uv_scale_factor);
			render_level(renderer, level_grid, { offset_from_viewer,  0.0, 0.0                 }, grid_scale_factor, grid_uv_scale_factor);
			render_level(renderer, level_grid, { offset_from_viewer,  0.0, -offset_from_viewer }, grid_scale_factor, grid_uv_scale_factor);
			render_level(renderer, level_grid, { 0.0,                 0.0, -offset_from_viewer }, grid_scale_factor, grid_uv_scale_factor);
			render_level(renderer, level_grid, { -offset_from_viewer, 0.0, -offset_from_viewer }, grid_scale_factor, grid_uv_scale_factor);
			render_level(renderer, level_grid, { -offset_from_viewer, 0.0, 0.0                 }, grid_scale_factor, grid_uv_scale_factor);
			render_level(renderer, level_grid, { -offset_from_viewer, 0.0, offset_from_viewer  }, grid_scale_factor, grid_uv_scale_factor);

			render_levels(
				renderer,
				m_ld_grid,
				level + 1, 
				max_level, 
				offset_from_viewer + (level_grid.width() - level_grid.stride() * m_overlapping_edge_cells) * grid_scale_factor * 2.0f,
				grid_scale_factor * 3.0f, 
				grid_uv_scale_factor * 3.0f);
		}
	}

	void Terrain::render(IvRenderer& renderer)
	{
		renderer.SetBlendFunc(kSrcAlphaBlendFunc, kOneMinusSrcAlphaBlendFunc, kAddBlendOp);

		auto params = calculate_render_level_parameters(renderer, m_viewer_position, m_planet_center, m_planet_radius);
		m_shader->GetUniform("curvature")->SetValue(params.curvature, 0);

		render_levels(renderer, *params.initial_level_grid, 0, params.max_level, 0.0f, params.initial_scale, 0.05f * params.initial_scale);
	}
}