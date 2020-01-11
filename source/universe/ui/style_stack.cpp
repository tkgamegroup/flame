#include <flame/universe/ui/style_stack.h>

namespace flame
{
	namespace ui
	{
		std::stack<StyleValue> _styles[StyleCount];

		const StyleValue& style(Style style)
		{
			return _styles[style].top();
		}

		void set_style(Style style, const StyleValue& v)
		{
			_styles[style].top() = v;
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
			set_style(FontSize, StyleValue(14U));
			set_style(TextColorNormal, StyleValue(Vec4c(0, 0, 0, 255)));
			set_style(TextColorElse, StyleValue(Vec4c(255, 255, 255, 255)));
			set_style(WindowColor, StyleValue(Vec4c(0.94f * 255.f, 0.94f * 255.f, 0.94f * 255.f, 1.00f * 255.f)));
			set_style(FrameColorNormal, StyleValue(Vec4c(255, 255, 255, 255)));
			set_style(FrameColorHovering, StyleValue(Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 0.40f * 255.f)));
			set_style(FrameColorActive, StyleValue(Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 0.67f * 255.f)));
			set_style(ButtonColorNormal, StyleValue(Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 0.40f * 255.f)));
			set_style(ButtonColorHovering, StyleValue(Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 1.00f * 255.f)));
			set_style(ButtonColorActive, StyleValue(Vec4c(color(Vec3f(45.f, 0.73f, 0.97f)), 1.00f * 255.f)));
			set_style(HeaderColorNormal, StyleValue(Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 0.31f * 255.f)));
			set_style(HeaderColorHovering, StyleValue(Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 0.80f * 255.f)));
			set_style(HeaderColorActive, StyleValue(Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 1.00f * 255.f)));
			set_style(SelectedColorNormal, StyleValue(Vec4c(color(Vec3f(22.f, 0.73f, 0.97f)), 0.31f * 255.f)));
			set_style(SelectedColorHovering, StyleValue(Vec4c(color(Vec3f(22.f, 0.73f, 0.97f)), 0.80f * 255.f)));
			set_style(SelectedColorActive, StyleValue(Vec4c(color(Vec3f(22.f, 0.73f, 0.97f)), 1.00f * 255.f)));
			set_style(UncheckedColorNormal, StyleValue(Vec4c(0, 0, 0, 0)));
			set_style(UncheckedColorHovering, StyleValue(Vec4c(135, 140, 145, 128)));
			set_style(UncheckedColorActive, StyleValue(Vec4c(115, 120, 125, 128)));
			set_style(CheckedColorNormal, StyleValue(Vec4c(40, 100, 145, 255)));
			set_style(CheckedColorHovering, StyleValue(Vec4c(75, 135, 180, 255)));
			set_style(CheckedColorActive, StyleValue(Vec4c(55, 115, 160, 255)));
			set_style(ScrollbarColor, StyleValue(Vec4c(232, 232, 236, 255)));
			set_style(ScrollbarThumbColorNormal, StyleValue(Vec4c(194, 195, 201, 255)));
			set_style(ScrollbarThumbColorHovering, StyleValue(Vec4c(104, 104, 104, 255)));
			set_style(ScrollbarThumbColorActive, StyleValue(Vec4c(91, 91, 91, 255)));
			set_style(TabColorNormal, StyleValue(Vec4c(64, 86, 141, 255)));
			set_style(TabColorElse, StyleValue(Vec4c(187, 198, 241, 255)));
			set_style(TabTextColorNormal, StyleValue(Vec4c(255, 255, 255, 255)));
			set_style(TabTextColorElse, StyleValue(Vec4c(0, 0, 0, 255)));
			set_style(SelectedTabColorNormal, StyleValue(Vec4c(245, 205, 132, 255)));
			set_style(SelectedTabColorElse, StyleValue(Vec4c(245, 205, 132, 255)));
			set_style(SelectedTabTextColorNormal, StyleValue(Vec4c(0, 0, 0, 255)));
			set_style(SelectedTabTextColorElse, StyleValue(Vec4c(0, 0, 0, 255)));
			set_style(DockerColor, StyleValue(Vec4c(93, 107, 153, 255)));
		}

		void style_set_to_dark()
		{
			set_style(FontSize, StyleValue(14U));
			set_style(TextColorNormal, StyleValue(Vec4c(255, 255, 255, 255)));
			set_style(TextColorElse, StyleValue(Vec4c(180, 180, 180, 255)));
			set_style(WindowColor, StyleValue(Vec4c(0.06f * 255.f, 0.06f * 255.f, 0.06f * 255.f, 0.94f * 255.f)));
			set_style(FrameColorNormal, StyleValue(Vec4c(color(Vec3f(55.f, 0.67f, 0.47f)), 0.54f * 255.f)));
			set_style(FrameColorHovering, StyleValue(Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 0.40f * 255.f)));
			set_style(FrameColorActive, StyleValue(Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 0.67f * 255.f)));
			set_style(ButtonColorNormal, StyleValue(Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 0.40f * 255.f)));
			set_style(ButtonColorHovering, StyleValue(Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 1.00f * 255.f)));
			set_style(ButtonColorActive, StyleValue(Vec4c(color(Vec3f(49.f, 0.93f, 0.97f)), 1.00f * 255.f)));
			set_style(HeaderColorNormal, StyleValue(Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 0.31f * 255.f)));
			set_style(HeaderColorHovering, StyleValue(Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 0.80f * 255.f)));
			set_style(HeaderColorActive, StyleValue(Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 1.00f * 255.f)));
			set_style(SelectedColorNormal, StyleValue(Vec4c(color(Vec3f(22.f, 0.73f, 0.97f)), 0.31f * 255.f)));
			set_style(SelectedColorHovering, StyleValue(Vec4c(color(Vec3f(22.f, 0.73f, 0.97f)), 0.80f * 255.f)));
			set_style(SelectedColorActive, StyleValue(Vec4c(color(Vec3f(22.f, 0.73f, 0.97f)), 1.00f * 255.f)));
			set_style(UncheckedColorNormal, StyleValue(Vec4c(0, 0, 0, 0)));
			set_style(UncheckedColorHovering, StyleValue(Vec4c(115, 120, 125, 128)));
			set_style(UncheckedColorActive, StyleValue(Vec4c(135, 140, 145, 128)));
			set_style(CheckedColorNormal, StyleValue(Vec4c(110, 115, 255, 255)));
			set_style(CheckedColorHovering, StyleValue(Vec4c(75, 80, 220, 255)));
			set_style(CheckedColorActive, StyleValue(Vec4c(95, 100, 240, 255)));
			set_style(ScrollbarColor, StyleValue(Vec4c(62, 62, 66, 255)));
			set_style(ScrollbarThumbColorNormal, StyleValue(Vec4c(104, 104, 104, 255)));
			set_style(ScrollbarThumbColorHovering, StyleValue(Vec4c(158, 158, 158, 255)));
			set_style(ScrollbarThumbColorActive, StyleValue(Vec4c(239, 235, 239, 255)));
			set_style(TabColorNormal, StyleValue(Vec4c(0, 0, 0, 0)));
			set_style(TabColorElse, StyleValue(Vec4c(28, 151, 234, 255)));
			set_style(TabTextColorNormal, StyleValue(Vec4c(255, 255, 255, 255)));
			set_style(TabTextColorElse, StyleValue(Vec4c(255, 255, 255, 255)));
			set_style(SelectedTabColorNormal, StyleValue(Vec4c(0, 122, 204, 255)));
			set_style(SelectedTabColorElse, StyleValue(Vec4c(0, 122, 204, 255)));
			set_style(SelectedTabTextColorNormal, StyleValue(Vec4c(255, 255, 255, 255)));
			set_style(SelectedTabTextColorElse, StyleValue(Vec4c(255, 255, 255, 255)));
			set_style(DockerColor, StyleValue(Vec4c(45, 45, 48, 255)));
		}

		struct _InitStyle
		{
			_InitStyle()
			{
				for (auto& s : _styles)
					s.push(StyleValue());
				style_set_to_dark();
			}
		};
		static _InitStyle _init_style;
	}
}
