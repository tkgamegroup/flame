#include "../xml.h"
#include "typeinfo_private.h"
#include "sheet_private.h"

namespace flame
{
	std::vector<std::unique_ptr<SheetT>> loaded_sheets;
	std::map<uint, SheetPtr> named_sheets;

	void SheetPrivate::clear_rows() 
	{
		for (auto& r : rows)
		{
			for (auto i = 0; i < columns.size(); i++)
				columns[i].type->destroy(r.datas[i]);
		}
		rows.clear();
	}

	void SheetPrivate::insert_column(const std::string& name, TypeInfo* type, int idx, const std::string& default_value)
	{ 
		Column column;
		if (idx < 0)
			idx = columns.size() + (idx + 1);
		column.name = name;
		column.name_hash = sh(name.c_str());
		column.type = type;
		column.default_value = default_value;
		columns.insert(columns.begin() + idx, column);
		columns_map[column.name_hash] = idx;

		for (auto& r : rows)
		{
			auto data = type->create();
			r.datas.insert(r.datas.begin() + idx, data);
			if (!default_value.empty())
				type->unserialize(default_value, r.datas[idx]);
		}
	}

	void SheetPrivate::alter_column(uint idx, const std::string& new_name, TypeInfo* new_type, const std::string& default_value)
	{
		assert(idx < columns.size());

		auto& column = columns[idx];
		if (column.name != new_name)
		{
			columns_map.erase(column.name_hash);
			column.name = new_name;
			column.name_hash = sh(new_name.c_str());
			columns_map[column.name_hash] = idx;
		}
		if (column.type != new_type)
		{
			for (auto& r : rows)
			{
				column.type->destroy(r.datas[idx]);
				r.datas[idx] = new_type->create();
				if (!default_value.empty())
					new_type->unserialize(default_value, r.datas[idx]);
			}
			column.type = new_type;
			column.default_value = default_value;
		}
	}

	void SheetPrivate::reorder_columns(uint target_column_index, int new_index)
	{
		assert(target_column_index < columns.size());
		assert(new_index < columns.size());

		if (target_column_index != new_index)
		{
			if (target_column_index < new_index)
				std::rotate(columns.begin() + target_column_index, columns.begin() + target_column_index + 1, columns.begin() + new_index + 1);
			else
				std::rotate(columns.begin() + new_index, columns.begin() + target_column_index, columns.begin() + target_column_index + 1);
			columns_map.clear();
			for (auto i = 0; i < columns.size(); i++)
				columns_map[columns[i].name_hash] = i;

			for (auto& r : rows)
			{
				if (target_column_index < new_index)
					std::rotate(r.datas.begin() + target_column_index, r.datas.begin() + target_column_index + 1, r.datas.begin() + new_index + 1);
				else
					std::rotate(r.datas.begin() + new_index, r.datas.begin() + target_column_index, r.datas.begin() + target_column_index + 1);
			}
		}
	}

	void SheetPrivate::remove_column(uint idx)
	{
		assert(idx < columns.size());

		auto type = columns[idx].type;
		columns_map.erase(columns[idx].name_hash);
		columns.erase(columns.begin() + idx);
		for (auto& r : rows)
		{
			type->destroy(r.datas[idx]);
			r.datas.erase(r.datas.begin() + idx);
		}
	}

	void SheetPrivate::insert_row(int idx)
	{
		if (idx < 0)
			idx = rows.size() + (idx + 1);
		Row row;
		row.datas.resize(columns.size());
		for (auto i = 0; i < columns.size(); i++)
		{
			auto& column = columns[i];
			row.datas[i] = column.type->create();
			if (!column.default_value.empty())
				column.type->unserialize(column.default_value, row.datas[i]);
		}
		rows.insert(rows.begin() + idx, row);
	}

	void SheetPrivate::remove_row(uint idx)
	{
		assert(idx < columns.size());

		auto& row = rows[idx];
		for (auto i = 0; i < columns.size(); i++)
			columns[i].type->destroy(row.datas[i]);

		rows.erase(rows.begin() + idx);
	}

	void SheetPrivate::save(const std::filesystem::path& path)
	{
		pugi::xml_document doc;

		auto write_ti = [&](TypeInfo* ti, pugi::xml_attribute a) {
			a.set_value((TypeInfo::serialize_t(ti->tag) + '@' + ti->name).c_str());
		};

		auto doc_root = doc.append_child("sheet");
		doc_root.append_attribute("name").set_value(name.c_str());
		auto n_columns = doc_root.append_child("columns");
		for (auto& c : columns)
		{
			auto n_column = n_columns.append_child("column");
			write_ti(c.type, n_column.append_attribute("type"));
			n_column.append_attribute("name").set_value(c.name.c_str());
			if (!c.default_value.empty())
				n_column.append_attribute("default_value").set_value(c.default_value.c_str());
		}
		auto n_rows = doc_root.append_child("rows");
		for (auto& r : rows)
		{
			auto n_row = n_rows.append_child("row");
			for (auto i = 0; i < columns.size(); i++)
				n_row.append_attribute(columns[i].name.c_str()).set_value(columns[i].type->serialize(r.datas[i]).c_str());
		}

		if (!path.empty())
			filename = path;
		doc.save_file(filename.c_str());
	}

	struct SheetGet : Sheet::Get
	{
		SheetPtr operator()(const std::filesystem::path& _filename) override
		{
			auto filename = Path::get(_filename);
			if (filename.empty())
				filename = _filename;

			for (auto& lib : loaded_sheets)
			{
				if (lib->filename == filename)
				{
					lib->ref++;
					return lib.get();
				}
			}

			auto ret = new SheetPrivate;
			if (std::filesystem::exists(filename))
			{
				pugi::xml_document doc;
				pugi::xml_node doc_root;

				if (!doc.load_file(filename.c_str()) || (doc_root = doc.first_child()).name() != std::string("sheet"))
				{
					wprintf(L"sheet does not exist or wrong format: %s\n", _filename.c_str());
					return nullptr;
				}

				auto read_ti = [&](pugi::xml_attribute a) {
					auto sp = SUS::to_string_vector(SUS::split(a.value(), '@'));
					TypeTag tag;
					TypeInfo::unserialize_t(sp[0], tag);
					return TypeInfo::get(tag, sp[1]);
				};

				if (auto a = doc_root.attribute("name"); a)
				{
					ret->name = a.value();
					ret->name_hash = sh(ret->name.c_str());
				}

				for (auto n_column : doc_root.child("columns"))
				{
					ret->insert_column(n_column.attribute("name").value(), 
						read_ti(n_column.attribute("type")), -1, n_column.attribute("default_value").value());
				}
				for (auto n_row : doc_root.child("rows"))
				{
					ret->insert_row();
					auto& row = ret->rows.back();
					for (auto i = 0; i < ret->columns.size(); i++)
					{
						auto& column = ret->columns[i];
						column.type->unserialize(n_row.attribute(column.name.c_str()).value(), row.datas[i]);
					}
				}
			}
			else
				ret->save(filename);
			ret->filename = filename;
			ret->ref = 1;
			loaded_sheets.emplace_back(ret);
			named_sheets[ret->name_hash] = ret;
			return ret;
		}

		SheetPtr operator()(uint name) override
		{
			auto it = named_sheets.find(name);
			if (it != named_sheets.end())
			{
				it->second->ref++;
				return it->second;
			}
			return nullptr;
		}
	}Sheet_get;
	Sheet::Get& Sheet::get = Sheet_get;

	struct SheetRelease : Sheet::Release
	{
		void operator()(SheetPtr Sheet) override
		{
			if (Sheet->ref == 1)
			{
				std::erase_if(loaded_sheets, [&](const auto& bp) {
					return bp.get() == Sheet;
				});
			}
			else
				Sheet->ref--;
		}
	}Sheet_release;
	Sheet::Release& Sheet::release = Sheet_release;
}
