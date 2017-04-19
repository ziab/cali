#pragma once
#include "Renderable.h"
#include "Model.h"
#include "IvRenderTexture.h"
#include "Bruneton.h"

#include <memory>

namespace Cali
{
	class Sky : public Physical
	{
		Bruneton m_bruneton;

		Model<kNPFormat, IvNPVertex> m_sky_box;
		IvShaderProgram* m_sky_shader;

	private:
		void create_sky_box(const IvVector3& size);

	public:
		Sky();
		~Sky();

		virtual void update(float dt);
		virtual void render(IvRenderer& renderer);
	};
}