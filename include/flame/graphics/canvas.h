#pragma once

#include <flame/graphics/graphics.h>

namespace flame
{
	namespace graphics
	{
		struct Device;
		struct Imageview;
		struct Commandbuffer;
		struct FontAtlas;

		struct Canvas
		{
			FLAME_GRAPHICS_EXPORTS void set_render_target(TargetType$ type, const void* v);
			FLAME_GRAPHICS_EXPORTS void set_clear_color(const Vec4c& col);
			FLAME_GRAPHICS_EXPORTS Imageview* get_image(uint index);
			FLAME_GRAPHICS_EXPORTS uint set_image(int index, Imageview* v, Filter filter = FilterLinear); // index=-1 to find an empty slot

			void stroke(std::vector<Vec2f>& points, const Vec4c& col, float thickness)
			{
				stroke(points, col, col, thickness);
			}
			FLAME_GRAPHICS_EXPORTS void stroke(const std::vector<Vec2f>& points, const Vec4c& inner_col, const Vec4c& outter_col, float thickness);
			FLAME_GRAPHICS_EXPORTS void fill(const std::vector<Vec2f>& points, const Vec4c& col);

			FLAME_GRAPHICS_EXPORTS Vec2f add_text(FontAtlas* f, const Vec2f& pos, const Vec4c& col, const std::wstring& text, float scale = 1.f);
			FLAME_GRAPHICS_EXPORTS void add_image(const Vec2f& pos, const Vec2f& size, uint id, const Vec2f& uv0 = Vec2f(0.f), const Vec2f& uv1 = Vec2f(1.f), const Vec4c& tint_col = Vec4c(255));
			FLAME_GRAPHICS_EXPORTS void set_scissor(const Vec4f& scissor);

			FLAME_GRAPHICS_EXPORTS void record(Commandbuffer* cb, uint image_idx);

			FLAME_GRAPHICS_EXPORTS static Canvas* create(Device* d, TargetType$ type, const void* v);
			FLAME_GRAPHICS_EXPORTS static void destroy(Canvas* c);
		};

		typedef Canvas* CanvasPtr;
	}
}
