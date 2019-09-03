int main(int argc, char **args)
{
	auto do_delete = [&](){
		UI::BP_Scene_Draw::SelType st;
		void *sel;
		bp_scene_draw->get_sel(st, &sel);
		if (st == UI::BP_Scene_Draw::SelTypeNode)
			bp_scene->remove_node((blueprint::Node*)sel);
		if (st == UI::BP_Scene_Draw::SelTypeLink)
			bp_scene->remove_link((blueprint::Link*)sel);
		bp_scene_draw->set_sel(UI::BP_Scene_Draw::SelTypeNone, nullptr);
	};

	auto menubar = UI::wMenuBar::create(ui);

	auto menu_file = UI::wMenu::create(ui, L"File");
	menubar->add_child(menu_file);
	auto mi_open = UI::wMenuItem::create(ui, L"Open");
	mi_open->add_listener(UI::ListenerClicked, do_open);
	menu_file->w_items()->add_child(mi_open);
	auto mi_save = UI::wMenuItem::create(ui, L"Save");
	mi_save->add_listener(UI::ListenerClicked, do_save);
	menu_file->w_items()->add_child(mi_save);

	auto menu_add = UI::wMenu::create(ui, L"Add");
	menubar->add_child(menu_add);
	auto menu_add_math = UI::wMenu::create(ui, L"Math");
	menu_add->w_items()->add_child(menu_add_math);
	for (auto &nt : math_nodes)
	{
		auto mi = UI::wMenuItem::create(ui, nt.second.c_str());
		mi->add_listener(UI::ListenerClicked, [&]() {
			bp_scene->add_node_udt(nt.first.c_str());
		});
		menu_add_math->w_items()->add_child(mi);
	}
	auto menu_add_socket = UI::wMenu::create(ui, L"Sockets");
	menu_add->w_items()->add_child(menu_add_socket);
	for (auto &nt : socket_nodes)
	{
		auto mi= UI::wMenuItem::create(ui, nt.second.c_str());
		mi->add_listener(UI::ListenerClicked, [&]() {
			bp_scene->add_node_udt(nt.first.c_str());
		});
		menu_add_socket->w_items()->add_child(mi);
	}
	auto menu_add_graphics = UI::wMenu::create(ui, L"Graphics");
	menu_add->w_items()->add_child(menu_add_graphics);
	for (auto &nt : graphics_nodes)
	{
		auto mi = UI::wMenuItem::create(ui, nt.second.c_str());
		mi->add_listener(UI::ListenerClicked, [&]() {
			bp_scene->add_node_udt(nt.first.c_str());
		});
		menu_add_graphics->w_items()->add_child(mi);
	}

	auto menu_edit = UI::wMenu::create(ui, L"Edit");
	menubar->add_child(menu_edit);
	auto mi_undo = UI::wMenuItem::create(ui, L"Undo");
	menu_edit->w_items()->add_child(mi_undo);
	auto mi_redo = UI::wMenuItem::create(ui, L"Redo");
	menu_edit->w_items()->add_child(mi_redo);
	auto mi_cut = UI::wMenuItem::create(ui, L"Cut");
	menu_edit->w_items()->add_child(mi_cut);
	auto mi_copy = UI::wMenuItem::create(ui, L"Copy");
	menu_edit->w_items()->add_child(mi_copy);
	auto mi_paste = UI::wMenuItem::create(ui, L"Paste");
	menu_edit->w_items()->add_child(mi_paste);
	auto mi_delete = UI::wMenuItem::create(ui, L"Delete");
	mi_delete->add_listener(UI::ListenerClicked, [&]() {
		do_delete();
	});
	menu_edit->w_items()->add_child(mi_delete);

	ui->root()->add_listener(UI::ListenerKeyDown, [&](int key) {
		switch (key)
		{
		case Key_Del:
			do_delete();
			break;
		case Key_O:
			if (ui->pressing_K(Key_Ctrl))
				do_open();
			break;
		case Key_S:
			if (ui->pressing_K(Key_Ctrl))
				do_save();
			break;
		}
	});
	
	auto t_status = UI::wText::create(ui);
	t_status->align = UI::AlignLeftBottomNoPadding;
	ui->root()->add_child(t_status, 1);

	app->run([&](){
		if (!w->minimized)
		{
			swprintf(buf, L"%d%% %d,%d", int(bp_scene_draw->w_scene->scale * 100.f), int(bp_scene_draw->w_scene->pos.x), int(bp_scene_draw->w_scene->pos.y));
			t_status->set_text_and_size(buf);
		}
	});

	return 0;
}
