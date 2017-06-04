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

#include <IvRendererHelp.h>
#include <IvConstantBuffer.h>

#include <chrono>
#include <thread>

#include "Game.h"
#include "Constants.h"

#if defined WORK_ON_ICOSAHEDRON
#include "TerrainIcosahedron.h"
#elif defined WORK_ON_QUAD_TREE
#include "TerrainQuad.h"
#else
#include "Terrain.h"
#endif // !WORK_ON_ICOSAHEDRON

#include <D3D11\IvRendererD3D11.h>

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
	m_debug_info(Cali::DebugInfo::get_debug_info()),
	m_camera({ 0.0f, 50.0f, 0.0f }, { 0.0f, 0.0f, 1.0f })
{
	m_render_wireframe = false;
	m_render_debug_info = false;
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

	m_bruneton = std::make_unique<Cali::Bruneton>();
	if (!m_bruneton) return false;

	m_bruneton->precompute(renderer);

#if defined WORK_ON_ICOSAHEDRON
	m_terrain = std::unique_ptr<Cali::TerrainIcosahedron>(new Cali::TerrainIcosahedron);
#elif defined WORK_ON_QUAD_TREE
	m_terrain = std::unique_ptr<Cali::TerrainQuad>(new Cali::TerrainQuad(*m_bruneton));
#else
	m_terrain = std::unique_ptr<Cali::Terrain>(new Cali::Terrain);
#endif // !WORK_ON_ICOSAHEDRON
    
	if (!m_terrain)	return false;

	m_sky = std::unique_ptr<Cali::Sky>(new Cali::Sky(*m_bruneton));
	if (!m_sky)	return false;

	m_sun = std::unique_ptr<Cali::Sun>(new Cali::Sun);
	if (!m_sun)	return false;

	renderer.RegisterOnResizeCbk(&(::on_window_resize));

	renderer.SetRenderInViewSpace(true);

	m_main_screen_buffer = std::unique_ptr<IvRenderTexture>(
		renderer.GetResourceManager()->CreateRenderTexture(renderer.GetWidth(), renderer.GetHeight(), IvTextureFormat::kRGBA32TexFmt));
	if (!m_main_screen_buffer) return false;

	m_bloom = std::unique_ptr<Cali::PostEffect>(new PostEffect);
	if (!m_bloom) return false;

	m_debug_info.initialize(renderer);
	m_camera.send_settings_to_renderer(renderer);

	m_global_state_cbuffer = renderer.GetResourceManager()->CreateConstantBuffer(sizeof(ConstantBuffer::GlobalState));
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
	m_controller.forward(std::bind(&Cali::Camera::move_forward, &m_camera, std::placeholders::_1));
	m_controller.backward(std::bind(&Cali::Camera::move_backward, &m_camera, std::placeholders::_1));
	m_controller.strafe_left(std::bind(&Cali::Camera::move_left, &m_camera, std::placeholders::_1));
	m_controller.strafe_right(std::bind(&Cali::Camera::move_right, &m_camera, std::placeholders::_1));
	m_controller.shift(std::bind(&Cali::Camera::enable_speed_mode, &m_camera, std::placeholders::_1));

	m_controller.mouse_x(std::bind(&Cali::Camera::yaw, &m_camera, std::placeholders::_1, std::placeholders::_2));
	m_controller.mouse_y(std::bind(&Cali::Camera::pitch, &m_camera, std::placeholders::_1, std::placeholders::_2));

	m_controller.wireframe(std::bind(&Game::toggle_wireframe, &(*this), std::placeholders::_1));
	m_controller.debug_info(std::bind(&Game::toggle_debug_info, &(*this), std::placeholders::_1));
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
	m_debug_info.set_debug_string(L"delta", dt);
	m_debug_info.set_debug_string(L"fps", 1 / dt);

	m_global_state_cbuffer->world_origin = { 0.f, 0.f, 0.f };
	m_global_state_cbuffer->world_up = Cali::Constants::c_world_up;
	m_global_state_cbuffer->sky_color_zenith = { 113.f / 255.f, 149.f / 255.f, 255.f / 255.f, 1.f };
	m_global_state_cbuffer->sky_color_horizon = { 254.f / 255.f, 251.f / 255.f, 181.f / 255.f, 1.f };

	m_camera.update(dt);
	m_controller.read_input(dt);
	m_terrain->set_viewer(m_camera.get_position());
	m_terrain->update(dt);
	m_sun->update(dt);

	m_sun->look_at(m_camera.get_position(), Cali::Constants::c_world_up);
	m_sun->update_global_state(m_global_state_cbuffer);
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
	//renderer.ClearRenderTarget(m_main_screen_buffer.get(), kColorClear, { 0.0, 0.0, 0.0, 0.0 });
	//renderer.SetRenderTarget(m_main_screen_buffer.get());

    m_camera.render(renderer);

	renderer.UpdateConstantBuffer(m_global_state_cbuffer.ivcbuffer());

	IvDrawAxes();
	m_sky->render(renderer);
	m_terrain->render(renderer, m_camera.get_frustum());
	//m_sun->render(renderer);

	//renderer.ReleaseRenderTarget();

	auto& d3drenderer = static_cast<IvRendererD3D11&>(renderer);
	
	// if wiferframe is enabled we disable any post effects
	if (m_render_wireframe)
	{
		//d3drenderer.CopyToBackBuffer(m_main_screen_buffer.get());
	}
	else
	{
		// Post effects should be placed here
		//m_bloom->apply(renderer, *m_main_screen_buffer);

		// if no post-effects just copy back the buffer
		//d3drenderer.CopyToBackBuffer(m_main_screen_buffer.get());
	}

	if (m_render_debug_info)
	{
		m_debug_info.render(renderer);
	}
} // End of Game::Render()

