#pragma once

#include <flame/graphics/image.h>
#include <flame/graphics/font.h>

namespace flame
{
	namespace graphics
	{
		struct Imageview;
		struct Atlas;

		struct Canvas : Object
		{
			Canvas() :
				Object("Canvas")
			{
			}

			virtual void set_clear_color(const Vec4c& col) = 0;
			virtual Imageview* get_image(uint index) = 0;
			virtual Atlas* get_atlas(uint index) = 0;
			virtual uint set_image(int index /* -1 to find an empty slot */, Imageview* v, Filter filter = FilterLinear, Atlas* atlas = nullptr) = 0;

			void add_atlas(Atlas* a)
			{
				a->canvas_ = this;
				a->canvas_slot_ = set_image(-1, a->imageview(), a->border ? FilterLinear : FilterNearest, a);
			}

			void add_font(FontAtlas* f)
			{
				f->canvas_ = this;
				f->canvas_slot_ = set_image(-1, f->imageview(), f->draw_type == FontDrawSdf ? FilterLinear : FilterNearest);
			}

			virtual void stroke(const std::vector<Vec2f>& points, const Vec4c& col, float thickness) = 0;
			virtual void fill(const std::vector<Vec2f>& points, const Vec4c& col) = 0;

			virtual void add_text(FontAtlas* f, const std::vector<Glyph*> glyphs, uint line_space, float scale, const Vec2f& pos, const Vec4c& col) = 0;
			virtual void add_image(const Vec2f& pos, const Vec2f& size, uint id, const Vec2f& uv0 = Vec2f(0.f), const Vec2f& uv1 = Vec2f(1.f), const Vec4c& tint_col = Vec4c(255), bool repeat = false) = 0;
			virtual const Vec4f& scissor() = 0;
			virtual void set_scissor(const Vec4f& scissor) = 0;
		};
	}
}