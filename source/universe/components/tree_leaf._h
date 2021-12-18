#pragma once

#include "../component.h"

namespace flame
{
	struct cTreeLeaf : Component
	{
		inline static auto type_name = "flame::cTreeLeaf";
		inline static auto type_hash = ch(type_name);

		cTreeLeaf() : Component(type_name, type_hash)
		{
		}

		virtual const wchar_t* get_title() const = 0;
		virtual void set_title(const wchar_t* v) = 0;

		FLAME_UNIVERSE_EXPORTS static cTreeLeaf* create(void* parms = nullptr);
	};
}
