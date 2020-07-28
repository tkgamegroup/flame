#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cList : Component // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cList";
		inline static auto type_hash = S<ch("cList")>::v;

		cList() :
			Component(type_name, type_hash)
		{
		}

		//		FLAME_UNIVERSE_EXPORTS void set_selected(Entity* e);
		//
		FLAME_UNIVERSE_EXPORTS static cList* create();
	};

	struct cListItem : Component // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cListItem";
		inline static auto type_hash = S<ch("cListItem")>::v;

		cListItem() :
			Component(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static cListItem* create();
	};
}
