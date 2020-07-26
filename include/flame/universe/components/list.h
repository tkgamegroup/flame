#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct FLAME_RU(cList : Component, all)
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

	struct FLAME_RU(cListItem : Component, all)
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
