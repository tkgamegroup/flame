#include "selection.h"

void Selection::clear()
{
	type = tNothing;
	path.clear();
	entity = nullptr;
}

void Selection::select(const std::filesystem::path& _path)
{
	if (selecting(_path))
		return;
	clear();
	type = tFile;
	path = _path;
}

bool Selection::selecting(const std::filesystem::path& _path)
{
	return type == tFile && _path == path;
}

Selection selection;
