#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;
	struct cText;
	struct cEventReceiver;

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
