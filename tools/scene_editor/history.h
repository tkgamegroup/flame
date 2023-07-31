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

	~SelectHistory();

	void init(int old_or_new);
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
	std::vector<GUID>			ids;
	uint						comp_type;
	uint						attr_hash;
	std::vector<std::string>	old_values;
	std::vector<std::string>	new_values;

	EntityModifyHistory(const std::vector<GUID>& ids, uint comp_type, uint attr_hash,
		const std::vector<std::string>& old_values, const std::vector<std::string>& new_values) :
		ids(ids),
		comp_type(comp_type),
		attr_hash(attr_hash),
		old_values(old_values),
		new_values(new_values)
	{
	}

	void set_values(const std::vector<std::string>& values);
	void undo() override;
	void redo() override;
};

struct EntityHistory : History
{
	std::vector<GUID>							ids;
	std::vector<GUID>							old_parents;
	std::vector<uint>							old_indices;
	std::vector<GUID>							new_parents;
	std::vector<uint>							new_indices;
	std::vector<std::vector<
		std::tuple<uint, uint, std::string>>>	contents; // each is an array of (comp_type, attr_hash, value) represents an entity

	void recreate_entities(const std::vector<GUID>& parents, const std::vector<uint>& indices);
	void remove_entities();
	void undo() override;
	void redo() override;
};

struct EntityPositionHistory : History
{
	std::vector<GUID>	ids;
	std::vector<GUID>	old_parents;
	std::vector<uint>	old_indices;
	std::vector<GUID>	new_parents;
	std::vector<uint>	new_indices;

	void set_positions(const std::vector<GUID>& parents, const std::vector<uint>& indices);
	void undo() override;
	void redo() override;
};

struct ComponentHistory : History
{
	std::vector<GUID>					old_ids;
	std::vector<uint>					old_indices;
	std::vector<GUID>					new_ids;
	std::vector<uint>					new_indices;
	uint								comp_type;
	std::vector<std::vector<
		std::pair<uint, std::string>>>	contents; // each is an array of (attr_hash, value) represents an component

	void recreate_components(const std::vector<GUID>& ids, const std::vector<uint>& indices);
	void remove_components(const std::vector<GUID>& ids);
	void undo() override;
	void redo() override;
};

struct ComponentPositionHistory : History
{
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

	void set_value(const std::string& value);
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