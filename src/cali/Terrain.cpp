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
		Physical::set_transformation_matrix(renderer);
		m_shader->GetUniform("modelMatrix")->SetValue(Physical::get_transformation_matrix(), 0);
		m_shader->GetUniform("grid_stride")->SetValue(m_grid_stride, 0);
		m_shader->GetUniform("grid_width")->SetValue((float)m_width, 0);
		m_shader->GetUniform("grid_height")->SetValue((float)m_height, 0);
		m_terrain.render(renderer, m_shader);

		auto current_origin = get_position();

		set_current_origin(current_origin + IvVector3{0.0, 0.0, (float)m_height * m_grid_stride - m_grid_stride });
		Physical::set_transformation_matrix(renderer);
		m_terrain.render(renderer, m_shader);

		set_current_origin(current_origin + IvVector3{ 0.0, 0.0, -(float)m_height * m_grid_stride + m_grid_stride });
		Physical::set_transformation_matrix(renderer);
		m_terrain.render(renderer, m_shader);

		set_current_origin(current_origin + IvVector3{ (float)m_height * m_grid_stride - m_grid_stride, 0.0, 0.0 });
		Physical::set_transformation_matrix(renderer);
		m_terrain.render(renderer, m_shader);

		set_current_origin(current_origin + IvVector3{ -(float)m_height * m_grid_stride + m_grid_stride, 0.0, 0.0 });
		Physical::set_transformation_matrix(renderer);
		m_terrain.render(renderer, m_shader);

		render_quad_nodes(renderer);
	}

	void Terrain::set_current_origin(const IvVector3 & camera_position)
	{
		IvVector2 grid_coordinates = { 
			floor(camera_position.x / m_grid_stride) * m_grid_stride,
			floor(camera_position.z / m_grid_stride) * m_grid_stride
		};

		set_position({ grid_coordinates.x, get_position().y, grid_coordinates.y });
	}

	Terrain::Terrain() : 
		m_tqtree({ { 0.0, 0.0 }, { 1000.0, 1000.0 } })
	{
		std::string vertex_shader = construct_shader_path("terrain.hlslv");
		std::string pixel_shader = construct_shader_path("terrain.hlslf");

		m_shader = IvRenderer::mRenderer->GetResourceManager()->CreateShaderProgram(
			IvRenderer::mRenderer->GetResourceManager()->CreateVertexShaderFromFile(
				vertex_shader.c_str(), "main"),
			IvRenderer::mRenderer->GetResourceManager()->CreateFragmentShaderFromFile(
				pixel_shader.c_str(), "main"));

		if (!m_shader) throw std::exception("Terrain: failed to load shader program");

		m_grid_stride = 2.0f;
		create_plain(1000, 1000, m_grid_stride);

		m_height_map_texture = load_height_map_texture(get_executable_file_directory() + "\\bitmaps\\heightmap.bmp");
		if (!m_height_map_texture) throw("Terrain: failed to load height map texture");

		m_shader->GetUniform("height_map")->SetValue(m_height_map_texture);

		m_tqtree.divide({ 100.0, 100.0 }, 3);
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

	void Terrain::create_plain(int32_t width, int32_t height, float stride)
	{
		if (width < 1 || height < 1) return;

		m_width = width;
		m_height = height;

		const size_t indices_total = (width - 1) * (height - 1) * 6;
		const size_t vertex_total = width * height;

		m_terrain.allocate(indices_total, vertex_total);
		auto& indices = m_terrain.load_indicies();
		auto& vertices = m_terrain.load_vertices();

		size_t i = 0;

		// TODO: make the ordering cache-friendly

		float u_stride = 1.f / (float)width;
		float v_stride = 1.f / (float)height;
		float u = 0.f, v = 0.f;
		for (int32_t y = -height / 2; y < height / 2 + height % 2; ++y)
		{
			float u = 0.f;
			for (int32_t x = -width / 2; x < width / 2 + width % 2; ++x)
			{
				auto& curr = vertices[i++];

				curr.normal = { 0.0f, 1.0f, 0.0f };
				//curr.color = { 0, 255, 0, 255 };
				curr.position.x = (float)x * stride;
				curr.position.y = 0.f;
				curr.position.z = (float)y * stride;
				curr.texturecoord = { u, v };
				u += u_stride;
			}
			v += v_stride;
		}

		i = 0;
		for (UInt32 r = 0; r < (UInt32)(width - 1); ++r)
		{
			for (UInt32 c = 0; c < (UInt32)(height - 1); ++c)
			{

				/*
				1 - 2 - 5
				| / | / |
				3 - 4 - 6

				 0  1  2    4  5  6    7  8  9
				{1, 2, 3}, {3, 2, 4}, {2, 5, 4}, {3, 5, 6}
				*/

				indices[i++] = r * height + c;             // 1
				indices[i++] = r * height + (c + 1);       // 2
				indices[i++] = (r + 1) * height + c;       // 3

				indices[i++] = (r + 1) * height + c;       // 3
				indices[i++] = r * height + (c + 1);       // 2
				indices[i++] = (r + 1) * height + (c + 1); // 4
			}
		}


		///////////////////////////////////////////////////////////////////

		//read_height_map(get_executable_file_directory() + "\\bitmaps\\heightmap.bmp", vertices, width, height);

		///////////////////////////////////////////////////////////////////

		// This calculation method of normals is probably not totally correct
		// as it doesn't seem to respect adjacent polygons
		for (size_t index = 0; index < indices_total - 2;)
		{
			auto& a = vertices[indices[index++]];
			auto& b = vertices[indices[index++]];
			auto& c = vertices[indices[index++]];

			a.normal = Cross((a.position - b.position), (c.position - a.position));
			a.normal.Normalize();

			b.normal = a.normal;
			c.normal = a.normal;
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

	void Terrain::render_quad_nodes(IvRenderer & renderer)
	{
		renderer.SetFillMode(kWireframeFill);

		std::vector<const TerrainQuadTree::Node*> nodes;
		m_tqtree.query_nodes(nodes);

		for (auto* node : nodes)
		{
			auto quad = node->get_centred_quad();
			m_aabb.set_position({ (float)quad.center.x, 0.0f, (float)quad.center.y });
			m_aabb.set_scale({ (float)(quad.half_size.x * 2.0), 250.0f, (float)(quad.half_size.y * 2.0) });
			m_aabb.render(renderer);
		}

		renderer.SetFillMode(kSolidFill);
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