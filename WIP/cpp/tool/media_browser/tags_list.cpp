TagsListLayout::TagsListLayout(UI::Instance *ui) :
	UI::Layout(ui)
{
	size_policy_hori = UI::SizeFitLayout;
	size_policy_vert = UI::SizeFitLayout;
	layout_type = UI::LayoutVertical;
	item_padding = 4.f;

	top = new UI::Layout(ui);
	top->align = UI::AlignLittleEnd;
	top->layout_type = UI::LayoutHorizontal;
	top->item_padding = 2.f;

	clear = new UI::Button(ui);
	clear->inner_padding = Vec4(4.f, 4.f, 2.f, 2.f);
	clear->align = UI::AlignLittleEnd;
	clear->set_text_and_size(L"Clear");
	clear->add_style_T<UI::ButtonStyleColor>(0, Vec3(0.f, 1.f, 1.f));
	top->add_widget(-1, clear);

	filter = new UI::Edit(ui);
	filter->align = UI::AlignLittleEnd;
	filter->set_size_by_width(100.f);
	top->add_widget(-1, filter);

	add_widget(-1, top);

	list = new UI::List(ui);
	list->background_col.w = 200;
	list->align = UI::AlignLittleEnd;
	add_widget(-1, list);
}

enum ShowType
{
	ShowAll,
	ShowUntagged,
	ShowTagged
};
ShowType show_type = ShowTagged;

void select_new_candidates()
{
	grid_pic_candidates.clear();

	switch (show_type)
	{
	case ShowAll:
		for (auto &p : pics)
			grid_pic_candidates.push_back(p.get());
		break;
	case ShowUntagged:
		for (auto &p : pics)
		{
			if (p->tags.empty() || std::filesystem::path(p->filename).stem().wstring()[0] != L'~')
				grid_pic_candidates.push_back(p.get());
		}
		break;
	case ShowTagged:
		for (auto &t : tags)
		{
			if (t->use)
			{
				for (auto src : t->pics)
				{
					auto has = false;
					for (auto p : grid_pic_candidates)
					{
						if (p == src)
						{
							has = true;
							break;
						}
					}
					if (!has)
						grid_pic_candidates.push_back(src);
				}
			}
		}
		break;
	}

	grid_curr_page = 0;
	grid_total_page = grid_pic_candidates.size() / (grid_hori_pic_cnt * grid_vert_pic_cnt) + 1;
	set_grid_page_text();

	clear_grids();
	for (auto i = 0; i < grid_hori_pic_cnt * grid_vert_pic_cnt; i++)
	{
		if (i > (int)grid_pic_candidates.size() - 1)
			break;

		grid_slots[i] = grid_pic_candidates[i];
	}
	create_grids();
}

TagsListItem::TagsListItem(UI::Instance *ui, Tag *_t) :
	UI::ListItem(ui),
	t(_t)
{
	w_c = new UI::Checkbox(ui);
	w_c->align = UI::AlignLittleEnd;
	w_c->checked = t->use;
	w_c->add_style_T<UI::ButtonStyleColor>(0, Vec3(0.f, 0.f, 0.7f));
	w_c->add_changed_listener([this]() {
		t->use = w_c->checked;
		select_new_candidates();
	});
	add_widget(0, w_c);

	w_right = new UI::Layout(ui);
	w_right->align = UI::AlignFloatRight;
	w_right->layout_type = UI::LayoutHorizontal;
	w_right->item_padding = 1.f;

	w_t = new UI::Text(ui);
	w_t->align = UI::AlignLittleEnd;
	w_t->text_col = Bvec4(120, 190, 40, 255);
	w_t->set_text_and_size(std::to_wstring((int)t->pics.size()).c_str());
	w_right->add_widget(-1, w_t);

	w_edt = new UI::Button(ui);
	w_edt->align = UI::AlignLittleEnd;
	w_edt->background_col.w = 0.f;
	w_edt->add_style_T<UI::ButtonStyleColor>(0, Vec3(280.f, 0.5f, 1.f));
	w_edt->set_text_and_size(UI::Icon_PENCIL);
	w_right->add_widget(-1, w_edt);

	w_edt->add_clicked_listener([this]() {
		auto d = new UI::InputDialog(::ui, L"Tag Name", -1.f, [this](bool ok, const wchar_t *input) {
			if (ok)
			{
				if (input[0] == 0)
				{
					new UI::MessageDialog(::ui, L"", -1.f, L"Tag name cannot be null.");
					return;
				}
				std::wstring name(input);
				if (t->name == name)
					return;
				auto exist_tag = find_tag(name);
				if (exist_tag && exist_tag != t)
				{
					new UI::YesNoDialog(::ui, L"", -1.f, (L"Tag " + name + L" already exists, would you like to combine?").c_str(),
						L"Yes", L"No", [this, exist_tag](bool ok)
					{
						if (ok)
						{
							for (auto &p : t->pics)
							{
								if (!p->has_tag(exist_tag))
									p->tags.push_back(exist_tag);
							}
							exist_tag->pics.insert(exist_tag->pics.begin(), t->pics.begin(), t->pics.end());
							for (auto it = tags.begin(); it != tags.end(); it++)
							{
								if (it->get() == t)
								{
									delete_tag(t);
									tags.erase(it);
									break;
								}
							}
							save_tags();
							refresh_tags_list();
						}
					});
				}
				else
				{
					t->name = name;
					sort_tags();
					save_tags();
					update_tag(this);
					refresh_tags_list();
				}
			}
		});
		d->w_input->set_text_and_len(t->name.c_str());
	});

	w_del = new UI::Button(ui);
	w_del->align = UI::AlignLittleEnd;
	w_del->background_col.w = 0.f;
	w_del->add_style_T<UI::ButtonStyleColor>(0, Vec3(280.f, 0.5f, 1.f));
	w_del->text_col = Bvec4(255, 0, 0, 255);
	w_del->set_text_and_size(UI::Icon_TIMES);
	w_right->add_widget(-1, w_del);

	w_del->add_clicked_listener([_t]() {
		new UI::YesNoDialog(::ui, L"", -1.f, (L"Are you sure to DELETE tag " + _t->name + L" ?").c_str(), L"Delete", L"Cancel", [&](bool ok) {
			if (ok)
			{
				for (auto it = tags.begin(); it != tags.end(); it++)
				{
					if (it->get() == _t)
					{
						delete_tag(_t);
						tags.erase(it);
						break;
					}
				}
				save_tags();
				refresh_tags_list();
			}
		});
	});

	add_widget(-1, w_right);
}

TagsListLayout *w_tags_list;

void create_tags_list()
{
	w_tags_list = new TagsListLayout(ui);
	w_tags_list->visible = false;
	auto sizedrag = new UI::Sizedrag(ui, w_tags_list);
	sizedrag->min_size = Vec2(160.f, 600.f);
	w_tags_list->set_size(sizedrag->min_size);
	w_tags_list->add_widget(-1, sizedrag);

	w_tags_list->clear->add_clicked_listener([]() {
		auto l = w_tags_list->list->w_items;
		for (auto i = 0; i < l->widget_count(); i++)
		{
			auto item = (TagsListItem*)l->widget(i);
			item->w_c->checked = false;
		}
		for (auto &t : tags)
			t->use = false;
		select_new_candidates();
	});

	w_tags_list->filter->add_changed_listener([]() {
		auto l = w_tags_list->list->w_items;
		for (auto i = 0; i < l->widget_count(); i++)
		{
			auto item = (TagsListItem*)l->widget(i);
			item->visible = (item->t->name.find(w_tags_list->filter->text.data) 
				!= std::wstring::npos);
		}
		l->arrange();
	});

	refresh_tags_list();

	auto w_tags_btn = new UI::Button(ui);
	UI::set_button_classic(w_tags_btn, L"Tags");
	ui->root()->add_widget(-1, w_tags_btn);

	w_tags_btn->add_clicked_listener([]() {
		w_tags_list->set_visibility(!w_tags_list->visible);
	});

	w_tags_list->pos = Vec2(5.f, w_tags_btn->size.y + 4.f);

	auto w_add_btn = new UI::Button(ui);
	UI::set_button_classic(w_add_btn, UI::Icon_PLUS);
	w_add_btn->pos.x = w_tags_btn->size.x + 4.f;
	ui->root()->add_widget(-1, w_add_btn);

	w_add_btn->add_clicked_listener([&]() {
		new UI::InputDialog(ui, L"Tag Name", -1.f, [&](bool ok, const wchar_t *input) {
			if (ok)
			{
				for (auto &t : tags)
				{
					if (t->name == input)
					{
						new UI::MessageDialog(ui, L"", -1.f, L"Tag already exists.");
						return;
					}
				}
				add_tag(input);
				save_tags();
				refresh_tags_list();
			}
		});
	});

	auto w_show_type = new UI::Combo(ui);
	w_show_type->pos.x = w_add_btn->pos.x + w_add_btn->size.x + 4.f;
	w_show_type->add_style_T<UI::ButtonStyleColor>(0, Vec3(0.f, 0.f, 0.7f));

	{
		auto i0 = w_show_type->add_item(L"All");
		i0->background_col.w = 255;
		i0->add_style_T<UI::ButtonStyleColor>(0, Vec3(0.f, 0.f, 0.7f));

		auto i1 = w_show_type->add_item(L"Untagged");
		i1->background_col.w = 255;
		i1->add_style_T<UI::ButtonStyleColor>(0, Vec3(0.f, 0.f, 0.7f));

		auto i2 = w_show_type->add_item(L"Tagged");
		i2->background_col.w = 255;
		i2->add_style_T<UI::ButtonStyleColor>(0, Vec3(0.f, 0.f, 0.7f));
	}

	w_show_type->set_sel(2);

	ui->root()->add_widget(-1, w_show_type);

	w_show_type->add_changed_listener([w_show_type]() {
		switch (w_show_type->sel)
		{
		case 0:
			show_type = ShowAll;
			break;
		case 1:
			show_type = ShowUntagged;
			break;
		case 2:
			show_type = ShowTagged;
			break;
		}
		select_new_candidates();
	});

	ui->root()->add_widget(-1, w_tags_list);
}

void refresh_tags_list()
{
	w_tags_list->list->w_items->clear_widgets(0, -1, true);

	for (auto &tt : tags)
	{
		auto t = tt.get();

		auto item = new TagsListItem(ui, t);
		item->w_btn->set_text_and_size(t->name.c_str());
		w_tags_list->list->add_item(item, true);
	}
};

void update_tag(TagsListItem *item)
{
	item->w_btn->set_text_and_size(item->t->name.c_str());
	item->w_t->set_text_and_size(std::to_wstring((int)item->t->pics.size()).c_str());
}

void update_all_tags()
{
	auto l = w_tags_list->list->w_items;
	for (auto i = 0; i < l->widget_count(); i++)
		update_tag((TagsListItem*)l->widget(i));
}
