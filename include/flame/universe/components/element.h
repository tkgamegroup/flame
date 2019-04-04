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

#pragma once

#include <flame/math.h>
#include <flame/universe/component.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
	}

	/*
			   pos                        size.x
				   +------------------------------------------------
				   |	              top inner padding
				   |			****************************
				   |	 left   *                          *  right
			size.y |	 inner  *          content         *  inner
				   |	padding *                          * padding
				   |	        ****************************
				   |			     bottom inner padding
	*/

	struct cElement$ : Component
	{
		Vec2 pos;
		float scale;
		Vec2 size;

		Vec4 inner_padding; // L T R B
		float layout_padding;

		float alpha;

		Vec4 background_offset; // L T R B
		float background_round_radius;
		int background_round_flags;
		float background_frame_thickness;
		Bvec4 background_color;
		Bvec4 background_frame_color;
		float background_shadow_thickness;

		FLAME_UNIVERSE_EXPORTS virtual ~cElement$() override;

		FLAME_UNIVERSE_EXPORTS virtual const char* type_name() const override;
		FLAME_UNIVERSE_EXPORTS virtual uint type_hash() const override;

		FLAME_UNIVERSE_EXPORTS virtual void on_attach() override;

		FLAME_UNIVERSE_EXPORTS virtual void update(float delta_time) override;

		FLAME_UNIVERSE_EXPORTS graphics::Canvas* canvas() const;
		FLAME_UNIVERSE_EXPORTS Vec2 pos_() const;
		FLAME_UNIVERSE_EXPORTS float scl_() const;

		FLAME_UNIVERSE_EXPORTS static cElement$* create$(void* data);
	};
}
