#include "TerrainQuad.h"

#include <IvShaderProgram.h>
#include <IvTexture.h>
#include <IvRenderTexture.h>
#include <IvUniform.h>

#include "World.h"
#include "Constants.h"
#include "CommonFileSystem.h"
#include "CommonTexture.h"
#include "CaliMath.h"
#include "CaliSphereMath.h"

#include "DebugInfo.h"

#include "IvDoubleVector3.h"

namespace Cali
{
	TerrainQuad::TerrainQuad() :
		m_qtree({ { 0.0, 0.0 }, { World::c_earth_radius, World::c_earth_radius } }),
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

		calculate_displacement_data_for_detail_levels();
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

		auto planet_center_relative_to_viewer = m_planet_center - m_viewer_position;
		auto height = abs(planet_center_relative_to_viewer.Length() - m_planet_radius);

		auto level_desc = get_level_from_distance((double)height, m_qtree.width(), c_detail_levels);
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

	void get_map_lon_lat_form_viewer_position(const IvVector3& sphere_center, double sphere_radius, const IvVector3& viewer,
		double& lon, double& lat, IvDoubleVector3& hit_point)
	{
		auto ray_direction = sphere_center - viewer;
		ray_direction.Normalize();

		double distance; IvDoubleVector3 normal;
		Math::intersect_ray_sphere(viewer, ray_direction, sphere_center, sphere_radius, hit_point, distance, normal);

		Math::get_lon_lat_from_point_on_sphere(sphere_center, sphere_radius, hit_point, lon, lat);

		/// Ok this is 
		auto& info = DebugInfo::get_debug_info();
		info.set_debug_string(L"lon", (float)lon);
		info.set_debug_string(L"lat", (float)lat);
	}

	void TerrainQuad::render(IvRenderer & renderer, const Frustum& frustum)
	{
		auto planet_center_relative_to_viewer = m_planet_center - m_viewer_position;
		auto height = abs(planet_center_relative_to_viewer.Length() - m_planet_radius);

		float curvature = 1.0;
		/*if (height > 1000.0f)
		{
			curvature = lerp(0.0f, 1.0f, float(height) / (m_planet_radius / 1000.0f));
			if (curvature > 1.0f) curvature = 1.0f;
		}*/

		m_shader->GetUniform("planet_center")->SetValue(planet_center_relative_to_viewer, 0);
		m_shader->GetUniform("planet_radius")->SetValue((float)m_planet_radius, 0);
		m_shader->GetUniform("curvature")->SetValue(curvature, 0);

		double lon, lat; IvDoubleVector3 hit_point;
		get_map_lon_lat_form_viewer_position(m_planet_center, m_planet_radius, m_viewer_position, lon, lat, hit_point);

		m_shader->GetUniform("planet_lon")->SetValue((float)lon, 0);
		m_shader->GetUniform("planet_lat")->SetValue((float)lat, 0);

		double map_x = lon * m_planet_radius;
		double map_y = lat * m_planet_radius;

		m_aabb.set_position(hit_point);
		m_aabb.set_scale(1.0f);
		m_aabb.render(renderer);

		auto& info = DebugInfo::get_debug_info();
		info.set_debug_string(L"map_x", (float)map_x);
		info.set_debug_string(L"map_y", (float)map_y);

		Circle circle({ map_x, map_y }, height * 32 < 1000.0f ? 1000.0f : height * 32);
		
		m_nodes_rendered_per_frame = 0;

		RenderContext render_context{ renderer, frustum };

		m_qtree.visit(circle, *this, &TerrainQuad::render_node, &render_context);

		info.set_debug_string(L"rendered_ndoes", (float)m_nodes_rendered_per_frame);
	}

	void set_quad_data_texture(void* quad_data_texture, size_t width, size_t height, size_t x, size_t y,
		const IvVector3& displacement, const IvVector3& normal)
	{
		float* data = reinterpret_cast<float*>(quad_data_texture);
		float& displacement_x = data[(x * 4     + y * width * 4)    ];
		float& displacement_y = data[(x * 4 + 1 + y * width * 4)    ];
		float& displacement_z = data[(x * 4 + 2 + y * width * 4)    ];
		float& displacement_w = data[(x * 4 + 3 + y * width * 4)    ];
		float& normal_x       = data[(x * 4     + y * width * 4) + height * width * 4];
		float& normal_y       = data[(x * 4 + 1 + y * width * 4) + height * width * 4];
		float& normal_z       = data[(x * 4 + 2 + y * width * 4) + height * width * 4];
		float& normal_w       = data[(x * 4 + 3 + y * width * 4) + height * width * 4];

		displacement_x = displacement.x;
		displacement_y = displacement.y;
		displacement_z = displacement.z;
		displacement_w = 1.0f;

		normal_x = normal.x;
		normal_y = normal.y;
		normal_z = normal.z;
		normal_w = 1.0f;
	}

	void TerrainQuad::calculate_displacement_data(const Cali::Quad& quad, int level, void* quad_data_texture)
	{
		//
		//  0 <----u----> 1
		//  a ----------- b    0
		//  |             |   /|\
		//  |             |    |
		//  |             |    v
		//  |  *(u,v)     |    |
		//  |             |   \|/
		//  d------------ c    1
		//

		IvDoubleVector3 A, B, C, D;
		A = Math::cube_to_sphere({ -quad.half_size.x, m_planet_radius, quad.half_size.y }, m_planet_radius, m_planet_center);
		B = Math::cube_to_sphere({ quad.half_size.x, m_planet_radius, quad.half_size.y }, m_planet_radius, m_planet_center);
		C = Math::cube_to_sphere({ quad.half_size.x, m_planet_radius, -quad.half_size.y }, m_planet_radius, m_planet_center);
		D = Math::cube_to_sphere({ -quad.half_size.x, m_planet_radius, -quad.half_size.y }, m_planet_radius, m_planet_center);

		double surface_grid_step = quad.half_size.x * 2.0 / c_gird_dimention;
		double surface_x = 0.0, surface_y = ((double)c_gird_dimention / 2.0) * surface_grid_step;
		for (int32_t y = 0; y < c_gird_dimention; ++y)
		{
			surface_x = -((double)c_gird_dimention / 2.0) * surface_grid_step;
			for (int32_t x = 0; x < c_gird_dimention; ++x)
			{
				double u = (double)x / (c_gird_dimention - 1); double v = (double)y / (c_gird_dimention - 1);

				IvDoubleVector3 current_vertex_3d = Math::quad_lerp(A, B, C, D, u, v);

				IvDoubleVector3 normal; 
				IvDoubleVector3 position = Math::cube_to_sphere({surface_x, m_planet_radius, surface_y }, m_planet_radius, m_planet_center);
				IvVector3 displacement = position - current_vertex_3d;

				set_quad_data_texture(quad_data_texture, c_gird_dimention, c_gird_dimention, x, y, displacement, normal);

				surface_x += surface_grid_step;
			}

			surface_y -= surface_grid_step;
		}
	}

	void TerrainQuad::calculate_displacement_data_for_detail_levels()
	{
		auto resman = IvRenderer::mRenderer->GetResourceManager();
		m_quad_data_textures.resize(c_detail_levels);
		Quad current_quad{ { 0.0, 0.0 }, { m_qtree.width() / 2.0, m_qtree.height() / 2.0 } };
		for (int i = 0; i < c_detail_levels; ++i)
		{
			auto texture = resman->CreateRenderTexture(c_gird_dimention, c_gird_dimention, 2, kFloat128Fmt);
			m_quad_data_textures[i] = texture;
			calculate_displacement_data(current_quad, i, texture->BeginLoadData());

			texture->EndLoadData();

			current_quad.half_size.x /= 2;
			current_quad.half_size.y /= 2;
		}
	}

	void TerrainQuad::render_node(const TerrainQuadTree::Node& node, void* render_context_ptr)
	{
		assert(render_context_ptr != nullptr);
		const RenderContext& render_context = *reinterpret_cast<RenderContext*>(render_context_ptr);

		auto& quad = node.get_centred_quad();
		auto detail_level = node.get_depth() - 1;

		//if (detail_level != 12) return;

		//if (!render_context.frustum.contains_aligned_bounding_box(
		//	(float)quad.center.x, 0.0f, (float)quad.center.y,
		//	(float)quad.width(), 1.0f, (float)quad.width()))
		//{
		//	return;
		//}

		IvDoubleVector3 position, normal, tangent;
		IvDoubleVector3 A, B, C, D;
		A = Math::cube_to_sphere({ quad.center.x - quad.half_size.x, m_planet_radius, quad.center.y + quad.half_size.y }, m_planet_radius, m_planet_center);
		B = Math::cube_to_sphere({ quad.center.x + quad.half_size.x, m_planet_radius, quad.center.y + quad.half_size.y }, m_planet_radius, m_planet_center);
		C = Math::cube_to_sphere({ quad.center.x + quad.half_size.x, m_planet_radius, quad.center.y - quad.half_size.y }, m_planet_radius, m_planet_center);
		D = Math::cube_to_sphere({ quad.center.x - quad.half_size.x, m_planet_radius, quad.center.y - quad.half_size.y }, m_planet_radius, m_planet_center);

		m_aabb.set_position(A); m_aabb.render(render_context.renderer);
		m_aabb.set_position(B); m_aabb.render(render_context.renderer);
		m_aabb.set_position(C); m_aabb.render(render_context.renderer);
		m_aabb.set_position(D); m_aabb.render(render_context.renderer);

		m_shader->GetUniform("quad_a")->SetValue((IvVector3)A - m_viewer_position, 0);
		m_shader->GetUniform("quad_b")->SetValue((IvVector3)B - m_viewer_position, 0);
		m_shader->GetUniform("quad_c")->SetValue((IvVector3)C - m_viewer_position, 0);
		m_shader->GetUniform("quad_d")->SetValue((IvVector3)D - m_viewer_position, 0);

		Math::position_on_sphere_from_surface(quad.center.x, quad.center.y, m_planet_radius, m_planet_center, position, normal, tangent);
		m_grid.set_position(position);
		m_grid.set_direction(normal, {1.0, 0.0, 0.0});
		m_shader->GetUniform("rotation_matrix")->SetValue(m_grid.get_rotation(), 0);
		float scale = (float)quad.width() / (m_grid.width() - m_grid.stride() * m_overlapping_edge_cells);

		m_grid.set_scale(IvVector3{ scale, scale, 1.0f });
		m_shader->GetUniform("grid_stride")->SetValue(m_grid.stride() * scale, 0);
		m_shader->GetUniform("grid_cols")->SetValue((float)m_grid.cols(), 0);
		m_shader->GetUniform("grid_rows")->SetValue((float)m_grid.rows(), 0);
		m_shader->GetUniform("grid_uv_quad_size")->SetValue(
			IvVector3{ scale * 0.1f, scale * 0.1f, 0.0f },
			0);
		m_shader->GetUniform("grid_center")->SetValue(m_grid.get_position(), 0);

		m_shader->GetUniform("quad_data")->SetValue(m_quad_data_textures[detail_level]);

		m_grid.render(render_context.renderer, m_shader);

		++m_nodes_rendered_per_frame;
	}

	void TerrainQuad::set_viewer(const IvVector3 & camera_position)
	{
		m_viewer_position = camera_position;
	}
}