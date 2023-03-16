#include "selection.h"

void selection_clear_no_history(uint caller = 0)
{
	if (selection.type == Selection::tNothing)
		return;

	switch (selection.type)
	{
	case Selection::tPath:
		for (auto o : selection.objects)
			delete (std::filesystem::path*)o;
		break;
	case Selection::tEntity:
		for (auto o : selection.objects)
			((EntityPtr)o)->message_listeners.remove("editor_selection"_h);
		break;
	}
	selection.type = Selection::tNothing;
	selection.objects.clear();

	if (caller)
	{
		for (auto& cb : selection.callbacks.list)
			cb.first(caller);
	}
}

void selection_select_no_history(const std::vector<std::filesystem::path>& paths, uint caller = 0)
{
	if (selection.selecting(paths))
		return;

	selection_clear_no_history();
	if (paths.empty())
		return;

	selection.type = Selection::tPath;
	selection.objects.resize(paths.size());
	for (auto i = 0; i < paths.size(); i++)
		selection.objects[i] = new std::filesystem::path(paths[i]);

	if (caller)
	{
		for (auto& cb : selection.callbacks.list)
			cb.first(caller);
	}
}

void selection_select_no_history(const std::vector<EntityPtr>& entities, uint caller = 0)
{
	if (selection.selecting(entities))
		return;

	selection_clear_no_history();
	if (entities.empty())
		return;

	selection.type = Selection::tEntity;
	selection.objects.resize(entities.size());
	for (auto i = 0; i < entities.size(); i++)
		selection.objects[i] = entities[i];

	for (auto e : entities)
	{
		e->message_listeners.add([e](uint hash, void*, void*) {
			if (hash == "destroyed"_h)
			{
				if (selection.type == Selection::tEntity)
				{
					for (auto it = selection.objects.begin(); it != selection.objects.end();)
					{
						if (*it == e)
							it = selection.objects.erase(it);
						else
							it++;
					}
					if (selection.objects.empty())
						selection.type = Selection::tNothing;
				}
			}
		}, "editor_selection"_h);
	}

	if (caller)
	{
		for (auto& cb : selection.callbacks.list)
			cb.first(caller);
	}
}

void Selection::History::redo()
{
	switch (type)
	{
	case Selection::tNothing:
		selection_clear_no_history();
	case Selection::tPath:
	{
		auto& h = (PathHistory&)*this;
		std::vector<std::filesystem::path> paths;
		for (auto& p : h.paths)
		{
			if (std::filesystem::exists(p))
				paths.push_back(p);
		}
		selection_select_no_history(paths);
	}
		break;
	case Selection::tEntity:
	{
		auto& h = (EntityHistory&)*this;
		std::vector<EntityPtr> entities;
		if (app.e_prefab)
		{
			for (auto& id : h.ids)
			{
				if (auto e = app.e_prefab->find_with_instance_id(id))
					entities.push_back(e);
			}
		}
		selection_select_no_history(entities);
	}
		break;
	}
}

void Selection::clear(uint caller)
{
	selection_clear_no_history(caller);
	add_history(new EmptyHistory);
}

void Selection::select(const std::vector<std::filesystem::path>& paths, uint caller)
{
	selection_select_no_history(paths, caller);
	add_history(new PathHistory(paths));
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
	selection_select_no_history(entities, caller);
	add_history(new EntityHistory(entities));
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

void Selection::add_history(History* his)
{
	auto it = histories.begin() + (histroy_idx + 1);
	it = histories.erase(it, histories.end());
	histories.emplace(it, his);
	if (histories.size() > 20)
		histories.erase(histories.begin());
	else
		histroy_idx++;
}

void Selection::forward()
{
	if (histroy_idx + 1 < histories.size())
	{
		histroy_idx++;
		histories[histroy_idx]->redo();
		histories.erase(histories.begin() + histroy_idx);
		histroy_idx--;
		forward();
	}
}

void Selection::backward()
{
	if (histroy_idx > 0)
	{
		histroy_idx--;
		histories[histroy_idx]->redo();
		histories.erase(histories.begin() + histroy_idx);
		backward();
	}
}

Selection selection;
