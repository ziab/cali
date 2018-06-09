#pragma once
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
#include "Icosahedron.h"

//-------------------------------------------------------------------------------
//-- Classes --------------------------------------------------------------------
//-------------------------------------------------------------------------------
namespace cali
{
	class terrain_icosahedron : public renderable
	{
		icosahedron m_icosahedron;
		IvVector3 m_viewer_position;
		const IvVector3 m_planet_center;
		const float m_planet_radius;

		IvShaderProgram* m_shader;
		IvTexture* m_height_map_texture;

	public:
		virtual void update(float dt) override;
		virtual void render(IvRenderer& renderer) override;
		void set_viewer(const IvVector3 & camera_position);

		terrain_icosahedron();
		~terrain_icosahedron();
	};
}