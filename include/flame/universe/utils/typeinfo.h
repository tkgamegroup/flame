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

		void update_view() override
		{
			int idx;
			info->find_item(*(int*)data, &idx);
			combobox->set_index(idx, INVALID_POINTER);
		}

		void on_added() override
		{
			combobox = entity->child(0)->get_component(cCombobox);

			update_view();
		}
	};

	struct cEnumMultiDataTracker : cDataTracker
	{
		std::vector<cCheckbox*> checkboxs;

		EnumInfo* info;

		void update_view() override
		{
			for (auto i = 0; i < checkboxs.size(); i++)
				checkboxs[i]->set_checked(*(int*)data & info->item(i)->value(), INVALID_POINTER);
		}

		void on_added() override
		{
			checkboxs.clear();
			for (auto i = 0; i < entity->child_count(); i++)
				checkboxs.push_back(entity->child(i)->child(0)->get_component(cCheckbox));

			update_view();
		}
	};

	struct cBoolDataTracker : cDataTracker
	{
		cCheckbox* checkbox;

		void update_view() override
		{
			checkbox->set_checked(*(bool*)data, INVALID_POINTER);
		}

		void on_added() override
		{
			checkbox = entity->child(0)->get_component(cCheckbox);

			update_view();
		}
	};

	template <class T>
	struct cDigitalDataTracker : cDataTracker
	{
		cText* edit_text;
		cText* drag_text;

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
		}
	};

	template <uint N, class T>
	struct cDigitalVecDataTracker : cDataTracker
	{
		cText* edit_texts[N];
		cText* drag_texts[N];

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
		}
	};

	struct cStringADataTracker : cDataTracker
	{
		cText* text;

		void update_view() override
		{
			auto& d = *(StringA*)data;
			text->set_text(d.v ? s2w(d.str()).c_str() : L"", INVALID_POINTER);
		}

		void on_added() override
		{
			text = entity->child(0)->get_component(cText);

			update_view();
		}
	};

	struct cStringWDataTracker : cDataTracker
	{
		cText* text;

		void update_view() override
		{
			auto& d = *(StringW*)data;
			text->set_text(d.v ? d.v : L"", INVALID_POINTER);
		}

		void on_added() override
		{
			text = entity->child(0)->get_component(cText);

			update_view();
		}
	};

	namespace utils
	{
		void create_enum_combobox(EnumInfo* info, float width)
		{
			utils::e_begin_combobox(width);
			for (auto i = 0; i < info->item_count(); i++)
				utils::e_combobox_item(s2w(info->item(i)->name()).c_str());
			utils::e_end_combobox();
		}

		void create_enum_checkboxs(EnumInfo* info)
		{
			for (auto i = 0; i < info->item_count(); i++)
				utils::e_checkbox(s2w(info->item(i)->name()).c_str());
		}
	}
}
