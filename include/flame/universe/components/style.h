#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;
	struct cText;
	struct cEventReceiver;

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

	FLAME_UNIVERSE_EXPORTS extern const StyleValue& (*get_style)(Style s);

	struct cStyleColor : Component
	{
		cElement* element;
		cEventReceiver* event_receiver;

		Vec4c color_normal;
		Vec4c color_hovering;
		Vec4c color_active;

		cStyleColor() :
			Component("cStyleColor")
		{
		}

		FLAME_UNIVERSE_EXPORTS void style();

		FLAME_UNIVERSE_EXPORTS static cStyleColor* create();
	};

	struct cStyleColor2 : Component
	{
		cElement* element;
		cEventReceiver* event_receiver;

		uint level;
		Vec4c color_normal[2];
		Vec4c color_hovering[2];
		Vec4c color_active[2];

		cStyleColor2() :
			Component("cStyleColor2")
		{
		}

		FLAME_UNIVERSE_EXPORTS void style();

		FLAME_UNIVERSE_EXPORTS static cStyleColor2* create();
	};

	struct cStyleTextColor : Component
	{
		cText* text;
		cEventReceiver* event_receiver;

		Vec4c color_normal;
		Vec4c color_else;

		cStyleTextColor() :
			Component("cStyleTextColor")
		{
		}

		FLAME_UNIVERSE_EXPORTS void style();

		FLAME_UNIVERSE_EXPORTS static cStyleTextColor* create();
	};

	struct cStyleTextColor2 : Component
	{
		cText* text;
		cEventReceiver* event_receiver;

		uint level;
		Vec4c color_normal[2];
		Vec4c color_else[2];

		cStyleTextColor2() :
			Component("cStyleTextColor2")
		{
		}

		FLAME_UNIVERSE_EXPORTS void style();

		FLAME_UNIVERSE_EXPORTS static cStyleTextColor2* create();
	};
}
