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

#include <flame/universe/universe.h>

namespace flame
{
	struct Element;
	typedef Element* ElementPtr;

	struct Animation;
	typedef Animation* AnimationPtr;

	FLAME_PACKAGE_BEGIN_2(AnimationParm, AnimationPtr, thiz, p, ElementPtr, e, p)
	FLAME_PACKAGE_END_2

	struct Animation : R
	{
		float time;
		float duration$;
		bool looping$;
		Function<AnimationParm> f$;

		Animation()
		{
		}

		Animation(float duration, bool looping, const Function<AnimationParm>& f) :
			time(0.f),
			duration$(duration),
			looping$(looping),
			f$(f)
		{
		}

		FLAME_UNIVERSE_EXPORTS static Function<AnimationParm> moveto(const Vec2& pos_a, const Vec2& pos_b);
		FLAME_UNIVERSE_EXPORTS static Function<AnimationParm> fade(float alpha_a, float alpha_b);
	};
}
