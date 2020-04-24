#pragma once

#include <flame/graphics/image.h>
#include <flame/graphics/font.h>

namespace flame
{
	namespace graphics
	{
		struct Device;
		struct Commandbuffer;

		struct Canvas
		{
			struct Resource
			{
				Imageview* view;
				Atlas* atlas;
				Vec2f white_uv;
			};

			Vec4f clear_color;

			void add_atlas(Atlas* a)
			{
				a->canvas_ = this;
				a->canvas_slot_ = set_resource(-1, a->imageview(), Sampler::get_default(a->border ? FilterLinear : FilterNearest), a);
			}

			void add_font(FontAtlas* f)
			{
				f->canvas_ = this;
				f->canvas_slot_ = set_resource(-1, f->imageview(), Sampler::get_default(FilterNearest));
			}

			FLAME_GRAPHICS_EXPORTS void set_target(uint view_count, Imageview* const* views);

			FLAME_GRAPHICS_EXPORTS Resource get_resource(uint slot);
			FLAME_GRAPHICS_EXPORTS uint set_resource(int slot /* -1 to find an empty slot */, Imageview* v, Sampler* sp = nullptr, Atlas* atlas = nullptr);

			FLAME_GRAPHICS_EXPORTS void stroke(uint point_count, const Vec2f* points, const Vec4c& col, float thickness);
			FLAME_GRAPHICS_EXPORTS void fill(uint point_count, const Vec2f* points, const Vec4c& col);

			FLAME_GRAPHICS_EXPORTS void add_text(FontAtlas* f, const wchar_t* text_begin, const wchar_t* text_end, uint font_size, const Vec2f& pos, const Vec4c& col);
			FLAME_GRAPHICS_EXPORTS void add_image(const Vec2f& pos, const Vec2f& size, uint id, const Vec2f& uv0 = Vec2f(0.f), const Vec2f& uv1 = Vec2f(1.f), const Vec4c& tint_col = Vec4c(255));
			FLAME_GRAPHICS_EXPORTS Vec4f scissor();
			FLAME_GRAPHICS_EXPORTS void set_scissor(const Vec4f& scissor);

			FLAME_GRAPHICS_EXPORTS void prepare();
			FLAME_GRAPHICS_EXPORTS void record(Commandbuffer* cb, uint image_index);

			FLAME_GRAPHICS_EXPORTS static Canvas* create(Device* d);
			FLAME_GRAPHICS_EXPORTS static void destroy(Canvas* canvas);
		};
	}
}
