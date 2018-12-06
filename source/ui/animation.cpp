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
		FLAME_DATA_PACKAGE_BEGIN(AnimationMovetoData, Widget::AnimationParm)
			FLAME_DATA_PACKAGE_CAPT(Vec2, pos_a, f2)
			FLAME_DATA_PACKAGE_CAPT(Vec2, pos_b, f2)
		FLAME_DATA_PACKAGE_END

		FLAME_REGISTER_FUNCTION_BEG(AnimationMoveto, FLAME_GID(21191), AnimationMovetoData)
			if (p.time() < 0.f)
			{
				p.thiz()->pos$ = p.pos_b();
				return;
			}

			p.thiz()->pos$ = p.pos_a() + (p.pos_b() - p.pos_a()) * (p.time() / p.duration());
		FLAME_REGISTER_FUNCTION_END(AnimationMoveto)

		void add_animation_moveto(Widget *w, float duration, const Vec2 &pos_a, const Vec2 &pos_b)
		{
			w->add_animation(duration, false, AnimationMoveto::v, { pos_a, pos_b });
		}

		FLAME_DATA_PACKAGE_BEGIN(AnimationFadeData, Widget::AnimationParm)
			FLAME_DATA_PACKAGE_CAPT(float, alpha_a, f1)
			FLAME_DATA_PACKAGE_CAPT(float, alpha_b, f1)
		FLAME_DATA_PACKAGE_END

		FLAME_REGISTER_FUNCTION_BEG(AnimationFade, FLAME_GID(9864), AnimationFadeData)
			if (p.time() < 0.f)
			{
				p.thiz()->alpha$ = p.alpha_b();
				return;
			}

			p.thiz()->alpha$ = p.alpha_a() + (p.alpha_b() - p.alpha_a()) * (p.time() / p.duration());
		FLAME_REGISTER_FUNCTION_END(AnimationFade)

		void add_animation_fade(Widget *w, float duration, float alpha_a, float alpha_b)
		{
			w->add_animation(duration, false, AnimationFade::v, { alpha_a, alpha_b });
		}
	}
}
