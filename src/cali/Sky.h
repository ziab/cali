#pragma once
#include "Renderable.h"
#include "Model.h"
#include "IvRenderTexture.h"
#include "Bruneton.h"

#include <memory>

namespace cali
{
	class sky : public physical
	{
		bruneton& m_bruneton;

		model<kTNPFormat, IvTNPVertex> m_sky_box;
		IvShaderProgram* m_sky_shader;

	private:
		void create_sky_box(const IvVector3& size);

	public:
		sky(bruneton& bruneton);
		~sky();

		virtual void update(float dt);
		virtual void render(IvRenderer& renderer);
	};
}