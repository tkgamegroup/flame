#include "../xml.h"
#include "system_private.h"
#include "typeinfo_private.h"
#include "typeinfo_serialize.h"

namespace flame
{
	TypeInfoDataBase _tidb;
	TypeInfoDataBase& tidb = _tidb;

	TypeInfo* TypeInfo::void_type = nullptr;

	TypeInfo* TypeInfo::get(TypeTag tag, const std::string& name, TypeInfoDataBase& db)
	{
		if (tag == TagD && name.empty())
			return void_type;

		auto key = get_hash(tag, name);
		auto it = db.typeinfos.find(key);
		if (it != db.typeinfos.end())
			return it->second.get();
		if (&_tidb != &db)
		{
			it = _tidb.typeinfos.find(key);
			if (it != _tidb.typeinfos.end())
				return it->second.get();
		}

		TypeInfo* t = nullptr;
		switch (tag)
		{
		case TagE:
			if (name.ends_with("Flags"))
				t = new TypeInfo_EnumMulti(name, db);
			else
				t = new TypeInfo_EnumSingle(name, db);
			break;
		case TagU:
			t = new TypeInfo_Udt(name, db);
			break;
		case TagPE:
			t = new TypeInfo_PointerOfEnum(name, db);
			break;
		case TagPD:
			t = new TypeInfo_PointerOfData(name, db);
			break;
		case TagPU:
			t = new TypeInfo_PointerOfUdt(name, db);
			break;
		case TagVE:
			t = new TypeInfo_VectorOfEnum(name, db);
			break;
		case TagVD:
			t = new TypeInfo_VectorOfData(name, db);
			break;
		case TagVU:
			t = new TypeInfo_VectorOfUdt(name, db);
			break;
		case TagVPE:
			t = new TypeInfo_VectorOfPointerOfEnum(name, db);
			break;
		case TagVPD:
			t = new TypeInfo_VectorOfPointerOfData(name, db);
			break;
		case TagVPU:
			t = new TypeInfo_VectorOfPointerOfUdt(name, db);
			break;
		}

		if (t)
			db.typeinfos.emplace(key, t);
		return t;
	}

	thread_local int TypeInfo_Enum::v;
	thread_local bool TypeInfo_bool::v;
	thread_local char TypeInfo_char::v;
	thread_local uchar TypeInfo_uchar::v;
	thread_local short TypeInfo_short::v;
	thread_local ushort TypeInfo_ushort::v;
	thread_local int TypeInfo_int::v;
	thread_local uint TypeInfo_uint::v;
	thread_local int64 TypeInfo_int64::v;
	thread_local uint64 TypeInfo_uint64::v;
	thread_local float TypeInfo_float::v;
	thread_local cvec2 TypeInfo_cvec2::v;
	thread_local cvec3 TypeInfo_cvec3::v;
	thread_local cvec4 TypeInfo_cvec4::v;
	thread_local ivec2 TypeInfo_ivec2::v;
	thread_local ivec3 TypeInfo_ivec3::v;
	thread_local ivec4 TypeInfo_ivec4::v;
	thread_local uvec2 TypeInfo_uvec2::v;
	thread_local uvec3 TypeInfo_uvec3::v;
	thread_local uvec4 TypeInfo_uvec4::v;
	thread_local vec2 TypeInfo_vec2::v;
	thread_local vec3 TypeInfo_vec3::v;
	thread_local vec4 TypeInfo_vec4::v;
	thread_local mat2 TypeInfo_mat2::v;
	thread_local mat3 TypeInfo_mat3::v;
	thread_local mat4 TypeInfo_mat4::v;
	thread_local quat TypeInfo_quat::v;
	thread_local std::string TypeInfo_string::v;
	thread_local std::wstring TypeInfo_wstring::v;
	thread_local std::filesystem::path TypeInfo_path::v;
	thread_local Rect TypeInfo_Rect::v;
	thread_local AABB TypeInfo_AABB::v;
	thread_local Plane TypeInfo_Plane::v;
	thread_local Frustum TypeInfo_Frustum::v;

	TypeInfoDataBase::TypeInfoDataBase()
	{
		if (!_tidb.typeinfos.empty())
			return;

		init_basic_types();

		for (auto& path : get_module_dependencies(get_app_path(true)))
		{
			auto ti_path = path;
			ti_path.replace_extension(".typeinfo");
			if (std::filesystem::exists(ti_path))
				_tidb.load(path);
		}
	}
	
	void TypeInfoDataBase::init_basic_types()
	{
		TypeInfo::void_type = new TypeInfo_void;
		_tidb.add_ti(TypeInfo::void_type);
		_tidb.add_ti(new TypeInfo_bool);
		_tidb.add_ti(new TypeInfo_char);
		_tidb.add_ti(new TypeInfo_uchar);
		_tidb.add_ti(new TypeInfo_short);
		_tidb.add_ti(new TypeInfo_ushort);
		_tidb.add_ti(new TypeInfo_int);
		_tidb.add_ti(new TypeInfo_uint);
		_tidb.add_ti(new TypeInfo_int64);
		_tidb.add_ti(new TypeInfo_uint64);
		_tidb.add_ti(new TypeInfo_float);
		_tidb.add_ti(new TypeInfo_cvec2);
		_tidb.add_ti(new TypeInfo_cvec3);
		_tidb.add_ti(new TypeInfo_cvec4);
		_tidb.add_ti(new TypeInfo_ivec2);
		_tidb.add_ti(new TypeInfo_ivec3);
		_tidb.add_ti(new TypeInfo_ivec4);
		_tidb.add_ti(new TypeInfo_uvec2);
		_tidb.add_ti(new TypeInfo_uvec3);
		_tidb.add_ti(new TypeInfo_uvec4);
		_tidb.add_ti(new TypeInfo_vec2);
		_tidb.add_ti(new TypeInfo_vec3);
		_tidb.add_ti(new TypeInfo_vec4);
		_tidb.add_ti(new TypeInfo_mat2);
		_tidb.add_ti(new TypeInfo_mat3);
		_tidb.add_ti(new TypeInfo_mat4);
		_tidb.add_ti(new TypeInfo_quat);
		_tidb.add_ti(new TypeInfo_string);
		_tidb.add_ti(new TypeInfo_wstring);
		_tidb.add_ti(new TypeInfo_path);
		_tidb.add_ti(new TypeInfo_Rect);
		_tidb.add_ti(new TypeInfo_AABB);
		_tidb.add_ti(new TypeInfo_Plane);
		_tidb.add_ti(new TypeInfo_Frustum);
	}

	bool TypeInfoDataBase::load(std::ifstream& file, void* library)
	{
		pugi::xml_document doc;
		pugi::xml_node doc_root;
		if (!doc.load(file) || (doc_root = doc.first_child()).name() != std::string("typeinfo"))
			return false;

		auto read_ti = [&](pugi::xml_attribute a) {
			auto sp = SUS::split(a.value(), '@');
			TypeTag tag;
			TypeInfo::unserialize_t(sp[0], &tag, *this);
			return TypeInfo::get(tag, sp[1], *this);
		};

		for (auto n_enum : doc_root.child("enums"))
		{
			auto name = std::string(n_enum.attribute("name").value());
			auto& e = enums.emplace(sh(name.c_str()), EnumInfo()).first->second;
			e.name = name;
			for (auto n_item : n_enum.child("items"))
			{
				auto& i = e.items.emplace_back();
				i.ei = &e;
				i.name = n_item.attribute("name").value();
				i.value = n_item.attribute("value").as_int();
			}
		}
		for (auto n_udt : doc_root.child("udts"))
		{
			auto name = std::string(n_udt.attribute("name").value());
			auto& u = udts.emplace(sh(name.c_str()), UdtInfo()).first->second;
			u.name = name;
			u.size = n_udt.attribute("size").as_uint();
			if (auto a = n_udt.attribute("base_class_name"); a)
				u.base_class_name = a.value();

			for (auto n_variable : n_udt.child("variables"))
			{
				auto& v = u.variables.emplace_back();
				v.ui = &u;
				v.type = read_ti(n_variable.attribute("type"));
				v.name = n_variable.attribute("name").value();
				v.offset = n_variable.attribute("offset").as_uint();
				if (auto a = n_variable.attribute("array_size"); a)
					v.array_size = a.as_uint();
				if (auto a = n_variable.attribute("array_stride"); a)
					v.array_stride = a.as_uint();
				if (auto a = n_variable.attribute("default_value"); a)
					v.default_value = a.value();
				if (auto a = n_variable.attribute("metas"); a)
					v.metas.from_string(a.value());
			}
			for (auto n_function : n_udt.child("functions"))
			{
				auto& f = u.functions.emplace_back();
				f.ui = &u;
				f.name = n_function.attribute("name").value();
				f.rva = n_function.attribute("rva").as_uint();
				f.voff = n_function.attribute("voff").as_int();
				if (auto a = n_function.attribute("is_static"); a)
					f.is_static = a.as_bool();
				f.return_type = read_ti(n_function.attribute("return_type"));
				if (auto a = n_function.attribute("metas"); a)
					f.metas.from_string(a.value());
				f.library = library;
				for (auto n_parameter : n_function)
					f.parameters.push_back(read_ti(n_parameter.attribute("v")));
			}
			for (auto n_attribute : n_udt.child("attributes"))
			{
				auto& a = u.attributes.emplace_back();
				a.ui = &u;
				a.name = n_attribute.attribute("name").value();
				a.type = read_ti(n_attribute.attribute("type"));
				if (auto att = n_attribute.attribute("var_idx"); att)
					a.var_idx = att.as_int();
				if (auto att = n_attribute.attribute("getter_idx"); att)
					a.getter_idx = att.as_int();
				if (auto att = n_attribute.attribute("setter_idx"); att)
					a.setter_idx = att.as_int();
				if (auto att = n_attribute.attribute("default_value"); att)
					a.default_value = att.value();
			}
		}

		return true;
	}

	void TypeInfoDataBase::load(const std::filesystem::path& filename)
	{
		std::filesystem::path path(filename);
		if (!path.is_absolute())
			path = get_app_path() / path;

		void* library = nullptr;
		if (path.extension() != L".typeinfo")
		{
			library = LoadLibraryW(path.c_str());
			path.replace_extension(L".typeinfo");
		}

		std::ifstream file(path);
		if (!file.good())
		{
			wprintf(L"typeinfo do not exist: %s\n", path.c_str());
			assert(0);
			return;
		}
		if (!load(file, library))
		{
			wprintf(L"typeinfo wrong format: %s\n", path.c_str());
			assert(0);
		}
		file.close();
	}

	void TypeInfoDataBase::save(std::ofstream& file)
	{
		pugi::xml_document doc;
		auto doc_root = doc.append_child("typeinfo");

		auto write_ti = [&](TypeInfo* ti, pugi::xml_attribute a) {
			a.set_value((TypeInfo::serialize_t(&ti->tag, *this) + '@' + ti->name).c_str());
		};

		if (!enums.empty())
		{
			std::vector<EnumInfo*> sorted_enums(enums.size());
			auto i = 0;
			for (auto& ei : enums)
			{
				sorted_enums[i] = &ei.second;
				i++;
			}
			std::sort(sorted_enums.begin(), sorted_enums.end(), [](EnumInfo* a, EnumInfo* b) {
				return a->name < b->name;
			});

			auto n_enums = doc_root.append_child("enums");
			for (auto ei : sorted_enums)
			{
				auto n_enum = n_enums.append_child("enum");
				n_enum.append_attribute("name").set_value(ei->name.c_str());
				auto n_items = n_enum.append_child("items");
				for (auto& i : ei->items)
				{
					auto n_item = n_items.append_child("item");
					n_item.append_attribute("name").set_value(i.name.c_str());
					n_item.append_attribute("value").set_value(i.value);
				}
			}
		}

		if (!udts.empty())
		{
			std::vector<std::pair<int, UdtInfo*>> sorted_udts(udts.size());
			{
				auto i = 0;
				for (auto& ui : udts)
				{
					sorted_udts[i].first = -1;
					sorted_udts[i].second = &ui.second;
					i++;
				}
			}
			std::function<int(int)> get_rank;
			get_rank = [&](int idx) {
				auto& item = sorted_udts[idx];
				if (item.first != -1)
					return item.first;
				item.first = 0;
				for (auto& vi : item.second->variables)
				{
					switch (vi.type->tag)
					{
					case TagU:
					case TagVU:
						for (auto i = 0; i < sorted_udts.size(); i++)
						{
							if (sorted_udts[i].second->name == vi.type->name)
							{
								item.first = max(item.first, get_rank(i));
								break;
							}
						}
						break;
					}
				}
				item.first++;
				return item.first;
			};
			for (auto i = 0; i < sorted_udts.size(); i++)
				get_rank(i);
			std::sort(sorted_udts.begin(), sorted_udts.end(), [](const auto& a, const auto& b) {
				if (a.first == b.first)
					return a.second->name < b.second->name;
				return a.first < b.first;
			});

			auto n_udts = doc_root.append_child("udts");
			for (auto ui : sorted_udts)
			{
				auto n_udt = n_udts.append_child("udt");
				n_udt.append_attribute("name").set_value(ui.second->name.c_str());
				if (ui.second->size != 0)
					n_udt.append_attribute("size").set_value(ui.second->size);
				if (!ui.second->base_class_name.empty())
					n_udt.append_attribute("base_class_name").set_value(ui.second->base_class_name.c_str());
				if (!ui.second->variables.empty())
				{
					auto n_variables = n_udt.prepend_child("variables");
					for (auto& vi : ui.second->variables)
					{
						auto n_variable = n_variables.append_child("variable");
						write_ti(vi.type, n_variable.append_attribute("type"));
						n_variable.append_attribute("name").set_value(vi.name.c_str());
						if (vi.offset != 0)
							n_variable.append_attribute("offset").set_value(vi.offset);
						if (vi.array_size != 0)
							n_variable.append_attribute("array_size").set_value(vi.array_size);
						if (vi.array_stride != 0)
							n_variable.append_attribute("array_stride").set_value(vi.array_stride);
						if (!vi.default_value.empty())
							n_variable.append_attribute("default_value").set_value(vi.default_value.c_str());
						if (auto str = vi.metas.to_string(); !str.empty())
							n_variable.append_attribute("metas").set_value(str.c_str());
					}
				}
				if (!ui.second->functions.empty())
				{
					auto n_functions = n_udt.append_child("functions");
					for (auto& fi : ui.second->functions)
					{
						auto n_function = n_functions.append_child("function");
						n_function.append_attribute("name").set_value(fi.name.c_str());
						n_function.append_attribute("rva").set_value(fi.rva);
						n_function.append_attribute("voff").set_value(fi.voff);
						if (fi.is_static)
							n_function.append_attribute("is_static").set_value(fi.is_static);
						write_ti(fi.return_type, n_function.append_attribute("return_type"));
						if (auto str = fi.metas.to_string(); !str.empty())
							n_function.append_attribute("metas").set_value(str.c_str());
						if (!fi.parameters.empty())
						{
							for (auto p : fi.parameters)
								write_ti(p, n_function.append_child("parameter").append_attribute("v"));
						}
					}
				}
				if (!ui.second->attributes.empty())
				{
					auto n_attributes = n_udt.append_child("attributes");
					for (auto& a : ui.second->attributes)
					{
						auto n_attribute = n_attributes.append_child("attribute");
						n_attribute.append_attribute("name").set_value(a.name.c_str());
						write_ti(a.type, n_attribute.append_attribute("type"));
						if (a.var_idx != -1)
							n_attribute.append_attribute("var_idx").set_value(a.var_idx);
						if (a.getter_idx != -1)
							n_attribute.append_attribute("getter_idx").set_value(a.getter_idx);
						if (a.setter_idx != -1)
							n_attribute.append_attribute("setter_idx").set_value(a.setter_idx);
						if (!a.default_value.empty())
							n_attribute.append_attribute("default_value").set_value(a.default_value.c_str());
					}
				}
			}
		}

		doc.save(file);
	}

	void TypeInfoDataBase::save(const std::filesystem::path& filename)
	{
		std::ofstream file(filename);
		save(file);
	}
}
