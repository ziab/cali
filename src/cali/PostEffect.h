#pragma once
#include <IvRenderer.h>
#include <IvShaderProgram.h>
#include <IvRenderTexture.h>
#include <IvResourceManager.h>

#include "Model.h"


namespace cali
{
	class effect
	{
	public:
		virtual void apply(IvRenderer& renderer, IvRenderTexture& target) = 0;
		virtual ~effect() {};
	};

	class PostEffect : effect
	{
		IvShaderProgram* m_shader_program;
		cali::model<kTNPFormat, IvTNPVertex> m_target_quad;

	public:
		PostEffect();
		virtual ~PostEffect()
		{
			
		}

		virtual void apply(IvRenderer& renderer, IvRenderTexture& target);
	};
}
