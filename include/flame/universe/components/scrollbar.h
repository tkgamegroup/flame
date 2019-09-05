#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;
	struct cEventReceiver;
	struct cLayout;

	struct cScrollbar : Component
	{
		cElement* element;
		cEventReceiver* event_receiver;

		cScrollbar() :
			Component("Scrollbar")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual ~cScrollbar() override;

		FLAME_UNIVERSE_EXPORTS virtual void start() override;
		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cScrollbar* create();
	};

	struct cScrollbarThumb : Component
	{
		cElement* element;
		cEventReceiver* event_receiver;
		cScrollbar* scrollbar;

		ScrollbarType type;
		cLayout* target_layout;

		cScrollbarThumb() :
			Component("ScrollbarThumb")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual ~cScrollbarThumb() override;

		FLAME_UNIVERSE_EXPORTS virtual void start() override;
		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cScrollbarThumb* create();
	};
}
