#include "selection.h"

bool Selection::History::select()
{
	switch (type)
	{
	case Selection::tNothing:
		selection.clear(true);
		return true;
	case Selection::tPath:
	{
		auto& path = ((PathHistory*)this)->path;
		if (std::filesystem::exists(path))
		{
			selection.select(path, true);
			return true;
		}
		else
			path.clear();
	}
		break;
	case Selection::tEntity:
	{
		auto& his = (EntityHistory&)*this;
		EntityPtr e = nullptr;
		if (app.e_prefab)
		{
			if (app.e_prefab->instance_id == his.ins_id)
				e = app.e_prefab;
			else
				e = app.e_prefab->find_child_with_instance_id(his.ins_id);
		}
		if (e)
		{
			selection.select(e, true);
			return true;
		}
		else
			his.ins_id.clear();
	}
		break;
	}
	return false;
}

void Selection::clear(bool from_histroy)
{
	if (type == tNothing)
		return;

	frame = frames;

	switch (type)
	{
	case tPath:
		delete (std::filesystem::path*)object;
		break;
	case tEntity:
		if (object)
			((EntityPtr)object)->message_listeners.remove("editor_selection"_h);
		break;
	}
	type = tNothing;
	object = nullptr;

	if (!from_histroy)
		add_history(new EmptyHistory);
}

void Selection::select(const std::filesystem::path& path, bool from_histroy)
{
	if (selecting(path))
		return;

	frame = frames;

	clear();
	type = tPath;
	object = new std::filesystem::path(path);

	if (!from_histroy)
		add_history(new PathHistory(path));
}

bool Selection::selecting(const std::filesystem::path& _path)
{
	return type == tPath && _path == *(std::filesystem::path*)object;
}

void Selection::select(EntityPtr e, bool from_histroy)
{
	if (selecting(e))
		return;

	frame = frames;

	clear();
	type = tEntity;
	object = e;

	e->message_listeners.add([e](uint hash, void*, void*) {
		if (hash == "destroyed"_h)
		{
			if (selection.selecting(e))
			{
				selection.object = nullptr;
				selection.clear(app_exiting);
			}
		}
	}, "editor_selection"_h);

	if (!from_histroy)
		add_history(new EntityHistory(e));
}

bool Selection::selecting(EntityPtr e)
{
	return type == tEntity && object == e;
}

void Selection::add_history(History* his)
{
	auto it = history.begin() + (histroy_idx + 1);
	it = history.erase(it, history.end());
	history.emplace(it, his);
	if (history.size() > 20)
		history.erase(history.begin());
	else
		histroy_idx++;
}

void Selection::forward()
{
	if (histroy_idx + 1 < history.size())
	{
		histroy_idx++;
		if (!history[histroy_idx]->select())
		{
			history.erase(history.begin() + histroy_idx);
			histroy_idx--;
			forward();
		}
	}
}

void Selection::backward()
{
	if (histroy_idx > 0)
	{
		histroy_idx--;
		if (!history[histroy_idx]->select())
		{
			history.erase(history.begin() + histroy_idx);
			backward();
		}
	}
}

Selection selection;
