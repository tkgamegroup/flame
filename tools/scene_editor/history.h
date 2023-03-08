#pragma once

#include "app.h"

struct History
{
	virtual ~History() {}

	virtual void undo() = 0;
	virtual void redo() = 0;
};

struct AssetModifyHistory : History
{
	std::vector<std::filesystem::path> paths;
	uint asset_type;
	uint attr_hash;
	std::vector<std::string> old_values;
	std::vector<std::string> new_values;

	AssetModifyHistory(const std::vector<std::filesystem::path>& paths, uint asset_type, uint attr_hash,
		const std::vector<std::string>& old_values, const std::vector<std::string>& new_values) :
		paths(paths),
		asset_type(asset_type),
		attr_hash(attr_hash),
		old_values(old_values),
		new_values(new_values)
	{
	}

	void set_value(const std::vector<std::string>& values);
	void undo() override;
	void redo() override;
};

struct EntityModifyHistory : History
{
	std::vector<std::string> ids;
	uint comp_type;
	uint attr_hash;
	std::vector<std::string> old_values;
	std::vector<std::string> new_values;

	EntityModifyHistory(const std::vector<std::string>& ids, uint comp_type, uint attr_hash,
		const std::vector<std::string>& old_values, const std::vector<std::string>& new_values) :
		ids(ids),
		comp_type(comp_type),
		attr_hash(attr_hash),
		old_values(old_values),
		new_values(new_values)
	{
	}

	void set_value(const std::vector<std::string>& values);
	void undo() override;
	void redo() override;
};

struct EntityHistory : History
{
	bool is_remove = true;
	std::vector<std::string> ids;
	std::vector<std::string> contents;

	void undo() override;
	void redo() override;
};

struct EntityParentHistory : History
{
	std::vector<std::string> ids;
	std::vector<std::string> old_parents;
	std::vector<std::string> new_parents;

	void undo() override;
	void redo() override;
};

struct EntityPositionHistory : History
{
	std::vector<std::string> ids;

	void undo() override;
	void redo() override;
};

struct ComponentHistory : History
{
	std::vector<std::string> ids;
	uint comp_type;

	void undo() override;
	void redo() override;
};

struct ComponentPositionHistory : History
{
	bool is_remove = true;
	std::vector<std::string> ids;
	uint comp_type;

	void undo() override;
	void redo() override;
};

extern int history_idx;
extern std::vector<std::unique_ptr<History>> histories;
inline void add_history(History* h)
{
	if (history_idx + 1 < histories.size())
		histories.erase(histories.begin() + (history_idx + 1), histories.end());
	histories.emplace_back(h);
	history_idx++;
}