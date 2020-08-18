#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cTree : Component // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cTree";
		inline static auto type_hash = ch(type_name);

		cTree() :
			Component(type_name, type_hash)
		{
		}

		virtual Entity* get_selected() const = 0;
		virtual void set_selected(Entity* e) = 0;

		virtual void expand_to_selected() = 0;

		FLAME_UNIVERSE_EXPORTS static cTree* create();
	};

	struct cTreeLeaf : Component // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cTreeLeaf";
		inline static auto type_hash = ch(type_name);

		cTreeLeaf() :
			Component(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static cTreeLeaf* create();
	};

	struct cTreeNode : Component // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cTreeNode";
		inline static auto type_hash = ch(type_name);

		cTreeNode() :
			Component(type_name, type_hash)
		{
		}

		virtual void toggle_collapse() = 0;

		FLAME_UNIVERSE_EXPORTS static cTreeNode* create();
	};
}
