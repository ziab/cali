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

	void Terrain::render(IvRenderer& renderer)
	{
		float gird_width = m_grid.width();
		float gird_height = m_grid.height();

		render_level(renderer, m_grid, 1.0f, m_viewer_position);
		render_level(renderer, m_grid, 1.0f, m_viewer_position + IvVector3{ 0.0, 0.0, gird_height });
		render_level(renderer, m_grid, 1.0f, m_viewer_position + IvVector3{ 0.0, 0.0, -gird_height });
		render_level(renderer, m_grid, 1.0f, m_viewer_position + IvVector3{ gird_width, 0.0, 0.0 });
		render_level(renderer, m_grid, 1.0f, m_viewer_position + IvVector3{ -gird_width, 0.0, 0.0 });
	}

	void Terrain::set_viewer(const IvVector3 & position)
	{
		m_viewer_position = position;
	}

	Terrain::Terrain() :
		m_grid(1000, 1000, 2.0f),
		m_hd_grid(c_hd_gird_dimention, c_hd_gird_dimention, 1.0f),
		m_ld_grid(c_hd_gird_dimention / 2, c_hd_gird_dimention / 2, 2.0f)
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

	void Terrain::render_level(IvRenderer & renderer, Grid & level_grid, float scale, const IvVector3& position)
	{
		level_grid.set_current_origin(position, { scale, 1.0f, scale } );

		m_shader->GetUniform("modelMatrix")->SetValue(level_grid.get_transformation_matrix(), 0);
		m_shader->GetUniform("grid_stride")->SetValue(level_grid.stride(), 0);
		m_shader->GetUniform("grid_cols")->SetValue((float)level_grid.cols(), 0);
		m_shader->GetUniform("grid_rows")->SetValue((float)level_grid.rows(), 0);

		level_grid.set_transformation_matrix(renderer);
		level_grid.render(renderer, m_shader);
	}

	void Terrain::render_levels(IvRenderer & renderer, size_t level, size_t max_level)
	{
		if (level == 1)
		{
		}
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