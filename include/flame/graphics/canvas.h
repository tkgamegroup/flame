#pragma once

#include <flame/graphics/graphics.h>

namespace flame
{
	namespace graphics
	{
		struct Device;
		struct Imageview;
		struct Swapchain;
		struct Commandbuffer;
		struct FontAtlas;

		enum DrawCmdType
		{
			DrawCmdElement,
			DrawCmdTextLcd,
			DrawCmdTextSdf,
			DrawCmdScissor
		};

		struct Canvas
		{
			FLAME_GRAPHICS_EXPORTS void set_clear_color(const Vec4c& col);

			FLAME_GRAPHICS_EXPORTS Imageview* get_imageview(uint index);
			FLAME_GRAPHICS_EXPORTS void set_imageview(uint index, Imageview* v);

			FLAME_GRAPHICS_EXPORTS uint add_font_atlas(FontAtlas* font_atlas);
			FLAME_GRAPHICS_EXPORTS FontAtlas* get_font_atlas(uint idx);

			FLAME_GRAPHICS_EXPORTS void start_cmd(DrawCmdType type, uint id);
			FLAME_GRAPHICS_EXPORTS void path_line_to(const Vec2f& p);
			FLAME_GRAPHICS_EXPORTS void path_rect(const Vec2f& pos, const Vec2f& size, float round_radius, Side round_flags);
			FLAME_GRAPHICS_EXPORTS void path_arc_to(const Vec2f& center, float radius, int a_min, int a_max);
			FLAME_GRAPHICS_EXPORTS void path_bezier(const Vec2f& p1, const Vec2f& p2, const Vec2f& p3, const Vec2f& p4, uint level = 0);
			FLAME_GRAPHICS_EXPORTS void clear_path();
			FLAME_GRAPHICS_EXPORTS void stroke(const Vec4c& col, float thickness, bool closed);
			FLAME_GRAPHICS_EXPORTS void stroke_col2(const Vec4c& inner_col, const Vec4c& outter_col, float thickness, bool closed);
			FLAME_GRAPHICS_EXPORTS void fill(const Vec4c& col);

			FLAME_GRAPHICS_EXPORTS void add_text(uint font_atlas_index, const Vec2f& pos, const Vec4c& col, const std::wstring& text, float scale = 1.f /* for sdf */);
			FLAME_GRAPHICS_EXPORTS void add_line(const Vec2f& p0, const Vec2f& p1, const Vec4c& col, float thickness);
			FLAME_GRAPHICS_EXPORTS void add_triangle_filled(const Vec2f& p0, const Vec2f& p1, const Vec2f& p2, const Vec4c& col);
			FLAME_GRAPHICS_EXPORTS void add_rect(const Vec2f& pos, const Vec2f& size, const Vec4c& col, float thickness, float round_radius = 0.f, Side round_flags = Side(SideNW | SideNE | SideSW | SideSE));
			FLAME_GRAPHICS_EXPORTS void add_rect_col2(const Vec2f& pos, const Vec2f& size, const Vec4c& inner_col, const Vec4c& outter_col, float thickness, float round_radius = 0.f, Side round_flags = Side(SideNW | SideNE | SideSW | SideSE));
			FLAME_GRAPHICS_EXPORTS void add_rect_rotate(const Vec2f& pos, const Vec2f& size, const Vec4c& col, float thickness, const Vec2f& rotate_center, float angle);
			FLAME_GRAPHICS_EXPORTS void add_rect_filled(const Vec2f& pos, const Vec2f& size, const Vec4c& col, float round_radius = 0.f, Side round_flags = Side(0));
			FLAME_GRAPHICS_EXPORTS void add_circle(const Vec2f& center, float radius, const Vec4c& col, float thickness);
			void add_circle_LT(const Vec2f& center, float diameter, const Vec4c& col, float thickness)
			{
				add_circle(center + diameter * 0.5f, diameter * 0.5f, col, thickness);
			}
			FLAME_GRAPHICS_EXPORTS void add_circle_filled(const Vec2f& center, float radius, const Vec4c& col);
			void add_circle_filled_LT(const Vec2f& center, float diameter, const Vec4c& col)
			{
				add_circle_filled(center + diameter * 0.5f, diameter * 0.5f, col);
			}
			FLAME_GRAPHICS_EXPORTS void add_bezier(const Vec2f& p1, const Vec2f& p2, const Vec2f& p3, const Vec2f& p4, const Vec4c& col, float thickness);
			FLAME_GRAPHICS_EXPORTS void add_image(const Vec2f& pos, const Vec2f& size, uint id, const Vec2f& uv0 = Vec2f(0.f), const Vec2f& uv1 = Vec2f(1.f), const Vec4c& tint_col = Vec4c(255));
			FLAME_GRAPHICS_EXPORTS void add_image_stretch(const Vec2f& pos, const Vec2f& size, uint id, const Vec4f& border, const Vec4c& tint_col = Vec4c(255));
			FLAME_GRAPHICS_EXPORTS void set_scissor(const Vec4f& scissor);

			FLAME_GRAPHICS_EXPORTS Commandbuffer* get_cb() const;
			FLAME_GRAPHICS_EXPORTS void record_cb();

			FLAME_GRAPHICS_EXPORTS static void initialize(Device* d, Swapchain* sc);
			FLAME_GRAPHICS_EXPORTS static void deinitialize();

			FLAME_GRAPHICS_EXPORTS static Canvas* create(Swapchain* sc); // all swapchains that used to create canvas should have the same sample_count as the one pass to initialize
			FLAME_GRAPHICS_EXPORTS static void destroy(Canvas* c);
		};

		typedef Canvas* CanvasPtr;
	}
}
