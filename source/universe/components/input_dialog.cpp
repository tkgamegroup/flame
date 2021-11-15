#include "../entity_private.h"
#include "text_private.h"
#include "receiver_private.h"
#include "input_dialog_private.h"

namespace flame
{
	void cInputDialogPrivate::set_text(const wchar_t* v)
	{
		text->set_text(v);
	}

	void* cInputDialogPrivate::add_callback(void (*callback)(Capture& c, bool ok, const wchar_t* text), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		callbacks.emplace_back(c);
		return c;
	}

	void cInputDialogPrivate::remove_callback(void* ret)
	{
		std::erase_if(callbacks, [&](const auto& i) {
			return i.get() == (decltype(i.get()))ret;
			});
	}

	void cInputDialogPrivate::on_load_finished()
	{
		auto etext = entity->find_child("text");
		assert(etext);
		text = etext->get_component_t<cTextPrivate>();

		auto ok_button = entity->find_child("ok");
		assert(ok_button);
		auto ok_receiver = ok_button->get_component_t<cReceiverPrivate>();
		assert(ok_receiver);
		ok_receiver->add_mouse_click_listener([](Capture& c) {
			auto thiz = c.thiz<cInputDialogPrivate>();
			auto str = thiz->text->get_text();
			for (auto& cb : thiz->callbacks)
				cb->call(true, str);
			}, Capture().set_thiz(this));

		auto cancel_button = entity->find_child("cancel");
		assert(cancel_button);
		auto cancel_receiver = cancel_button->get_component_t<cReceiverPrivate>();
		assert(cancel_receiver);
		cancel_receiver->add_mouse_click_listener([](Capture& c) {
			auto thiz = c.thiz<cInputDialogPrivate>();
			for (auto& cb : thiz->callbacks)
				cb->call(false, nullptr);
			}, Capture().set_thiz(this));
	}

	cInputDialog* cInputDialog::create(void* parms)
	{
		return new cInputDialogPrivate();
	}
}
