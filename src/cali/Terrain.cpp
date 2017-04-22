#include "Terrain.h"

#include <IvUniform.h>
#include <IvResourceManager.h>
#include <IvTexture.h>
#include "CommonFileSystem.h"

#pragma warning(disable : 4996)
#include "..\..\depends\bitmap_image\bitmap_image.hpp"
#pragma warning(default : 4996)

#include <vector>

namespace Cali
{
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
		m_overlapping_edge_quads(5.0f)
	{
		std::string vertex_shader = construct_shader_path("terrain.hlslv");
		std::string pixel_shader = construct_shader_path("terrain.hlslf");

		m_shader = IvRenderer::mRenderer->GetResourceManager()->CreateShaderProgram(
			IvRenderer::mRenderer->GetResourceManager()->CreateVertexShaderFromFile(
				vertex_shader.c_str(), "main"),
			IvRenderer::mRenderer->GetResourceManager()->CreateFragmentShaderFromFile(
				pixel_shader.c_str(), "main"));

		if (!m_shader) throw std::exception("Terrain: failed to load shader program");

		m_height_map_texture = load_height_map_texture(get_executable_file_directory() + "\\bitmaps\\heightmap.bmp");
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

	void Terrain::read_height_map(
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

	IvTexture* Terrain::load_height_map_texture(const std::string & path)
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


		level_grid.set_transformation_matrix(renderer);
		level_grid.render(renderer, m_shader);
	}

	void Terrain::render_levels(
		IvRenderer& renderer,
		size_t level,
		size_t max_level,
		float offset_from_viewer,
		float grid_scale_factor,
		float grid_uv_scale_factor)
	{
		if (level > max_level) return;

		if (level == 0)
		{
			render_level(renderer, m_hd_grid, { 0.0f, 0.0f, 0.0f }, grid_scale_factor, grid_uv_scale_factor);
			
			render_levels(
				renderer, 
				level + 1, 
				max_level, 
				(m_hd_grid.width() - m_hd_grid.stride() * m_overlapping_edge_quads) * grid_scale_factor,
				grid_scale_factor, 
				grid_uv_scale_factor);
		}
		else
		{
			render_level(renderer, m_ld_grid, { 0.0,                 0.0, offset_from_viewer  }, grid_scale_factor, grid_uv_scale_factor);
			render_level(renderer, m_ld_grid, { offset_from_viewer,  0.0, offset_from_viewer  }, grid_scale_factor, grid_uv_scale_factor);
			render_level(renderer, m_ld_grid, { offset_from_viewer,  0.0, 0.0                 }, grid_scale_factor, grid_uv_scale_factor);
			render_level(renderer, m_ld_grid, { offset_from_viewer,  0.0, -offset_from_viewer }, grid_scale_factor, grid_uv_scale_factor);
			render_level(renderer, m_ld_grid, { 0.0,                 0.0, -offset_from_viewer }, grid_scale_factor, grid_uv_scale_factor);
			render_level(renderer, m_ld_grid, { -offset_from_viewer, 0.0, -offset_from_viewer }, grid_scale_factor, grid_uv_scale_factor);
			render_level(renderer, m_ld_grid, { -offset_from_viewer, 0.0, 0.0                 }, grid_scale_factor, grid_uv_scale_factor);
			render_level(renderer, m_ld_grid, { -offset_from_viewer, 0.0, offset_from_viewer  }, grid_scale_factor, grid_uv_scale_factor);

			render_levels(
				renderer, 
				level + 1, 
				max_level, 
				offset_from_viewer + (m_ld_grid.width() - m_ld_grid.stride() * m_overlapping_edge_quads) * grid_scale_factor * 2.0f,
				grid_scale_factor * 3.0f, 
				grid_uv_scale_factor * 3.0f);
		}
	}

	void Terrain::render(IvRenderer& renderer)
	{
		render_levels(renderer, 0, 5, 0.0f, 1.0f, 0.15f);
	}

	AABB::AABB()
	{
		std::string vertex_shader = construct_shader_path("simple.hlslv");
		std::string pixel_shader = construct_shader_path("simple.hlslf");

		m_shader = IvRenderer::mRenderer->GetResourceManager()->CreateShaderProgram(
			IvRenderer::mRenderer->GetResourceManager()->CreateVertexShaderFromFile(
				vertex_shader.c_str(), "main"),
			IvRenderer::mRenderer->GetResourceManager()->CreateFragmentShaderFromFile(
				pixel_shader.c_str(), "main"));

		if (!m_shader) throw std::exception("Terrain: failed to load shader program");

		create_box(m_box, { 1.0, 1.0, 1.0 }, true, false);
	}

	void AABB::update(float dt)
	{
	}

	void AABB::render(IvRenderer & renderer)
	{
		Physical::set_transformation_matrix(renderer);
		m_box.render(renderer, m_shader);
	}
}