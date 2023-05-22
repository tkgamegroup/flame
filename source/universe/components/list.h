#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cList : Component
	{
		// Reflect
		std::filesystem::path prefab_name;
		// Reflect
		virtual void set_prefab_name(const std::filesystem::path& name) = 0;

		// Reflect
		uint count = 0;
		// Reflect
		virtual void set_count(uint count) = 0;

		// Reflect
		std::vector<std::pair<std::string, std::string>> modifiers; // address(component_name|attribute_chain), expression (use i for item index)
		// Reflect
		virtual void set_modifiers(const std::vector<std::pair<std::string, std::string>>& modifiers) = 0;

		struct Create
		{
			virtual cListPtr operator()(EntityPtr) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
