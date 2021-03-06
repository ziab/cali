//===============================================================================
// @ Game.cpp
// ------------------------------------------------------------------------------
// Game core routines
//
// Copyright (C) 2008-2015 by James M. Van Verth and Lars M. Bishop.
// All rights reserved.
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
//
//
//===============================================================================

//-------------------------------------------------------------------------------
//-- Dependencies ---------------------------------------------------------------
//-------------------------------------------------------------------------------

#include <D3D11\IvRendererD3D11.h>
#include <IvRendererHelp.h>
#include <IvConstantBuffer.h>

#include <chrono>
#include <thread>

#include "Game.h"
#include "Constants.h"
#include "World.h"

#if defined WORK_ON_ICOSAHEDRON
#include "TerrainIcosahedron.h"
#elif defined WORK_ON_QUAD_TREE
#include "TerrainQuad.h"
#else
#include "Terrain.h"
#endif // !WORK_ON_ICOSAHEDRON


//-------------------------------------------------------------------------------
//-- Static Members -------------------------------------------------------------
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
//-- Methods --------------------------------------------------------------------
//-------------------------------------------------------------------------------

void on_window_resize(unsigned int width, unsigned int height)
{
	Game* game = static_cast<Game*>(IvGame::mGame);
	game->on_window_resize(width, height);
}

//-------------------------------------------------------------------------------
// @ IvGame::Create()
//-------------------------------------------------------------------------------
// Static constructor
//-------------------------------------------------------------------------------
bool IvGame::Create()
{
    IvGame::mGame = new Game();
    return ( IvGame::mGame != 0 );
}   // End of IvGame::Create()

//-------------------------------------------------------------------------------
// @ Game::Game()
//-------------------------------------------------------------------------------
// Constructor
//-------------------------------------------------------------------------------
Game::Game() : 
	IvGame(),
	m_debug_info(cali::debug_info::get_debug_info()),
	m_camera({ 0.0f, 50.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }),
	m_render_wireframe(false),
	m_render_debug_info(false),
	m_stop_time(false)
{
}   // End of Game::Game()


//-------------------------------------------------------------------------------
// @ Game::~Game()
//-------------------------------------------------------------------------------
// Destructor
//-------------------------------------------------------------------------------
Game::~Game()
{
	m_controller.clear();
}   // End of Game::~Game()


//-------------------------------------------------------------------------------
// @ Game::Initialize()
//-------------------------------------------------------------------------------
// Set up internal subsystems
//-------------------------------------------------------------------------------
bool 
Game::PostRendererInitialize()
{
    // Set up base class 
    IvGame::PostRendererInitialize();
	setup_controls();

	auto& renderer = *IvRenderer::mRenderer;

	m_bruneton = std::make_unique<cali::bruneton>();
	if (!m_bruneton) return false;

	m_bruneton->precompute(renderer);

#if defined WORK_ON_ICOSAHEDRON
	m_terrain = std::unique_ptr<Cali::terrain_icosahedron>(new Cali::terrain_icosahedron);
#elif defined WORK_ON_QUAD_TREE
	m_terrain = std::unique_ptr<cali::terrain_quad>(new cali::terrain_quad(*m_bruneton));
#else
	m_terrain = std::unique_ptr<Cali::terrain>(new Cali::terrain);
#endif // !WORK_ON_ICOSAHEDRON
    
	if (!m_terrain)	return false;

	m_sky = std::unique_ptr<cali::sky>(new cali::sky(*m_bruneton));
	if (!m_sky)	return false;

    m_stars = std::unique_ptr<cali::stars>(new cali::stars);
    if (!m_stars)	return false;

	m_sun = std::unique_ptr<cali::sun>(new cali::sun);
	if (!m_sun)	return false;

	renderer.RegisterOnResizeCbk(&(::on_window_resize));

	renderer.SetRenderInViewSpace(true);

	m_main_screen_buffer = std::unique_ptr<IvRenderTexture>(
		renderer.GetResourceManager()->CreateRenderTexture(renderer.GetWidth(), renderer.GetHeight(), IvTextureFormat::kRGBA32TexFmt));
	if (!m_main_screen_buffer) return false;

	m_bloom = std::unique_ptr<cali::PostEffect>(new PostEffect);
	if (!m_bloom) return false;

	m_debug_info.initialize(renderer);
	m_camera.send_settings_to_renderer(renderer);

	m_global_state_cbuffer = renderer.GetResourceManager()->CreateConstantBuffer(sizeof(constant_buffer::GlobalState));
	if (!m_global_state_cbuffer.valid()) return false;

	renderer.SetConstantBuffer(m_global_state_cbuffer.ivcbuffer(), 1);

	renderer.SetBlendFunc(kOneBlendFunc, kOneMinusSrcAlphaBlendFunc, kAddBlendOp);
	renderer.SetClearColor(0.0, 0.0, 0.0, 1.0);

    return true;
}   // End of Game::Initialize()


void Game::toggle_wireframe(float dt)
{
	m_render_wireframe = !m_render_wireframe;
	IvRenderer::mRenderer->SetFillMode(m_render_wireframe ? kWireframeFill : kSolidFill);
}

void Game::toggle_debug_info(float dt)
{
	m_render_debug_info = !m_render_debug_info;
}

void Game::setup_controls()
{
	m_controller.forward(std::bind(&cali::camera::move_forward, &m_camera, std::placeholders::_1));
	m_controller.backward(std::bind(&cali::camera::move_backward, &m_camera, std::placeholders::_1));
	m_controller.strafe_left(std::bind(&cali::camera::move_left, &m_camera, std::placeholders::_1));
	m_controller.strafe_right(std::bind(&cali::camera::move_right, &m_camera, std::placeholders::_1));
	m_controller.shift(std::bind(&cali::camera::enable_speed_mode, &m_camera, std::placeholders::_1));

	m_controller.mouse_x(std::bind(&cali::camera::yaw, &m_camera, std::placeholders::_1, std::placeholders::_2));
	m_controller.mouse_y(std::bind(&cali::camera::pitch, &m_camera, std::placeholders::_1, std::placeholders::_2));

	m_controller.wireframe(std::bind(&Game::toggle_wireframe, &(*this), std::placeholders::_1));
	m_controller.debug_info(std::bind(&Game::toggle_debug_info, &(*this), std::placeholders::_1));
	m_controller.reset(std::bind(&Game::reset_scene, &(*this), std::placeholders::_1));
	m_controller.stop(std::bind(&Game::stop_time, &(*this), std::placeholders::_1));
}

void Game::reset_scene(float dt)
{
	m_sun->set_position({ 0.f, 1000.f, cali::world::c_horizon_distance / 10 });
}

void Game::stop_time(float dt)
{
	m_stop_time = !m_stop_time;
}

void Game::on_window_resize(size_t width, size_t height)
{
	IvRenderer& renderer = *IvRenderer::mRenderer;
	IvResourceManager& resman = *renderer.GetResourceManager();
	m_main_screen_buffer->Resize(width, height, m_main_screen_buffer->GetFormat(), resman);
}

//-------------------------------------------------------------------------------
// @ Game::Update()
//-------------------------------------------------------------------------------
// Main update loop
//-------------------------------------------------------------------------------
void Game::UpdateObjects(float dt)
{
	m_controller.read_input(dt);
	m_camera.update(dt);

	if (m_stop_time) dt = 0.0f;

	m_debug_info.set_debug_string(L"delta", dt);
	m_debug_info.set_debug_string(L"fps", 1 / dt);

	m_global_state_cbuffer->world_origin = { 0.f, 0.f, 0.f };
	m_global_state_cbuffer->world_up = cali::constants::c_world_up;
	m_global_state_cbuffer->sky_color_zenith = { 113.f / 255.f, 149.f / 255.f, 255.f / 255.f, 1.f };
	m_global_state_cbuffer->sky_color_horizon = { 254.f / 255.f, 251.f / 255.f, 181.f / 255.f, 1.f };
	
	m_terrain->set_viewer(m_camera.get_position());
	m_terrain->update(dt);

	m_sun->update(dt);
	m_sun->look_at(m_camera.get_position(), cali::constants::c_world_up);
	m_sun->update_global_state(m_global_state_cbuffer);

    m_stars->update(dt);

	m_camera.update_global_state(m_global_state_cbuffer);
}   // End of Game::Update()

//-------------------------------------------------------------------------------
// @ Game::Render()
//-------------------------------------------------------------------------------
// Render stuff
//-------------------------------------------------------------------------------

void Game::Render() // Here's Where We Do All The Drawing
{
	IvRenderer& renderer = *IvRenderer::mRenderer;
	renderer.ClearRenderTarget(m_main_screen_buffer.get(), kColorClear, { 0.0, 0.0, 0.0, 0.0 });
	renderer.SetRenderTarget(m_main_screen_buffer.get());

    m_camera.render(renderer);

	renderer.UpdateConstantBuffer(m_global_state_cbuffer.ivcbuffer());

	IvDrawAxes();
    m_stars->render(renderer);
	m_sky->render(renderer);
	m_terrain->render(renderer, m_camera.get_frustum());
	//m_sun->render(renderer);

	renderer.ReleaseRenderTarget();

	auto& d3drenderer = static_cast<IvRendererD3D11&>(renderer);
	
	// if wiferframe is enabled we disable any post effects
	if (m_render_wireframe)
	{
		d3drenderer.CopyToBackBuffer(m_main_screen_buffer.get());
	}
	else
	{
		// Post effects should be placed here
		m_bloom->apply(renderer, *m_main_screen_buffer);

		// if no post-effects just copy back the buffer
		//d3drenderer.CopyToBackBuffer(m_main_screen_buffer.get());
	}

	if (m_render_debug_info)
	{
		m_debug_info.render(renderer);
	}
} // End of Game::Render()

