#pragma once

#include "app.h"

struct Selection
{
	enum Type
	{
		tNothing,
		tFile,
		tEntity
	};

	Type type = tNothing;
	std::filesystem::path path;
	Entity* entity = nullptr;

	void clear();
	void select(const std::filesystem::path& path);
	bool selecting(const std::filesystem::path& path);
};

extern Selection selection;
