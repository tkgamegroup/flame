#pragma once

#include <flame/math.h>
#include <flame/universe/universe.h>

namespace flame
{
	namespace utils
	{
		enum Style
		{
			FontSize, // uint
			BackgroundColor, // Vec4c
			ForegroundColor, // Vec4c
			TextColorNormal, // Vec4c
			TextColorElse, // Vec4c
			FrameColorNormal, // Vec4c
			FrameColorHovering, // Vec4c
			FrameColorActive, // Vec4c
			ButtonColorNormal, // Vec4c
			ButtonColorHovering, // Vec4c
			ButtonColorActive, // Vec4c
			SelectedColorNormal, // Vec4c
			SelectedColorHovering, // Vec4c
			SelectedColorActive, // Vec4c
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

			StyleCount
		};

		union StyleValue
		{
			Vec4u u;
			Vec4i i;
			Vec4f f;
			Vec4c c;
		};

		FLAME_UNIVERSE_EXPORTS const StyleValue& style(Style s);

		inline uint style_1u(Style s)
		{
			return style(s).u[0];
		}

		inline const Vec4c& style_4c(Style s)
		{
			return style(s).c;
		}

		FLAME_UNIVERSE_EXPORTS void push_style(Style s, const StyleValue& v);

		inline void push_style_1u(Style s, uint x)
		{
			StyleValue sv;
			sv.u = Vec4c(x, 0, 0, 0);
			push_style(s, sv);
		}

		inline void push_style_4c(Style s, uchar x, uchar y, uchar z, uchar w)
		{
			StyleValue sv;
			sv.c = Vec4c(x, y, z, w);
			push_style(s, sv);
		}

		inline void push_style_4c(Style s, const Vec4c& v)
		{
			StyleValue sv;
			sv.c = v;
			push_style(s, sv);
		}

		FLAME_UNIVERSE_EXPORTS void pop_style(Style style);
		FLAME_UNIVERSE_EXPORTS void style_set_to_light();
		FLAME_UNIVERSE_EXPORTS void style_set_to_dark();
	}
}
