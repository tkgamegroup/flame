#pragma once

struct cDataTracker : Component
{
	bool auto_update;

	cDataTracker() :
		Component("DataTracker")
	{
		auto_update = false;
	}

	virtual void update_view() = 0;

	virtual void update() override
	{
		if (auto_update)
			update_view();
	}
};

struct cEnumSingleDataTracker : cDataTracker
{
	cCombobox* combobox;

	int* data;
	EnumInfo* info;

	virtual void update_view() override
	{
		int idx;
		info->find_item(*data, &idx);
		combobox->set_index(idx, false);
	}

	virtual void start() override
	{
		combobox = (cCombobox*)entity->child(0)->find_component(cH("Combobox"));

		if (!auto_update)
			update_view();
	}
};

struct cEnumMultiDataTracker : cDataTracker
{
	std::vector<cCheckbox*> checkboxs;

	int* data;
	EnumInfo* info;

	virtual void update_view() override
	{
		for (auto i = 0; i < checkboxs.size(); i++)
			checkboxs[i]->set_checked(*data & info->item(i)->value(), false);
	}

	virtual void start() override
	{
		for (auto i = 0; i < entity->child_count(); i++)
			checkboxs.push_back((cCheckbox*)entity->child(i)->find_component(cH("Checkbox")));

		if (!auto_update)
			update_view();
	}
};

struct cBoolDataTracker : cDataTracker
{
	cCheckbox* checkbox;

	bool* data;

	virtual void update_view() override
	{
		checkbox->set_checked(*data, false);
	}

	virtual void start() override
	{
		checkbox = (cCheckbox*)entity->child(0)->find_component(cH("Checkbox"));

		if (!auto_update)
			update_view();
	}
};

template<class T>
struct cDigitalDataTracker : cDataTracker
{
	cEventReceiver* event_receiver;
	cText* text;

	T* data;

	virtual void update_view() override
	{
		if (!(auto_update && event_receiver->focusing))
			text->set_text(to_wstring(*data));
	}

	virtual void start() override
	{
		event_receiver = (cEventReceiver*)entity->child(0)->find_component(cH("EventReceiver"));
		text = (cText*)entity->child(0)->find_component(cH("Text"));

		if (!auto_update)
			update_view();
	}
};

template<uint N, class T>
struct cDigitalVecDataTracker : cDataTracker
{
	cEventReceiver* event_receivers[N];
	cText* texts[N];

	Vec<N, T>* data;

	virtual void update_view() override
	{
		for (auto i = 0; i < N; i++)
		{
			if (!(auto_update && event_receivers[i]->focusing))
				texts[i]->set_text(to_wstring((*data)[i]));
		}
	}

	virtual void start() override
	{
		for (auto i = 0; i < N; i++)
		{
			auto e = entity->child(i)->child(0);
			event_receivers[i] = (cEventReceiver*)e->find_component(cH("EventReceiver"));
			texts[i] = (cText*)e->find_component(cH("Text"));
		}

		if (!auto_update)
			update_view();
	}
};

struct cStringDataTracker : cDataTracker
{
	cText* text;

	std::string* data;

	virtual void update_view() override
	{
		text->set_text(s2w(*data));
	}

	virtual void start() override
	{
		text = (cText*)entity->child(0)->find_component(cH("Text"));

		if (!auto_update)
			update_view();
	}
};

struct cWStringDataTracker : cDataTracker
{
	cText* text;

	std::wstring* data;

	virtual void update_view() override
	{
		text->set_text(*data);
	}

	virtual void start() override
	{
		text = (cText*)entity->child(0)->find_component(cH("Text"));

		if (!auto_update)
			update_view();
	}
};