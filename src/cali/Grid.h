#pragma once

#include <vector>
#include <IvMatrix33.h>
#include <IvVector2.h>
#include <IvVector3.h>
#include <IvVector4.h>
#include <IvVertexShader.h>
#include <IvVertexBuffer.h>
#include <IvIndexBuffer.h>
#include <IvShaderProgram.h>
#include <IvRenderer.h>

#include "Model.h"

namespace Cali
{
	class Grid : public Physical
	{
		uint32_t m_cols;
		uint32_t m_rows;
		float m_stride;

		Model<kTNPFormat, IvTNPVertex> m_model;

	private:
		void create_grid(int32_t cols, int32_t rows, float stride);
	public:

		Grid(uint32_t width, uint32_t height, float stride)
		{
			create_grid(width, height, stride);
		}

		~Grid() {}

		uint32_t cols() { return m_cols; }
		uint32_t rows() { return m_rows; }
		float width() { return m_stride * m_cols; }
		float height() { return m_stride * m_rows; }
		float stride() { return m_stride; }
		Model<kTNPFormat, IvTNPVertex>& get_model() { return m_model; }

		void set_current_origin(const IvVector3 & camera_position, const IvVector3& scale);

		void render(IvRenderer& renderer, IvShaderProgram* shader) const;
	};
}