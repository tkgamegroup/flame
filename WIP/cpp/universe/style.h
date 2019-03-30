#include <flame/universe/universe.h>

namespace flame
{
	struct Element;
	typedef Element* ElementPtr;

	struct Style;
	typedef Style* StylePtr;

	FLAME_PACKAGE_BEGIN_3(StyleParm, StylePtr, thiz, p, ElementPtr, e, p, int, out_active, i1)
	FLAME_PACKAGE_END_3

	struct Style : R
	{
		int closet_id$;
		int level$;
		Function<StyleParm> f$;

		FLAME_UNIVERSE_EXPORTS Style();
		FLAME_UNIVERSE_EXPORTS Style(int closet_id, int level, const Function<StyleParm>& f);

		FLAME_UNIVERSE_EXPORTS static Function<StyleParm> background_offset(const Vec4& active_offset, const Vec4& else_offset);
		FLAME_UNIVERSE_EXPORTS static Function<StyleParm> background_color(const Bvec4& normal_col, const Bvec4& hovering_col, const Bvec4& active_col);
		FLAME_UNIVERSE_EXPORTS static Function<StyleParm> text_color(const Bvec4& normal_col, const Bvec4& else_col);
	};
}
