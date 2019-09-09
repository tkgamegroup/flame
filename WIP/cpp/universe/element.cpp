namespace flame
{
	Element::Element(UI* ui)
	{
		grid_hori_count$ = 1;
	}

	void Element::remove_animations()
	{
		for (auto i = 0; i < animations$.size; i++)
		{
			auto& a = animations$[i];
			a.time = -1.f;
			a.f$.p.thiz() = &a;
			a.f$.p.e() = this;
			a.f$.exec();
		}
		animations$.resize(0);
		for (auto i = 0; i < children_1$.size; i++)
			children_1$[i]->remove_animations();
		for (auto i = 0; i < children_2$.size; i++)
			children_2$[i]->remove_animations();
	}

	void wMessageDialog::init(int font_atlas_index, const wchar_t* text)
	{
		w_text() = createT<wText>(ui, font_atlas_index);
		w_text()->text$ = text;
		add_child(w_text());

		w_ok() = createT<wButton>(ui, font_atlas_index, L"OK");
		w_ok()->align$ = AlignMiddle;
		w_ok()->mouse_listeners$.push_back(Function<MouseListenerParm>(message_dialog_ok_mouse_event$, {}));
		add_child(w_ok());
	}

	//void wYesNoDialog::init(const wchar_t *title, float sdf_scale, const wchar_t *text, const wchar_t *yes_text, const wchar_t *no_text, const std::function<void(bool)> &callback)
	//{
	//	add_listener(ListenerKeyDown, [this](int key) {
	//		switch (key)
	//		{
	//		case Key_Enter:
	//			w_yes()->on_clicked();
	//			break;
	//		case Key_Esc:
	//			w_no()->on_clicked();
	//			break;
	//		}
	//	});

	//	if (w_title())
	//		w_title()->background_col = Bvec4(200, 40, 20, 255);

	//	w_content()->layout_type = LayoutVertical;
	//	w_content()->item_padding = 8.f;
	//	if (sdf_scale > 0.f)
	//		w_content()->item_padding *= sdf_scale;

	//	if (text[0])
	//	{
	//		w_text() = wText::create(instance());
	//		w_text()->align = AlignMiddle;
	//		w_text()->sdf_scale() = sdf_scale;
	//		w_text()->set_text_and_size(text);
	//		w_content()->add_child(w_text());
	//	}
	//	else
	//		w_text() = nullptr;

	//	w_buttons() = wLayout::create(instance());
	//	w_buttons()->align = AlignMiddle;
	//	w_buttons()->layout_type = LayoutHorizontal;
	//	w_buttons()->item_padding = 4.f;
	//	if (sdf_scale > 0.f)
	//		w_buttons()->item_padding *= sdf_scale;

	//	w_yes() = wButton::create(instance());
	//	w_yes()->set_classic(yes_text, sdf_scale);
	//	w_buttons()->add_child(w_yes());

	//	w_no() = wButton::create(instance());
	//	w_no()->set_classic(no_text, sdf_scale);
	//	w_buttons()->add_child(w_no());

	//	w_yes()->add_listener(ListenerClicked, [this, callback]() {
	//		callback(true);
	//		remove_from_parent(true);
	//	});

	//	w_no()->add_listener(ListenerClicked, [this, callback]() {
	//		callback(false);
	//		remove_from_parent(true);
	//	});

	//	w_content()->add_child(w_buttons());

	//	pos = (Vec2(instance()->root()->size) - size) * 0.5f;

	//	instance()->root()->add_child(this, 0, -1, true, [](CommonData *d) {
	//		auto thiz = (Element*)d[0].p;

	//		thiz->instance()->set_focus_Element(thiz);
	//		thiz->instance()->set_dragging_Element(nullptr);
	//	}, "p", this);
	//}

	//void wInputDialog::init(const wchar_t *title, float sdf_scale, const std::function<void(bool ok, const wchar_t *input)> &callback)
	//{
	//	((wDialog*)this)->init(title, sdf_scale, false);

	//	resize_data_storage(7);

	//	event_attitude = EventBlackHole;
	//	want_key_focus = true;

	//	if (w_title())
	//		w_title()->background_col = Bvec4(200, 40, 20, 255);

	//	w_content()->layout_type = LayoutVertical;
	//	w_content()->item_padding = 8.f;
	//	if (sdf_scale > 0.f)
	//		w_content()->item_padding *= sdf_scale;

	//	w_input() = wEdit::create(instance());
	//	w_input()->sdf_scale() = sdf_scale;
	//	w_input()->set_size_by_width(100.f);
	//	w_content()->add_child(w_input());

	//	w_buttons() = wLayout::create(instance());
	//	w_buttons()->align = AlignMiddle;
	//	w_buttons()->layout_type = LayoutHorizontal;
	//	w_buttons()->item_padding = 4.f;
	//	if (sdf_scale > 0.f)
	//		w_buttons()->item_padding *= sdf_scale;

	//	w_ok() = wButton::create(instance());
	//	w_ok()->set_classic(L"OK", sdf_scale);
	//	w_buttons()->add_child(w_ok());

	//	w_cancel() = wButton::create(instance());
	//	w_cancel()->set_classic(L"Cancel", sdf_scale);
	//	w_buttons()->add_child(w_cancel());

	//	w_content()->add_child(w_buttons());

	//	pos = (Vec2(instance()->root()->size) - size) * 0.5f;

	//	w_ok()->add_listener(ListenerClicked, [this, callback]() {
	//		callback(true, w_input()->text());
	//		remove_from_parent(true);
	//	});

	//	w_cancel()->add_listener(ListenerClicked, [this, callback]() {
	//		callback(false, nullptr);
	//		remove_from_parent(true);
	//	});

	//	instance()->root()->add_child(this, 0, -1, true, [](CommonData *d) {
	//		auto thiz = (Element*)d[0].p;

	//		thiz->instance()->set_focus_Element(thiz);
	//		thiz->instance()->set_dragging_Element(nullptr);
	//	}, "p", this);
	//}

	//void wFileDialog::init(const wchar_t *title, int io, const std::function<void(bool ok, const wchar_t *filename)> &callback, const wchar_t *exts)
	//{
	//	w_pathstems() = wMenuBar::create(instance());
	//	w_pathstems()->layout_type = LayoutHorizontal;
	//	w_content()->add_child(w_pathstems());

	//	w_list() = wList::create(instance());

	//	auto upward_item = wListItem::create(instance());
	//	upward_item->w_btn()->set_text_and_size(L"..");
	//	w_list()->add_child(upward_item);

	//	upward_item->w_btn()->add_listener(ListenerDoubleClicked, [this]() {
	//		std::filesystem::path fs_path(string_storage(0));
	//		if (fs_path.root_path() != fs_path)
	//			set_path(fs_path.parent_path().generic_wstring().c_str());
	//	});

	//	w_content()->add_child(w_list());

	//	w_input() = wEdit::create(instance());
	//	w_input()->size_policy_hori = SizeFitLayout;
	//	w_input()->set_size_by_width(100.f);
	//	w_content()->add_child(w_input());

	//	w_ext() = wCombo::create(instance());
	//	w_ext()->size_policy_hori = SizeFitLayout;
	//	add_style_buttoncolor(w_ext(), 0, Vec3(0.f, 0.f, 0.7f));
	//	{
	//		if (exts == nullptr)
	//			exts = L"All Files (*.*)\0";
	//		auto sp = doublenull_string_split(exts);
	//		for (auto &s : sp)
	//		{
	//			auto i = wMenuItem::create(instance(), s.c_str());
	//			w_ext()->w_items()->add_child(i);
	//		}
	//	}

	//	w_ext()->add_listener(ListenerChanged, [this]() {
	//		set_string_storage(1, w_ext()->w_btn()->text());
	//		set_path(string_storage(0));
	//	});
	//	w_content()->add_child(w_ext());
	//	w_ext()->set_sel(0);

	//	w_buttons() = wLayout::create(instance());
	//	w_buttons()->align = AlignMiddle;
	//	w_buttons()->layout_type = LayoutHorizontal;
	//	w_buttons()->item_padding = 4.f;

	//	w_ok() = wButton::create(instance());
	//	w_ok()->set_classic(io == 0 ? L"Open" : L"Save");
	//	w_buttons()->add_child(w_ok());

	//	w_cancel() = wButton::create(instance());
	//	w_cancel()->set_classic(L"Cancel");
	//	w_buttons()->add_child(w_cancel());

	//	w_ok()->add_listener(ListenerClicked, [this, io, callback]() {
	//		auto full_filename = std::wstring(string_storage(0)) + L"/" + w_input()->text();
	//		if (io == 0)
	//		{
	//			if (std::filesystem::exists(full_filename))
	//			{
	//				callback(true, full_filename.c_str());
	//				remove_from_parent(true);
	//			}
	//			else
	//				wMessageDialog::create(instance(), L"File doesn't exist.", -1.f, L"");
	//		}
	//		else
	//		{
	//			if (std::filesystem::exists(full_filename))
	//			{
	//				wYesNoDialog::create(instance(), L"", -1.f,
	//					L"File already exists, would you like to cover it?", L"Cover", L"Cancel", [&](bool b)
	//				{
	//					if (b)
	//					{
	//						callback(true, full_filename.c_str());
	//						remove_from_parent(true);
	//					}
	//				});
	//			}
	//			else
	//			{
	//				callback(true, full_filename.c_str());
	//				remove_from_parent(true);
	//			}
	//		}
	//	});

	//	w_cancel()->add_listener(ListenerClicked, [this, callback]() {
	//		callback(false, nullptr);
	//		remove_from_parent(true);
	//	});

	//	w_content()->add_child(w_buttons());

	//	pos = (Vec2(instance()->root()->size) - size) * 0.5f;

	//	instance()->root()->add_child(this, 0, -1, true, [](CommonData *d) {
	//		auto thiz = (Element*)d[0].p;

	//		thiz->instance()->set_focus_Element(thiz);
	//		thiz->instance()->set_dragging_Element(nullptr);
	//	}, "p", this);
	//}

	//void wFileDialog::set_path(const wchar_t *path)
	//{
	//	w_pathstems()->clear_children(0, 0, -1, true);
	//	w_list()->clear_children(0, 1, -1, true);
	//	w_list()->w_sel() = nullptr;

	//	std::filesystem::path fs_path(path);
	//	{
	//		std::vector<std::wstring> stems;
	//		auto fs_root_path = fs_path.root_path();
	//		std::filesystem::path p(path);
	//		while (p != fs_root_path)
	//		{
	//			stems.push_back(p.filename().generic_wstring());
	//			p = p.parent_path();
	//		}
	//		auto root_path = fs_root_path.generic_wstring();
	//		if (root_path[root_path.size() - 1] != ':')
	//			root_path = string_cut(root_path, -1);
	//		stems.push_back(root_path);
	//		std::reverse(stems.begin(), stems.end());

	//		std::wstring curr_path(L"");
	//		auto build_btn_pop = [&](const std::wstring &path) {
	//			auto btn_pop = wMenu::create(instance(), Icon_CARET_RIGHT, 0.f);
	//			btn_pop->align = AlignMiddle;
	//			w_pathstems()->add_child(btn_pop, 0, -1, true);

	//			if (path == L"")
	//			{
	//				wchar_t drive_strings[256];
	//				GetLogicalDriveStringsW(FLAME_ARRAYSIZE(drive_strings), drive_strings);

	//				auto drives = doublenull_string_split(drive_strings);
	//				for (auto &d : drives)
	//				{
	//					auto i = wMenuItem::create(instance(), d.c_str());
	//					auto path = string_cut(d, -1);
	//					i->add_listener(ListenerClicked, [this, path]() {
	//						set_path(path.c_str());
	//					});
	//					btn_pop->w_items()->add_child(i, 0, -1, true);
	//				}
	//			}
	//			else
	//			{
	//				for (std::filesystem::directory_iterator end, it(path); it != end; it++)
	//				{
	//					if (std::filesystem::is_directory(it->status()))
	//					{
	//						auto str = it->path().filename().generic_wstring();
	//						auto i = wMenuItem::create(instance(), str.c_str());
	//						str = path + L"/" + str;
	//						i->add_listener(ListenerClicked, [this, str]() {
	//							set_path(str.c_str());
	//						});
	//						btn_pop->w_items()->add_child(i, 0, -1, true);
	//					}
	//				}
	//			}

	//			return btn_pop;
	//		};
	//		for (auto &s : stems)
	//		{
	//			auto btn_pop = build_btn_pop(curr_path);

	//			curr_path += s;

	//			auto btn_path = wButton::create(instance());
	//			btn_path->background_col.w = 0;
	//			btn_path->align = AlignMiddle;
	//			btn_path->set_classic(s.c_str());
	//			w_pathstems()->add_child(btn_path, 0, -1, true);

	//			btn_path->add_listener(ListenerClicked, [this, curr_path]() {
	//				set_path(curr_path.c_str());
	//			});

	//			curr_path += L"/";
	//		}
	//		build_btn_pop(curr_path);
	//	}

	//	std::vector<wListItem*> dir_list;
	//	std::vector<wListItem*> file_list;

	//	std::vector<std::wstring> exts_sp;
	//	auto sp = string_regex_split(std::wstring(string_storage(1)), std::wstring(LR"(\(*(\.\w+)\))"), 1);
	//	for (auto &e : sp)
	//		exts_sp.push_back(e);

	//	for (std::filesystem::directory_iterator end, it(fs_path); it != end; it++)
	//	{
	//		auto filename = it->path().filename().generic_wstring();
	//		auto item = wListItem::create(instance());

	//		if (std::filesystem::is_directory(it->status()))
	//		{
	//			dir_list.push_back(item);

	//			item->w_btn()->add_listener(ListenerDoubleClicked, [this, filename]() {
	//				set_path((std::wstring(string_storage(0)) + L"/" + filename).c_str());
	//			});
	//		}
	//		else
	//		{
	//			auto found_ext = false;
	//			for (auto &e : exts_sp)
	//			{
	//				if (e == L".*" || it->path().extension() == e)
	//				{
	//					found_ext = true;
	//					break;
	//				}
	//			}
	//			if (!found_ext)
	//				continue;

	//			file_list.push_back(item);

	//			item->w_btn()->add_listener(ListenerClicked, [this, filename]() {
	//				w_input()->set_text(filename.c_str());
	//			});

	//			item->w_btn()->add_listener(ListenerDoubleClicked, [this]() {
	//				w_ok()->on_clicked();
	//			});
	//		}
	//	}

	//	for (auto &i : dir_list)
	//		w_list()->add_child(i, 0, -1, true);
	//	for (auto &i : file_list)
	//		w_list()->add_child(i, 0, -1, true);
	//}
}

