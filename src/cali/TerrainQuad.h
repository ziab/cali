#pragma once

//-------------------------------------------------------------------------------
//-- Dependencies ---------------------------------------------------------------
//-------------------------------------------------------------------------------

#include <IvMatrix33.h>
#include <IvVector2.h>
#include <IvVector3.h>
#include <IvDoubleVector3.h>
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
		const IvDoubleVector3 m_planet_center;
		const double m_planet_radius;

		IvShaderProgram* m_shader;
		IvTexture* m_height_map_texture;

		size_t m_nodes_rendered_per_frame;

		static const uint32_t c_gird_dimention = 129;

		struct RenderContext
		{
			IvRenderer& renderer;
			const Frustum& frustum;
		};

	public:
		// Renderable
		virtual void update(float dt) override;
		virtual void render(IvRenderer & renderer) override;

		// CompoundRenderable
		virtual void render(IvRenderer & renderer, const Frustum& frustum) override;

		void render_node(const struct TerrainQuadTree::Node& node, void* render_context);
		void set_viewer(const IvVector3 & camera_position);

		TerrainQuad();
		~TerrainQuad();
	};
}