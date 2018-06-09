#include "PostEffect.h"
#include "IvUniform.h"
#include "CommonFileSystem.h"

namespace cali
{
	PostEffect::PostEffect() : m_shader_program(nullptr)
	{
		std::string vertex_shader_file = construct_shader_path("bloom.hlslv");
		std::string pixel_shader_file = construct_shader_path("bloom.hlslf");

		auto* resman = IvRenderer::mRenderer->GetResourceManager();
		m_shader_program = IvRenderer::mRenderer->GetResourceManager()->CreateShaderProgram(
			IvRenderer::mRenderer->GetResourceManager()->CreateVertexShaderFromFile(
				vertex_shader_file.c_str(), "vs_bloom"),
			IvRenderer::mRenderer->GetResourceManager()->CreateFragmentShaderFromFile(
				pixel_shader_file.c_str(), "ps_bloom"));

		create_quad(m_target_quad, { 1.f, 1.f, 1.f }, false, false);
	}

	void PostEffect::apply(IvRenderer & renderer, IvRenderTexture& target)
	{
		m_shader_program->GetUniform("screen_width")->SetValue((float)(renderer.GetWidth()), 0);
		m_shader_program->GetUniform("screen_height")->SetValue((float)(renderer.GetHeight()), 0);
		m_shader_program->GetUniform("target")->SetValue(&target);
		
		m_target_quad.render(renderer, m_shader_program);

		m_shader_program->GetUniform("target")->Unbind();
	}
}
