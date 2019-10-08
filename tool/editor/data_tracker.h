#pragma once

struct cDataTracker : Component
{
	void* data;

	cDataTracker() :
		Component("DataTracker")
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

	virtual void start() override
	{
		combobox = (cCombobox*)entity->child(0)->find_component(cH("Combobox"));

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

	virtual void start() override
	{
		for (auto i = 0; i < entity->child_count(); i++)
			checkboxs.push_back((cCheckbox*)entity->child(i)->child(0)->find_component(cH("Checkbox")));

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

	virtual void start() override
	{
		checkbox = (cCheckbox*)entity->child(0)->find_component(cH("Checkbox"));

		update_view();
	}
};

template<class T>
struct cDigitalDataTracker : cDataTracker
{
	cEventReceiver* event_receiver;
	cText* text;

	virtual void update_view() override
	{
		text->set_text(to_wstring(*(T*)data));
	}

	virtual void start() override
	{
		event_receiver = (cEventReceiver*)entity->child(0)->find_component(cH("EventReceiver"));
		text = (cText*)entity->child(0)->find_component(cH("Text"));

		update_view();
	}
};

template<uint N, class T>
struct cDigitalVecDataTracker : cDataTracker
{
	cEventReceiver* event_receivers[N];
	cText* texts[N];

	virtual void update_view() override
	{
		for (auto i = 0; i < N; i++)
			texts[i]->set_text(to_wstring((*(Vec<N, T>*)data)[i]));
	}

	virtual void start() override
	{
		for (auto i = 0; i < N; i++)
		{
			auto e = entity->child(i)->child(0);
			event_receivers[i] = (cEventReceiver*)e->find_component(cH("EventReceiver"));
			texts[i] = (cText*)e->find_component(cH("Text"));
		}

		update_view();
	}
};

struct cStringDataTracker : cDataTracker
{
	cText* text;

	virtual void update_view() override
	{
		text->set_text(s2w(*(std::string*)data));
	}

	virtual void start() override
	{
		text = (cText*)entity->child(0)->find_component(cH("Text"));

		update_view();
	}
};

struct cWStringDataTracker : cDataTracker
{
	cText* text;

	virtual void update_view() override
	{
		text->set_text(*(std::wstring*)data);
	}

	virtual void start() override
	{
		text = (cText*)entity->child(0)->find_component(cH("Text"));

		update_view();
	}
};