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
#include "Bruneton.h"
#include "Stars.h"

#include <IvRenderTexture.h>

//-------------------------------------------------------------------------------
//-- Classes --------------------------------------------------------------------
//-------------------------------------------------------------------------------

namespace cali
{
	class terrain;
	class terrain_icosahedron;
	class terrain_quad;
}

using namespace cali;

class Game : public IvGame
{
#if defined WORK_ON_ICOSAHEDRON
	std::unique_ptr<Cali::terrain_icosahedron> m_terrain;
#elif defined WORK_ON_QUAD_TREE
	std::unique_ptr<cali::terrain_quad> m_terrain;
#else
	std::unique_ptr<Cali::terrain> m_terrain;
#endif // !WORK_ON_ICOSAHEDRON
	std::unique_ptr<cali::bruneton> m_bruneton;
	std::unique_ptr<cali::sky> m_sky;
	std::unique_ptr<cali::sun> m_sun;
    std::unique_ptr<cali::stars> m_stars;
	std::unique_ptr<IvRenderTexture> m_main_screen_buffer;
	std::unique_ptr<cali::PostEffect> m_bloom;
	cali::input_controller m_controller;
	cali::camera m_camera;
	cali::debug_info& m_debug_info;
	cali::constant_buffer_wrapper<constant_buffer::GlobalState> m_global_state_cbuffer;

	bool m_render_wireframe;
	bool m_render_debug_info;
	bool m_stop_time;

private:
	void toggle_wireframe(float dt);
	void toggle_debug_info(float dt);
	void setup_controls();

	void reset_scene(float dt);
	void stop_time(float dt);

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
