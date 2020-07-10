#pragma once

#include <flame/universe/components/text.h>
#include "element_private.h"

namespace flame
{
	struct cTextBridge : cText
	{
		void set_text(const wchar_t* text) override;
	};

	struct cTextPrivate : cTextBridge, Drawer
	{
		std::wstring text;

		cElementPrivate* element = nullptr;

		const wchar_t* get_text() const override { return text.c_str(); }
		void set_text(const std::wstring& text);

		void on_added() override;
		void on_removed() override;

		void draw(graphics::Canvas* canvas) override;

		static cTextPrivate* create();

		//sLayoutManagement* management;

		//bool pending_sizing;

		//cTextPrivate();
		//cTextPrivate::~cTextPrivate();
		//void auto_set_size();
		//void on_event(EntityEvent e, void* t) override;
	};
}
