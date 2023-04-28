#pragma once

#include "app.h"

struct History
{
	virtual ~History() {}

	virtual void undo() = 0;
	virtual void redo() = 0;
};

struct SelectHistory : History
{
	Selection::Type old_type;
	Selection::Type new_type;
	std::vector<void*> old_objects;
	std::vector<void*> new_objects;

	SelectHistory(Selection::Type old_type, Selection::Type new_type,
		const std::vector<void*> old_objects, const std::vector<void*> new_objects) :
		old_type(old_type),
		new_type(new_type),
		old_objects(old_objects),
		new_objects(new_objects)
	{
	}

	void select(Selection::Type type, const std::vector<void*> objects);
	void undo() override;
	void redo() override;
};

struct AssetModifyHistory : History
{
	std::filesystem::path path;
	uint asset_type;
	uint attr_hash;
	std::string old_value;
	std::string new_value;

	AssetModifyHistory(const std::filesystem::path& path, uint asset_type, uint attr_hash,
		const std::string& old_value, const std::string& new_value) :
		path(path),
		asset_type(asset_type),
		attr_hash(attr_hash),
		old_value(old_value),
		new_value(new_value)
	{
	}

	void set_value(const std::string& value);
	void undo() override;
	void redo() override;
};

struct EntityModifyHistory : History
{
	std::vector<GUID> ids;
	uint comp_type;
	uint attr_hash;
	std::vector<std::string> old_values;
	std::vector<std::string> new_values;

	EntityModifyHistory(const std::vector<GUID>& ids, uint comp_type, uint attr_hash,
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

struct PrefabModifyHistory : History
{
	std::filesystem::path path;
	uint comp_type;
	uint attr_hash;
	std::string old_value;
	std::string new_value;

	PrefabModifyHistory(const std::filesystem::path& paths, uint comp_type, uint attr_hash,
		const std::string& old_value, const std::string& new_value) :
		path(path),
		comp_type(comp_type),
		attr_hash(attr_hash),
		old_value(old_value),
		new_value(new_value)
	{
	}

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