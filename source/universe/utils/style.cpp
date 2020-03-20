#include <flame/universe/utils/style.h>

namespace flame
{
	namespace utils
	{
		std::stack<StyleValue> _styles[StyleCount];

		const StyleValue& style(Style style)
		{
			return _styles[style].top();
		}

		void push_style(Style style, const StyleValue& v)
		{
			_styles[style].push(v);
		}

		void pop_style(Style style)
		{
			_styles[style].pop();
		}

		void style_set_to_light()
		{
			pop_style(FontSize); push_style_1u(FontSize, 14U);
			pop_style(BackgroundColor); push_style_4c(BackgroundColor, 240, 240, 240, 255);
			pop_style(ForegroundColor); push_style_4c(ForegroundColor, 0, 0, 0, 255);
			pop_style(TextColorNormal); push_style_4c(TextColorNormal, 0, 0, 0, 255);
			pop_style(TextColorElse); push_style_4c(TextColorElse, 0, 120, 205, 255);
			pop_style(FrameColorNormal); push_style_4c(FrameColorNormal, 220, 220, 220, 255);
			pop_style(FrameColorHovering); push_style_4c(FrameColorHovering, 230, 230, 230, 255);
			pop_style(FrameColorActive); push_style_4c(FrameColorActive, 225, 225, 225, 255);
			pop_style(ButtonColorNormal); push_style_4c(ButtonColorNormal, 5, 170, 245, 255);
			pop_style(ButtonColorHovering); push_style_4c(ButtonColorHovering, 5, 155, 230, 255);
			pop_style(ButtonColorActive); push_style_4c(ButtonColorActive, 0, 135, 210, 255);
			pop_style(SelectedColorNormal); push_style_4c(SelectedColorNormal, 200, 220, 245, 255);
			pop_style(SelectedColorHovering); push_style_4c(SelectedColorHovering, 205, 225, 250, 255);
			pop_style(SelectedColorActive); push_style_4c(SelectedColorActive, 195, 215, 240, 255);
			pop_style(ScrollbarColor); push_style_4c(ScrollbarColor, 245, 245, 245, 255);
			pop_style(ScrollbarThumbColorNormal); push_style_4c(ScrollbarThumbColorNormal, 194, 195, 201, 255);
			pop_style(ScrollbarThumbColorHovering); push_style_4c(ScrollbarThumbColorHovering, 104, 104, 104, 255);
			pop_style(ScrollbarThumbColorActive); push_style_4c(ScrollbarThumbColorActive, 91, 91, 91, 255);
			pop_style(TabColorNormal); push_style_4c(TabColorNormal, 0, 0, 0, 0);
			pop_style(TabColorElse); push_style_4c(TabColorElse, 28, 151, 234, 255);
			pop_style(TabTextColorNormal); push_style_4c(TabTextColorNormal, 0, 0, 0, 255);
			pop_style(TabTextColorElse); push_style_4c(TabTextColorElse, 255, 255, 255, 255);
			pop_style(SelectedTabColorNormal); push_style_4c(SelectedTabColorNormal, 0, 122, 204, 255);
			pop_style(SelectedTabColorElse); push_style_4c(SelectedTabColorElse, 0, 122, 204, 255);
			pop_style(SelectedTabTextColorNormal); push_style_4c(SelectedTabTextColorNormal, 255, 255, 255, 255);
			pop_style(SelectedTabTextColorElse); push_style_4c(SelectedTabTextColorElse, 255, 255, 255, 255);
		}

		void style_set_to_dark()
		{
			pop_style(FontSize); push_style_1u(FontSize, 14U);
			pop_style(BackgroundColor); push_style_4c(BackgroundColor, 40, 40, 40, 255);
			pop_style(ForegroundColor); push_style_4c(ForegroundColor, 255, 255, 255, 255);
			pop_style(TextColorNormal); push_style_4c(TextColorNormal, 255, 255, 255, 255);
			pop_style(TextColorElse); push_style_4c(TextColorElse, 180, 180, 180, 255);
			pop_style(FrameColorNormal); push_style_4c(FrameColorNormal, 55, 55, 55, 255);
			pop_style(FrameColorHovering); push_style_4c(FrameColorHovering, 65, 65, 65, 255);
			pop_style(FrameColorActive); push_style_4c(FrameColorActive, 60, 60, 60, 255);
			pop_style(ButtonColorNormal); push_style_4c(ButtonColorNormal, 5, 170, 245, 255);
			pop_style(ButtonColorHovering); push_style_4c(ButtonColorHovering, 5, 155, 230, 255);
			pop_style(ButtonColorActive); push_style_4c(ButtonColorActive, 0, 135, 210, 255);
			pop_style(SelectedColorNormal); push_style_4c(SelectedColorNormal, 100, 110, 125, 255);
			pop_style(SelectedColorHovering); push_style_4c(SelectedColorHovering, 105, 115, 130, 255);
			pop_style(SelectedColorActive); push_style_4c(SelectedColorActive, 90, 105, 120, 255);
			pop_style(ScrollbarColor); push_style_4c(ScrollbarColor, 62, 62, 66, 255);
			pop_style(ScrollbarThumbColorNormal); push_style_4c(ScrollbarThumbColorNormal, 104, 104, 104, 255);
			pop_style(ScrollbarThumbColorHovering); push_style_4c(ScrollbarThumbColorHovering, 158, 158, 158, 255);
			pop_style(ScrollbarThumbColorActive); push_style_4c(ScrollbarThumbColorActive, 239, 235, 239, 255);
			pop_style(TabColorNormal); push_style_4c(TabColorNormal, 0, 0, 0, 0);
			pop_style(TabColorElse); push_style_4c(TabColorElse, 28, 151, 234, 255);
			pop_style(TabTextColorNormal); push_style_4c(TabTextColorNormal, 255, 255, 255, 255);
			pop_style(TabTextColorElse); push_style_4c(TabTextColorElse, 255, 255, 255, 255);
			pop_style(SelectedTabColorNormal); push_style_4c(SelectedTabColorNormal, 0, 122, 204, 255);
			pop_style(SelectedTabColorElse); push_style_4c(SelectedTabColorElse, 0, 122, 204, 255);
			pop_style(SelectedTabTextColorNormal); push_style_4c(SelectedTabTextColorNormal, 255, 255, 255, 255);
			pop_style(SelectedTabTextColorElse); push_style_4c(SelectedTabTextColorElse, 255, 255, 255, 255);
		}

		struct _InitStyle
		{
			_InitStyle()
			{
				StyleValue sv;
				for (auto& s : _styles)
					s.push(sv);
				style_set_to_dark();
			}
		};
		static _InitStyle _init_style;
	}
}
