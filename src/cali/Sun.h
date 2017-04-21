#pragma once
#include "Renderable.h"
#include "Model.h"
#include "ConstantBufferWrapper.h"
#include "ConstantBuffer.h"

namespace Cali
{
	class Sun : public Physical, public Renderable
	{
		Model<kNPFormat, IvNPVertex> m_sun_rect;
		IvShaderProgram* m_shader;

	private:
		void create_sun_rect(const IvVector3& size);

	public:
		Sun();
		~Sun();

		virtual void update(float dt) override;
		virtual void render(IvRenderer& renderer) override;

		void update_global_state(ConstantBufferWrapper<ConstantBuffer::GlobalState>& global_state);
	};
}