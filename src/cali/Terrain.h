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

//-------------------------------------------------------------------------------
//-- Classes --------------------------------------------------------------------
//-------------------------------------------------------------------------------
namespace cali
{
	static const uint32_t c_hd_gird_dimention = 256;

	class terrain : public renderable
	{
		grid m_hd_grid;
		grid m_ld_grid;
		IvVector3 m_viewer_position;
		const float m_overlapping_edge_cells;
		const IvVector3 m_planet_center;
		const float m_planet_radius;

		IvShaderProgram* m_shader;
		IvTexture* m_height_map_texture;

	private:
		void render_level(
			IvRenderer & renderer, 
			grid & level_grid, 
			const IvVector3& offset, 
			float scale, 
			float grid_scale_factor);

		void render_levels(
			IvRenderer& renderer, 
			grid& level_grid,
			size_t level, 
			size_t max_level, 
			float offset_from_viewer, 
			float grid_scale_factor,
			float grid_uv_scale_factor);

		struct RenderLevelParamerters
		{
			grid* initial_level_grid;
			size_t max_level;
			float initial_scale;
			IvVector3 initial_position;
			float curvature;
		};

		RenderLevelParamerters calculate_render_level_parameters(
			IvRenderer& renderer,
			const IvVector3& position, 
			const IvVector3& planet_center, 
			double radius);

	public:
		virtual void update(float dt) override;
		virtual void render(IvRenderer& renderer) override;
		void set_viewer(const IvVector3 & camera_position);

		terrain();
		~terrain();
	};
}