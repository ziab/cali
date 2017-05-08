#ifndef __GameDefs__
#define __GameDefs__

//#define WORK_ON_ICOSAHEDRON
#define WORK_ON_QUAD_TREE

//-------------------------------------------------------------------------------
//-- Dependencies ---------------------------------------------------------------
//-------------------------------------------------------------------------------

#include <IvGame.h>
#include <memory>

#include "InputController.h"
#include "Camera.h"
#include "DebugInfo.h"
#include "Sky.h"
#include "Sun.h"
#include "ConstantBuffer.h"
#include "ConstantBufferWrapper.h"
#include "PostEffect.h"

#include <IvRenderTexture.h>

//-------------------------------------------------------------------------------
//-- Classes --------------------------------------------------------------------
//-------------------------------------------------------------------------------

namespace Cali
{
	class Terrain;
	class TerrainIcosahedron;
	class TerrainQuad;
}

using namespace Cali;

class Game : public IvGame
{
#if defined WORK_ON_ICOSAHEDRON
	std::unique_ptr<Cali::TerrainIcosahedron> m_terrain;
#elif defined WORK_ON_QUAD_TREE
	std::unique_ptr<Cali::TerrainQuad> m_terrain;
#else
	std::unique_ptr<Cali::Terrain> m_terrain;
#endif // !WORK_ON_ICOSAHEDRON
	std::unique_ptr<Cali::Sky> m_sky;
	std::unique_ptr<Cali::Sun> m_sun;
	std::unique_ptr<IvRenderTexture> m_main_screen_buffer;
	std::unique_ptr<Cali::PostEffect> m_bloom;
	Cali::InputController m_controller;
	Cali::Camera m_camera;
	Cali::DebugInfo& m_debug_info;
	Cali::ConstantBufferWrapper<ConstantBuffer::GlobalState> m_global_state_cbuffer;

	bool m_render_wireframe;
	bool m_render_debug_info;

private:
	void toggle_wireframe(float dt);
	void toggle_debug_info(float dt);
	void setup_controls();

	friend void on_window_resize(unsigned int width, unsigned int height);
	virtual void on_window_resize(size_t width, size_t height);

protected:
	virtual void UpdateObjects(float dt);
	virtual void Render();

public:
    Game();
    ~Game();
    bool PostRendererInitialize();    
};

#endif
