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
#include "AABB.h"

#include "DebugInfo.h"

#include "IvDoubleVector3.h"

namespace cali
{
	void set_quad_data_texture(void* quad_data_texture, size_t width, size_t height, size_t x, size_t y,
		const IvVector3& displacement, const IvVector3& normal)
	{
		float* data = reinterpret_cast<float*>(quad_data_texture);
		float& displacement_x = data[(x * 4 + y * width * 4)];
		float& displacement_y = data[(x * 4 + 1 + y * width * 4)];
		float& displacement_z = data[(x * 4 + 2 + y * width * 4)];
		float& displacement_w = data[(x * 4 + 3 + y * width * 4)];
		float& normal_x = data[(x * 4 + y * width * 4) + height * width * 4];
		float& normal_y = data[(x * 4 + 1 + y * width * 4) + height * width * 4];
		float& normal_z = data[(x * 4 + 2 + y * width * 4) + height * width * 4];
		float& normal_w = data[(x * 4 + 3 + y * width * 4) + height * width * 4];

		displacement_x = displacement.x;
		displacement_y = displacement.y;
		displacement_z = displacement.z;
		displacement_w = 1.0f;

		normal_x = normal.x;
		normal_y = normal.y;
		normal_z = normal.z;
		normal_w = 1.0f;
	}

	void terrain_quad::calculate_displacement_data(const cali::quad& quad, int level, void* quad_data_texture)
	{
		// Keep original for face 0 (PosY) – displacement textures not used in current shader
		IvDoubleVector3 normal;
		IvDoubleVector3 A, B, C, D;
		A = Math::adjusted_cube_to_sphere(-quad.half_size.x, quad.half_size.y, m_planet_radius, m_planet_center, normal);
		B = Math::adjusted_cube_to_sphere(quad.half_size.x, quad.half_size.y, m_planet_radius, m_planet_center, normal);
		C = Math::adjusted_cube_to_sphere(quad.half_size.x, -quad.half_size.y, m_planet_radius, m_planet_center, normal);
		D = Math::adjusted_cube_to_sphere(-quad.half_size.x, -quad.half_size.y, m_planet_radius, m_planet_center, normal);

		double surface_grid_step = quad.half_size.x * 2.0 / (c_gird_cells - 1);
		double surface_x = 0.0, surface_y = ((double)c_gird_cells / 2.0) * surface_grid_step;
		for (int32_t y = 0; y < c_gird_cells; ++y)
		{
			surface_x = -((double)c_gird_cells / 2.0) * surface_grid_step;
			for (int32_t x = 0; x < c_gird_cells; ++x)
			{
				double u = (double)x / (c_gird_cells - 1); double v = (double)y / (c_gird_cells - 1);
				IvDoubleVector3 current_vertex_3d = Math::quad_lerp(A, B, C, D, u, v);
				IvDoubleVector3 position = Math::adjusted_cube_to_sphere(surface_x, surface_y, m_planet_radius, m_planet_center, normal);
				IvVector3 displacement = position - current_vertex_3d;
				set_quad_data_texture(quad_data_texture, c_gird_cells, c_gird_cells, x, y, displacement, normal);
				surface_x += surface_grid_step;
			}
			surface_y -= surface_grid_step;
		}
	}

	void terrain_quad::calculate_displacement_data_for_detail_levels()
	{
		auto resman = IvRenderer::mRenderer->GetResourceManager();
		for (auto* texture : m_quad_data_textures)
		{
			resman->Destroy(texture);
		}
		m_quad_data_textures.resize(c_detail_levels);
		quad current_quad{ { 0.0, 0.0 },{ m_qtrees[0].width() / 2.0, m_qtrees[0].height() / 2.0 } };
		for (int i = 0; i < c_detail_levels; ++i)
		{
			auto texture = resman->CreateRenderTexture(c_gird_cells, c_gird_cells, 2, kFloat128Fmt);
			m_quad_data_textures[i] = texture;
			calculate_displacement_data(current_quad, i, texture->BeginLoadData());
			texture->EndLoadData();
			current_quad.half_size.x /= 2;
			current_quad.half_size.y /= 2;
		}
	}

	terrain_quad::terrain_quad(bruneton& bruneton) :
		m_qtrees{
			terrain_quad_tree({ { 0.0, 0.0 }, { world::c_earth_radius, world::c_earth_radius } }),
			terrain_quad_tree({ { 0.0, 0.0 }, { world::c_earth_radius, world::c_earth_radius } }),
			terrain_quad_tree({ { 0.0, 0.0 }, { world::c_earth_radius, world::c_earth_radius } }),
			terrain_quad_tree({ { 0.0, 0.0 }, { world::c_earth_radius, world::c_earth_radius } }),
			terrain_quad_tree({ { 0.0, 0.0 }, { world::c_earth_radius, world::c_earth_radius } }),
			terrain_quad_tree({ { 0.0, 0.0 }, { world::c_earth_radius, world::c_earth_radius } })
		},
		m_grid(c_gird_cells, c_gird_cells, 1.0f),
		m_bruneton(bruneton),
		m_viewer_position{ 0.0f, 0.0f, 0.0f },
		m_overlapping_edge_cells(c_gird_cells / 16),
		m_planet_center(cali::world::c_earth_center),
		m_planet_radius(cali::world::c_earth_radius)
	{

		std::string vertex_shader = construct_shader_path("terrain_quad.hlslv");
		std::string pixel_shader = construct_shader_path("terrain.hlslf");

		m_shader = IvRenderer::mRenderer->GetResourceManager()->CreateShaderProgram(
			IvRenderer::mRenderer->GetResourceManager()->CreateVertexShaderFromFile(
				vertex_shader.c_str(), "main"),
			IvRenderer::mRenderer->GetResourceManager()->CreateFragmentShaderFromFile(
				pixel_shader.c_str(), "main"));

		if (!m_shader) throw std::exception("terrain: failed to load shader program");

		m_height_map_texture = texture::load_texture_from_bmp(get_executable_file_directory() + "\\bitmaps\\heightmap.bmp");
		if (!m_height_map_texture) throw("terrain: failed to load height map texture");

		m_shader->GetUniform("height_map")->SetValue(m_height_map_texture);

		texture::set_texture_safely(m_shader, "transmittance_texture", m_bruneton.get_transmittance_texture());
		texture::set_texture_safely(m_shader, "scattering_texture", m_bruneton.get_scattering_texture());
		texture::set_texture_safely(m_shader, "irradiance_texture", m_bruneton.get_irradiance_texture());
	}

	void terrain_quad::update(float dt)
	{
	}

	void terrain_quad::render(IvRenderer & renderer)
	{
		assert("terrain_quad::render() is not supported");
	}

	terrain_quad::~terrain_quad()
	{
	}

	struct LevelDesc
	{
		int level;
		double area_size;
	};

	void get_map_lon_lat_form_viewer_position(const IvVector3& sphere_center, double sphere_radius, const IvVector3& viewer,
		double& lon, double& lat, IvDoubleVector3& hit_point)
	{
		auto ray_direction = sphere_center - viewer;
		ray_direction.Normalize();

		double distance; IvDoubleVector3 normal;
		Math::intersect_ray_sphere(viewer, ray_direction, sphere_center, sphere_radius, hit_point, distance, normal);

		Math::get_lon_lat_from_point_on_sphere(sphere_center, sphere_radius, hit_point, lon, lat);

		auto& info = debug_info::get_debug_info();
		info.set_debug_string(L"lon", (float)lon);
		info.set_debug_string(L"lat", (float)lat);
	}

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

	void terrain_quad::render(IvRenderer & renderer, const frustum& frustum)
	{
		renderer.SetBlendFunc(kOneBlendFunc, kZeroBlendFunc, kAddBlendOp);

		auto planet_center_relative_to_viewer = m_planet_center - m_viewer_position;
		auto height = abs(planet_center_relative_to_viewer.Length() - m_planet_radius);

		auto level_desc = get_level_from_distance((double)height, m_qtrees[0].width(), c_detail_levels);
		auto& info = debug_info::get_debug_info();
		info.set_debug_string(L"lod_level", (float)level_desc.level);

		m_shader->GetUniform("planet_center")->SetValue(planet_center_relative_to_viewer, 0);
		m_shader->GetUniform("planet_radius")->SetValue((float)m_planet_radius, 0);

		// global hit point for debug box
		double lon, lat; IvDoubleVector3 hit_point;
		get_map_lon_lat_form_viewer_position(m_planet_center, m_planet_radius, m_viewer_position, lon, lat, hit_point);
		m_box.set_position(hit_point);
		m_box.set_scale(1.0f);
		m_box.render(renderer);

		m_nodes_rendered_per_frame = 0;

		// Render all 6 cube faces
		for (int face = 0; face < c_face_count; ++face)
		{
			auto& qtree = m_qtrees[face];
			qtree.collapse();

			Math::CubeFace cf = static_cast<Math::CubeFace>(face);
			double map_x, map_y;
			bool ok = Math::world_to_cube_face(hit_point, m_planet_center, m_planet_radius, cf, map_x, map_y);
			// For faces not containing hit point, map_x/map_y will be outside [-R,R]; still use it as center.
			// If world_to_cube_face chose different face, we need to recompute for this face by projecting hit_point
			// onto this face via rotation. Instead directly transform hit_point to this face's local space.
			// So recompute locally:
			{
				IvDoubleVector3 d = hit_point - m_planet_center;
				IvDoubleVector3 d_local = Math::rotate_face_to_top(d, cf);
				double len = d_local.Length();
				if (len > 1e-9) {
					double cLon = atan2(d_local.x, d_local.y);
					double cLat = asin(d_local.z / len);
					Math::adjusted_sphere_to_cube(cLon, cLat, m_planet_radius, map_x, map_y);
				} else {
					map_x = 0; map_y = 0;
				}
			}

			circle c{ { map_x, map_y }, level_desc.area_size * 1.2 };
			qtree.divide(c, level_desc.level);
			qtree.divide(c * 2, level_desc.level - 1);
			qtree.divide(c * 4, level_desc.level - 2);
			qtree.divide(c * 8, level_desc.level - 3);
			qtree.divide(c * 32, level_desc.level - 4);

			if (face == (int)cf) {
				info.set_debug_string(L"map_x", (float)map_x);
				info.set_debug_string(L"map_y", (float)map_y);
			}

			double cull_radius = height < 1000.0f ? m_planet_radius / 4 : height * 32;
			circle cull{ { map_x, map_y }, cull_radius };

			RenderContext render_context{ renderer, frustum, face };
			qtree.visit(cull, *this, &terrain_quad::render_node, &render_context);
		}

		info.set_debug_string(L"rendered_nodes", (float)m_nodes_rendered_per_frame);
	}

	inline void terrain_quad::calculate_sphere_surface_quad(
		int face,
		const quad& quad, 
		IvDoubleVector3& A, 
		IvDoubleVector3& B, 
		IvDoubleVector3& C, 
		IvDoubleVector3& D,
		IvDoubleVector3& quad_center_lerped,
		IvDoubleVector3& quad_center_on_sphere,
		double& overlapping_area)
	{
		IvDoubleVector3 normal;
		Math::CubeFace cf = static_cast<Math::CubeFace>(face);
		overlapping_area = (quad.width() / c_gird_cells) * (m_overlapping_edge_cells / 2.0f);
		A = Math::adjusted_cube_to_sphere_face(cf,
			quad.center.x - quad.half_size.x - overlapping_area, 
			quad.center.y + quad.half_size.y + overlapping_area, 
			m_planet_radius, m_planet_center, normal);
		B = Math::adjusted_cube_to_sphere_face(cf,
			quad.center.x + quad.half_size.x + overlapping_area, 
			quad.center.y + quad.half_size.y + overlapping_area, 
			m_planet_radius, m_planet_center, normal);
		C = Math::adjusted_cube_to_sphere_face(cf,
			quad.center.x + quad.half_size.x + overlapping_area, 
			quad.center.y - quad.half_size.y - overlapping_area, 
			m_planet_radius, m_planet_center, normal);
		D = Math::adjusted_cube_to_sphere_face(cf,
			quad.center.x - quad.half_size.x - overlapping_area, 
			quad.center.y - quad.half_size.y - overlapping_area, 
			m_planet_radius, m_planet_center, normal);
		quad_center_lerped = Math::quad_lerp(A, B, C, D, 0.5, 0.5);
		quad_center_on_sphere = Math::adjusted_cube_to_sphere_face(cf, quad.center.x, quad.center.y, m_planet_radius, m_planet_center, normal);
	}

	void terrain_quad::render_node(const terrain_quad_tree::Node& node, void* render_context_ptr)
	{
		assert(render_context_ptr != nullptr);
		const RenderContext& render_context = *reinterpret_cast<RenderContext*>(render_context_ptr);

		auto& quad = node.get_centred_quad();

		double overlapping_area;
		IvDoubleVector3 A, B, C, D, quad_center_lerped, quad_center_on_sphere;
		calculate_sphere_surface_quad(render_context.face, quad, A, B, C, D, quad_center_lerped, quad_center_on_sphere, overlapping_area);

		AABB aabb;
		aabb.set_points(&A, &B, &C, &D, &quad_center_lerped, &quad_center_on_sphere);
		auto aabb_center = aabb.get_center();
		auto aabb_extents = aabb.get_extents();

		if (!render_context.frustum.contains_aligned_bounding_box(
			(float)aabb_center.x, (float)aabb_center.y, (float)aabb_center.z,
			(float)aabb_extents.x, (float)aabb_extents.y, (float)aabb_extents.z))
		{
			return;
		}

		m_shader->GetUniform("quad_a")->SetValue((IvVector3)A - m_viewer_position, 0);
		m_shader->GetUniform("quad_b")->SetValue((IvVector3)B - m_viewer_position, 0);
		m_shader->GetUniform("quad_c")->SetValue((IvVector3)C - m_viewer_position, 0);
		m_shader->GetUniform("quad_d")->SetValue((IvVector3)D - m_viewer_position, 0);
		m_shader->GetUniform("quad_center")->SetValue(IvVector3{ (float)quad.center.x, (float)quad.center.y, 0.0f}, 0);
		m_shader->GetUniform("cube_face")->SetValue((float)render_context.face, 0);

		m_grid.set_position(quad_center_on_sphere);
		m_grid.set_direction({ 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });
		m_shader->GetUniform("rotation_matrix")->SetValue(m_grid.get_rotation(), 0);

		m_shader->GetUniform("gird_cells")->SetValue((float)m_grid.cols(), 0);
		m_shader->GetUniform("quad_size")->SetValue(IvVector3{ (float)(quad.width() + overlapping_area), (float)(quad.width() + overlapping_area), 0.0f }, 0);
		m_shader->GetUniform("quad_scale_factor")->SetValue(20.0f, 0);

		auto detail_level = node.get_depth() - 1;
		if (detail_level > 6)
		{
			m_shader->GetUniform("curvature")->SetValue((float)0.0f, 0);
		}
		else
		{
			m_shader->GetUniform("curvature")->SetValue((float)1.0f, 0);
		}

		m_grid.render(render_context.renderer, m_shader);

		++m_nodes_rendered_per_frame;
	}

	void terrain_quad::set_viewer(const IvVector3 & camera_position)
	{
		m_viewer_position = camera_position;
	}
}
