#pragma once

#include "tree_leaf.h"

namespace flame
{
	struct cTreeLeafPrivate : cTreeLeaf
	{
		std::wstring title;

		cTreePrivate* tree;
		cTextPrivate* title_text;

		const wchar_t* get_title() const override { return title.c_str(); }
		void set_title(const wchar_t* v) override { set_title(std::wstring(v)); }
		void set_title(std::wstring_view v);

		void notify_selected(bool v);

		void on_load_finished() override;
	};
}
