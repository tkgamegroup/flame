#pragma once

#include <flame/foundation/blueprint.h>
#include <flame/graphics/image.h>
#include <flame/graphics/font.h>
#include <flame/graphics/swapchain.h>

namespace flame
{
	namespace graphics
	{
		struct Swapchain;
		struct Commandbuffer;

		struct Canvas
		{
			BP* scene;

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

			virtual void stroke(uint point_count, const Vec2f* points, const Vec4c& col, float thickness) = 0;
			virtual void fill(uint point_count, const Vec2f* points, const Vec4c& col) = 0;

			virtual void add_text(FontAtlas* f, const wchar_t* text, uint font_size, float scale, const Vec2f& pos, const Vec4c& col) = 0;
			virtual void add_image(const Vec2f& pos, const Vec2f& size, uint id, const Vec2f& uv0 = Vec2f(0.f), const Vec2f& uv1 = Vec2f(1.f), const Vec4c& tint_col = Vec4c(255)) = 0;
			virtual const Vec4f& scissor() = 0;
			virtual void set_scissor(const Vec4f& scissor) = 0;

			virtual void set_draw(void(*draw)(void* c), const Mail<>& capture) = 0;

			inline static Canvas* create(const wchar_t* filename, void* dst, uint dst_hash, void* cbs)
			{
				auto bp = BP::create_from_file(filename);
				if (!bp)
					return nullptr;
				if (dst_hash == FLAME_CHASH("Swapchain"))
					((graphics::Swapchain*)dst)->link_bp(bp, cbs);
				bp->update();
				return (Canvas*)bp->find_output("make_cmd.canvas")->data_p();
			}
		};
	}
}
