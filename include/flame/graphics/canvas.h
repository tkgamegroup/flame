#pragma once

#include <flame/foundation/foundation.h>
#include <flame/graphics/graphics.h>

namespace flame
{
	namespace graphics
	{
		struct Device;
		struct ImageView;
		struct ImageAtlas;
		struct FontAtlas;
		struct Sampler;
		struct CommandBuffer;

		struct CanvasResource
		{
			virtual const char* get_name() const = 0;
			virtual ImageView* get_view() const = 0;
			virtual ImageAtlas* get_image_atlas() const = 0;
			virtual FontAtlas* get_font_atlas() const = 0;
		};

		struct Canvas
		{
			virtual void release() = 0;

			virtual Vec4c get_clear_color() const = 0;
			virtual void set_clear_color(const Vec4c& color) = 0;

			virtual void set_target(uint views_count, ImageView* const* views) = 0;

			virtual CanvasResource* get_resource(uint slot) = 0;
			virtual uint set_resource(int slot /* -1 to find an empty slot */, ImageView* v, Sampler* sp = nullptr, const char* name = nullptr) = 0;
			virtual uint set_resource(int slot /* -1 to find an empty slot */, ImageAtlas* image_atlas, const char* name = nullptr) = 0;
			virtual uint set_resource(int slot /* -1 to find an empty slot */, FontAtlas* font_atlas, const char* name = nullptr) = 0;

			//virtual uint add_model(const char* name) = 0;
			//virtual void remove_model(uint mod_id) = 0;

			virtual void begin_path() = 0;
			virtual void move_to(const Vec2f& pos) = 0;
			virtual void line_to(const Vec2f& pos) = 0;
			virtual void close_path() = 0;

			virtual void stroke(const Vec4c& col, float thickness, bool aa = false) = 0;
			virtual void fill(const Vec4c& col, bool aa = false) = 0;
			// null text_end means render until '\0'
			virtual void add_text(uint res_id, const wchar_t* text_beg, const wchar_t* text_end, uint font_size, const Vec4c& col, const Vec2f& pos, const Mat2f& axes = Mat2f(1.f)) = 0;
			virtual void add_image(uint res_id, uint tile_id, const Vec2f& LT, const Vec2f& RT, const Vec2f& RB, const Vec2f& LB, const Vec2f& uv0 = Vec2f(0.f), const Vec2f& uv1 = Vec2f(1.f), const Vec4c& tint_col = Vec4c(255)) = 0;

			virtual void add_object(const Mat4f& mat, uint mod_id) = 0;

			virtual void add_blur(const Vec4f& range, uint radius) = 0;

			virtual Vec4f get_scissor() const = 0;
			virtual void set_scissor(const Vec4f& scissor) = 0;

			virtual void prepare() = 0;
			virtual void record(CommandBuffer* cb, uint image_index) = 0;

			FLAME_GRAPHICS_EXPORTS static Canvas* create(Device* d);
		};
	}
}
