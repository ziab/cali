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

//-------------------------------------------------------------------------------
//-- Classes --------------------------------------------------------------------
//-------------------------------------------------------------------------------
namespace Cali
{
	class Terrain : public Physical
	{
		uint32_t m_width;
		uint32_t m_height;

		Model<kTNPFormat, IvTNPVertex> m_terrain;
		IvShaderProgram* m_shader;
		IvTexture* m_height_map_texture;

	private:
		void read_height_map(const std::string & path, BufferRAIIWrapper<IvVertexBuffer, IvNPVertex>& vertices, size_t width, size_t height);
		void create_plain(int32_t width, int32_t height, float stride);
		IvTexture* load_height_map_texture(const std::string & path);

	public:
		virtual void update(float dt) override;
		virtual void render(IvRenderer& renderer) override;

		Terrain();
		~Terrain();
	};
}