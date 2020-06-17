#pragma once

#include <flame/graphics/graphics.h>

namespace flame
{
	namespace graphics
	{
		struct Device;
		struct ImageAtlas;
		struct FontAtlas;
		struct Commandbuffer;

		struct Canvas
		{
			struct Resource
			{
				Imageview* view;
				ImageAtlas* atlas;
				Vec2f white_uv;
			};

			virtual void release() = 0;

			virtual Vec4f get_clear_color() const = 0;
			virtual void set_clear_color(const Vec4f& color) = 0;

			virtual void set_target(uint view_count, Imageview* const* views) = 0;

			virtual Resource get_resource(uint slot) = 0;
			virtual uint set_resource(int slot /* -1 to find an empty slot */, Imageview* v, Sampler* sp = nullptr, ImageAtlas* atlas = nullptr) = 0;
			virtual void add_atlas(ImageAtlas* a) = 0;
			virtual void add_font(FontAtlas* f) = 0;

			virtual void stroke(uint point_count, const Vec2f* points, const Vec4c& col, float thickness) = 0;
			virtual void fill(uint point_count, const Vec2f* points, const Vec4c& col) = 0;

			virtual void add_text(FontAtlas* f, const wchar_t* text_begin, const wchar_t* text_end, uint font_size, const Vec2f& pos, const Vec4c& col) = 0;
			virtual void add_image(const Vec2f& pos, const Vec2f& size, uint id, const Vec2f& uv0 = Vec2f(0.f), const Vec2f& uv1 = Vec2f(1.f), const Vec4c& tint_col = Vec4c(255)) = 0;
			virtual Vec4f get_scissor() const = 0;
			virtual void set_scissor(const Vec4f& scissor) = 0;

			virtual void prepare() = 0;
			virtual void record(Commandbuffer* cb, uint image_index) = 0;

			FLAME_GRAPHICS_EXPORTS static Canvas* create(Device* d);
		};
	}
}
