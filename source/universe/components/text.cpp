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
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/default_style.h>

namespace flame
{
	struct cText$Private : cText$
	{
		cElement$* element;
		std::wstring text;

		cText$Private(void* data) :
			element(nullptr)
		{
			if (!data)
			{
				font_atlas_index = -1;
				color = default_style.text_color_normal;
				sdf_scale = default_style.sdf_scale;
			}
			else
			{
				auto src = (cTextArchive$*)data;
				font_atlas_index = src->font_atlas_index$;
				color = src->color$;
				sdf_scale = src->sdf_scale$;
			}
		}

		void on_attach()
		{
			element = (cElement$*)(entity->component(cH("Element")));
			assert(element);
		}

		void update(float delta_time)
		{
			element->canvas()->add_text(font_atlas_index, Vec2(element->global_x, element->global_y) + 
				Vec2(element->inner_padding[0], element->inner_padding[1]) * element->global_scale, 
				Bvec4(color, element->alpha), text.c_str(), sdf_scale * element->global_scale);
		}
	};

	cText$::~cText$()
	{
	}

	const char* cText$::type_name() const
	{
		return "Text";
	}

	uint cText$::type_hash() const
	{
		return cH("Text");
	}

	void cText$::on_attach()
	{
		((cText$Private*)this)->on_attach();
	}

	void cText$::update(float delta_time)
	{
		((cText$Private*)this)->update(delta_time);
	}

	const wchar_t* cText$::text() const
	{
		return ((cText$Private*)this)->text.c_str();
	}

	void cText$::set_text(const wchar_t* text)
	{
		((cText$Private*)this)->text = text;
	}

	cText$* cText$::create$(void* data)
	{
		return new cText$Private(data);
	}
}
