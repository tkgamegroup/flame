#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cCombobox : Component  // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cCombobox";
		inline static auto type_hash = ch(type_name);

		cCombobox() :
			Component(type_name, type_hash)
		{
		}

		virtual int get_index() const = 0;
		virtual void set_index(int index) = 0;

		FLAME_UNIVERSE_EXPORTS static cCombobox* create();
	};

	struct cComboboxItem : Component  // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cComboboxItem";
		inline static auto type_hash = ch(type_name);

		cComboboxItem() :
			Component(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static cComboboxItem* create();
	};
}
