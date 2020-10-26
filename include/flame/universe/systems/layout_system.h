#pragma once

#include <flame/universe/system.h>

namespace flame
{
	struct cElement;
	struct cLayout;

	struct sLayoutSystem : System
	{
		inline static auto type_name = "flame::sLayoutSystem";
		inline static auto type_hash = ch(type_name);

		sLayoutSystem() :
			System(type_name, type_hash)
		{
		}

		virtual void add_to_sizing_list(cElement* e) = 0;
		virtual void remove_from_sizing_list(cElement* e) = 0;
		virtual void add_to_layouting_list(cLayout* l) = 0;
		virtual void remove_from_layouting_list(cLayout* l) = 0;

		FLAME_UNIVERSE_EXPORTS static sLayoutSystem* create();
	};
}
