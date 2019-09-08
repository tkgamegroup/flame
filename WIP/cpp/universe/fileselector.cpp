
		FileSelector::FileSelector(const std::string &_title, FileSelectorIo io, const std::string &_default_dir, unsigned int window_flags, unsigned int flags)
		{
			file_watcher = nullptr;

			if (_default_dir != "")
			{
				default_dir = _default_dir;
				set_current_path(default_dir);
			}
			else
				set_current_path(get_app_path());

			if (tree_mode)
			{
				folder_image = get_texture("folder.png");
				file_image = get_texture("file.png");
				empty_image = get_texture("empty.png");
			}
		}

		void FileSelector::set_current_path(const std::string &s)
		{
			if (s == curr_dir.filename)
				return;

			curr_dir.filename = s;
			need_refresh = true;
			if (tree_mode)
			{
				std::filesystem::path path(s);
				auto str = path.filename().string();
				curr_dir.value = str;
			}

			if (file_watcher)
				remove_file_watcher(file_watcher);
			file_watcher = add_file_watcher(FileWatcherModeAll, s);
		}

		void FileSelector::refresh()
		{
			std::string select_dir_filename;
			if (select_dir)
				select_dir_filename = select_dir->filename;

			if (!on_refresh())
				return;

			std::function<void(DirItem *, const std::filesystem::path &)> fIterDir;
			fIterDir = [&](DirItem *dst, const std::filesystem::path &src) {
				if (src.string() == select_dir_filename)
					select_dir = dst;

				std::filesystem::directory_iterator end_it;
				for (std::filesystem::directory_iterator it(src); it != end_it; it++)
				{
					if (std::filesystem::is_directory(it->status()))
					{
						if (tree_mode)
							fIterDir(i, it->path());
						dst->dir_list.emplace_back(i);
					}
					else if (enable_file)
					{
						auto ext = it->path().extension().string();
						const char *prefix;
						if (is_text_file(ext))
						{
							i->file_type = FileTypeText;
							prefix = ICON_FA_FILE_TEXT_O" ";
						}
						else if (is_image_file(ext))
						{
							i->file_type = FileTypeImage;
							prefix = ICON_FA_FILE_IMAGE_O" ";
							if (tree_mode)
							{
								i->preview_image = get_texture(i->filename);
								if (!i->preview_image)
									i->preview_image = empty_image;
								else
									increase_texture_ref(i->preview_image.get());
							}
						}
						else if (is_model_file(ext))
						{
							i->file_type = FileTypeModel;
							prefix = ICON_FA_FILE_O" ";
						}
						else if (is_terrain_file(ext))
						{
							i->file_type = FileTypeTerrain;
							prefix = ICON_FA_FILE_O" ";
						}
						else if (is_scene_file(ext))
						{
							i->file_type = FileTypeScene;
							prefix = ICON_FA_FILE_O" ";
						}
						else
							prefix = ICON_FA_FILE_O" ";
						i->name = prefix + str;

						on_add_file_item(i);
					}
				}
			};

			fIterDir(&curr_dir, curr_dir.filename);
		}

		void FileSelector::on_show()
		{
			if (file_watcher->dirty)
			{
				file_watcher->dirty = false;
				need_refresh = true;
			}

			if (!tree_mode)
			{
				on_top_area_show();
				//		if (ImGui::BeginDragDropSource())
				//		{
				//			ImGui::SetDragDropPayload("file", i->filename.c_str(), i->filename.size() + 1);
				//			ImGui::TextUnformatted(i->filename.c_str());
				//			ImGui::EndDragDropSource();
				//		}

				on_bottom_area_show();
			}
			else
			{
				auto need_open = select_dir != nullptr;
				std::function<void(DirItem *)> fShowDir;
				fShowDir = [&](DirItem *src) {
					auto node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
					node_flags |= select_dir == src ? ImGuiTreeNodeFlags_Selected : 0;
					if (src->dir_list.empty())
						node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
					else
					{
						if (select_dir == src)
							need_open = false;
						if (need_open)
							ImGui::SetNextTreeNodeOpen(true);
					}
					auto node_open = ImGui::TreeNodeEx(src->name.c_str(), node_flags);
					if (ImGui::IsItemClicked())
						select_dir = src;
					if (node_open && !(node_flags & ImGuiTreeNodeFlags_Leaf))
					{
						for (auto &d : src->dir_list)
							fShowDir(d.get());
						ImGui::TreePop();
					}
				};
				fShowDir(&curr_dir);
			}

			if (enable_right_area)
				on_right_area_show();
		}

