#include "DebugInfo.h"

#include <D3D11\IvRendererD3D11.h>
#include <string>

namespace Cali
{
	DebugInfo* DebugInfo::m_debug_info = nullptr;
	const float DebugInfo::m_scaling = 0.5f;

	DebugInfo & DebugInfo::get_debug_info()
	{
		if (!m_debug_info)
		{
			m_debug_info = new DebugInfo;
		}

		return *m_debug_info;
	}

	void DebugInfo::initialize(IvRenderer & renderer)
	{
		auto& d3d_renderer = dynamic_cast<IvRendererD3D11&>(renderer);

		m_sprite_batch = std::make_unique<DirectX::SpriteBatch>(d3d_renderer.GetContext());
		m_sprite_font = std::make_unique<DirectX::SpriteFont>(d3d_renderer.GetDevice(), L"courier_new.spritefont");
	}


	void DebugInfo::set_debug_string(const wchar_t* param, float value)
	{
		std::lock_guard<decltype(m_mutex)> lg(m_mutex);
		m_strings[param] = value;
	}

	void DebugInfo::update(float dt)
	{
	}

	void DebugInfo::render(IvRenderer & renderer)
	{
		std::lock_guard<decltype(m_mutex)> lg(m_mutex);

		if (!m_sprite_batch || !m_sprite_font) return;

		DirectX::XMFLOAT2 current_line(5, 5);

		auto& d3d_renderer = dynamic_cast<IvRendererD3D11&>(renderer);
		
		// Remember the state of the rasterizer before calling rendering using DirecXTK helpers
		ID3D11RasterizerState* raster_state = nullptr;
		ID3D11DepthStencilState* stencil_state = nullptr;
		UINT stencil_ref = 0;
		d3d_renderer.GetContext()->RSGetState(&raster_state);
		d3d_renderer.GetContext()->OMGetDepthStencilState(&stencil_state, &stencil_ref);
		
		if (!raster_state || !stencil_state) return;

		for (auto entry : m_strings)
		{
			m_sprite_batch->Begin();

			wchar_t value[20];
			swprintf_s(value, L"%f\0", entry.second);

			DirectX::XMVECTOR name_length = m_sprite_font->MeasureString(entry.first);
			DirectX::XMFLOAT2 value_offset(current_line.x + DirectX::XMVectorGetX(name_length), current_line.y);

			m_sprite_font->DrawString(
				m_sprite_batch.get(),
				entry.first,
				current_line,
				DirectX::Colors::White, 
				0, DirectX::XMFLOAT2(0.f, 0.f), 
				m_scaling);

			m_sprite_font->DrawString(
				m_sprite_batch.get(),
				&value[0],
				value_offset,
				DirectX::Colors::Turquoise,
				0, DirectX::XMFLOAT2(0.f, 0.f),
				m_scaling);

			m_sprite_batch->End();

			current_line.y += DirectX::XMVectorGetY(name_length);
		}

		// Restore the cached state back
		d3d_renderer.GetContext()->RSSetState(raster_state);
		d3d_renderer.GetContext()->OMSetDepthStencilState(stencil_state, stencil_ref);
		raster_state->Release();
		stencil_state->Release();
	}
}