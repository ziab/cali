#pragma once
#include <unordered_map>
#include <mutex>
#include <SpriteFont.h>
#include <IvRenderer.h>

#include "Renderable.h"

namespace cali
{
	class debug_info : public renderable
	{
		static debug_info* m_debug_info;
		
		static const float m_scaling;

		std::unordered_map<const wchar_t*, float> m_strings;
		mutable std::mutex m_mutex;

		std::unique_ptr<DirectX::SpriteBatch> m_sprite_batch;
		std::unique_ptr<DirectX::SpriteFont> m_sprite_font;

		debug_info() {}
		~debug_info() {}
	public:

		static debug_info& get_debug_info();

		void initialize(IvRenderer & renderer);

		void set_debug_string(const wchar_t* param, float value);

		virtual void update(float dt);
		virtual void render(IvRenderer& renderer);
	};
}