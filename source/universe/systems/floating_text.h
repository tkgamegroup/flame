#pragma once

#include "../universe.h"

namespace flame
{
	struct FloatingText
	{
		virtual uint add(const std::wstring& text, uint font_size, const cvec4& color, float duration, cNodePtr bind_target, 
			const vec3& offset_3d = vec3(0.f), const vec2& offset_2d = vec2(0.f), const vec2& speed = vec2(0.f)) = 0;
		virtual void remove(uint id) = 0;

		struct Instance
		{
			virtual FloatingTextPtr operator()() = 0;
		};
		FLAME_UNIVERSE_API static Instance& instance;
	};
}
