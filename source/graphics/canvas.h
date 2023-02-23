#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Canvas
		{
			WindowPtr window;

			virtual ~Canvas() {}

			virtual void add_rect(const vec2& a, const vec2& b, float thickness, const cvec4& col) = 0;
			virtual void add_rect_filled(const vec2& a, const vec2& b, const cvec4& col) = 0;
			virtual void add_text(const vec2& pos, std::wstring_view str, const cvec4& col) = 0;

			struct Create
			{
				virtual CanvasPtr operator()(WindowPtr window) = 0;
			};
			FLAME_GRAPHICS_API static Create& create;
		};
	}
}
