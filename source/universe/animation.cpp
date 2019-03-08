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
#include <flame/universe/element.h>

namespace flame
{
	Animation::Animation() :
		time(0.f),
		duration$(0.f),
		looping$(false)
	{
	}

	Animation::Animation(float duration, bool looping, const Function<AnimationParm>& f) :
		time(0.f),
		duration$(duration),
		looping$(looping),
		f$(f)
	{
	}

	FLAME_PACKAGE_BEGIN_2(AnimationMovetoData, Vec2, pos_a, f2, Vec2, pos_b, f2)
	FLAME_PACKAGE_END_2

	void animation_moveto(AnimationParm& p)
	{
		auto e = p.e();
		auto& thiz = *p.thiz();
		auto& c = p.get_capture<AnimationMovetoData>();

		if (thiz.time < 0.f)
		{
			e->pos$ = c.pos_b();
			return;
		}

		e->pos$ = c.pos_a() + (c.pos_b() - c.pos_a()) * (thiz.time / thiz.duration$);
	}

	Function<AnimationParm> Animation::moveto(const Vec2& pos_a, const Vec2& pos_b)
	{
		return Function<AnimationParm>(animation_moveto, { pos_a, pos_b });
	}

	FLAME_PACKAGE_BEGIN_2(AnimationFadeData, float, alpha_a, f1, float, alpha_b, f1)
	FLAME_PACKAGE_END_2

	void animation_fade(AnimationParm& p)
	{
		auto e = p.e();
		auto& thiz = *p.thiz();
		auto& c = p.get_capture<AnimationFadeData>();

		if (thiz.time < 0.f)
		{
			e->alpha$ = c.alpha_b();
			return;
		}

		e->alpha$ = c.alpha_a() + (c.alpha_b() - c.alpha_a()) * (thiz.time / thiz.duration$);
	}

	Function<AnimationParm> Animation::fade(float alpha_a, float alpha_b)
	{
		return Function<AnimationParm>(animation_fade, { alpha_a, alpha_b });
	}
}
