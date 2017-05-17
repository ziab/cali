#include "TerrainQuad.h"

#include <IvShaderProgram.h>
#include <IvTexture.h>
#include <IvUniform.h>

#include "World.h"
#include "Constants.h"
#include "CommonFileSystem.h"
#include "CommonTexture.h"

#include "DebugInfo.h"

#include "IvDoubleVector3.h"

namespace Cali
{
	TerrainQuad::TerrainQuad() :
		m_qtree({ { 0.0, 0.0 }, { World::c_earth_radius * kPI, World::c_earth_radius * kPI } }),
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

	inline double sum(const IvDoubleVector3& vec)
	{
		return vec.x + vec.y + vec.z;
	}

	//////////////////////////////////////////////////////
	// TODO: make it using double prescision
	//////////////////////////////////////////////////////
	inline bool intersect(const IvDoubleVector3& raydir, const IvDoubleVector3& rayorig, const IvDoubleVector3& spherepos,
		double rad, IvDoubleVector3& hitpoint, double& distance, IvDoubleVector3& normal)
	{
		double a = sum(raydir*raydir);
		double b = sum(raydir * (2.0 * (rayorig - spherepos)));
		double c = sum(spherepos*spherepos) + sum(rayorig*rayorig) - 2.0*sum(rayorig*spherepos) - rad*rad;
		double D = b*b + (-4.0)*a*c;

		// If ray can not intersect then stop
		if (D < 0)
			return false;
		D = sqrt(D);

		// Ray can intersect the sphere, solve the closer hitpoint
		double t = (-0.5)*(b + D) / a;
		if (t > 0.0)
		{
			distance = sqrt(a) * t;
			hitpoint = rayorig + t * raydir;
			normal = (hitpoint - spherepos) / rad;
		}
		else
		{
			return false;
		}
		
		return true;
	}

	// Intersects ray r = p + td, |d| = 1, with sphere s and, if intersecting, 
	// returns t value of intersection and intersection point q 
	int intersect_ray_sphere(const IvDoubleVector3 p, const IvDoubleVector3& d, const IvDoubleVector3& C, 
		double R, IvDoubleVector3& hit, double &t, IvDoubleVector3& normal)
	{
		IvDoubleVector3 m = p - C;
		double b = Dot(m, d);
		double c = Dot(m, m) - R * R;

		// Exit if r’s origin outside s (c > 0) and r pointing away from s (b > 0) 
		if (c > 0.0f && b > 0.0f) return 0;
		double discr = b*b - c;

		// A negative discriminant corresponds to ray missing sphere 
		if (discr < 0.0f) return 0;

		// Ray now found to intersect sphere, compute smallest t value of intersection
		t = -b - sqrt(discr);

		// If t is negative, ray started inside sphere
		if (t < 0.0f)
		{
			double a = Dot(d, d);
			t /= (2 * a);
		}

		hit = p + t * d;

		normal = hit - C / R;

		return 1;
	}

	inline void get_lon_lat_from_point_on_sphere(const IvVector3& sphere_center, double sphere_radius, 
		const IvVector3& point, double& lon, double& lat)
	{
		IvVector3 point_coord_related_to_sphere = point - sphere_center;
		// note that the origin is shifted by (kPI / 2.0)
		lat = acos(- point_coord_related_to_sphere.z / sphere_radius) - (kPI / 2.0);
		lon = atan(-point_coord_related_to_sphere.y / point_coord_related_to_sphere.x) - (kPI / 2.0);

		if (point_coord_related_to_sphere.x >= 0.0)
			lon += kPI;
	}

	void position_on_sphere(double lon, double lat, double R, const IvDoubleVector3& C, 
		IvDoubleVector3& position, IvDoubleVector3& normal, IvDoubleVector3& tangent)
	{
		double cos_lat = cos(lat);

		IvDoubleVector3 ps;
		ps.x = R * cos_lat * sin(lon);
		ps.y = R * cos_lat * cos(lon);
		ps.z = R * sin(lat);

		position = ps + C;
		normal = ps;
		normal.Normalize();

		double sin_lon = cos(lat);

		tangent = normal.Cross(IvDoubleVector3::zAxis);
		tangent.Normalize();
	}

	/// R is sphere radius
	/// C is sphere center position
	/// x,y - are coordinates on surface
	void position_on_sphere_from_surface(double x, double y, double R, const IvDoubleVector3& C, 
		IvDoubleVector3& position, IvDoubleVector3& normal, IvDoubleVector3& tangent)
	{
		double lon = x / R;
		double lat = 2 * atan(exp(y / R)) - kPI / 2.0;

		return position_on_sphere(lon, lat, R, C, position, normal, tangent);
	}

	void get_map_lon_lat_form_viewer_position(const IvVector3& sphere_center, double sphere_radius, const IvVector3& viewer,
		double& lon, double& lat, IvDoubleVector3& hit_point)
	{
		auto ray_direction = sphere_center - viewer;
		ray_direction.Normalize();

		double distance; IvDoubleVector3 normal;
		intersect_ray_sphere(viewer, ray_direction, sphere_center, sphere_radius, hit_point, distance, normal);

		get_lon_lat_from_point_on_sphere(sphere_center, sphere_radius, hit_point, lon, lat);

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

	void TerrainQuad::calculate_displacement_data(const Cali::Quad& quad, int level)
	{
		/*     -  
		   /       \
			 A - B
		  |  |   |  |
			 C - D
			       /
			   -
		*/

		double step = quad.half_size.x * 2.0 / c_gird_dimention;
		IvDoubleVector3 A, B, C;
		IvDoubleVector3 position, normal, tangent;
		double distance;
		position_on_sphere_from_surface(-quad.half_size.x, quad.half_size.y, m_planet_radius, m_planet_center, A, normal, tangent);
		position_on_sphere_from_surface(quad.half_size.x, quad.half_size.y, m_planet_radius, m_planet_center, B, normal, tangent);
		position_on_sphere_from_surface(-quad.half_size.x, -quad.half_size.y, m_planet_radius, m_planet_center, C, normal, tangent);

		IvDoubleVector3 x_step_vector = (B - A) / c_gird_dimention;
		IvDoubleVector3	y_step_vector = (A - C) / c_gird_dimention;

		IvDoubleVector3 current_vertex;
		for (int32_t y = 0; y < c_gird_dimention; ++y)
		{
			current_vertex = A + y * y_step_vector;
			for (int32_t x = 0; x < c_gird_dimention; ++x)
			{
				IvDoubleVector3 direction = current_vertex - m_planet_center;
				direction.Normalize();
				intersect_ray_sphere(current_vertex, direction, m_planet_center, m_planet_radius, position, distance, normal);

				IvVector3 displacement = position - current_vertex;

				current_vertex += x_step_vector;
			}
		}
	}

	void TerrainQuad::calculate_displacement_data_for_detail_levels()
	{
		Quad current_quad{ { 0.0, 0.0 },{ m_qtree.width() / 2.0, m_qtree.height() / 2.0 } };
		for (int i = 0; i < 22; ++i)
		{
			calculate_displacement_data(current_quad, i);
			current_quad.half_size.x /= 2;
			current_quad.half_size.y /= 2;
		}
	}

	void TerrainQuad::render_node(const TerrainQuadTree::Node& node, void* render_context_ptr)
	{
		assert(render_context_ptr != nullptr);
		const RenderContext& render_context = *reinterpret_cast<RenderContext*>(render_context_ptr);

		auto& quad = node.get_centred_quad();

		//if (!render_context.frustum.contains_aligned_bounding_box(
		//	(float)quad.center.x, 0.0f, (float)quad.center.y,
		//	(float)quad.width(), 1.0f, (float)quad.width()))
		//{
		//	return;
		//}

		IvDoubleVector3 position, normal, tangent;
		position_on_sphere_from_surface(quad.center.x, quad.center.y, m_planet_radius, m_planet_center, position, normal, tangent);

		m_grid.set_position(position);
		m_grid.set_direction(normal, {1.0, 0.0, 0.0});
		float scale = (float)quad.width() / (m_grid.width() - m_grid.stride() * m_overlapping_edge_cells);

		m_grid.set_scale(IvVector3{ scale, scale, 1.0f });
		m_shader->GetUniform("grid_stride")->SetValue(m_grid.stride() * scale, 0);
		m_shader->GetUniform("grid_cols")->SetValue((float)m_grid.cols(), 0);
		m_shader->GetUniform("grid_rows")->SetValue((float)m_grid.rows(), 0);
		m_shader->GetUniform("grid_uv_quad_size")->SetValue(
			IvVector3{ scale * 0.1f, scale * 0.1f, 0.0f },
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