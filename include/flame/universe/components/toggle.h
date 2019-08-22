#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;
	struct cEventReceiver;
	struct cStyleBgCol;

	struct cToggle : Component
	{
		cElement* element;
		cEventReceiver* event_receiver;
		cStyleBgCol* style;

		bool toggled;

		Vec4c untoggled_color_normal;
		Vec4c untoggled_color_hovering;
		Vec4c untoggled_color_active;
		Vec4c toggled_color_normal;
		Vec4c toggled_color_hovering;
		Vec4c toggled_color_active;

		cToggle() :
			Component("Toggle")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual ~cToggle() override;

		FLAME_UNIVERSE_EXPORTS virtual void on_add_to_parent() override;

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cToggle* create();
	};
}
