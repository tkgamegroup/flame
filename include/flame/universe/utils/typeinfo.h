#pragma once

#include <flame/foundation/typeinfo.h>
#include <flame/universe/utils/ui.h>

namespace flame
{
	struct cDataTracker : Component
	{
		cDataTracker() :
			Component("cDataTracker")
		{
		}

		virtual void update_view() = 0;
	};

	struct cEnumSingleDataTracker : cDataTracker
	{
		cCombobox* combobox;

		int* data;
		EnumInfo* info;
		void(*on_changed)(void* c, int v);
		Mail capture;

		cEnumSingleDataTracker(int* data, EnumInfo* info, void(*on_changed)(void* c, int v), const Mail& capture) :
			data(data),
			info(info),
			on_changed(on_changed),
			capture(capture)
		{
		}

		~cEnumSingleDataTracker() override
		{
			f_free(capture.p);
		}

		void update_view() override
		{
			auto idx = -1;
			info->find_item(*data, &idx);
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

		int* data;
		EnumInfo* info;
		void(*on_changed)(void* c, int v);
		Mail capture;

		cEnumMultiDataTracker(int* data, EnumInfo* info, void(*on_changed)(void* c, int v), const Mail& capture) :
			data(data),
			info(info),
			on_changed(on_changed),
			capture(capture)
		{
		}

		~cEnumMultiDataTracker() override
		{
			f_free(capture.p);
		}

		void update_view() override
		{
			for (auto i = 0; i < checkboxs.size(); i++)
				checkboxs[i]->set_checked(*data & info->item(i)->value(), INVALID_POINTER);
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
						auto v = *capture.thiz->data;
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

		bool* data;
		void(*on_changed)(void* c, bool v);
		Mail capture;

		cBoolDataTracker(bool* data, void(*on_changed)(void* c, bool v), const Mail& capture) :
			data(data),
			on_changed(on_changed),
			capture(capture)
		{
		}

		~cBoolDataTracker() override
		{
			f_free(capture.p);
		}

		void update_view() override
		{
			checkbox->set_checked(*data, INVALID_POINTER);
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

		T* data;
		T drag_start;
		bool drag_changed;
		bool edit_changed;
		void(*on_changed)(void* c, T v, bool exit_editing);
		Mail capture;

		cDigitalDataTracker(T* data, void(*on_changed)(void* c, T v, bool exit_editing), const Mail& capture) :
			data(data),
			drag_changed(false),
			edit_changed(false),
			on_changed(on_changed),
			capture(capture)
		{
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
			e_er->key_listeners.add([](void* c, KeyStateFlags action, int value) {
				if ((action == KeyStateDown && value == Key_Enter) ||
					(action == KeyStateNull && (value == '\r' || value == '\n')))
				{
					auto r = utils::current_root();
					(*(cEventReceiver**)c)->dispatcher->next_focusing = r ? r->get_component(cEventReceiver) : nullptr;
					return false;
				}
				return true;
			}, Mail::from_p(e_er), 0);

			auto d_er = drag_text->entity->get_component(cEventReceiver);
			struct Capture
			{
				cDigitalDataTracker* thiz;
				cEventReceiver* d_er;
			}capture;
			capture.thiz = this;
			capture.d_er = d_er;

			d_er->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
				auto& capture = *(Capture*)c;
				if (utils::is_active(capture.d_er) && is_mouse_move(action, key))
				{
					auto v = *capture.thiz->data;
					if (!capture.thiz->drag_changed)
						capture.thiz->drag_start = v;
					if constexpr (std::is_floating_point<T>::value)
						v += pos.x() * 0.05f;
					else
						v += pos.x();
					capture.thiz->on_changed(capture.thiz->capture.p, v, false);
					capture.thiz->drag_changed = true;
					capture.thiz->update_view();
				}
				return true;
			}, Mail::from_t(&capture));
			d_er->state_listeners.add([](void* c, EventReceiverStateFlags) {
				auto& capture = *(Capture*)c;
				if (capture.thiz->drag_changed && !utils::is_active(capture.d_er))
				{
					auto temp = *capture.thiz->data;
					*capture.thiz->data = capture.thiz->drag_start;
					capture.thiz->on_changed(capture.thiz->capture.p, temp, true);
					capture.thiz->drag_changed = false;
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

		Vec<N, T>* data;
		Vec<N, T> drag_start;
		bool drag_changed;
		bool edit_changed;
		void(*on_changed)(void* c, const Vec<N, T>& v, bool exit_editing);
		Mail capture;

		cDigitalVecDataTracker(Vec<N, T>* data, void(*on_changed)(void* c, const Vec<N, T>& v, bool exit_editing), const Mail& capture) :
			data(data),
			drag_changed(false),
			edit_changed(false),
			on_changed(on_changed),
			capture(capture)
		{
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
				auto e = entity->child(i)->child(0);
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
								auto v = *capture.thiz->data;
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
				e_er->key_listeners.add([](void* c, KeyStateFlags action, int value) {
					if ((action == KeyStateDown && value == Key_Enter) ||
						(action == KeyStateNull && (value == '\r' || value == '\n')))
					{
						auto r = utils::current_root();
						(*(cEventReceiver**)c)->dispatcher->next_focusing = r ? r->get_component(cEventReceiver) : nullptr;
						return false;
					}
					return true;
				}, Mail::from_p(e_er), 0);

				auto d_er = drag_texts[i]->entity->get_component(cEventReceiver);

				{
					struct Capture
					{
						cDigitalVecDataTracker* thiz;
						int i;
						cEventReceiver* d_er;
					}capture;
					capture.thiz = this;
					capture.i = i;
					capture.d_er = d_er;
					d_er->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
						auto& capture = *(Capture*)c;
						if (utils::is_active(capture.d_er) && is_mouse_move(action, key))
						{
							auto v = *capture.thiz->data;
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
					d_er->state_listeners.add([](void* c, EventReceiverStateFlags) {
						auto& capture = *(Capture*)c;
						if (capture.thiz->drag_changed && !utils::is_active(capture.d_er))
						{
							auto temp = *capture.thiz->data;
							*capture.thiz->data = capture.thiz->drag_start;
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

		StringA* data;
		bool changed;
		void(*on_changed)(void* c, const char* v);
		Mail capture;

		cStringADataTracker(StringA* data, void(*on_changed)(void* c, const char* v), const Mail& capture) :
			data(data),
			changed(false),
			on_changed(on_changed),
			capture(capture)
		{
		}

		~cStringADataTracker() override
		{
			f_free(capture.p);
		}

		void update_view() override
		{
			text->set_text(data->v ? s2w(data->str()).c_str() : L"", INVALID_POINTER);
		}

		void on_added() override
		{
			text = entity->child(0)->get_component(cText);

			update_view();

			text->data_changed_listeners.add([](void* c, uint hash, void*) {
				(*(cStringADataTracker**)c)->changed = true;
				return true;
			}, Mail::from_p(this));
			auto event_receiver = text->entity->get_component(cEventReceiver);
			event_receiver->focus_listeners.add([](void* c, bool focusing) {
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
			event_receiver->key_listeners.add([](void* c, KeyStateFlags action, int value) {
				if (action == KeyStateDown && value == Key_Enter)
				{
					auto r = utils::current_root();
					(*(cEventReceiver**)c)->dispatcher->next_focusing = r ? r->get_component(cEventReceiver) : nullptr;
					return false;
				}
				return true;
			}, Mail::from_p(event_receiver));
		}
	};

	struct cStringWDataTracker : cDataTracker
	{
		cText* text;

		StringW* data;
		bool changed;
		void(*on_changed)(void* c, const wchar_t* v);
		Mail capture;

		cStringWDataTracker(StringW* data, void(*on_changed)(void* c, const wchar_t* v), const Mail& capture) :
			data(data),
			changed(false),
			on_changed(on_changed),
			capture(capture)
		{
		}

		~cStringWDataTracker() override
		{
			f_free(capture.p);
		}

		void update_view() override
		{
			text->set_text(data->v ? data->v : L"", INVALID_POINTER);
		}

		void on_added() override
		{
			text = entity->child(0)->get_component(cText);

			update_view();

			text->data_changed_listeners.add([](void* c, uint hash, void*) {
				(*(cStringWDataTracker**)c)->changed = true;
				return true;
			}, Mail::from_p(this));
			auto event_receiver = text->entity->get_component(cEventReceiver);
			event_receiver->focus_listeners.add([](void* c, bool focusing) {
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
			event_receiver->key_listeners.add([](void* c, KeyStateFlags action, int value) {
				if (action == KeyStateDown && value == Key_Enter)
				{
					auto r = utils::current_root();
					(*(cEventReceiver**)c)->dispatcher->next_focusing = r ? r->get_component(cEventReceiver) : nullptr;
					return false;
				}
				return true;
			}, Mail::from_p(event_receiver));
		}
	};

	namespace utils
	{
		void create_enum_combobox(EnumInfo* info, float width)
		{
			e_begin_combobox(width);
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
