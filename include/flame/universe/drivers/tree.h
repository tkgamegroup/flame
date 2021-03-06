#pragma once

#include <flame/universe/driver.h>

namespace flame
{
	struct dTree : Driver
	{
		inline static auto type_name = "flame::dTree";
		inline static auto type_hash = ch(type_name);

		dTree() :
			Driver(type_name, type_hash)
		{
		}

		virtual Entity* get_selected() const = 0;
		virtual void set_selected(Entity* e) = 0;

		virtual void expand_to_selected() = 0;

		FLAME_UNIVERSE_EXPORTS static dTree* create(void* parms = nullptr);
	};

	struct dTreeLeaf : Driver
	{
		inline static auto type_name = "flame::dTreeLeaf";
		inline static auto type_hash = ch(type_name);

		dTreeLeaf() :
			Driver(type_name, type_hash)
		{
		}

		virtual const wchar_t* get_title() const = 0;
		virtual void set_title(const wchar_t* title) = 0;

		FLAME_UNIVERSE_EXPORTS static dTreeLeaf* create(void* parms = nullptr);
	};

	struct dTreeNode : Driver
	{
		inline static auto type_name = "flame::dTreeNode";
		inline static auto type_hash = ch(type_name);

		dTreeNode() :
			Driver(type_name, type_hash)
		{
		}

		virtual const wchar_t* get_title() const = 0;
		virtual void set_title(const wchar_t* title) = 0;

		virtual void toggle_collapse() = 0;

		FLAME_UNIVERSE_EXPORTS static dTreeNode* create(void* parms = nullptr);
	};
}
