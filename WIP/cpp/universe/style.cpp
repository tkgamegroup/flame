#include <flame/foundation/foundation.h>
#include <flame/universe/element.h>

using namespace flame;

namespace flame
{
	FLAME_PACKAGE_BEGIN_2(StyleBackgroundOffsetData, Vec4, active_offset, f4, Vec4, else_offset, f4)
	FLAME_PACKAGE_END_2

	void style_background_offset(StyleParm& p)
	{
		auto e = p.e();
		auto& c = p.get_capture<StyleBackgroundOffsetData>();
		switch (e->state)
		{
		case StateNormal: case StateHovering:
			e->background_offset$ = c.else_offset();
			break;
		case StateActive:
			e->background_offset$ = c.active_offset();
			break;
		}
	}

	Function<StyleParm> Style::background_offset(const Vec4& active_offset, const Vec4& else_offset)
	{
		return Function<StyleParm>(style_background_offset, { active_offset, else_offset });
	}

	FLAME_PACKAGE_BEGIN_2(StyleTextColorData, Bvec4, normal_color, b4, Bvec4, else_color, b4)
	FLAME_PACKAGE_END_2

	void style_text_color(StyleParm& p)
	{
		auto e = (wTextPtr)p.e();
		auto& c = p.get_capture<StyleTextColorData>();
		switch (e->state)
		{
		case StateNormal:
			e->text_col() = c.normal_color();
			break;
		case StateHovering: case StateActive:
			e->text_col() = c.else_color();
			break;
		}
	}

	Function<StyleParm> Style::text_color(const Bvec4& normal_col, const Bvec4& else_col)
	{
		return Function<StyleParm>(style_text_color, { normal_col, else_col });
	}
}

