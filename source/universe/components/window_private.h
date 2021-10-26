#pragma once

#include "window.h"

namespace flame
{
	struct cWindowPrivate : cWindow
	{
		std::wstring title;
		bool nomove = false;
		bool noresize = false;

		std::vector<std::unique_ptr<Closure<void(Capture&)>>> close_listeners;

		cElementPrivate* element;
		cReceiverPrivate* receiver;
		EntityPrivate* size_dragger;
		cReceiverPrivate* size_dragger_receiver;
		cTextPrivate* title_text;
		EntityPrivate* close_button;
		EntityPrivate* content;
		cElementPrivate* content_element;

		const wchar_t* get_title() const override { return title.c_str(); }
		void set_title(const wchar_t* title) override;

		bool get_nomove() const override { return nomove; }
		void set_nomove(bool v) override;
		bool get_noresize() const override { return noresize; }
		void set_noresize(bool v) override;

		void* add_close_listener(void (*callback)(Capture& c), const Capture& capture) override;
		void remove_close_listener(void* lis) override;

		void on_load_finished() override;
		bool on_before_adding_child(EntityPtr e) override;
	};
}
