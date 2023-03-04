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

	struct History
	{
		Type type;
		virtual ~History() {}
		void redo();
	};
	struct EmptyHistory : History
	{
		EmptyHistory() { type = tNothing; }
	};
	struct PathHistory : History
	{
		std::vector<std::filesystem::path> paths;
		PathHistory(const std::vector<std::filesystem::path>& paths) : paths(paths) { type = tPath; }
	};
	struct EntityHistory : History
	{
		std::vector<std::string> ids;
		EntityHistory(const std::vector<EntityPtr>& entities) 
		{ 
			type = tEntity;
			ids.resize(entities.size());
			for (auto i = 0; i < entities.size(); i++)
				ids[i] = entities[i]->instance_id;
		}
	};

	Type type = tNothing;
	std::vector<void*> objects;
	std::vector<std::unique_ptr<History>> histories;
	int histroy_idx = -1;
	Listeners<void(uint)> callbacks;

	void clear(uint caller = 0);

	void select(const std::vector<std::filesystem::path>& paths, uint caller = 0);
	inline void select(const std::filesystem::path& path, uint caller = 0)
	{
		std::vector<std::filesystem::path> paths;
		paths.push_back(path);
		select(paths, caller);
	}
	bool selecting(const std::vector<std::filesystem::path>& paths);
	bool selecting(const std::filesystem::path& path);

	void select(const std::vector<EntityPtr>& entities, uint caller = 0);
	inline void select(EntityPtr entity, uint caller = 0)
	{
		std::vector<EntityPtr> entities;
		entities.push_back(entity);
		select(entities, caller);
	}
	bool selecting(const std::vector<EntityPtr>& entities);
	bool selecting(EntityPtr entity);

	void add_history(History* his);
	void forward();
	void backward();

	inline std::filesystem::path	as_path() { return *(std::filesystem::path*)objects[0]; }
	inline std::vector<std::filesystem::path> get_paths()
	{
		std::vector<std::filesystem::path> ret;
		ret.resize(objects.size());
		for (auto i = 0; i < objects.size(); i++)
			ret[i] = *(std::filesystem::path*)objects[i];
		return ret;
	}
	inline EntityPtr				as_entity() { return (EntityPtr)objects[0]; }
	inline std::vector<EntityPtr> get_entities()
	{
		std::vector<EntityPtr> ret;
		ret.resize(objects.size());
		for (auto i = 0; i < objects.size(); i++)
			ret[i] = (EntityPtr)objects[i];
		return ret;
	}
};

extern Selection selection;
