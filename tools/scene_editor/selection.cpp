#include "selection.h"

void Selection::clear()
{
	frame = frames;

	type = tNothing;
	path.clear();
	entity = nullptr;
}

void Selection::select(const std::filesystem::path& _path)
{
	frame = frames;

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

void Selection::select(EntityPtr e)
{
	frame = frames;

	if (selecting(e))
		return;
	clear();
	type = tEntity;
	entity = e;
}

bool Selection::selecting(EntityPtr e)
{
	return type == tEntity && entity == e;
}

Selection selection;
