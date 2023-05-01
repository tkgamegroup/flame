#pragma once

#include "app.h"

struct Selection
{
	enum Type
	{
		tNothing,
		tPath,
		tEntity
	};

	struct PathsRange
	{
		std::filesystem::path* p0;
		std::filesystem::path* p1;

		std::filesystem::path* begin() { return p0; }
		std::filesystem::path* end() { return p1;  }
	};

	struct EntitiesRange
	{
		EntityPtr* p0;
		EntityPtr* p1;

		EntityPtr* begin() { return p0; }
		EntityPtr* end() { return p1; }
	};

	Type type = tNothing;
	std::vector<void*> objects;
	Listeners<void(uint)> callbacks;
	bool lock = false;

	void clear_ll();
	void clear(uint caller = 0);

	void select_ll(const std::vector<std::filesystem::path>& paths);
	void select(const std::vector<std::filesystem::path>& paths, uint caller = 0);
	inline void select(const std::filesystem::path& path, uint caller = 0)
	{
		if (lock)
			return;
		std::vector<std::filesystem::path> paths;
		paths.push_back(path);
		select(paths, caller);
	}
	bool selecting(const std::vector<std::filesystem::path>& paths);
	bool selecting(const std::filesystem::path& path);

	void select_ll(const std::vector<EntityPtr>& entities);
	void select(const std::vector<EntityPtr>& entities, uint caller = 0);
	inline void select(EntityPtr entity, uint caller = 0)
	{
		if (lock)
			return;
		std::vector<EntityPtr> entities;
		entities.push_back(entity);
		select(entities, caller);
	}
	bool selecting(const std::vector<EntityPtr>& entities);
	bool selecting(EntityPtr entity);

	inline std::filesystem::path	as_path(int idx = 0) 
	{ 
		if (idx < 0)
			idx += objects.size();
		return *(std::filesystem::path*)objects[idx]; 
	}
	inline PathsRange				paths()
	{
		PathsRange ret;
		if (type != tPath)
			ret.p0 = ret.p1 = nullptr;
		else
		{
			ret.p0 = (std::filesystem::path*)objects.data();
			ret.p1 = (std::filesystem::path*)objects.data() + objects.size();
		}
		return ret;
	}
	inline std::vector<std::filesystem::path> get_paths()
	{
		std::vector<std::filesystem::path> ret;
		if (type == tPath)
		{
			ret.resize(objects.size());
			for (auto i = 0; i < objects.size(); i++)
				ret[i] = *(std::filesystem::path*)objects[i];
		}
		return ret;
	}
	inline EntityPtr		as_entity(int idx = 0) 
	{ 
		if (idx < 0)
			idx += objects.size();
		return (EntityPtr)objects[idx]; 
	}
	inline EntitiesRange	entities()
	{
		EntitiesRange ret;
		if (type != tEntity)
			ret.p0 = ret.p1 = nullptr;
		else
		{
			ret.p0 = (EntityPtr*)objects.data();
			ret.p1 = (EntityPtr*)objects.data() + objects.size();
		}
		return ret;
	}
	inline std::vector<EntityPtr> get_entities()
	{
		std::vector<EntityPtr> ret;
		if (type == tEntity)
		{
			ret.resize(objects.size());
			for (auto i = 0; i < objects.size(); i++)
				ret[i] = (EntityPtr)objects[i];
		}
		return ret;
	}
};

extern Selection selection;
