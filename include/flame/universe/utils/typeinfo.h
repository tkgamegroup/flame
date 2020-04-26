#pragma once

#include <flame/foundation/typeinfo.h>
#include <flame/universe/utils/ui.h>

namespace flame
{
	struct cDataTracker : Component
	{
		void* data;

		cDataTracker() :
			Component("cDataTracker")
		{
		}

		virtual void update_view() = 0;
	};

	struct cEnumSingleDataTracker : cDataTracker
	{
		cCombobox* combobox;

		EnumInfo* info;
		void(*on_changed)(Capture& c, int v);
		Capture capture;

		cEnumSingleDataTracker(void* _data, EnumInfo* info, void(*on_changed)(Capture& c, int v), const Capture& capture) :
			info(info),
			on_changed(on_changed),
			capture(capture)
		{
			data = _data;
		}

		~cEnumSingleDataTracker() override
		{
			f_free(capture._data);
		}

		void update_view() override
		{
			auto idx = -1;
			info->find_item(*(int*)data, &idx);
			combobox->set_index(idx, INVALID_POINTER);
		}

		void on_added() override
		{
			combobox = entity->child(0)->get_component(cCombobox);

			update_view();

			combobox->data_changed_listeners.add([](Capture& c, uint hash, void*) {
				if (hash == FLAME_CHASH("index"))
				{
					auto thiz = *(cEnumSingleDataTracker**)c;
					thiz->on_changed(thiz->capture.p, thiz->info->item(thiz->combobox->index)->value());
				}
				return true;
			}, Capture().set_thiz(this));
		}
	};

	struct cEnumMultiDataTracker : cDataTracker
	{
		std::vector<cCheckbox*> checkboxs;

		EnumInfo* info;
		void(*on_changed)(Capture& c, int v);
		Capture capture;

		cEnumMultiDataTracker(void* _data, EnumInfo* info, void(*on_changed)(Capture& c, int v), const Capture& capture) :
			info(info),
			on_changed(on_changed),
			capture(capture)
		{
			data = _data;
		}

		~cEnumMultiDataTracker() override
		{
			f_free(capture._data);
		}

		void update_view() override
		{
			for (auto i = 0; i < checkboxs.size(); i++)
				checkboxs[i]->set_checked(*(int*)data & info->item(i)->value(), INVALID_POINTER);
		}

		void on_added() override
		{
			for (auto i = 0; i < entity->child_count(); i++)
				checkboxs.push_back(entity->child(i)->child(0)->get_component(cCheckbox));

			update_view();

			for (auto i = 0; i < checkboxs.size(); i++)
			{
				struct Capturing
				{
					cEnumMultiDataTracker* thiz;
					int idx;
				}capture;
				capture.thiz = this;
				capture.idx = i;
				checkboxs[i]->data_changed_listeners.add([](Capture& c, uint hash, void*) {
					if (hash == FLAME_CHASH("checked"))
					{
						auto& capture = c.data<Capturing>();
						auto v = *(int*)capture.thiz->data;
						auto f = capture.thiz->info->item(capture.idx)->value();
						if (capture.thiz->checkboxs[capture.idx]->checked)
							v |= f;
						else
							v &= ~f;
						capture.thiz->on_changed(capture.thiz->capture.p, v);
					}
					return true;
				}, Capture().set_data(&capture));
			}
		}
	};

	struct cBoolDataTracker : cDataTracker
	{
		cCheckbox* checkbox;

		void(*on_changed)(Capture& c, bool v);
		Capture capture;

		cBoolDataTracker(void* _data, void(*on_changed)(Capture& c, bool v), const Capture& capture) :
			on_changed(on_changed),
			capture(capture)
		{
			data = _data;
		}

		~cBoolDataTracker() override
		{
			f_free(capture._data);
		}

		void update_view() override
		{
			checkbox->set_checked(*(bool*)data, INVALID_POINTER);
		}

		void on_added() override
		{
			checkbox = entity->child(0)->get_component(cCheckbox);

			update_view();

			checkbox->data_changed_listeners.add([](Capture& c, uint hash, void*) {
				if (hash == FLAME_CHASH("checked"))
				{
					auto thiz = *(cBoolDataTracker**)c;
					thiz->on_changed(thiz->capture.p, thiz->checkbox->checked);
				}
				return true;
			}, Capture().set_thiz(this));
		}
	};

	template <class T>
	struct cDigitalDataTracker : cDataTracker
	{
		cText* edit_text;
		cText* drag_text;

		T drag_start;
		bool drag_changed;
		void(*on_changed)(Capture& c, T v, bool exit_editing);
		Capture capture;

		cDigitalDataTracker(void* _data, void(*on_changed)(Capture& c, T v, bool exit_editing), const Capture& capture) :
			drag_changed(false),
			on_changed(on_changed),
			capture(capture)
		{
			data = _data;
		}

		~cDigitalDataTracker() override
		{
			f_free(capture._data);
		}

		void update_view() override
		{
			std::wstring str;
			if constexpr (std::is_floating_point<T>::value)
				str = to_wstring(*(T*)data);
			else
				str = to_wstring(*(T*)data);
			edit_text->set_text(str.c_str(), -1, INVALID_POINTER);
			drag_text->set_text(str.c_str(), -1, INVALID_POINTER);
		}

		void on_added() override
		{
			auto e = entity->child(0);
			edit_text = e->child(0)->get_component(cText);
			drag_text = e->child(1)->get_component(cText);

			update_view();

			auto e_e = edit_text->entity->get_component(cEdit);
			e_e->enter_to_throw_focus = true;
			e_e->trigger_changed_on_lost_focus = true;

			edit_text->data_changed_listeners.add([](Capture& c, uint hash, void*) {
				if (hash == FLAME_CHASH("text"))
				{
					auto thiz = *(cDigitalDataTracker**)c;
					T v;
					try
					{
						v = sto<T>(((cText*)Component::current())->text.v);
					}
					catch (...)
					{
						v = 0;
					}
					thiz->on_changed(thiz->capture.p, v, true);
					thiz->update_view();
				}
				return true;
			}, Capture().set_thiz(this));

			auto d_er = drag_text->entity->get_component(cEventReceiver);
			d_er->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
				auto thiz = *(cDigitalDataTracker**)c;
				if (cEventReceiver::current()->is_active() && is_mouse_move(action, key))
				{
					auto v = *(T*)thiz->data;
					if (!thiz->drag_changed)
						thiz->drag_start = v;
					if constexpr (std::is_floating_point<T>::value)
						v += pos.x() * 0.05f;
					else
						v += pos.x();
					thiz->on_changed(thiz->capture.p, v, false);
					thiz->drag_changed = true;
					thiz->update_view();
				}
				return true;
			}, Capture().set_thiz(this));
			d_er->state_listeners.add([](Capture& c, EventReceiverState) {
				auto thiz = *(cDigitalDataTracker**)c;
				if (thiz->drag_changed && !cEventReceiver::current()->is_active())
				{
					auto temp = *(T*)thiz->data;
					*(T*)thiz->data = thiz->drag_start;
					thiz->on_changed(thiz->capture.p, temp, true);
					thiz->drag_changed = false;
				}
				return true;
			}, Capture().set_thiz(this));
		}
	};

	template <uint N, class T>
	struct cDigitalVecDataTracker : cDataTracker
	{
		cText* edit_texts[N];
		cText* drag_texts[N];

		Vec<N, T> drag_start;
		bool drag_changed;
		void(*on_changed)(Capture& c, const Vec<N, T>& v, bool exit_editing);
		Capture capture;

		cDigitalVecDataTracker(void* _data, void(*on_changed)(Capture& c, const Vec<N, T>& v, bool exit_editing), const Capture& capture) :
			drag_changed(false),
			on_changed(on_changed),
			capture(capture)
		{
			data = _data;
		}

		~cDigitalVecDataTracker() override
		{
			f_free(capture._data);
		}

		void update_view() override
		{
			for (auto i = 0; i < N; i++)
			{
				std::wstring str;
				if constexpr (std::is_floating_point<T>::value)
					str = to_wstring((*(Vec<N, T>*)data)[i]);
				else
					str = to_wstring((*(Vec<N, T>*)data)[i]);
				edit_texts[i]->set_text(str.c_str(), -1, INVALID_POINTER);
				drag_texts[i]->set_text(str.c_str(), -1, INVALID_POINTER);
			}
		}

		void on_added() override
		{
			for (auto i = 0; i < N; i++)
			{
				auto e = entity->child(i);
				edit_texts[i] = e->child(0)->get_component(cText);
				drag_texts[i] = e->child(1)->get_component(cText);
			}

			update_view();

			for (auto i = 0; i < N; i++)
			{
				auto e_e = edit_texts[i]->entity->get_component(cEdit);
				e_e->enter_to_throw_focus = true;
				e_e->trigger_changed_on_lost_focus = true;

				{
					struct Capturing
					{
						cDigitalVecDataTracker* thiz;
						int i;
					}capture;
					capture.thiz = this;
					capture.i = i; 
					edit_texts[i]->data_changed_listeners.add([](Capture& c, uint hash, void*) {
						if (hash == FLAME_CHASH("text"))
						{
							auto& capture = c.data<Capturing>();
							auto v = *(Vec<N, T>*)capture.thiz->data;
							try
							{
								v[capture.i] = sto<T>(((cText*)Component::current())->text.v);
							}
							catch (...)
							{
								v[capture.i] = 0;
							}
							capture.thiz->on_changed(capture.thiz->capture.p, v, true);
							capture.thiz->update_view();
						}
						return true;
					}, Capture().set_data(&capture));
				}

				auto d_er = drag_texts[i]->entity->get_component(cEventReceiver);

				{
					struct Capturing
					{
						cDigitalVecDataTracker* thiz;
						int i;
					}capture;
					capture.thiz = this;
					capture.i = i;
					d_er->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
						auto& capture = c.data<Capturing>();
						if (cEventReceiver::current()->is_active() && is_mouse_move(action, key))
						{
							auto v = *(Vec<N, T>*)capture.thiz->data;
							if (!capture.thiz->drag_changed)
								capture.thiz->drag_start = v;
							if constexpr (std::is_floating_point<T>::value)
								v[capture.i] += pos.x() * 0.05f;
							else
								v[capture.i] += pos.x();
							capture.thiz->on_changed(capture.thiz->capture.p, v, false);
							capture.thiz->drag_changed = true;
							capture.thiz->update_view();
						}
						return true;
					}, Capture().set_data(&capture));
					d_er->state_listeners.add([](Capture& c, EventReceiverState) {
						auto& capture = c.data<Capturing>();
						if (capture.thiz->drag_changed && !cEventReceiver::current()->is_active())
						{
							auto temp = *(Vec<N, T>*)capture.thiz->data;
							*(Vec<N, T>*)capture.thiz->data = capture.thiz->drag_start;
							capture.thiz->on_changed(capture.thiz->capture.p, temp, true);
							capture.thiz->drag_changed = false;
						}
						return true;
					}, Capture().set_data(&capture));
				}
			}
		}
	};

	struct cStringADataTracker : cDataTracker
	{
		cText* text;

		void(*on_changed)(Capture& c, const char* v);
		Capture capture;

		cStringADataTracker(void* _data, void(*on_changed)(Capture& c, const char* v), const Capture& capture) :
			on_changed(on_changed),
			capture(capture)
		{
			data = _data;
		}

		~cStringADataTracker() override
		{
			f_free(capture._data);
		}

		void update_view() override
		{
			text->set_text(((StringA*)data)->v ? s2w(((StringA*)data)->str()).c_str() : L"", -1, INVALID_POINTER);
		}

		void on_added() override
		{
			text = entity->child(0)->get_component(cText);

			update_view();

			auto e_e = text->entity->get_component(cEdit);
			e_e->enter_to_throw_focus = true;
			e_e->trigger_changed_on_lost_focus = true;

			text->data_changed_listeners.add([](Capture& c, uint hash, void*) {
				if (hash == FLAME_CHASH("text"))
				{
					auto thiz = *(cStringADataTracker**)c;
					thiz->on_changed(thiz->capture.p, w2s(((cText*)Component::current())->text.str()).c_str());
				}
				return true;
			}, Capture().set_thiz(this));
		}
	};

	struct cStringWDataTracker : cDataTracker
	{
		cText* text;

		void(*on_changed)(Capture& c, const wchar_t* v);
		Capture capture;

		cStringWDataTracker(void* _data, void(*on_changed)(Capture& c, const wchar_t* v), const Capture& capture) :
			on_changed(on_changed),
			capture(capture)
		{
			data = _data;
		}

		~cStringWDataTracker() override
		{
			f_free(capture._data);
		}

		void update_view() override
		{
			text->set_text(((StringW*)data)->v ? ((StringW*)data)->v : L"", -1, INVALID_POINTER);
		}

		void on_added() override
		{
			text = entity->child(0)->get_component(cText);

			update_view();

			auto e_e = text->entity->get_component(cEdit);
			e_e->enter_to_throw_focus = true;
			e_e->trigger_changed_on_lost_focus = true;

			text->data_changed_listeners.add([](Capture& c, uint hash, void*) {
				if (hash == FLAME_CHASH("text"))
				{
					auto thiz = *(cStringWDataTracker**)c;
					thiz->on_changed(thiz->capture.p, ((cText*)Component::current())->text.v);
				}
				return true;
			}, Capture().set_thiz(this));
		}
	};

	namespace utils
	{
		void create_enum_combobox(EnumInfo* info)
		{
			e_begin_combobox();
			for (auto i = 0; i < info->item_count(); i++)
				e_combobox_item(s2w(info->item(i)->name()).c_str());
			e_end_combobox();
		}

		void create_enum_checkboxs(EnumInfo* info)
		{
			for (auto i = 0; i < info->item_count(); i++)
				e_checkbox(s2w(info->item(i)->name()).c_str());
		}
	}
}
