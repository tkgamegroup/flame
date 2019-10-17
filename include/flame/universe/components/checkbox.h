#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;
	struct cEventReceiver;
	struct cStyleColor;

	struct cCheckbox : Component
	{
		cElement* element;
		cEventReceiver* event_receiver;
		cStyleColor* style;

		bool checked;

		Vec4c unchecked_color_normal;
		Vec4c unchecked_color_hovering;
		Vec4c unchecked_color_active;
		Vec4c checked_color_normal;
		Vec4c checked_color_hovering;
		Vec4c checked_color_active;

		cCheckbox() :
			Component("Checkbox")
		{
		}

		FLAME_UNIVERSE_EXPORTS void* add_changed_listener(void (*listener)(void* c, bool checked), const Mail<>& capture);

		FLAME_UNIVERSE_EXPORTS void remove_changed_listener(void* ret_by_add);

		FLAME_UNIVERSE_EXPORTS void set_checked(bool checked, bool trigger_changed = true);

		FLAME_UNIVERSE_EXPORTS virtual void on_component_added(Component* c) override;
		FLAME_UNIVERSE_EXPORTS static cCheckbox* create();
	};

	FLAME_UNIVERSE_EXPORTS Entity* create_standard_checkbox();
}
