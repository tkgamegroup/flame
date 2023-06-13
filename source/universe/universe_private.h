#pragma once

#include "../foundation/typeinfo.h"
#include "universe.h"

namespace flame
{
	void resolve_address(const std::string& address, EntityPtr e, const Attribute*& attr, voidptr& obj, uint& index);

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
