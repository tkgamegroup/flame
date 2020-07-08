#pragma once

#include <flame/foundation/foundation.h>
#include <flame/graphics/graphics.h>

namespace flame
{
	namespace graphics
	{
		struct Device;
		struct Imageview;
		struct ImageAtlas;
		struct FontAtlas;
		struct Sampler;
		struct Commandbuffer;

		struct CanvasResource
		{
			virtual Imageview* get_view() const = 0;
			virtual ImageAtlas* get_image_atlas() const = 0;
			virtual FontAtlas* get_font_atlas() const = 0;
			virtual Vec2f get_white_uv() const = 0;
		};

		struct Canvas
		{
			virtual void release() = 0;

			virtual Vec4f get_clear_color() const = 0;
			virtual void set_clear_color(const Vec4c& color) = 0;

			virtual void set_target(uint views_count, Imageview* const* views) = 0;

			virtual CanvasResource* get_resource(uint slot) = 0;
			virtual uint set_resource(int slot /* -1 to find an empty slot */, Imageview* v, Sampler* sp = nullptr, ImageAtlas* image_atlas = nullptr, FontAtlas* font_atlas = nullptr) = 0;
			virtual void add_atlas(ImageAtlas* a) = 0;
			virtual void add_font(FontAtlas* f) = 0;

			virtual void stroke(uint points_count, const Vec2f* points, const Vec4c& col, float thickness, bool aa = false) = 0;
			virtual void fill(uint points_count, const Vec2f* points, const Vec4c& col, bool aa = false) = 0;
			virtual void add_text(uint res_id, const wchar_t* text, uint size, const Vec2f& pos, const Vec4c& col) = 0;
			virtual void add_image(uint res_id, uint tile_id, const Vec2f& pos, const Vec2f& size, const Vec2f& uv0 = Vec2f(0.f), const Vec2f& uv1 = Vec2f(1.f), const Vec4c& tint_col = Vec4c(255)) = 0;

			virtual Vec4f get_scissor() const = 0;
			virtual void set_scissor(const Vec4f& scissor) = 0;

			virtual void prepare() = 0;
			virtual void record(Commandbuffer* cb, uint image_index) = 0;

			FLAME_GRAPHICS_EXPORTS static Canvas* create(Device* d);
		};
	}
}
