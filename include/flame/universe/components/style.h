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

		FLAME_UNIVERSE_EXPORTS static cStyleColor* create(const Vec4c& color_normal = Vec4c(0), const Vec4c& color_hovering = Vec4c(0), const Vec4c& color_active = Vec4c(0));
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

		FLAME_UNIVERSE_EXPORTS static cStyleTextColor* create(const Vec4c& color_normal = Vec4c(0), const Vec4c& color_else = Vec4c(0));
	};
}
