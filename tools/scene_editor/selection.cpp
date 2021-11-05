#include "selection.h"

void Selection::clear()
{
	type = Nothing;
	path.clear();
}

void Selection::select(const std::filesystem::path& _path)
{
	if (selecting(_path))
		return;
	clear();
	type = File;
	path = _path;
}

bool Selection::selecting(const std::filesystem::path& _path)
{
	return type == File && _path == path;
}

Selection selection;
