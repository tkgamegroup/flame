//MediumString curr_path;
//get_curr_path(&curr_path);

//for (std::filesystem::directory_iterator end, it(curr_path.data); it != end; it++)
//{
//	if (!std::filesystem::is_directory(it->status()))
//	{
//		if (it->path().extension() == ".cpp")
//			compile(it->path().string());
//	}
//}

//add_file_watcher(FileWatcherModeContent, curr_path.data, [&](FileChangeType type, const char *filename) {
//	if (type == FileModified)
//	{
//		std::filesystem::path path(filename);
//		if (path.extension() == ".cpp")
//			compile(filename);
//	}
//});
