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

	Type type = tNothing;
	void* content = nullptr;

	uint frame = 0;

	void clear();
	void select(const std::filesystem::path& path);
	bool selecting(const std::filesystem::path& path);
	void select(EntityPtr e);
	bool selecting(EntityPtr e);

	inline std::filesystem::path& path() { return *(std::filesystem::path*)content; }
	inline EntityPtr entity() { return (EntityPtr)content; }
};

extern Selection selection;
