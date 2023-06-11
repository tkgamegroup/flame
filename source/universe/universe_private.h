#pragma once

#include "universe.h"

namespace flame
{
	struct ModifierPrivate
	{
		const Attribute* attr = nullptr;
		void* obj = nullptr;
		std::unique_ptr<Expression> expr = nullptr;

		ModifierPrivate(const Modifier& m, EntityPtr e, 
			const std::vector<std::pair<const char*, float*>>& extra_variables = {}, 
			const std::vector<std::pair<const char*, float>>& extra_consts = {});
		void update(bool first_time);
	};
}
