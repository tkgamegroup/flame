#pragma once

#include <flame/universe/system.h>

namespace flame
{
	struct cLayout;

	struct sTypeSetting : System
	{
		inline static auto type_name = "sTypeSetting";
		inline static auto type_hash = ch(type_name);

		struct AutoSizer
		{
			bool pending_sizing = false;

			virtual Vec2f measure() = 0;
		};

		sTypeSetting() :
			System(type_name, type_hash)
		{
		}

		virtual void add_to_sizing_list(AutoSizer* s, Entity* e) = 0;
		virtual void remove_from_sizing_list(AutoSizer* s) = 0;
		virtual void add_to_layouting_list(cLayout* l) = 0;
		virtual void remove_from_layouting_list(cLayout* l) = 0;

		FLAME_UNIVERSE_EXPORTS static sTypeSetting* create();
	};
}
