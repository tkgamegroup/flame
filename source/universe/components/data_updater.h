#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cDataUpdater : Component
	{
		// Reflect
		std::vector<std::pair<std::string, std::string>> items; // address(component_name|attribute_chain), expression
		// Reflect
		virtual void set_items(const std::vector<std::pair<std::string, std::string>>& items) = 0;

		struct Create
		{
			virtual cDataUpdaterPtr operator()(EntityPtr) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
