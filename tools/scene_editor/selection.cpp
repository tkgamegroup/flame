#include "selection.h"

void Selection::clear()
{
	frame = frames;

	switch (type)
	{
	case tPath:
		delete (std::filesystem::path*)content;
		break;
	case tEntity:
		((EntityPtr)content)->message_listeners.remove("editor_selection"_h);
		break;
	}
	type = tNothing;
	content = nullptr;
}

void Selection::select(const std::filesystem::path& _path)
{
	frame = frames;

	if (selecting(_path))
		return;
	clear();
	type = tPath;
	content = new std::filesystem::path(_path);
}

bool Selection::selecting(const std::filesystem::path& _path)
{
	return type == tPath && _path == *(std::filesystem::path*)content;
}

void Selection::select(EntityPtr e)
{
	frame = frames;

	if (selecting(e))
		return;
	clear();
	type = tEntity;
	content = e;

	e->message_listeners.add([](uint hash, void*, void*) {
		if (hash == "destroyed"_h)
			selection.clear();
	}, "editor_selection"_h);
}

bool Selection::selecting(EntityPtr e)
{
	return type == tEntity && content == e;
}

Selection selection;
