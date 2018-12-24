#include "Box.h"

#include "CommonFileSystem.h"

namespace cali
{
	Box::Box()
	{
		std::string vertex_shader = construct_shader_path("simple.hlslv");
		std::string pixel_shader = construct_shader_path("simple.hlslf");

		m_shader = IvRenderer::mRenderer->GetResourceManager()->CreateShaderProgram(
			IvRenderer::mRenderer->GetResourceManager()->CreateVertexShaderFromFile(
				vertex_shader.c_str(), "main"),
			IvRenderer::mRenderer->GetResourceManager()->CreateFragmentShaderFromFile(
				pixel_shader.c_str(), "main"));

		if (!m_shader) throw std::exception("terrain: failed to load shader program");

		m_box = create_box({ 1.0, 1.0, 1.0 }, true, false);
	}

	void Box::update(float dt)
	{
	}

	void Box::render(IvRenderer & renderer)
	{
		physical::set_transformation_matrix(renderer);
		m_box.render(renderer, m_shader);
	}
}