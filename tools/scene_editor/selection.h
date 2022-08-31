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
		bool select();
	};
	struct EmptyHistory : History
	{
		EmptyHistory() { type = tNothing; }
	};
	struct PathHistory : History
	{
		std::filesystem::path path;
		PathHistory(const std::filesystem::path& path) : path(path) { type = tPath; }
	};
	struct EntityHistory : History
	{
		std::string ins_id;
		EntityHistory(EntityPtr e) : ins_id(e->instance_id) { type = tEntity; }
	};

	Type type = tNothing;
	void* object = nullptr;
	std::vector<std::unique_ptr<History>> history;
	int histroy_idx = -1;
	Listeners<void(uint)> callbacks;

	void _clear(uint caller = 0);
	void clear(uint caller = 0);
	void _select(const std::filesystem::path& path, uint caller = 0);
	void select(const std::filesystem::path& path, uint caller = 0);
	bool selecting(const std::filesystem::path& path);
	void _select(EntityPtr e, uint caller = 0);
	void select(EntityPtr e, uint caller = 0);
	bool selecting(EntityPtr e);

	void add_history(History* his);
	void forward();
	void backward();

	inline std::filesystem::path&	path() { return *(std::filesystem::path*)object; }
	inline EntityPtr				entity() { return (EntityPtr)object; }
};

extern Selection selection;
