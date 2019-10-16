#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;
	struct cEventReceiver;
	struct cStyleColor;

	struct cToggle : Component
	{
		cEventReceiver* event_receiver;
		cStyleColor* style;

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

		FLAME_UNIVERSE_EXPORTS void* add_changed_listener(void (*listener)(void* c, bool toggled), const Mail<>& capture);

		FLAME_UNIVERSE_EXPORTS void remove_changed_listener(void* ret_by_add);

		FLAME_UNIVERSE_EXPORTS void set_toggled(bool toggled, bool trigg_changed = true);

		FLAME_UNIVERSE_EXPORTS virtual void on_enter_hierarchy(Component* c) override;

		FLAME_UNIVERSE_EXPORTS static cToggle* create();
	};
}
