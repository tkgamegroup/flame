#pragma once

#include "../entity_private.h"
#include "window.h"

namespace flame
{
	struct dWindowPrivate : dWindow
	{
		cElementPrivate* element;
		cReceiverPrivate* receiver;
		cReceiverPrivate* size_dragger_receiver;
		cTextPrivate* title_text;
		EntityPrivate* close_button;
		EntityPrivate* content;

		std::wstring title;

		std::vector<std::unique_ptr<Closure<void(Capture&)>>> close_listeners;

		const wchar_t* get_title() const override;
		void set_title(const wchar_t* title) override;

		void* add_close_listener(void (*callback)(Capture& c), const Capture& capture) override;
		void remove_close_listener(void* lis) override;

		void on_load_finished() override;
		bool on_child_added(EntityPtr e) override;
	};
}
