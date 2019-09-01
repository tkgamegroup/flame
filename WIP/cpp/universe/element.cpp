namespace flame
{
	Element::Element(UI* ui)
	{
		grid_hori_count$ = 1;

		scroll_offset$ = 0.f;
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

	void Element::create_from_typeinfo(UI * ui, int font_atlas_index, VariableInfo * info, void* p, Element * dst)
	{
		switch (info->tag())
		{
		case VariableTagEnumSingle:
		{
			auto c = createT<wCombo>(ui, font_atlas_index, find_enum(info->type_hash()), (char*)p + info->offset());
			dst->add_child(c, 0, -1, true);
		}
			break;
		case VariableTagEnumMulti:
			break;
		case VariableTagVariable:
		{
			switch (info->type_hash())
			{
			case cH("bool"):
			{
				auto c = createT<wCheckbox>(ui, (char*)p + info->offset());
				dst->add_child(c, 0, -1, true);
			}
				break;
			case cH("uint"):
			case cH("int"):
			case cH("Ivec2"):
			case cH("Ivec3"):
			case cH("Ivec4"):
			case cH("float"):
			case cH("Vec2"):
			case cH("Vec3"):
			case cH("Vec4"):
			{
				auto pp = (Ivec2*)((char*)p + info->offset());
				auto count = info->size() / 4;

				for (auto i = 0; i < count; i++)
				{
					auto e = createT<wEdit>(ui, font_atlas_index, info, &((*pp)[i]));
					e->size_policy_hori$ = SizeFitLayout;
					e->set_size_by_width(10.f);
					dst->add_child(e, 0, -1, true);
				}
			}
				break;
			case cH("uchar"):
			case cH("Bvec2"):
			case cH("Bvec3"):
			case cH("Bvec4"):
			{
				auto pp = (Ivec2*)((char*)p + info->offset());
				auto count = info->size();

				for (auto i = 0; i < count; i++)
				{
					auto e = createT<wEdit>(ui, font_atlas_index, info, &((*pp)[i]));
					e->size_policy_hori$ = SizeFitLayout;
					e->set_size_by_width(10.f);
					dst->add_child(e, 0, -1, true);
				}
			}
				break;
			}
		}
			break;
		}
	}

	FLAME_PACKAGE_BEGIN_1(ObjGeneratorData, UIPtr, ui, p)
	FLAME_PACKAGE_END_1

	Element* Element::create_from_file(UI * ui, SerializableNode * src)
	{
		return (Element*)src->unserialize(find_udt(cH("Element")), Function<SerializableNode::ObjGeneratorParm>([](SerializableNode::ObjGeneratorParm & p) {
			auto c = p.get_capture<ObjGeneratorData>();
			auto w = create(c.ui());
			p.out_obj() = w;

			if (p.parent())
			{
				w->parent = (ElementPtr)p.parent();
				if (p.att_hash() == cH("children_1"))
					w->layer = 0;
				else /* if (att_hash == cH("children_2")) */
					w->layer = 1;
			}
		}, { ui }));
	}

	void wCombo::init(int font_atlas_index, void* _enum_info, void* _target)
	{
		if (enum_info())
		{
			auto e = (EnumInfo*)enum_info();

			for (auto i = 0; i < e->item_count(); i++)
				w_items()->add_child(createT<wMenuItem>(ui, font_atlas_index, s2w(e->item(i)->name()).c_str()));
		}

		if (target())
		{
			auto p = (int*)target();
			if (enum_info())
			{
				auto e = (EnumInfo*)enum_info();
				set_sel(e->find_item(*p), true);
			}
			else
				set_sel(*p, true);
		}
	}

	void wCombo::set_sel(int idx, bool from_inner)
	{
		if (target())
		{
			auto p = (int*)target();
			if (enum_info())
			{
				auto e = (EnumInfo*)enum_info();
				*p = e->item(sel())->value();
			}
			else
				*p = sel();
		}
	}

	void edit_key_event$(Element::KeyListenerParm &p)
	{
		auto thiz = (wEditPtr)p.thiz();
		if (p.action() == KeyStateNull)
		{
			auto info = (VariableInfo*)thiz->info();
			if (info && p.value() != '\b' && p.value() != 22 && p.value() != 27)
			{
				switch (info->type_hash())
				{
				case cH("int"):
				case cH("Ivec2"):
				case cH("Ivec3"):
				case cH("Ivec4"):
					if (p.value() == L'-')
					{
						if (thiz->cursor() != 0 || thiz->text$.v[0] == L'-')
							return;
					}
					if (p.value() < '0' || p.value() > '9')
						return;
					break;
				case cH("uint"):
				case cH("uchar"):
				case cH("Bvec2"):
				case cH("Bvec3"):
				case cH("Bvec4"):
					if (p.value() < '0' || p.value() > '9')
						return;
					break;
				case cH("float"):
				case cH("Vec2"):
				case cH("Vec3"):
				case cH("Vec4"):
					if (p.value() == L'.')
					{
						if (thiz->text$.find(L'.') != -1)
							return;
					}
					if (p.value() < '0' || p.value() > '9')
						return;
					break;
				}
			}
		}
	}

	void edit_focus_event$(Element::FoucusListenerParm& p)
	{
		if (p.is_keyfocus() != 1)
			return;

		auto thiz = (wEditPtr)p.thiz();
		switch (p.type())
		{
		case Focus_Gain:
			if (thiz->target())
				thiz->cursor() = thiz->text$.size;
			break;
		case Focus_Lost:
		{
			auto info = (VariableInfo*)thiz->info();
			if (info && thiz->target())
			{
				switch (info->type_hash())
				{
				case cH("uint"):
				{
					auto v = (uint*)(thiz->target());
					*v = stoi1(thiz->text$.v);
					thiz->text$ = to_wstring(*v);
				}
				break;
				case cH("int"):
				case cH("Ivec2"):
				case cH("Ivec3"):
				case cH("Ivec4"):
				{
					auto v = (int*)(thiz->target());
					*v = stoi1(thiz->text$.v);
					thiz->text$ = to_wstring(*v);
				}
				break;
				case cH("float"):
				case cH("Vec2"):
				case cH("Vec3"):
				case cH("Vec4"):
				{
					auto v = (float*)(thiz->target());
					*v = stof1(thiz->text$.v);
					thiz->text$ = to_wstring(*v);
				}
				break;
				case cH("uchar"):
				case cH("Bvec2"):
				case cH("Bvec3"):
				case cH("Bvec4"):
				{
					auto v = (uchar*)(thiz->target());
					*v = stob1(thiz->text$.v);
					thiz->text$ = to_wstring(*v);
				}
				break;
				}
				thiz->cursor() = 0;
			}
		}
			break;
		}
	}

	void wEdit::init(int font_atlas_index, void* _info, void* _target)
	{
		focus_listeners$.push_back(Function<FoucusListenerParm>(edit_focus_event$, {}));

		if (info() && target())
		{
			auto vinfo = (VariableInfo*)_info;
			switch (vinfo->type_hash())
			{
			case cH("uint"):
			{
				auto p = (uint*)target();
				text$ = to_wstring(*p);
			}
				break;
			case cH("int"):
			case cH("Ivec2"):
			case cH("Ivec3"):
			case cH("Ivec4"):
			{
				auto p = (int*)target();
				text$ = to_wstring(*p);
			}
				break;
			case cH("float"):
			case cH("Vec2"):
			case cH("Vec3"):
			case cH("Vec4"):
			{
				auto p = (float*)target();
				text$ = to_wstring(*p);
			}
				break;
			case cH("uchar"):
			case cH("Bvec2"):
			case cH("Bvec3"):
			case cH("Bvec4"):
			{
				auto p = (uchar*)target();
				text$ = to_wstring(*p);
			}
				break;
			}
		}
	}

	void splitter_mouse_event$(Element::MouseListenerParm& p)
	{
		if (!p.is_move())
			return;

		auto thiz = (wSplitterPtr)p.thiz();
		if (thiz == thiz->ui->dragging_element())
		{
			auto target1 = thiz->w_target1();
			auto target2 = thiz->w_target2();
			auto d = p.value() / thiz->parent->scale$;
			if (thiz->dir() == 0)
			{
				target1->set_width(target1->size$.x + d.x);
				target2->set_width(target2->size$.x - d.x);
			}
			else
			{
				target1->set_height(target1->size$.y + d.y);
				target2->set_height(target2->size$.y - d.y);
			}
		}
	}

	void wSplitter::init(int _dir, Element* target1, Element* target2)
	{
		dir() = _dir;
		w_target1() = target1;
		w_target2() = target2;

		size$ = Vec2(10.f);
		background_col$ = Bvec4(255, 225, 255, 255);

		if (dir() == 0)
			size_policy_vert$ = SizeFitLayout;
		else
			size_policy_hori$ = SizeFitLayout;

		mouse_listeners$.push_back(Function<MouseListenerParm>(splitter_mouse_event$, {}));
	}

	void wDialog::init(bool resize, bool modual)
	{
		wLayout::init();

		auto radius = 8.f;

		inner_padding$ = Vec4(radius);
		background_col$ = Colorf(0.5f, 0.5f, 0.5f, 0.9f);
		item_padding$ = radius;
		if (resize)
		{
			size_policy_hori$ = SizeFitLayout;
			size_policy_vert$ = SizeFitLayout;
			clip$ = true;
		}

		if (resize)
		{
			w_scrollbar() = createT<wScrollbar>(ui, this);
			add_child(w_scrollbar(), 1);

			w_sizedrag() = createT<wSizeDrag>(ui, this);
			w_sizedrag()->min_size() = Vec2(10.f);
			set_size(Vec2(100.f));
			add_child(w_sizedrag(), 1);
		}
		else
			w_sizedrag() = nullptr;

		if (modual)
		{
			pos$ = (Vec2(ui->root()->size$) - size$) * 0.5f;

			ui->root()->add_child(this, 0, -1, true);
		}
	}

	void message_dialog_ok_mouse_event$(Element::MouseListenerParm& p)
	{
		if (!p.is_clicked())
			return;

		p.thiz()->remove_from_parent(true);
	}

	void wMessageDialog::init(int font_atlas_index, const wchar_t* text)
	{
		((wDialog*)this)->init(false, true);

		want_key_focus$ = true;

		layout_type$ = LayoutVertical;
		item_padding$ = 8.f;

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
	//	((wDialog*)this)->init(title, sdf_scale, false);

	//	resize_data_storage(7);

	//	event_attitude = EventBlackHole;
	//	want_key_focus = true;

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

	//wTextPtr &wYesNoDialog::w_text()
	//{
	//	return *((wTextPtr*)&data_storage(3).p);
	//}

	//wLayoutPtr &wYesNoDialog::w_buttons()
	//{
	//	return *((wLayoutPtr*)&data_storage(4).p);
	//}

	//wButtonPtr &wYesNoDialog::w_yes()
	//{
	//	return *((wButtonPtr*)&data_storage(5).p);
	//}

	//wButtonPtr &wYesNoDialog::w_no()
	//{
	//	return *((wButtonPtr*)&data_storage(6).p);
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

	//wEditPtr &wInputDialog::w_input()
	//{
	//	return *((wEditPtr*)&data_storage(3).p);
	//}

	//wLayoutPtr &wInputDialog::w_buttons()
	//{
	//	return *((wLayoutPtr*)&data_storage(4).p);
	//}

	//wButtonPtr &wInputDialog::w_ok()
	//{
	//	return *((wButtonPtr*)&data_storage(5).p);
	//}

	//wButtonPtr &wInputDialog::w_cancel()
	//{
	//	return *((wButtonPtr*)&data_storage(6).p);
	//}

	//void wFileDialog::init(const wchar_t *title, int io, const std::function<void(bool ok, const wchar_t *filename)> &callback, const wchar_t *exts)
	//{
	//	((wDialog*)this)->init(title, -1.f, true);

	//	resize_data_storage(10);
	//	resize_string_storage(2);

	//	event_attitude = EventBlackHole;
	//	want_key_focus = true;

	//	if (w_title())
	//		w_title()->background_col = Bvec4(200, 40, 20, 255);

	//	set_string_storage(0, get_curr_path());
	//	set_string_storage(1, L"");

	//	w_sizedrag()->min_size() = Vec2(300.f, 400.f);
	//	set_size(w_sizedrag()->min_size());
	//	event_attitude = EventBlackHole;

	//	w_content()->size_policy_hori = SizeFitLayout;
	//	w_content()->size_policy_vert = SizeFitLayout;
	//	w_content()->layout_type = LayoutVertical;
	//	w_content()->item_padding = 8.f;

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

	//wMenuBarPtr &wFileDialog::w_pathstems()
	//{
	//	return *((wMenuBarPtr*)&data_storage(3).p);
	//}

	//wListPtr &wFileDialog::w_list()
	//{
	//	return *((wListPtr*)&data_storage(4).p);
	//}

	//wEditPtr &wFileDialog::w_input()
	//{
	//	return *((wEditPtr*)&data_storage(5).p);
	//}

	//wComboPtr &wFileDialog::w_ext()
	//{
	//	return *((wComboPtr*)&data_storage(6).p);
	//}

	//wLayoutPtr &wFileDialog::w_buttons()
	//{
	//	return *((wLayoutPtr*)&data_storage(7).p);
	//}

	//wButtonPtr &wFileDialog::w_ok()
	//{
	//	return *((wButtonPtr*)&data_storage(8).p);
	//}

	//wButtonPtr &wFileDialog::w_cancel()
	//{
	//	return *((wButtonPtr*)&data_storage(9).p);
	//}

	//const wchar_t *wFileDialog::curr_path()
	//{
	//	return string_storage(0);
	//}

	//int wFileDialog::curr_path_len()
	//{
	//	return string_storage_len(0);
	//}

	//void wFileDialog::set_curr_path(const wchar_t *path)
	//{
	//	return set_string_storage(0, path);
	//}

	//const wchar_t *wFileDialog::curr_exts()
	//{
	//	return string_storage(1);
	//}

	//int wFileDialog::curr_exts_len()
	//{
	//	return string_storage_len(1);
	//}

	//void wFileDialog::set_curr_exts(const wchar_t *exts)
	//{
	//	return set_string_storage(1, exts);
	//}

	//void wFileDialog::set_path(const wchar_t *path)
	//{
	//	w_pathstems()->clear_children(0, 0, -1, true);
	//	w_list()->clear_children(0, 1, -1, true);
	//	w_list()->w_sel() = nullptr;

	//	set_string_storage(0, path);
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
	//			item->w_btn()->set_text_and_size((Icon_FOLDER_O + std::wstring(L" ") + filename).c_str());
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

	//			item->w_btn()->set_text_and_size((Icon_FILE_O + std::wstring(L" ") + filename).c_str());
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

