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
#include "Box.h"
#include "Frustum.h"
#include "Bruneton.h"

//-------------------------------------------------------------------------------
//-- Classes --------------------------------------------------------------------
//-------------------------------------------------------------------------------

namespace Cali
{
	class TerrainQuad : public Renderable, public CompoundRenderable
	{
		TerrainQuadTree m_qtree;
		Box m_box;
		Grid m_grid;
		Bruneton& m_bruneton;
		IvVector3 m_viewer_position;
		const float m_overlapping_edge_cells;
		const IvDoubleVector3 m_planet_center;
		const double m_planet_radius;

		IvShaderProgram* m_shader;
		IvTexture* m_height_map_texture;

		size_t m_nodes_rendered_per_frame;

		static const uint32_t c_gird_cells = 129;
		static const uint32_t c_detail_levels = 22;

		std::vector<IvRenderTexture*> m_quad_data_textures;

		struct RenderContext
		{
			IvRenderer& renderer;
			const Frustum& frustum;
		};

		void calculate_sphere_surface_quad(
			const Quad & quad,
			IvDoubleVector3 & A,
			IvDoubleVector3 & B,
			IvDoubleVector3 & C,
			IvDoubleVector3 & D,
			IvDoubleVector3 & quad_center_lerped,
			IvDoubleVector3 & quad_center_on_sphere,
			double & overlapping_area);

		void calculate_displacement_data(const Cali::Quad & quad, int level, void* quad_data_texture);
		void calculate_displacement_data_for_detail_levels();

		void render_node(const struct TerrainQuadTree::Node& node, void* render_context);

	public:
		// Renderable
		virtual void update(float dt) override;
		virtual void render(IvRenderer & renderer) override;

		// CompoundRenderable
		virtual void render(IvRenderer & renderer, const Frustum& frustum) override;
		void set_viewer(const IvVector3 & camera_position);

		TerrainQuad(Bruneton& bruneton);
		~TerrainQuad();
	};
}