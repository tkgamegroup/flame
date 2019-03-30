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

		FLAME_UNIVERSE_EXPORTS Animation();
		FLAME_UNIVERSE_EXPORTS Animation(float duration, bool looping, const Function<AnimationParm>& f);

		FLAME_UNIVERSE_EXPORTS static Function<AnimationParm> moveto(const Vec2& pos_a, const Vec2& pos_b);
		FLAME_UNIVERSE_EXPORTS static Function<AnimationParm> fade(float alpha_a, float alpha_b);
	};
}
