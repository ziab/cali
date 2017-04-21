#include "Grid.h"

namespace Cali
{
	void Grid::create_grid(int32_t cols, int32_t rows, float stride)
	{
		if (cols < 1 || rows < 1) return;

		m_cols = cols;
		m_rows = rows;
		m_stride = stride;

		const size_t indices_total = (cols - 1) * (rows - 1) * 6;
		const size_t vertex_total = cols * rows;

		m_model.allocate(indices_total, vertex_total);
		auto& indices = m_model.load_indicies();
		auto& vertices = m_model.load_vertices();

		size_t i = 0;

		// TODO: make the ordering cache-friendly

		float u_stride = 1.f / (float)cols;
		float v_stride = 1.f / (float)rows;
		float u = 0.f, v = 0.f;
		for (int32_t y = -rows / 2; y < rows / 2 + rows % 2; ++y)
		{
			float u = 0.f;
			for (int32_t x = -cols / 2; x < cols / 2 + cols % 2; ++x)
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
		for (UInt32 r = 0; r < (UInt32)(cols - 1); ++r)
		{
			for (UInt32 c = 0; c < (UInt32)(rows - 1); ++c)
			{

				/*
				1 - 2 - 5
				| / | / |
				3 - 4 - 6

				0  1  2    4  5  6    7  8  9
				{1, 2, 3}, {3, 2, 4}, {2, 5, 4}, {3, 5, 6}
				*/

				indices[i++] = r * rows + c;             // 1
				indices[i++] = r * rows + (c + 1);       // 2
				indices[i++] = (r + 1) * rows + c;       // 3

				indices[i++] = (r + 1) * rows + c;       // 3
				indices[i++] = r * rows + (c + 1);       // 2
				indices[i++] = (r + 1) * rows + (c + 1); // 4
			}
		}
	}

	void Grid::render(IvRenderer & renderer, IvShaderProgram * shader) const
	{
		m_model.render(renderer, shader);
	}

	void Grid::set_current_origin(const IvVector3 & origin, const IvVector3& scale)
	{
		float total_scale_x = m_stride * scale.x;
		float total_scale_z = m_stride * scale.z;

		IvVector2 grid_coordinates = {
			floor(origin.x / total_scale_x) * total_scale_x,
			floor(origin.z / total_scale_z) * total_scale_z
		};

		set_position({ grid_coordinates.x, get_position().y, grid_coordinates.y });
		set_scale(scale);
	}
}