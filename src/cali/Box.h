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

namespace cali
{
	class Box : public physical, public renderable
	{
		model<kTNPFormat, IvTNPVertex> m_box;
		IvShaderProgram* m_shader;

	public:
		Box();

		virtual void update(float dt) override;
		virtual void render(IvRenderer& renderer) override;
	};
}
