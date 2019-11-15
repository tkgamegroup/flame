namespace flame
{
	void wFileDialog::init(const wchar_t *title, int io, const std::function<void(bool ok, const wchar_t *filename)> &callback, const wchar_t *exts)
	{
		w_input() = wEdit::create(instance());
		w_input()->size_policy_hori = SizeFitLayout;
		w_input()->set_size_by_width(100.f);
		w_content()->add_child(w_input());

		w_ext() = wCombo::create(instance());
		w_ext()->size_policy_hori = SizeFitLayout;
		add_style_buttoncolor(w_ext(), 0, Vec3(0.f, 0.f, 0.7f));
		{
			if (exts == nullptr)
				exts = L"All Files (*.*)\0";
			auto sp = doublenull_string_split(exts);
			for (auto &s : sp)
			{
				auto i = wMenuItem::create(instance(), s.c_str());
				w_ext()->w_items()->add_child(i);
			}
		}

		w_ext()->add_listener(ListenerChanged, [this]() {
			set_string_storage(1, w_ext()->w_btn()->text());
			set_path(string_storage(0));
		});
		w_content()->add_child(w_ext());
		w_ext()->set_sel(0);

		w_ok() = wButton::create(instance());
		w_ok()->set_classic(io == 0 ? L"Open" : L"Save");
		w_buttons()->add_child(w_ok());

		w_cancel() = wButton::create(instance());
		w_cancel()->set_classic(L"Cancel");
		w_buttons()->add_child(w_cancel());

		w_ok()->add_listener(ListenerClicked, [this, io, callback]() {
			auto full_filename = std::wstring(string_storage(0)) + L"/" + w_input()->text();
			if (io == 0)
			{
				if (std::filesystem::exists(full_filename))
				{
					callback(true, full_filename.c_str());
					remove_from_parent(true);
				}
				else
					wMessageDialog::create(instance(), L"File doesn't exist.", -1.f, L"");
			}
			else
			{
				if (std::filesystem::exists(full_filename))
				{
					wYesNoDialog::create(instance(), L"", -1.f,
						L"File already exists, would you like to cover it?", L"Yes", L"Cancel", [&](bool b)
					{
						if (b)
						{
							callback(true, full_filename.c_str());
							remove_from_parent(true);
						}
					});
				}
				else
				{
					callback(true, full_filename.c_str());
					remove_from_parent(true);
				}
			}
		});

		w_cancel()->add_listener(ListenerClicked, [this, callback]() {
			callback(false, nullptr);
			remove_from_parent(true);
		});

		w_content()->add_child(w_buttons());
	}

	void wFileDialog::set_path(const wchar_t *path)
	{
		{
			std::wstring curr_path(L"");
				if (path == L"")
				{
					wchar_t drive_strings[256];
					GetLogicalDriveStringsW(FLAME_ARRAYSIZE(drive_strings), drive_strings);

					auto drives = doublenull_string_split(drive_strings);
					for (auto &d : drives)
					{
						auto i = wMenuItem::create(instance(), d.c_str());
						auto path = string_cut(d, -1);
						i->add_listener(ListenerClicked, [this, path]() {
							set_path(path.c_str());
						});
						btn_pop->w_items()->add_child(i, 0, -1, true);
					}
				}

				return btn_pop;
			};
		}

		std::vector<std::wstring> exts_sp;
		auto sp = string_regex_split(std::wstring(string_storage(1)), std::wstring(LR"(\(*(\.\w+)\))"), 1);
		for (auto &e : sp)
			exts_sp.push_back(e);

		for (std::filesystem::directory_iterator end, it(fs_path); it != end; it++)
		{
			auto filename = it->path().filename().generic_wstring();
			auto item = wListItem::create(instance());

			if (!std::filesystem::is_directory(it->status()))
			{
				auto found_ext = false;
				for (auto &e : exts_sp)
				{
					if (e == L".*" || it->path().extension() == e)
					{
						found_ext = true;
						break;
					}
				}
				if (!found_ext)
					continue;
			}
		}
	}
}

