#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cList : Component // R
	{
		inline static auto type_name = "cList";
		inline static auto type_hash = ch(type_name);

		cList() :
			Component(type_name, type_hash)
		{
		}

		//		FLAME_UNIVERSE_EXPORTS void set_selected(Entity* e);
		//
		FLAME_UNIVERSE_EXPORTS static cList* create();
	};

	struct cListItem : Component // R
	{
		inline static auto type_name = "cListItem";
		inline static auto type_hash = ch(type_name);

		cListItem() :
			Component(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static cListItem* create();
	};
}
