#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;
	struct cEventReceiver;

	struct cStyleBgCol : Component
	{
		cElement* element;
		cEventReceiver* event_receiver;

		Vec4c col_normal;
		Vec4c col_hovering;
		Vec4c col_active;

		FLAME_UNIVERSE_EXPORTS cStyleBgCol(Entity* e);
		FLAME_UNIVERSE_EXPORTS virtual ~cStyleBgCol() override;

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cStyleBgCol* create(Entity* e, const Vec4c& col_normal, const Vec4c& col_hovering, const Vec4c& col_active);
	};
}
