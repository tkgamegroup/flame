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
	uint frame = 0;
	std::vector<std::unique_ptr<History>> history;
	int histroy_idx = -1;

	void clear(bool from_histroy = false);
	void select(const std::filesystem::path& path, bool from_histroy = false);
	bool selecting(const std::filesystem::path& path);
	void select(EntityPtr e, bool from_histroy = false);
	bool selecting(EntityPtr e);

	void add_history(History* his);
	void forward();
	void backward();

	inline std::filesystem::path& path() { return *(std::filesystem::path*)object; }
	inline EntityPtr entity() { return (EntityPtr)object; }
};

extern Selection selection;
