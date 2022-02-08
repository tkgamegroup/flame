#include "selection.h"

void Selection::clear()
{
	type = tNothing;
	path.clear();
	entity = nullptr;

	frame = frames;
}

void Selection::select(const std::filesystem::path& _path)
{
	if (selecting(_path))
		return;
	clear();
	type = tFile;
	path = _path;

	frame = frames;
}

bool Selection::selecting(const std::filesystem::path& _path)
{
	return type == tFile && _path == path;
}

void Selection::select(EntityPtr e)
{
	if (selecting(e))
		return;
	clear();
	type = tEntity;
	entity = e;

	frame = frames;
}

bool Selection::selecting(EntityPtr e)
{
	return type == tEntity && entity == e;
}

Selection selection;
