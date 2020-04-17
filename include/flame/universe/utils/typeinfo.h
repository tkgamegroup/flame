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
		void(*on_changed)(void* c, int v);
		Mail capture;

		cEnumSingleDataTracker(void* _data, EnumInfo* info, void(*on_changed)(void* c, int v), const Mail& capture) :
			info(info),
			on_changed(on_changed),
			capture(capture)
		{
			data = _data;
		}

		~cEnumSingleDataTracker() override
		{
			f_free(capture.p);
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

			combobox->data_changed_listeners.add([](void* c, uint hash, void*) {
				if (hash == FLAME_CHASH("index"))
				{
					auto thiz = *(cEnumSingleDataTracker**)c;
					thiz->on_changed(thiz->capture.p, thiz->info->item(thiz->combobox->index)->value());
				}
				return true;
			}, Mail::from_p(this));
		}
	};

	struct cEnumMultiDataTracker : cDataTracker
	{
		std::vector<cCheckbox*> checkboxs;

		EnumInfo* info;
		void(*on_changed)(void* c, int v);
		Mail capture;

		cEnumMultiDataTracker(void* _data, EnumInfo* info, void(*on_changed)(void* c, int v), const Mail& capture) :
			info(info),
			on_changed(on_changed),
			capture(capture)
		{
			data = _data;
		}

		~cEnumMultiDataTracker() override
		{
			f_free(capture.p);
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
				struct Capture
				{
					cEnumMultiDataTracker* thiz;
					int idx;
				}capture;
				capture.thiz = this;
				capture.idx = i;
				checkboxs[i]->data_changed_listeners.add([](void* c, uint hash, void*) {
					if (hash == FLAME_CHASH("checked"))
					{
						auto& capture = *(Capture*)c;
						auto v = *(int*)capture.thiz->data;
						auto f = capture.thiz->info->item(capture.idx)->value();
						if (capture.thiz->checkboxs[capture.idx]->checked)
							v |= f;
						else
							v &= ~f;
						capture.thiz->on_changed(capture.thiz->capture.p, v);
					}
					return true;
				}, Mail::from_t(&capture));
			}
		}
	};

	struct cBoolDataTracker : cDataTracker
	{
		cCheckbox* checkbox;

		void(*on_changed)(void* c, bool v);
		Mail capture;

		cBoolDataTracker(void* _data, void(*on_changed)(void* c, bool v), const Mail& capture) :
			on_changed(on_changed),
			capture(capture)
		{
			data = _data;
		}

		~cBoolDataTracker() override
		{
			f_free(capture.p);
		}

		void update_view() override
		{
			checkbox->set_checked(*(bool*)data, INVALID_POINTER);
		}

		void on_added() override
		{
			checkbox = entity->child(0)->get_component(cCheckbox);

			update_view();

			checkbox->data_changed_listeners.add([](void* c, uint hash, void*) {
				if (hash == FLAME_CHASH("checked"))
				{
					auto thiz = *(cBoolDataTracker**)c;
					thiz->on_changed(thiz->capture.p, thiz->checkbox->checked);
				}
				return true;
			}, Mail::from_p(this));
		}
	};

	template <class T>
	struct cDigitalDataTracker : cDataTracker
	{
		cText* edit_text;
		cText* drag_text;

		T drag_start;
		bool drag_changed;
		bool edit_changed;
		void(*on_changed)(void* c, T v, bool exit_editing);
		Mail capture;

		cDigitalDataTracker(void* _data, void(*on_changed)(void* c, T v, bool exit_editing), const Mail& capture) :
			drag_changed(false),
			edit_changed(false),
			on_changed(on_changed),
			capture(capture)
		{
			data = _data;
		}

		~cDigitalDataTracker() override
		{
			f_free(capture.p);
		}

		void update_view() override
		{
			std::wstring str;
			if constexpr (std::is_floating_point<T>::value)
				str = to_wstring(*(T*)data, 2);
			else
				str = to_wstring(*(T*)data);
			edit_text->set_text(str.c_str(), INVALID_POINTER);
			drag_text->set_text(str.c_str(), INVALID_POINTER);
		}

		void on_added() override
		{
			auto e = entity->child(0);
			edit_text = e->child(0)->get_component(cText);
			drag_text = e->child(1)->get_component(cText);

			update_view();

			edit_text->data_changed_listeners.add([](void* c, uint hash, void*) {
				(*(cDigitalDataTracker**)c)->edit_changed = true;
				return true;
			}, Mail::from_p(this));

			auto e_er = edit_text->entity->get_component(cEventReceiver);
			e_er->focus_listeners.add([](void* c, bool focusing) {
				if (!focusing)
				{
					auto thiz = *(cDigitalDataTracker**)c;
					if (thiz->edit_changed)
					{
						T v;
						try
						{
							v = sto<T>(thiz->edit_text->text());
						}
						catch (...)
						{
							v = 0;
						}
						thiz->on_changed(thiz->capture.p, v, true);
						thiz->update_view();
						thiz->edit_changed = false;
					}
				}
				return true;
			}, Mail::from_p(this));
			e_er->key_listeners.add([](void*, KeyStateFlags action, int value) {
				if ((action == KeyStateDown && value == Key_Enter) ||
					(action == KeyStateNull && (value == '\r' || value == '\n')))
				{
					auto r = utils::current_root();
					cEventReceiver::current()->dispatcher->next_focusing = r ? r->get_component(cEventReceiver) : nullptr;
					return false;
				}
				return true;
			}, Mail(), 0);

			auto d_er = drag_text->entity->get_component(cEventReceiver);
			d_er->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
				auto thiz = *(cDigitalDataTracker**)c;
				if (utils::is_active(cEventReceiver::current()) && is_mouse_move(action, key))
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
			}, Mail::from_p(this));
			d_er->state_listeners.add([](void* c, EventReceiverState) {
				auto thiz = *(cDigitalDataTracker**)c;
				if (thiz->drag_changed && !utils::is_active(cEventReceiver::current()))
				{
					auto temp = *(T*)thiz->data;
					*(T*)thiz->data = thiz->drag_start;
					thiz->on_changed(thiz->capture.p, temp, true);
					thiz->drag_changed = false;
				}
				return true;
			}, Mail::from_t(&capture));
		}
	};

	template <uint N, class T>
	struct cDigitalVecDataTracker : cDataTracker
	{
		cText* edit_texts[N];
		cText* drag_texts[N];

		Vec<N, T> drag_start;
		bool drag_changed;
		bool edit_changed;
		void(*on_changed)(void* c, const Vec<N, T>& v, bool exit_editing);
		Mail capture;

		cDigitalVecDataTracker(void* _data, void(*on_changed)(void* c, const Vec<N, T>& v, bool exit_editing), const Mail& capture) :
			drag_changed(false),
			edit_changed(false),
			on_changed(on_changed),
			capture(capture)
		{
			data = _data;
		}

		~cDigitalVecDataTracker() override
		{
			f_free(capture.p);
		}

		void update_view() override
		{
			for (auto i = 0; i < N; i++)
			{
				std::wstring str;
				if constexpr (std::is_floating_point<T>::value)
					str = to_wstring((*(Vec<N, T>*)data)[i], 2);
				else
					str = to_wstring((*(Vec<N, T>*)data)[i]);
				edit_texts[i]->set_text(str.c_str(), INVALID_POINTER);
				drag_texts[i]->set_text(str.c_str(), INVALID_POINTER);
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
				edit_texts[i]->data_changed_listeners.add([](void* c, uint hash, void*) {
					(*(cDigitalVecDataTracker**)c)->edit_changed = true;
					return true;
				}, Mail::from_p(this));

				auto e_er = edit_texts[i]->entity->get_component(cEventReceiver);

				{
					struct Capture
					{
						cDigitalVecDataTracker* thiz;
						int i;
					}capture;
					capture.thiz = this;
					capture.i = i;
					e_er->focus_listeners.add([](void* c, bool focusing) {
						if (!focusing)
						{
							auto& capture = *(Capture*)c;
							if (capture.thiz->edit_changed)
							{
								auto v = *(Vec<N, T>*)capture.thiz->data;
								try
								{
									v[capture.i] = sto<T>(capture.thiz->edit_texts[capture.i]->text());
								}
								catch (...)
								{
									v[capture.i] = 0;
								}
								capture.thiz->on_changed(capture.thiz->capture.p, v, true);
								capture.thiz->update_view();
								capture.thiz->edit_changed = false;
							}
						}
						return true;
					}, Mail::from_t(&capture));
				}
				e_er->key_listeners.add([](void*, KeyStateFlags action, int value) {
					if ((action == KeyStateDown && value == Key_Enter) ||
						(action == KeyStateNull && (value == '\r' || value == '\n')))
					{
						auto r = utils::current_root();
						cEventReceiver::current()->dispatcher->next_focusing = r ? r->get_component(cEventReceiver) : nullptr;
						return false;
					}
					return true;
				}, Mail(), 0);

				auto d_er = drag_texts[i]->entity->get_component(cEventReceiver);

				{
					struct Capture
					{
						cDigitalVecDataTracker* thiz;
						int i;
					}capture;
					capture.thiz = this;
					capture.i = i;
					d_er->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
						auto& capture = *(Capture*)c;
						if (utils::is_active(cEventReceiver::current()) && is_mouse_move(action, key))
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
					}, Mail::from_t(&capture));
					d_er->state_listeners.add([](void* c, EventReceiverState) {
						auto& capture = *(Capture*)c;
						if (capture.thiz->drag_changed && !utils::is_active(cEventReceiver::current()))
						{
							auto temp = *(Vec<N, T>*)capture.thiz->data;
							*(Vec<N, T>*)capture.thiz->data = capture.thiz->drag_start;
							capture.thiz->on_changed(capture.thiz->capture.p, temp, true);
							capture.thiz->drag_changed = false;
						}
						return true;
					}, Mail::from_t(&capture));
				}
			}
		}
	};

	struct cStringADataTracker : cDataTracker
	{
		cText* text;

		bool changed;
		void(*on_changed)(void* c, const char* v);
		Mail capture;

		cStringADataTracker(void* _data, void(*on_changed)(void* c, const char* v), const Mail& capture) :
			changed(false),
			on_changed(on_changed),
			capture(capture)
		{
			data = _data;
		}

		~cStringADataTracker() override
		{
			f_free(capture.p);
		}

		void update_view() override
		{
			text->set_text(((StringA*)data)->v ? s2w(((StringA*)data)->str()).c_str() : L"", INVALID_POINTER);
		}

		void on_added() override
		{
			text = entity->child(0)->get_component(cText);

			update_view();

			text->data_changed_listeners.add([](void* c, uint hash, void*) {
				(*(cStringADataTracker**)c)->changed = true;
				return true;
			}, Mail::from_p(this));
			auto er = text->entity->get_component(cEventReceiver);
			er->focus_listeners.add([](void* c, bool focusing) {
				if (!focusing)
				{
					auto thiz = *(cStringADataTracker**)c;
					if (thiz->changed)
					{
						thiz->on_changed(thiz->capture.p, w2s(thiz->text->text()).c_str());
						thiz->changed = false;
					}
				}
				return true;
			}, Mail::from_p(this));
			er->key_listeners.add([](void*, KeyStateFlags action, int value) {
				if (action == KeyStateDown && value == Key_Enter)
				{
					auto r = utils::current_root();
					cEventReceiver::current()->dispatcher->next_focusing = r ? r->get_component(cEventReceiver) : nullptr;
					return false;
				}
				return true;
			}, Mail());
		}
	};

	struct cStringWDataTracker : cDataTracker
	{
		cText* text;

		bool changed;
		void(*on_changed)(void* c, const wchar_t* v);
		Mail capture;

		cStringWDataTracker(void* _data, void(*on_changed)(void* c, const wchar_t* v), const Mail& capture) :
			changed(false),
			on_changed(on_changed),
			capture(capture)
		{
			data = _data;
		}

		~cStringWDataTracker() override
		{
			f_free(capture.p);
		}

		void update_view() override
		{
			text->set_text(((StringW*)data)->v ? ((StringW*)data)->v : L"", INVALID_POINTER);
		}

		void on_added() override
		{
			text = entity->child(0)->get_component(cText);

			update_view();

			text->data_changed_listeners.add([](void* c, uint hash, void*) {
				(*(cStringWDataTracker**)c)->changed = true;
				return true;
			}, Mail::from_p(this));
			auto er = text->entity->get_component(cEventReceiver);
			er->focus_listeners.add([](void* c, bool focusing) {
				if (!focusing)
				{
					auto thiz = *(cStringWDataTracker**)c;
					if (thiz->changed)
					{
						thiz->on_changed(thiz->capture.p, thiz->text->text());
						thiz->changed = false;
					}
				}
				return true;
			}, Mail::from_p(this));
			er->key_listeners.add([](void*, KeyStateFlags action, int value) {
				if (action == KeyStateDown && value == Key_Enter)
				{
					auto r = utils::current_root();
					cEventReceiver::current()->dispatcher->next_focusing = r ? r->get_component(cEventReceiver) : nullptr;
					return false;
				}
				return true;
			}, Mail());
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
