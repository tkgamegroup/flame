#pragma once

#include "app.h"

struct Selection
{
	enum Type
	{
		Nothing,
		File
	};

	Type type = Nothing;
	std::filesystem::path path;

	void clear();
	void select(const std::filesystem::path& path);
	bool selecting(const std::filesystem::path& path);
};

extern Selection selection;
