#pragma once
#include <IvRenderer.h>
#include <IvShaderProgram.h>
#include <IvRenderTexture.h>
#include <IvResourceManager.h>

#include "Model.h"


namespace Cali
{
	class Effect
	{
	public:
		virtual void apply(IvRenderer& renderer, IvRenderTexture& target) = 0;
		virtual ~Effect() {};
	};

	class PostEffect : Effect
	{
		IvShaderProgram* m_shader_program;
		Cali::Model<kTNPFormat, IvTNPVertex> m_target_quad;

	public:
		PostEffect();
		virtual ~PostEffect()
		{
			
		}

		virtual void apply(IvRenderer& renderer, IvRenderTexture& target);
	};
}
