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
#ifdef USE_SCRIPT_MODULE
		if (!callback)
		{
			auto slot = (uint)&capture;
			callback = [](Capture& c, bool ok, const wchar_t* text) {
				auto scr_ins = script::Instance::get_default();
				scr_ins->get_global("callbacks");
				scr_ins->get_member(nullptr, c.data<uint>());
				scr_ins->get_member("f");
				scr_ins->push_bool(ok);
				scr_ins->push_string(text ? w2s(text).c_str() : "");
				scr_ins->call(2);
				scr_ins->pop(2);
			};
			auto c = new Closure(callback, Capture().set_data(&slot));
			callbacks.emplace_back(c);
			return c;
		}
#endif
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
		fassert(etext);
		text = etext->get_component_t<cTextPrivate>();

		auto ok_button = entity->find_child("ok");
		fassert(ok_button);
		auto ok_receiver = ok_button->get_component_t<cReceiverPrivate>();
		fassert(ok_receiver);
		ok_receiver->add_mouse_click_listener([](Capture& c) {
			auto thiz = c.thiz<cInputDialogPrivate>();
			auto str = thiz->text->get_text();
			for (auto& cb : thiz->callbacks)
				cb->call(true, str);
			}, Capture().set_thiz(this));

		auto cancel_button = entity->find_child("cancel");
		fassert(cancel_button);
		auto cancel_receiver = cancel_button->get_component_t<cReceiverPrivate>();
		fassert(cancel_receiver);
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
