#pragma once
#include "Renderable.h"
#include "Model.h"
#include "ConstantBufferWrapper.h"
#include "ConstantBuffer.h"

namespace cali
{
	class sun : public physical, public renderable
	{
		model<kNPFormat, IvNPVertex> m_sun_rect;
		IvShaderProgram* m_shader;

	private:
		void create_sun_rect(const IvVector3& size);

	public:
		sun();
		~sun();

		virtual void update(float dt) override;
		virtual void render(IvRenderer& renderer) override;

		void update_global_state(constant_buffer_wrapper<constant_buffer::GlobalState>& global_state);
	};
}