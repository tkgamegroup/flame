#pragma once

#include "../entity_private.h"
#include "menu.h"

namespace flame
{
	struct dMenuPrivate : dMenu
	{
		cElementPrivate* element;
		cReceiverPrivate* receiver;
		cTextPrivate* text;
		EntityPrivate* arrow;

		MenuType type = MenuTop;
		EntityPrivate* items;
		EntityPrivate* root;
		bool ancestor_opened = false;
		bool opened = false;

		void on_load_finished() override;
		bool on_child_added(EntityPtr e) override;

		void open();
		void close();

		void mark_ancestor_opened(bool v);
	};

	struct dMenuItemPrivate : dMenuItem
	{
		cElementPrivate* element;
		cReceiverPrivate* receiver;
		cTextPrivate* text;
		EntityPrivate* arrow;

		bool checkable = false;
		bool checked = false;

		bool get_checkable() const override { return checkable; }
		void set_checkable(bool v) override;

		bool get_checked() const override { return checked; }
		void set_checked(bool v) override;
		void set_single_checked() override;

		void on_load_finished() override;
	};

	struct dMenuBarPrivate : dMenuBar
	{
	};
}
