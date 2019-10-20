#pragma once

#include <flame/math.h>
#include <flame/graphics/graphics.h>

namespace flame
{
	namespace graphics
	{
		struct Imageview;
		struct FontAtlas;

		struct Canvas
		{
			virtual void set_clear_color(const Vec4c& col) = 0;
			virtual Imageview* get_image(uint index) = 0;
			virtual uint set_image(int index, Imageview* v, Filter filter = FilterLinear) = 0; // index=-1 to find an empty slot

			void stroke(std::vector<Vec2f>& points, const Vec4c& col, float thickness)
			{
				stroke(points, col, col, thickness);
			}
			virtual void stroke(const std::vector<Vec2f>& points, const Vec4c& inner_col, const Vec4c& outter_col, float thickness) = 0;
			virtual void fill(const std::vector<Vec2f>& points, const Vec4c& col) = 0;

			virtual Vec2f add_text(FontAtlas* f, const Vec2f& pos, const Vec4c& col, const std::wstring& text, float scale = 1.f) = 0;
			virtual void add_image(const Vec2f& pos, const Vec2f& size, uint id, const Vec2f& uv0 = Vec2f(0.f), const Vec2f& uv1 = Vec2f(1.f), const Vec4c& tint_col = Vec4c(255)) = 0;
			virtual const Vec4f& scissor() = 0;
			virtual void set_scissor(const Vec4f& scissor) = 0;
		};
	}
}
