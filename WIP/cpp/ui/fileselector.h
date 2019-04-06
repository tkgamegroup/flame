		enum FileSelectorCreateFlag
		{
			FileSelectorNoFiles = 1 << 0,
			FileSelectorNoRightArea = 1 << 1,
			FileSelectorTreeMode = 1 << 2
		};

		struct FileSelector : Window
		{
			ImGui::Splitter splitter;

			struct ItemData
			{
				std::string value;
				std::string name;
				std::string filename;
			};

			struct DirItem : ItemData
			{
				std::vector<std::unique_ptr<DirItem>> dir_list;
				std::vector<std::unique_ptr<FileItem>> file_list;
			};

			struct FileItem : ItemData
			{
				FileType file_type;
				std::shared_ptr<Texture> preview_image;

				FileItem(FileSelector *_parent);
				virtual ~FileItem();
			};

			DirItem curr_dir;

			std::function<bool(std::string)> callback;

			FileWatcher *file_watcher;

			std::shared_ptr<Texture> folder_image;
			std::shared_ptr<Texture> file_image;
			std::shared_ptr<Texture> empty_image;

			FileSelector(const std::string &_title, FileSelectorIo io, const std::string &_default_dir = "",
				unsigned int window_flags = WindowCreateFlagNull, unsigned int flags = FileSelectorCreateFlagNull);
		};

		struct DirSelectorDialog : FileSelector
		{
			DirSelectorDialog();
			static void open(const std::string &default_dir, const std::function<bool(std::string)> &_callback);
		};