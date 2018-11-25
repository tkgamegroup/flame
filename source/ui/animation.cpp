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

#include <flame/ui/widget.h>
#include <flame/ui/animation.h>

namespace flame
{
	namespace ui
	{
		FLAME_REGISTER_FUNCTION_BEG(Animation_moveto, FLAME_GID(21191), "p f")
			auto &w = *(Widget**)&d[0].p();
			auto &t = d[1].f1();
			auto &duration = d[2].f1();
			auto &pos_a = d[4].f2();
			auto &pos_b = d[5].f2();

			if (t < 0.f)
			{
				w->pos$ = pos_b;
				return;
			}

			w->pos$ = pos_a + (pos_b - pos_a) * (t / duration);
		FLAME_REGISTER_FUNCTION_END(Animation_moveto)

		void add_animation_moveto(Widget *w, float duration, const Vec2 &pos_a, const Vec2 &pos_b)
		{
			w->add_animation(Animation_moveto::v, { duration, 0, pos_a, pos_b });
		}

		FLAME_REGISTER_FUNCTION_BEG(Animation_fade, FLAME_GID(9864), "p f")
			auto &w = *(Widget**)&d[0].p();
			auto &t = d[1].f1();
			auto &duration = d[2].f1();
			auto &alpha_a = d[4].f1();
			auto &alpha_b = d[5].f1();

			if (t < 0.f)
			{
				w->alpha$ = alpha_b;
				return;
			}

			w->alpha$ = alpha_a + (alpha_b - alpha_a) * (t / duration);
		FLAME_REGISTER_FUNCTION_END(Animation_fade)

		void add_animation_fade(Widget *w, float duration, float alpha_a, float alpha_b)
		{
			w->add_animation(Animation_fade::v, { duration, 0, alpha_a, alpha_b });
		}
	}
}
