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

	struct Histroy
	{
		Type type;
		virtual ~Histroy() {}
		bool select();
	};
	struct PathHistroy : Histroy
	{
		std::filesystem::path path;
		PathHistroy(const std::filesystem::path& path) : path(path) { type = tPath; }
	};
	struct EntityHistroy : Histroy
	{
		std::string ins_id;
		EntityHistroy(EntityPtr e) : ins_id(e->instance_id) { type = tEntity; }
	};

	Type type = tNothing;
	void* object = nullptr;
	uint frame = 0;
	std::vector<std::unique_ptr<Histroy>> history;
	int histroy_idx = -1;

	void clear();
	void select(const std::filesystem::path& path, bool from_histroy = false);
	bool selecting(const std::filesystem::path& path);
	void select(EntityPtr e, bool from_histroy = false);
	bool selecting(EntityPtr e);

	void forward();
	void backward();

	inline std::filesystem::path& path() { return *(std::filesystem::path*)object; }
	inline EntityPtr entity() { return (EntityPtr)object; }
};

extern Selection selection;
