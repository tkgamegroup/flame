#pragma once

#include <flame/universe/components/text.h>
#include "element_private.h"

namespace flame
{
	struct cTextPrivate : cText, Drawer
	{
		std::wstring _string;

		cElementPrivate* _element = nullptr;

		void _set_string(const std::wstring& str);

		void _on_added();
		void _on_removed();

		void _draw(graphics::Canvas* canvas) override;

		static cTextPrivate* _create();

		//sLayoutManagement* management;

		//bool pending_sizing;

		//cTextPrivate();
		//cTextPrivate::~cTextPrivate();
		//void auto_set_size();
		//void on_event(EntityEvent e, void* t) override;

		const wchar_t* get_string() const override { return _string.c_str(); }
		void set_string(const wchar_t* str) override { _set_string(str); }

		void on_added() override { _on_added(); }
		void on_removed() override { _on_removed(); }
	};
}
