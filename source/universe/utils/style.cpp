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
			pop_style(TextColorElse); push_style_4c(TextColorElse, 255, 255, 255, 255);
			pop_style(FrameColorNormal); push_style_4c(FrameColorNormal, 220, 220, 220, 255);
			pop_style(FrameColorHovering); push_style_4c(FrameColorHovering, 230, 230, 230, 255);
			pop_style(FrameColorActive); push_style_4c(FrameColorActive, 225, 225, 225, 255);
			pop_style(ButtonColorNormal); push_style_4c(ButtonColorNormal, hsv_2_col4(52.f, 0.73f, 0.97f, 0.40f * 255.f));
			pop_style(ButtonColorHovering); push_style_4c(ButtonColorHovering, hsv_2_col4(52.f, 0.73f, 0.97f, 1.00f * 255.f));
			pop_style(ButtonColorActive); push_style_4c(ButtonColorActive, hsv_2_col4(45.f, 0.73f, 0.97f, 1.00f * 255.f));
			pop_style(HeaderColorNormal); push_style_4c(HeaderColorNormal, hsv_2_col4(52.f, 0.73f, 0.97f, 0.31f * 255.f));
			pop_style(HeaderColorHovering); push_style_4c(HeaderColorHovering, hsv_2_col4(52.f, 0.73f, 0.97f, 0.80f * 255.f));
			pop_style(HeaderColorActive); push_style_4c(HeaderColorActive, hsv_2_col4(52.f, 0.73f, 0.97f, 1.00f * 255.f));
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
			pop_style(BackgroundColor); push_style_4c(BackgroundColor, 0, 0, 0, 255);
			pop_style(ForegroundColor); push_style_4c(ForegroundColor, 255, 255, 255, 255);
			pop_style(TextColorNormal); push_style_4c(TextColorNormal, 255, 255, 255, 255);
			pop_style(TextColorElse); push_style_4c(TextColorElse, 180, 180, 180, 255);
			pop_style(FrameColorNormal); push_style_4c(FrameColorNormal, 135, 135, 135, 255);
			pop_style(FrameColorHovering); push_style_4c(FrameColorHovering, 150, 150, 150, 255);
			pop_style(FrameColorActive); push_style_4c(FrameColorActive, 142, 142, 142, 255);
			pop_style(ButtonColorNormal); push_style_4c(ButtonColorNormal, hsv_2_col4(52.f, 0.73f, 0.97f, 0.40f * 255.f));
			pop_style(ButtonColorHovering); push_style_4c(ButtonColorHovering, hsv_2_col4(52.f, 0.73f, 0.97f, 1.00f * 255.f));
			pop_style(ButtonColorActive); push_style_4c(ButtonColorActive, hsv_2_col4(49.f, 0.93f, 0.97f, 1.00f * 255.f));
			pop_style(HeaderColorNormal); push_style_4c(HeaderColorNormal, hsv_2_col4(52.f, 0.73f, 0.97f, 0.31f * 255.f));
			pop_style(HeaderColorHovering); push_style_4c(HeaderColorHovering, hsv_2_col4(52.f, 0.73f, 0.97f, 0.80f * 255.f));
			pop_style(HeaderColorActive); push_style_4c(HeaderColorActive, hsv_2_col4(52.f, 0.73f, 0.97f, 1.00f * 255.f));
			pop_style(SelectedColorNormal); push_style_4c(SelectedColorNormal, hsv_2_col4(22.f, 0.73f, 0.97f, 0.31f * 255.f));
			pop_style(SelectedColorHovering); push_style_4c(SelectedColorHovering, hsv_2_col4(22.f, 0.73f, 0.97f, 0.80f * 255.f));
			pop_style(SelectedColorActive); push_style_4c(SelectedColorActive, hsv_2_col4(22.f, 0.73f, 0.97f, 1.00f * 255.f));
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
