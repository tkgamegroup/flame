#include "../xml.h"
#include "system_private.h"
#include "typeinfo_private.h"

namespace flame
{
	TypeInfoDataBase tidb;

	static TypeInfo* void_type = nullptr;

	TypeInfo* TypeInfo::get(TypeTag tag, const std::string& name, TypeInfoDataBase& db)
	{
		if (tag == TypeData && name.empty())
			return void_type;

		auto key = ch(name.data());
		key ^= std::hash<int>()(tag);
		auto it = db.typeinfos.find(key);
		if (it != db.typeinfos.end())
			return it->second.get();
		if (&tidb != &db)
		{
			it = tidb.typeinfos.find(key);
			if (it != tidb.typeinfos.end())
				return it->second.get();
		}

		TypeInfo* t = nullptr;
		switch (tag)
		{
		case TypeEnumSingle:
			t = new TypeInfo_EnumSingle(name, db);
			break;
		case TypeEnumMulti:
			t = new TypeInfo_EnumMulti(name, db);
			break;
		case TypePointer:
			t = new TypeInfo_Pointer(name, db);
			break;
		case TypeData:
		{
			auto udt = find_udt(name, db);
			if (udt)
				t = new TypeInfo(TypeData, name, udt->size);
		}
			break;
		}

		if (t)
			db.typeinfos.emplace(key, t);
		return t;
	}

	TypeInfoDataBase::TypeInfoDataBase()
	{
		if (void_type)
			return;

		{
			void_type = new TypeInfo_void;
			tidb.typeinfos.emplace(void_type->hash, void_type);
		}
		{
			auto t = new TypeInfo_bool;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_char;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_uchar;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_wchar;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_short;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_ushort;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_int;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_uint;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_int64;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_uint64;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_float;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_cvec2;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_cvec3;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_cvec4;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_ivec2;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_ivec3;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_ivec4;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_uvec2;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_uvec3;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_uvec4;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_vec2;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_vec3;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_vec4;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_string;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_wstring;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_Rect;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_AABB;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_Plane;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_Frustum;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_mat2;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_mat3;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_mat4;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_quat;
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_charp(tidb);
			tidb.typeinfos.emplace(t->hash, t);
		}
		{
			auto t = new TypeInfo_wcharp(tidb);
			tidb.typeinfos.emplace(t->hash, t);
		}

		auto app_name = get_app_path(true);
		if (app_name.filename() != L"typeinfogen.exe")
		{
			for (auto& path : get_module_dependencies(app_name))
			{
				auto ti_path = path;
				ti_path.replace_extension(".typeinfo");
				if (std::filesystem::exists(ti_path))
					tidb.load_typeinfo(path);
			}
		}
	}

	void TypeInfoDataBase::load_typeinfo(const std::filesystem::path& filename)
	{
		std::filesystem::path path(filename);
		if (!path.is_absolute())
			path = get_app_path() / path;

		void* library = nullptr;
		if (path.extension() == L".dll")
		{
			library = LoadLibraryW(path.c_str());
			path.replace_extension(L".typeinfo");
		}

		pugi::xml_document file;
		pugi::xml_node file_root;
		if (!file.load_file(path.c_str()) || (file_root = file.first_child()).name() != std::string("typeinfo"))
		{
			printf("cannot find typeinfo or wrong format: %s\n", path.string().c_str());
			assert(0);
		}

		auto read_ti = [&](pugi::xml_node n) {
			TypeTag tag;
			TypeInfo::get(TypeEnumSingle, "flame::TypeTag", *this)->unserialize(n.attribute("type_tag").value(), &tag);
			return TypeInfo::get(tag, n.attribute("type_name").value(), *this);
		};

		for (auto n_enum : file_root.child("enums"))
		{
			auto name = std::string(n_enum.attribute("name").value());
			auto& e = enums.emplace(name, EnumInfo()).first->second;
			e.name = name;
			for (auto n_item : n_enum.child("items"))
			{
				auto& i = e.items.emplace_back();
				i.ei = &e;
				i.index = e.items.size() - 1;
				i.name = n_item.attribute("name").value();
				i.value = n_item.attribute("value").as_int();
			}
		}
		for (auto n_udt : file_root.child("udts"))
		{
			auto name = std::string(n_udt.attribute("name").value());
			auto& u = udts.emplace(name, UdtInfo()).first->second;
			u.name = name;
			u.size = n_udt.attribute("size").as_uint();
			u.base_class_name = n_udt.attribute("base_class_name").as_uint();

			for (auto n_variable : n_udt.child("variables"))
			{
				auto& v = u.variables.emplace_back();
				v.udt = &u;
				v.index = u.variables.size() - 1;
				v.type = read_ti(n_variable);
				v.name = n_variable.attribute("name").value();
				v.offset = n_variable.attribute("offset").as_uint();
				v.array_size = n_variable.attribute("array_size").as_uint();
				v.array_stride = n_variable.attribute("array_stride").as_uint();
				v.default_value = n_variable.attribute("default_value").value();
				v.metas.from_string(n_variable.attribute("metas").value());
			}
			for (auto n_function : n_udt.child("functions"))
			{
				auto& f = u.functions.emplace_back();
				f.udt = &u;
				f.index = u.functions.size() - 1;
				f.name = n_function.attribute("name").value();
				f.rva = n_function.attribute("rva").as_uint();
				f.voff = n_function.attribute("voff").as_int();
				f.is_static = n_function.attribute("is_static").as_bool();
				f.type = read_ti(n_function);
				f.metas.from_string(n_function.attribute("metas").value());
				f.library = library;
				for (auto n_parameter : n_function)
					f.parameters.push_back(read_ti(n_parameter));
			}
		}
	}

	void TypeInfoDataBase::save_typeinfo(const std::filesystem::path& filename)
	{
		pugi::xml_document file;
		auto file_root = file.append_child("typeinfo");

		auto e_tag = TypeInfo::get(TypeEnumSingle, "flame::TypeTag", *this);
		auto write_ti = [&](TypeInfo* ti, pugi::xml_node n) {
			n.append_attribute("type_tag").set_value(e_tag->serialize(&ti->tag).c_str());
			n.append_attribute("type_name").set_value(ti->name.c_str());
		};

		auto n_enums = file_root.append_child("enums");
		for (auto& ei : enums)
		{
			auto n_enum = n_enums.append_child("enum");
			n_enum.append_attribute("name").set_value(ei.second.name.c_str());
			auto n_items = n_enum.append_child("items");
			for (auto& i : ei.second.items)
			{
				auto n_item = n_items.append_child("item");
				n_item.append_attribute("name").set_value(i.name.c_str());
				n_item.append_attribute("value").set_value(i.value);
			}
		}

		auto n_udts = file_root.append_child("udts");
		for (auto& ui : udts)
		{
			auto n_udt = n_udts.append_child("udt");
			n_udt.append_attribute("name").set_value(ui.second.name.c_str());
			n_udt.append_attribute("size").set_value(ui.second.size);
			n_udt.append_attribute("base_class_name").set_value(ui.second.base_class_name.c_str());
			if (!ui.second.variables.empty())
			{
				auto n_variables = n_udt.prepend_child("variables");
				for (auto& vi : ui.second.variables)
				{
					auto n_variable = n_variables.append_child("variable");
					write_ti(vi.type, n_variable);
					n_variable.append_attribute("name").set_value(vi.name.c_str());
					n_variable.append_attribute("offset").set_value(vi.offset);
					n_variable.append_attribute("array_size").set_value(vi.array_size);
					n_variable.append_attribute("array_stride").set_value(vi.array_stride);
					n_variable.append_attribute("default_value").set_value(vi.default_value.c_str());
					n_variable.append_attribute("metas").set_value(vi.metas.to_string().c_str());
				}
			}
			if (!ui.second.functions.empty())
			{
				auto n_functions = n_udt.append_child("functions");
				for (auto& fi : ui.second.functions)
				{
					auto n_function = n_functions.append_child("function");
					n_function.append_attribute("name").set_value(fi.name.c_str());
					n_function.append_attribute("rva").set_value(fi.rva);
					n_function.append_attribute("voff").set_value(fi.voff);
					n_function.append_attribute("is_static").set_value(fi.is_static);
					n_function.append_attribute("metas").set_value(fi.metas.to_string().c_str());
					write_ti(fi.type, n_function);
					if (!fi.parameters.empty())
					{
						for (auto p : fi.parameters)
							write_ti(p, n_function.append_child("parameter"));
					}
				}
			}
		}

		file.save_file(filename.c_str());
	}
}
