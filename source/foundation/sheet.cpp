#include "../xml.h"
#include "typeinfo_private.h"
#include "sheet_private.h"

namespace flame
{
	std::vector<std::unique_ptr<SheetT>> loaded_sheets;

	void SheetPrivate::clear_rows() 
	{
		for (auto& r : rows)
		{
			for (auto i = 0; i < header.size(); i++)
				header[i].type->destroy(r.datas[i]);
		}
		rows.clear();
	}

	void SheetPrivate::insert_column(const std::string& name, TypeInfo* type, int idx, const std::string& default_value)
	{ 
		Column column;
		if (idx < 0)
			idx = header.size() + (idx + 1);
		column.name = name;
		column.name_hash = sh(name.c_str());
		column.type = type;
		column.default_value = default_value;
		header.insert(header.begin() + idx, column);

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
		assert(idx < header.size());

		auto& column = header[idx];
		if (column.name != new_name)
		{
			column.name = new_name;
			column.name_hash = sh(new_name.c_str());
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

	void SheetPrivate::reposition_columns(uint idx1, uint idx2)
	{
		assert(idx1 < header.size());
		assert(idx2 < header.size());

		if (idx1 != idx2)
		{
			std::swap(header[idx1], header[idx2]);
			for (auto& r : rows)
				std::swap(r.datas[idx1], r.datas[idx2]);
		}
	}

	void SheetPrivate::remove_column(uint idx)
	{
		assert(idx < header.size());

		auto type = header[idx].type;
		header.erase(header.begin() + idx);
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
		row.datas.resize(header.size());
		for (auto i = 0; i < header.size(); i++)
		{
			auto& column = header[i];
			row.datas[i] = column.type->create();
			if (!column.default_value.empty())
				column.type->unserialize(column.default_value, row.datas[i]);
		}
		rows.insert(rows.begin() + idx, row);
	}

	void SheetPrivate::remove_row(uint idx)
	{
		assert(idx < header.size());

		auto& row = rows[idx];
		for (auto i = 0; i < header.size(); i++)
			header[i].type->destroy(row.datas[i]);

		rows.erase(rows.begin() + idx);
	}

	void SheetPrivate::save(const std::filesystem::path& path)
	{
		pugi::xml_document doc;

		auto write_ti = [&](TypeInfo* ti, pugi::xml_attribute a) {
			a.set_value((TypeInfo::serialize_t(ti->tag) + '@' + ti->name).c_str());
		};

		auto doc_root = doc.append_child("sheet");
		auto n_header = doc_root.append_child("header");
		for (auto& c : header)
		{
			auto n_column = n_header.append_child("column");
			write_ti(c.type, n_column.append_attribute("type"));
			n_column.append_attribute("name").set_value(c.name.c_str());
			n_column.append_attribute("default_value").set_value(c.default_value.c_str());
		}
		auto n_rows = doc_root.append_child("rows");
		for (auto& r : rows)
		{
			auto n_row = n_rows.append_child("row");
			for (auto i = 0; i < header.size(); i++)
				n_row.append_attribute(header[i].name.c_str()).set_value(header[i].type->serialize(r.datas[i]).c_str());
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

				for (auto n_column : doc_root.child("header"))
				{
					ret->insert_column(n_column.attribute("name").value(), 
						read_ti(n_column.attribute("type")), -1, n_column.attribute("default_value").value());
				}
				for (auto n_row : doc_root.child("rows"))
				{
					ret->insert_row();
					auto& row = ret->rows.back();
					for (auto i = 0; i < ret->header.size(); i++)
					{
						auto& column = ret->header[i];
						column.type->unserialize(n_row.attribute(column.name.c_str()).value(), row.datas[i]);
					}
				}
			}
			else
				ret->save(filename);
			ret->filename = filename;
			ret->ref = 1;
			loaded_sheets.emplace_back(ret);
			return ret;
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
