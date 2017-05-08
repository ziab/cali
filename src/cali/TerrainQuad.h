#pragma once

//-------------------------------------------------------------------------------
//-- Dependencies ---------------------------------------------------------------
//-------------------------------------------------------------------------------

#include <IvMatrix33.h>
#include <IvVector2.h>
#include <IvVector3.h>
#include <IvVector4.h>
#include <IvVertexShader.h>
#include <IvVertexBuffer.h>
#include <IvIndexBuffer.h>
#include <IvShaderProgram.h>
#include <IvRenderer.h>

#include <memory>

#include "Renderable.h"
#include "Model.h"
#include "Grid.h"
#include "TerrainQuadTree.h"
#include "AABB.h"
#include "Frustum.h"

//-------------------------------------------------------------------------------
//-- Classes --------------------------------------------------------------------
//-------------------------------------------------------------------------------

namespace Cali
{
	class TerrainQuad : public Renderable, public CompoundRenderable
	{
		TerrainQuadTree m_qtree;
		AABB m_aabb;
		Grid m_grid;
		IvVector3 m_viewer_position;
		const float m_overlapping_edge_cells;
		const IvVector3 m_planet_center;
		const float m_planet_radius;

		IvShaderProgram* m_shader;
		IvTexture* m_height_map_texture;

		static const uint32_t c_gird_dimention = 129;

	public:
		virtual void update(float dt) override;
		virtual void render(IvRenderer & renderer) override;
		virtual void render(IvRenderer & renderer, const Frustum& frustum) override;
		void set_viewer(const IvVector3 & camera_position);

		TerrainQuad();
		~TerrainQuad();
	};
}