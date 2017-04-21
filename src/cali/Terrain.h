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
namespace Cali
{
	static const uint32_t c_hd_gird_dimention = 256;

	class AABB : public Physical, public Renderable
	{
		Model<kNPFormat, IvNPVertex> m_box;
		IvShaderProgram* m_shader;

	public:
		AABB();

		virtual void update(float dt) override;
		virtual void render(IvRenderer& renderer) override;
	};

	class Terrain : public Renderable
	{
		Grid m_grid;
		Grid m_hd_grid;
		Grid m_ld_grid;
		IvVector3 m_planet_center;
		float m_planet_radius;
		IvVector3 m_viewer_position;

		IvShaderProgram* m_shader;
		IvTexture* m_height_map_texture;

	private:
		void read_height_map(const std::string & path, BufferRAIIWrapper<IvVertexBuffer, IvNPVertex>& vertices, size_t width, size_t height);
		IvTexture* load_height_map_texture(const std::string & path);
		void render_level(IvRenderer & renderer, Grid & level_grid, float scale, const IvVector3& position);
		void render_levels(IvRenderer& renderer, size_t level, size_t max_level);

	public:
		virtual void update(float dt) override;
		virtual void render(IvRenderer& renderer) override;
		void set_viewer(const IvVector3 & camera_position);

		Terrain();
		~Terrain();
	};
}