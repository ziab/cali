#include "AABB.h"

#include "CommonFileSystem.h"

namespace Cali
{
	AABB::AABB()
	{
		std::string vertex_shader = construct_shader_path("simple.hlslv");
		std::string pixel_shader = construct_shader_path("simple.hlslf");

		m_shader = IvRenderer::mRenderer->GetResourceManager()->CreateShaderProgram(
			IvRenderer::mRenderer->GetResourceManager()->CreateVertexShaderFromFile(
				vertex_shader.c_str(), "main"),
			IvRenderer::mRenderer->GetResourceManager()->CreateFragmentShaderFromFile(
				pixel_shader.c_str(), "main"));

		if (!m_shader) throw std::exception("Terrain: failed to load shader program");

		create_box(m_box, { 1.0, 1.0, 1.0 }, true, false);
	}

	void AABB::update(float dt)
	{
	}

	void AABB::render(IvRenderer & renderer)
	{
		Physical::set_transformation_matrix(renderer);
		m_box.render(renderer, m_shader);
	}
}