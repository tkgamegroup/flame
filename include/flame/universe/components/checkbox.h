#pragma once

#include <flame/universe/component.h>

namespace flame
{
	namespace graphics
	{
		struct FontAtlas;
	}

	struct cElement;
	struct cEventReceiver;

	struct cCheckbox : Component
	{
		cElement* element;
		cEventReceiver* event_receiver;

		bool checked;

		cCheckbox() :
			Component("Checkbox")
		{
		}

		FLAME_UNIVERSE_EXPORTS void* add_changed_listener(void (*listener)(void* c, bool checked), const Mail<>& capture);

		FLAME_UNIVERSE_EXPORTS void remove_changed_listener(void* ret_by_add);

		FLAME_UNIVERSE_EXPORTS void set_checked(bool checked, bool trigg_changed = true);

		FLAME_UNIVERSE_EXPORTS virtual void start() override;
		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cCheckbox* create();
	};

	FLAME_UNIVERSE_EXPORTS Entity* create_standard_checkbox(graphics::FontAtlas* font_atlas, float sdf_scale, const std::wstring& text, bool checked);
}
