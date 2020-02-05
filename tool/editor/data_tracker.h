#pragma once

template <class T>
T sto_s(const wchar_t* s)
{
	try
	{
		return sto<T>(s);
	}
	catch (...)
	{
		return 0;
	}
}

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

	virtual void update_view() override
	{
		int idx;
		info->find_item(*(int*)data, &idx);
		combobox->set_index(idx, false);
	}

	virtual void on_added() override
	{
		combobox = entity->child(0)->get_component(cCombobox);

		update_view();
	}
};

struct cEnumMultiDataTracker : cDataTracker
{
	std::vector<cCheckbox*> checkboxs;

	EnumInfo* info;

	virtual void update_view() override
	{
		for (auto i = 0; i < checkboxs.size(); i++)
			checkboxs[i]->set_checked(*(int*)data & info->item(i)->value(), false);
	}

	virtual void on_added() override
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

	virtual void update_view() override
	{
		checkbox->set_checked(*(bool*)data, false);
	}

	virtual void on_added() override
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

	virtual void update_view() override
	{
		std::wstring str;
		if constexpr (std::is_floating_point<T>::value)
			str = to_wstring(*(T*)data, 2);
		else
			str = to_wstring(*(T*)data);
		edit_text->set_text(str.c_str());
		drag_text->set_text(str.c_str());
	}

	virtual void on_added() override
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

	virtual void update_view() override
	{
		for (auto i = 0; i < N; i++)
		{
			std::wstring str;
			if constexpr (std::is_floating_point<T>::value)
				str = to_wstring((*(Vec<N, T>*)data)[i], 2);
			else
				str = to_wstring((*(Vec<N, T>*)data)[i]);
			edit_texts[i]->set_text(str.c_str());
			drag_texts[i]->set_text(str.c_str());
		}
	}

	virtual void on_added() override
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

	virtual void update_view() override
	{
		text->set_text(s2w(((StringA*)data)->str()).c_str()) ;
	}

	virtual void on_added() override
	{
		text = entity->child(0)->get_component(cText);

		update_view();
	}
};

struct cStringWDataTracker : cDataTracker
{
	cText* text;

	virtual void update_view() override
	{
		text->set_text(((StringW*)data)->v);
	}

	virtual void on_added() override
	{
		text = entity->child(0)->get_component(cText);

		update_view();
	}
};
