#pragma once

#include "list.h"

namespace flame
{
	struct cListPrivate : cList
	{
		std::vector<EntityPtr> items;

		void set_prefab_name(const std::filesystem::path& name) override;
		void set_count(uint count) override;
		void set_modifiers(const std::vector<std::pair<std::string, std::string>>& modifiers) override;

		void refresh_items();
	};
}
