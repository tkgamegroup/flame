#include "selection.h"
#include "history.h"

void Selection::clear_ll()
{
	switch (type)
	{
	case Selection::tPath:
		for (auto o : objects)
			delete (std::filesystem::path*)o;
		break;
	case Selection::tEntity:
		for (auto o : objects)
			((EntityPtr)o)->message_listeners.remove("editor_selection"_h);
		break;
	}
	type = Selection::tNothing;
	objects.clear();
}

void Selection::select_ll(const std::vector<std::filesystem::path>& paths)
{
	clear_ll();
	if (paths.empty())
		return;

	type = Selection::tPath;
	objects.resize(paths.size());
	for (auto i = 0; i < paths.size(); i++)
		objects[i] = new std::filesystem::path(paths[i]);
}

void Selection::select_ll(const std::vector<EntityPtr>& entities)
{
	clear_ll();
	if (entities.empty())
		return;

	type = Selection::tEntity;
	objects.resize(entities.size());
	for (auto i = 0; i < entities.size(); i++)
		objects[i] = entities[i];

	for (auto e : entities)
	{
		e->message_listeners.add([this, e](uint hash, void*, void*) {
			if (hash == "destroyed"_h)
			{
				if (type == Selection::tEntity)
				{
					for (auto it = objects.begin(); it != objects.end();)
					{
						if (*it == e)
							it = objects.erase(it);
						else
							it++;
					}
					if (objects.empty())
						type = Selection::tNothing;
				}
			}
		}, "editor_selection"_h);
	}
}

void Selection::clear(uint caller)
{
	if (lock)
		return;
	if (type == Selection::tNothing)
		return;
	auto h = new SelectHistory;
	h->init(0);
	clear_ll();
	h->init(1);
	add_history(h);
	if (caller)
		callbacks.call(caller);
}

void Selection::select(const std::vector<std::filesystem::path>& paths, uint caller)
{
	if (lock)
		return;
	if (selecting(paths))
		return;
	auto h = new SelectHistory;
	h->init(0);
	select_ll(paths);
	h->init(1);
	add_history(h);
	if (caller)
		callbacks.call(caller);
}

bool Selection::selecting(const std::vector<std::filesystem::path>& paths)
{
	if (type != tPath)
		return false;
	if (objects.size() != paths.size())
		return false;
	for (auto i = 0; i < paths.size(); i++)
	{
		if (*(std::filesystem::path*)objects[i] != paths[i])
			return false;
	}
	return true;
}

bool Selection::selecting(const std::filesystem::path& path)
{
	if (type != tPath)
		return false;
	for (auto i = 0; i < objects.size(); i++)
	{
		if (*(std::filesystem::path*)objects[i] == path)
			return true;
	}
	return false;
}

void Selection::select(const std::vector<EntityPtr>& entities, uint caller)
{
	if (lock)
		return;
	if (selecting(entities))
		return;
	auto h = new SelectHistory;
	h->init(0);
	select_ll(entities);
	h->init(1);
	add_history(h);
	if (caller)
		callbacks.call(caller);
}

bool Selection::selecting(const std::vector<EntityPtr>& entities)
{
	if (type != tEntity)
		return false;
	if (objects.size() != entities.size())
		return false;
	for (auto i = 0; i < entities.size(); i++)
	{
		if (objects[i] != entities[i])
			return false;
	}
	return true;
}

bool Selection::selecting(EntityPtr entity)
{
	if (type != tEntity)
		return false;
	for (auto i = 0; i < objects.size(); i++)
	{
		if (objects[i] == entity)
			return true;
	}
	return false;
}

Selection selection;
