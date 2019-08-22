#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cList : Component
	{
		Entity* select;

		cList() :
			Component("List")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual ~cList() override;

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cList* create();
	};
}
