// MIT License
// 
// Copyright (c) 2019 wjs
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
		cElementPrivate* p_element;
		graphics::Canvas* canvas;

		cElementPrivate(void* data) :
			p_element(nullptr),
			canvas(nullptr)
		{
			if (!data)
			{
				x = 0.f;
				y = 0.f;
				scale = 1.f;
				width = 0.;
				height = 0.f;

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
				p_element = (cElementPrivate*)(e->component(cH("Element")));
				if (p_element)
				{
					canvas = p_element->canvas;
					return;
				}
			}

			while (e)
			{
				auto c = (cUI$*)(e->component(cH("UI")));
				if (c)
				{
					canvas = c->canvas();
					break;
				}
				e = e->parent();
			}
		}

		void update(float delta_time)
		{
			if (!p_element)
			{
				if (x.frame > global_x.frame)
					global_x = x;
				if (y.frame > global_y.frame)
					global_y = y;
				if (scale.frame > global_scale.frame)
					global_scale = scale;
			}
			else
			{
				if (x.frame > global_x.frame || p_element->global_x.frame > global_x.frame || p_element->global_scale.frame > global_x.frame)
					global_x = p_element->global_x + p_element->global_scale * x;
				if (y.frame > global_y.frame || p_element->global_y.frame > global_y.frame || p_element->global_scale.frame > global_y.frame)
					global_y = p_element->global_y + p_element->global_scale * y;
				if (scale.frame > global_scale.frame || p_element->global_scale.frame > global_scale.frame)
					global_scale = p_element->global_scale * scale;
			}
			if (width.frame > global_width.frame || global_scale.frame > global_width.frame)
				global_width = width * global_scale;
			if (height.frame > global_height.frame || global_scale.frame > global_height.frame)
				global_height = height * global_scale;

			if (canvas)
			{
				auto p = Vec2(global_x, global_y) - (Vec2(background_offset[0], background_offset[1])) * global_scale;
				auto s = Vec2(global_width, global_height) + (Vec2(background_offset[0] + background_offset[2], background_offset[1] + background_offset[3])) * global_scale;
				auto rr = background_round_radius * global_scale;

				if (background_shadow_thickness > 0.f)
				{
					canvas->add_rect_col2(p - Vec2(background_shadow_thickness * 0.5f), s + Vec2(background_shadow_thickness), Bvec4(0, 0, 0, 128), Bvec4(0),
						background_shadow_thickness, rr, background_round_flags);
				}
				if (alpha > 0.f)
				{
					if (background_color.w() > 0)
						canvas->add_rect_filled(p, s, Bvec4(background_color, alpha), rr, background_round_flags);
					if (background_frame_thickness > 0.f && background_frame_color.w() > 0)
						canvas->add_rect(p, s, Bvec4(background_frame_color, alpha), background_frame_thickness, rr, background_round_flags);
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
		return ((cElementPrivate*)this)->canvas;
	}

	cElement$* cElement$::create$(void* data)
	{
		return new cElementPrivate(data);
	}
}
