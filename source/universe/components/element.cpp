// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <flame/graphics/canvas.h>
#include <flame/universe/entity.h>
#include <flame/universe/components/ui.h>
#include <flame/universe/components/element.h>

namespace flame
{
	struct cElementPrivate : cElement$
	{
		cElementPrivate* p_element_;
		graphics::Canvas* canvas_;

		Vec2 pos_;
		float scl_;
		Vec2 size_;

		cElementPrivate(void* data) :
			p_element_(nullptr),
			canvas_(nullptr),
			pos_(0.f),
			scl_(1.f),
			size_(0.f)
		{
			if (!data)
			{
				pos = Vec2(0.f);
				scale = 1.f;
				size = Vec2(0.f);

				inner_padding = Vec4(0.f);
				layout_padding = 0.f;

				alpha = 1.f;

				background_offset = Vec4(0.f);
				background_round_radius = 0.f;
				background_round_flags = 0;
				background_frame_thickness = 0.f;
				background_color = Bvec4(0);
				background_frame_color = Bvec4(255);
				background_shadow_thickness = 0.f;
			}
		}

		void on_attach()
		{
			auto e = entity->parent();
			if (e)
			{
				p_element_ = (cElementPrivate*)(e->component(cH("Element")));
				if (p_element_)
				{
					canvas_ = p_element_->canvas_;
					return;
				}
			}

			while (e)
			{
				auto c = (cUI$*)(e->component(cH("UI")));
				if (c)
				{
					canvas_ = c->canvas();
					break;
				}
				e = e->parent();
			}
		}

		void update(float delta_time)
		{
			if (p_element_)
			{
				pos_ = p_element_->pos_ + p_element_->scl_ * pos;
				scl_ = p_element_->scl_ * scale;
			}
			else
			{
				pos_ = pos;
				scl_ = scale;
			}
			size_ = scl_ * size;

			if (canvas_)
			{
				auto p = pos_ - (Vec2(background_offset[0], background_offset[1])) * scl_;
				auto s = size_ + (Vec2(background_offset[0] + background_offset[2], background_offset[1] + background_offset[3])) * scl_;
				auto rr = background_round_radius * scl_;

				if (background_shadow_thickness > 0.f)
				{
					canvas_->add_rect_col2(p - Vec2(background_shadow_thickness * 0.5f), s + Vec2(background_shadow_thickness), Bvec4(0, 0, 0, 128), Bvec4(0),
						background_shadow_thickness, rr, background_round_flags);
				}
				if (alpha > 0.f)
				{
					if (background_color.w > 0)
						canvas_->add_rect_filled(p, s, Bvec4(background_color, alpha), rr, background_round_flags);
					if (background_frame_thickness > 0.f && background_frame_color.w > 0)
						canvas_->add_rect(p, s, Bvec4(background_frame_color, alpha), background_frame_thickness, rr, background_round_flags);
				}
			}
		}
	};

	cElement$::~cElement$()
	{
	}

	const char* cElement$::type_name() const
	{
		return "Element";
	}

	uint cElement$::type_hash() const
	{
		return cH("Element");
	}

	void cElement$::on_attach()
	{
		((cElementPrivate*)this)->on_attach();
	}

	void cElement$::update(float delta_time)
	{
		((cElementPrivate*)this)->update(delta_time);
	}

	graphics::Canvas* cElement$::canvas() const
	{
		return ((cElementPrivate*)this)->canvas_;
	}

	Vec2 cElement$::pos_() const 
	{
		return ((cElementPrivate*)this)->pos_;
	}

	float cElement$::scl_() const
	{
		return ((cElementPrivate*)this)->scl_;
	}

	cElement$* cElement$::create$(void* data)
	{
		return new cElementPrivate(data);
	}
}
