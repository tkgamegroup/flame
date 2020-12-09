#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cList : Component
	{
		inline static auto type_name = "flame::cList";
		inline static auto type_hash = ch(type_name);

		cList() :
			Component(type_name, type_hash)
		{
		}

		virtual Entity* get_selected() const = 0;
		virtual void set_selected(Entity* e) = 0;

		FLAME_UNIVERSE_EXPORTS static cList* create();
	};

	struct cListItem : Component
	{
		inline static auto type_name = "flame::cListItem";
		inline static auto type_hash = ch(type_name);

		cListItem() :
			Component(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static cListItem* create();
	};
}
