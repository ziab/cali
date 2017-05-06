#pragma once

//-------------------------------------------------------------------------------
//-- Dependencies ---------------------------------------------------------------
//-------------------------------------------------------------------------------

#include <IvVertexShader.h>
#include <IvVertexBuffer.h>
#include <IvIndexBuffer.h>
#include <IvShaderProgram.h>
#include <IvRenderer.h>

#include "Renderable.h"
#include "Model.h"

//-------------------------------------------------------------------------------
//-- Classes --------------------------------------------------------------------
//-------------------------------------------------------------------------------

namespace Cali
{
	class AABB : public Physical, public Renderable
	{
		Model<kNPFormat, IvNPVertex> m_box;
		IvShaderProgram* m_shader;

	public:
		AABB();

		virtual void update(float dt) override;
		virtual void render(IvRenderer& renderer) override;
	};
}
