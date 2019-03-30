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
