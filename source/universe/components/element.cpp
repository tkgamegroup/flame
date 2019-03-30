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

#include <flame/foundation/foundation.h>
#include <flame/universe/entity.h>
#include <flame/universe/components/element.h>

namespace flame
{
	struct cElementPrivate : cElement
	{
		graphics::Canvas* canvas_;

		cElementPrivate::cElementPrivate()
		{
			pos = Vec2(0.f);
			size = Vec2(0.f);

			alpha = 1.f;
			scale = 1.f;

			inner_padding = Vec4(0.f);
			layout_padding = 0.f;

			background_offset = Vec4(0.f);
			background_round_radius = 0.f;
			background_round_flags = 0;
			background_frame_thickness = 0.f;
			background_color = Bvec4(0);
			background_frame_color = Bvec4(255);
			background_shaow_thickness = 0.f;
		}

		void update(float delta_time)
		{
			canvas_ = nullptr;

		}
	};

	const char* cElement::type_name() const
	{
		return "Element";
	}

	uint cElement::type_hash() const
	{
		return cH("Element");
	}

	void cElement::update(float delta_time)
	{
		((cElementPrivate*)this)->update(delta_time);
	}

	cElement* cElement::create()
	{
		return new cElementPrivate();
	}
}
