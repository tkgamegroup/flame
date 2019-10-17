#pragma once

#include <flame/universe/component.h>

namespace flame
{
	namespace graphics
	{
		struct FontAtlas;
	}

	struct cElement;
	struct cText;
	struct cEventReceiver;

	struct cEdit : Component
	{
		cElement* element;
		cText* text;
		cEventReceiver* event_receiver;

		uint cursor;

		cEdit() :
			Component("Edit")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual void on_component_added(Component* c) override;
		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cEdit* create();
	};

	FLAME_UNIVERSE_EXPORTS Entity* create_standard_edit(float width, graphics::FontAtlas* font_atlas, float sdf_scale);
}
