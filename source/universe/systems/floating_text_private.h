#pragma once

#include "floating_text.h"

namespace flame
{
	struct FloatingTextPrivate : FloatingText
	{
		struct Item
		{
			uint id;
			float time;
			std::wstring text;
			uint font_size; 
			cvec4 color;
			cNodePtr bind_target;
			vec3 offset_3d;
			vec2 offset_2d;
			vec2 speed;
		};

		uint next_id = 1;
		std::vector<Item> items;
		void* ev = nullptr;

		uint add(const std::wstring& text, uint font_size, const cvec4& color, float duration, cNodePtr bind_target,
			const vec3& offset_3d = vec3(0.f), const vec2& offset_2d = vec2(0.f), const vec2& speed = vec2(0.f)) override;
		void remove(uint id) override;
		void update();
	};
}
