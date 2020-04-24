#pragma once

#include <flame/universe/utils/ui.h>

#include <flame/universe/utils/entity_impl.h>

namespace flame
{
	namespace utils
	{
		static std::stack<graphics::FontAtlas*> _font_atlases;

		graphics::FontAtlas* current_font_atlas()
		{
			return _font_atlases.top();
		}

		void push_font_atlas(graphics::FontAtlas* font_atlas)
		{
			_font_atlases.push(font_atlas);
		}

		void pop_font_atlas()
		{
			_font_atlases.pop();
		}

		Vec2f next_element_pos = Vec2f(0.f);
		Vec2f next_element_size = Vec2f(0.f);
		Vec4f next_element_padding = Vec4f(0.f);
		Vec4f next_element_roundness = Vec4f(0.f);
		uint next_element_roundness_lod = 0;
		float next_element_frame_thickness = 0.f;
		Vec4c next_element_color = Vec4c(0);
		Vec4c next_element_frame_color = Vec4c(255);

		std::stack<CommonValue> _styles[StyleCount];

		const CommonValue& style(Style style)
		{
			return _styles[style].top();
		}

		void push_style(Style style, const CommonValue& v)
		{
			_styles[style].push(v);
		}

		void pop_style(Style style)
		{
			_styles[style].pop();
		}

		void style_set_to_light()
		{
			pop_style(FontSize); push_style(FontSize, common(Vec1u(16U)));
			pop_style(BackgroundColor); push_style(BackgroundColor, common(Vec4c(240, 240, 240, 255)));
			pop_style(ForegroundColor); push_style(ForegroundColor, common(Vec4c(0, 0, 0, 255)));
			pop_style(TextColorNormal); push_style(TextColorNormal, common(Vec4c(0, 0, 0, 255)));
			pop_style(TextColorElse); push_style(TextColorElse, common(Vec4c(0, 120, 205, 255)));
			pop_style(FrameColorNormal); push_style(FrameColorNormal, common(Vec4c(220, 220, 220, 255)));
			pop_style(FrameColorHovering); push_style(FrameColorHovering, common(Vec4c(230, 230, 230, 255)));
			pop_style(FrameColorActive); push_style(FrameColorActive, common(Vec4c(225, 225, 225, 255)));
			pop_style(ButtonColorNormal); push_style(ButtonColorNormal, common(Vec4c(86, 119, 252, 255)));
			pop_style(ButtonColorHovering); push_style(ButtonColorHovering, common(Vec4c(78, 108, 239, 255)));
			pop_style(ButtonColorActive); push_style(ButtonColorActive, common(Vec4c(69, 94, 222, 255)));
			pop_style(SelectedColorNormal); push_style(SelectedColorNormal, common(Vec4c(200, 220, 245, 255)));
			pop_style(SelectedColorHovering); push_style(SelectedColorHovering, common(Vec4c(205, 225, 250, 255)));
			pop_style(SelectedColorActive); push_style(SelectedColorActive, common(Vec4c(195, 215, 240, 255)));
			pop_style(ScrollbarColor); push_style(ScrollbarColor, common(Vec4c(245, 245, 245, 255)));
			pop_style(ScrollbarThumbColorNormal); push_style(ScrollbarThumbColorNormal, common(Vec4c(194, 195, 201, 255)));
			pop_style(ScrollbarThumbColorHovering); push_style(ScrollbarThumbColorHovering, common(Vec4c(104, 104, 104, 255)));
			pop_style(ScrollbarThumbColorActive); push_style(ScrollbarThumbColorActive, common(Vec4c(91, 91, 91, 255)));
			pop_style(TabColorNormal); push_style(TabColorNormal, common(Vec4c(0, 0, 0, 0)));
			pop_style(TabColorElse); push_style(TabColorElse, common(Vec4c(28, 151, 234, 255)));
			pop_style(TabTextColorNormal); push_style(TabTextColorNormal, common(Vec4c(0, 0, 0, 255)));
			pop_style(TabTextColorElse); push_style(TabTextColorElse, common(Vec4c(255, 255, 255, 255)));
			pop_style(SelectedTabColorNormal); push_style(SelectedTabColorNormal, common(Vec4c(0, 122, 204, 255)));
			pop_style(SelectedTabColorElse); push_style(SelectedTabColorElse, common(Vec4c(0, 122, 204, 255)));
			pop_style(SelectedTabTextColorNormal); push_style(SelectedTabTextColorNormal, common(Vec4c(255, 255, 255, 255)));
			pop_style(SelectedTabTextColorElse); push_style(SelectedTabTextColorElse, common(Vec4c(255, 255, 255, 255)));
		}

		void style_set_to_dark()
		{
			pop_style(FontSize); push_style(FontSize, common(Vec1u(16U)));
			pop_style(BackgroundColor); push_style(BackgroundColor, common(Vec4c(40, 40, 40, 255)));
			pop_style(ForegroundColor); push_style(ForegroundColor, common(Vec4c(255, 255, 255, 255)));
			pop_style(TextColorNormal); push_style(TextColorNormal, common(Vec4c(255, 255, 255, 255)));
			pop_style(TextColorElse); push_style(TextColorElse, common(Vec4c(180, 180, 180, 255)));
			pop_style(FrameColorNormal); push_style(FrameColorNormal, common(Vec4c(55, 55, 55, 255)));
			pop_style(FrameColorHovering); push_style(FrameColorHovering, common(Vec4c(65, 65, 65, 255)));
			pop_style(FrameColorActive); push_style(FrameColorActive, common(Vec4c(60, 60, 60, 255)));
			pop_style(ButtonColorNormal); push_style(ButtonColorNormal, common(Vec4c(86, 119, 252, 255)));
			pop_style(ButtonColorHovering); push_style(ButtonColorHovering, common(Vec4c(78, 108, 239, 255)));
			pop_style(ButtonColorActive); push_style(ButtonColorActive, common(Vec4c(69, 94, 222, 255)));
			pop_style(SelectedColorNormal); push_style(SelectedColorNormal, common(Vec4c(100, 110, 125, 255)));
			pop_style(SelectedColorHovering); push_style(SelectedColorHovering, common(Vec4c(105, 115, 130, 255)));
			pop_style(SelectedColorActive); push_style(SelectedColorActive, common(Vec4c(90, 105, 120, 255)));
			pop_style(ScrollbarColor); push_style(ScrollbarColor, common(Vec4c(62, 62, 66, 255)));
			pop_style(ScrollbarThumbColorNormal); push_style(ScrollbarThumbColorNormal, common(Vec4c(104, 104, 104, 255)));
			pop_style(ScrollbarThumbColorHovering); push_style(ScrollbarThumbColorHovering, common(Vec4c(158, 158, 158, 255)));
			pop_style(ScrollbarThumbColorActive); push_style(ScrollbarThumbColorActive, common(Vec4c(239, 235, 239, 255)));
			pop_style(TabColorNormal); push_style(TabColorNormal, common(Vec4c(0, 0, 0, 0)));
			pop_style(TabColorElse); push_style(TabColorElse, common(Vec4c(28, 151, 234, 255)));
			pop_style(TabTextColorNormal); push_style(TabTextColorNormal, common(Vec4c(255, 255, 255, 255)));
			pop_style(TabTextColorElse); push_style(TabTextColorElse, common(Vec4c(255, 255, 255, 255)));
			pop_style(SelectedTabColorNormal); push_style(SelectedTabColorNormal, common(Vec4c(0, 122, 204, 255)));
			pop_style(SelectedTabColorElse); push_style(SelectedTabColorElse, common(Vec4c(0, 122, 204, 255)));
			pop_style(SelectedTabTextColorNormal); push_style(SelectedTabTextColorNormal, common(Vec4c(255, 255, 255, 255)));
			pop_style(SelectedTabTextColorElse); push_style(SelectedTabTextColorElse, common(Vec4c(255, 255, 255, 255)));
		}

		struct _InitStyle
		{
			_InitStyle()
			{
				CommonValue sv;
				for (auto& s : _styles)
					s.push(sv);
				style_set_to_dark();
			}
		};
		static _InitStyle _init_style;
	}
}
