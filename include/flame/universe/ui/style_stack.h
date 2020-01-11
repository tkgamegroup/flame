#pragma once

#include <flame/math.h>
#include <flame/universe/universe.h>

namespace flame
{
	namespace ui
	{
		enum Style
		{
			FontSize, // uint
			TextColorNormal, // Vec4c
			TextColorElse, // Vec4c
			WindowColor, // Vec4c
			FrameColorNormal, // Vec4c
			FrameColorHovering, // Vec4c
			FrameColorActive, // Vec4c
			ButtonColorNormal, // Vec4c
			ButtonColorHovering, // Vec4c
			ButtonColorActive, // Vec4c
			HeaderColorNormal, // Vec4c
			HeaderColorHovering, // Vec4c
			HeaderColorActive, // Vec4c
			SelectedColorNormal, // Vec4c
			SelectedColorHovering, // Vec4c
			SelectedColorActive, // Vec4c
			UncheckedColorNormal, // Vec4c
			UncheckedColorHovering, // Vec4c
			UncheckedColorActive, // Vec4c
			CheckedColorNormal, // Vec4c
			CheckedColorHovering, // Vec4c
			CheckedColorActive, // Vec4c
			ScrollbarColor, // Vec4c
			ScrollbarThumbColorNormal, // Vec4c
			ScrollbarThumbColorHovering, // Vec4c
			ScrollbarThumbColorActive, // Vec4c
			TabColorNormal, // Vec4c
			TabColorElse, // Vec4c
			TabTextColorNormal, // Vec4c
			TabTextColorElse, // Vec4c
			SelectedTabColorNormal, // Vec4c
			SelectedTabColorElse, // Vec4c
			SelectedTabTextColorNormal, // Vec4c
			SelectedTabTextColorElse, // Vec4c
			DockerColor, // Vec4c

			StyleCount
		};

		struct StyleValue
		{
			union
			{
				Vec4u u;
				Vec4i i;
				Vec4f f;
				Vec4c c;
			}v;

			StyleValue()
			{
				v.u = Vec4u(0);
			}

			StyleValue(uint _v)
			{
				v.u = Vec4u(_v, 0, 0, 0);
			}

			StyleValue(const Vec4c& _v)
			{
				v.c = _v;
			}

			const Vec4u& u() const
			{
				return v.u;
			}

			const Vec4i& i() const
			{
				return v.i;
			}

			const Vec4f& f() const
			{
				return v.f;
			}

			const Vec4c& c() const
			{
				return v.c;
			}
		};

		FLAME_UNIVERSE_EXPORTS const StyleValue& style(Style style);
		FLAME_UNIVERSE_EXPORTS void set_style(Style style, const StyleValue& v);
		FLAME_UNIVERSE_EXPORTS void push_style(Style style, const StyleValue& v);
		FLAME_UNIVERSE_EXPORTS void pop_style(Style style);
		FLAME_UNIVERSE_EXPORTS void style_set_to_light();
		FLAME_UNIVERSE_EXPORTS void style_set_to_dark();
	}
}
