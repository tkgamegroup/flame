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

		virtual const wchar_t* get_title() const = 0;
		virtual void set_title(const wchar_t* v) = 0;

		virtual void toggle_collapse() = 0;

		FLAME_UNIVERSE_EXPORTS static cTreeNode* create(void* parms = nullptr);
	};
}
