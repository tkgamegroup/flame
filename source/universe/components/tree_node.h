#pragma once

#include "../component.h"

namespace flame
{
	struct cTreeNode : Component
	{
		inline static auto type_name = "flame::cTreeNode";
		inline static auto type_hash = ch(type_name);

		cTreeNode() : Component(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static cTreeNode* create(void* parms = nullptr);
	};
}
