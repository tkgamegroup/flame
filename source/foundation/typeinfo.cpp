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
		case TagD:
			t = new TypeInfo_Data(name, 0);
			break;
		case TagU:
			t = new TypeInfo_Udt(name, db);
			break;
		case TagR:
			t = new TypeInfo_Pair(name, db);
			break;
		case TagT:
			t = new TypeInfo_Tuple(name, db);
			break;
		case TagO:
			t = new TypeInfo_VirtualUdt(name, db);
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
		case TagQU:
			t = new TypeInfo_UniquePointerOfUdt(name, db);
			break;
		case TagPVE:
			t = new TypeInfo_PointerOfVectorOfEnum(name, db);
			break;
		case TagPVD:
			t = new TypeInfo_PointerOfVectorOfData(name, db);
			break;
		case TagPVU:
			t = new TypeInfo_PointerOfVectorOfUdt(name, db);
			break;
		case TagPVR:
			t = new TypeInfo_PointerOfVectorOfPair(name, db);
			break;
		case TagPVT:
			t = new TypeInfo_PointerOfVectorOfTuple(name, db);
			break;
		case TagAE:
			t = new TypeInfo_ArrayOfEnum(name, db);
			break;
		case TagAD:
			t = new TypeInfo_ArrayOfData(name, db);
			break;
		case TagAU:
			t = new TypeInfo_ArrayOfUdt(name, db);
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
		case TagVR:
			t = new TypeInfo_VectorOfPair(name, db);
			break;
		case TagVT:
			t = new TypeInfo_VectorOfTuple(name, db);
			break;
		case TagVPU:
			t = new TypeInfo_VectorOfPointerOfUdt(name, db);
			break;
		case TagVQU:
			t = new TypeInfo_VectorOfUniquePointerOfUdt(name, db);
			break;
		}

		if (t)
			db.typeinfos.emplace(key, t);
		return t;
	}

	void* UdtInfo::create_object(void* p) const
	{
		if (!p)
		{
			if (auto fi = find_function("create"_h); fi && is_in(fi->return_type->tag, TagP_Beg, TagP_End))
			{
				if (fi->parameters.empty())
					p = fi->call<void*>(nullptr);
				else if (fi->parameters.size() == 1 && is_in(fi->parameters[0]->tag, TagP_Beg, TagP_End))
					p = fi->call<void*>(nullptr, nullptr);
				return p;
			}
			p = malloc(size);
		}
		if (auto fi = find_function("dctor"_h); fi && fi->rva)
			fi->call<void>(p);
		else
		{
			memset(p, 0, size);
			if (!pod)
			{
				for (auto& v : variables)
					v.type->create((char*)p + v.offset);
			}
			for (auto& v : variables)
			{
				if (!v.default_value.empty())
					v.type->unserialize(v.default_value, (char*)p + v.offset);
			}
		}
		return p;
	}

	void UdtInfo::destroy_object(void* p, bool free_memory) const
	{
		if (auto fi = find_function("destroy"_h); free_memory && fi && fi->return_type == TypeInfo::void_type &&
			fi->parameters.size() == 1 && is_pointer(fi->parameters[0]->tag))
			fi->call<void>(nullptr, p);
		else
		{
			if (auto fi = find_function("dtor"_h); fi && fi->rva)
				fi->call<void>(p);
			else
			{
				if (!pod)
				{
					for (auto& v : variables)
						v.type->destroy((char*)p + v.offset, false);
				}
			}
			if (free_memory)
				free(p);
		}
	}

	void UdtInfo::copy_object(void* dst, const void* src) const
	{
		if (auto fi = find_function("operator="_h); fi && fi->rva)
			fi->call<void*>(dst, src);
		else
		{
			if (!pod)
			{
				for (auto& v : variables)
					v.type->copy((char*)dst + v.offset, (char*)src + v.offset);
			}
			else
				memcpy(dst, src, size);
		}
	}

	UdtInfo* UdtInfo::transform_to_serializable() const
	{
		auto ret = new UdtInfo;
		ret->pod = pod;
		for (auto& svi : variables)
		{
			auto& dvi = ret->variables.emplace_back();
			dvi.ui = ret;
			dvi.name = svi.name;
			dvi.name_hash = svi.name_hash;
			dvi.type = svi.type;
			switch (dvi.type->tag)
			{
			case TagPE:
			case TagPD:
			case TagPU:
			case TagPR:
			case TagPT:
				dvi.type = TypeInfo::get(TagD, "std::string");
				ret->pod = false;
				break;
			case TagVPU:
				dvi.type = TypeInfo::get(TagVD, "std::string");
				ret->pod = false;
				break;
			}
			dvi.offset = ret->size;
			dvi.default_value = svi.default_value;
			dvi.metas = svi.metas;

			ret->size += dvi.type->size;
		}
		for (auto i = 0; i < ret->variables.size(); i++)
			ret->variables_map[ret->variables[i].name_hash] = i;
		return ret;
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
	thread_local Rect TypeInfo_Rect::v;
	thread_local AABB TypeInfo_AABB::v;
	thread_local Plane TypeInfo_Plane::v;
	thread_local Frustum TypeInfo_Frustum::v;
	thread_local Curve<1> TypeInfo_Curve1::v;
	thread_local Curve<2> TypeInfo_Curve2::v;
	thread_local Curve<3> TypeInfo_Curve3::v;
	thread_local Curve<4> TypeInfo_Curve4::v;
	thread_local GUID TypeInfo_GUID::v;
	thread_local std::string TypeInfo_string::v;
	thread_local std::wstring TypeInfo_wstring::v;
	thread_local std::filesystem::path TypeInfo_path::v;

	void init_typeinfo()
	{
		_tidb.init_basic_types();

		for (auto& path : get_module_dependencies(get_app_path(true)))
		{
			auto ti_path = path;
			ti_path.replace_extension(".typeinfo");
			if (std::filesystem::exists(ti_path))
				_tidb.load(path);
		}
	}

	TypeInfoDataBase::TypeInfoDataBase()
	{
	}
	
	void TypeInfoDataBase::init_basic_types()
	{
		assert(typeinfos.empty());
		if (!typeinfos.empty())
			return;

		auto add_basic_type = [&](TypeInfo* ti) {
			typeinfos.emplace(TypeInfo::get_hash(ti->tag, ti->name), ti);
			basic_types.push_back(ti);
		};

		TypeInfo::void_type = new TypeInfo_void;
		add_basic_type(TypeInfo::void_type);
		add_basic_type(new TypeInfo_bool);
		add_basic_type(new TypeInfo_char);
		add_basic_type(new TypeInfo_uchar);
		add_basic_type(new TypeInfo_short);
		add_basic_type(new TypeInfo_ushort);
		add_basic_type(new TypeInfo_int);
		add_basic_type(new TypeInfo_uint);
		add_basic_type(new TypeInfo_int64);
		add_basic_type(new TypeInfo_uint64);
		add_basic_type(new TypeInfo_float);
		add_basic_type(new TypeInfo_cvec2);
		add_basic_type(new TypeInfo_cvec3);
		add_basic_type(new TypeInfo_cvec4);
		add_basic_type(new TypeInfo_ivec2);
		add_basic_type(new TypeInfo_ivec3);
		add_basic_type(new TypeInfo_ivec4);
		add_basic_type(new TypeInfo_uvec2);
		add_basic_type(new TypeInfo_uvec3);
		add_basic_type(new TypeInfo_uvec4);
		add_basic_type(new TypeInfo_vec2);
		add_basic_type(new TypeInfo_vec3);
		add_basic_type(new TypeInfo_vec4);
		add_basic_type(new TypeInfo_mat2);
		add_basic_type(new TypeInfo_mat3);
		add_basic_type(new TypeInfo_mat4);
		add_basic_type(new TypeInfo_quat);
		add_basic_type(new TypeInfo_string);
		add_basic_type(new TypeInfo_wstring);
		add_basic_type(new TypeInfo_path);
		add_basic_type(new TypeInfo_Rect);
		add_basic_type(new TypeInfo_AABB);
		add_basic_type(new TypeInfo_Plane);
		add_basic_type(new TypeInfo_Frustum);
		add_basic_type(new TypeInfo_Curve1);
		add_basic_type(new TypeInfo_Curve2);
		add_basic_type(new TypeInfo_Curve3);
		add_basic_type(new TypeInfo_Curve4);
		add_basic_type(new TypeInfo_GUID);
	}

	bool TypeInfoDataBase::load_from_string(const std::string& content, void* library)
	{
		pugi::xml_document doc;
		pugi::xml_node doc_root;
		if (!doc.load_string(content.c_str()) || (doc_root = doc.first_child()).name() != std::string("typeinfo"))
			return false;

		for (auto n_enum : doc_root.child("enums"))
		{
			auto name = std::string(n_enum.attribute("name").value());
			auto& e = enums.emplace(sh(name.c_str()), EnumInfo()).first->second;
			e.db = this;
			e.name = name;
			e.name_hash = sh(name.c_str());
			e.is_flags = name.ends_with("Flags");
			for (auto n_item : n_enum.child("items"))
			{
				auto& i = e.items.emplace_back();
				i.ei = &e;
				i.name = n_item.attribute("name").value();
				i.name_hash = sh(i.name.c_str());
				i.value = n_item.attribute("value").as_int();
			}
			e.library = library;
			if (auto a = n_enum.attribute("source_file"); a)
				e.source_file = a.value();
		}

		// get TypeTag ti not using the static interface, because the ti may change
		auto ti_type_tag = TypeInfo::get(TagE, "flame::TypeTag", *this);
		auto read_ti = [&](pugi::xml_attribute a) {
			auto sp = SUS::to_string_vector(SUS::split(a.value(), '@'));
			TypeTag tag;
			ti_type_tag->unserialize(sp[0], &tag);
			return TypeInfo::get(tag, sp[1], *this);
		};

		for (auto n_udt : doc_root.child("udts"))
		{
			auto name = std::string(n_udt.attribute("name").value());
			auto& u = udts.emplace(sh(name.c_str()), UdtInfo()).first->second;
			u.db = this;
			u.name = name;
			u.name_hash = sh(name.c_str());
			u.size = n_udt.attribute("size").as_uint();
			if (auto a = n_udt.attribute("base_class_name"); a)
				u.base_class_name = a.value();
			u.pod = n_udt.attribute("pod").as_bool();
			for (auto n_variable : n_udt.child("variables"))
			{
				auto& v = u.variables.emplace_back();
				v.ui = &u;
				v.type = read_ti(n_variable.attribute("type"));
				v.name = n_variable.attribute("name").value();
				v.name_hash = sh(v.name.c_str());
				v.offset = n_variable.attribute("offset").as_uint();
				if (auto a = n_variable.attribute("default_value"); a)
					v.default_value = a.value();
				if (auto a = n_variable.attribute("metas"); a)
					v.metas.from_string(a.value());
			}
			for (auto i = 0; i < u.variables.size(); i++)
				u.variables_map.emplace(u.variables[i].name_hash, i);
			for (auto n_function : n_udt.child("functions"))
			{
				auto& f = u.functions.emplace_back();
				f.db = this;
				f.ui = &u;
				f.name = n_function.attribute("name").value();
				f.name_hash = sh(f.name.c_str());
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
			for (auto i = 0; i < u.functions.size(); i++)
				u.functions_map.emplace(u.functions[i].name_hash, i);
			for (auto n_attribute : n_udt.child("attributes"))
			{
				auto& a = u.attributes.emplace_back();
				a.ui = &u;
				a.name = n_attribute.attribute("name").value();
				a.name_hash = sh(a.name.c_str());
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
			for (auto i = 0; i < u.attributes.size(); i++)
				u.attributes_map.emplace(u.attributes[i].name_hash, i);
			u.library = library;
			if (auto a = n_udt.attribute("source_file"); a)
				u.source_file = a.value();
		}
		for (auto n_data : doc_root.child("datas"))
		{
			auto name = std::string(n_data.attribute("name").value());
			auto& d = datas.emplace(sh(name.c_str()), DataInfo()).first->second;
			d.db = this;
			d.name = name;
			d.name_hash = sh(name.c_str());
			d.rva = n_data.attribute("rva").as_uint();
			d.type = read_ti(n_data.attribute("type"));
			if (auto a = n_data.attribute("metas"); a)
				d.metas.from_string(a.value());
			d.library = library;
			if (auto a = n_data.attribute("source_file"); a)
				d.source_file = a.value();
		}
		for (auto& pair : typeinfos)
		{
			if (pair.second->tag == TagU)
			{
				if (auto ti = (TypeInfo_Udt*)pair.second.get(); ti->ui == nullptr)
				{
					auto ui = find_udt(sh(ti->name.c_str()), *this);
					if (ui)
					{
						ti->ui = ui;
						ti->size = ui->size;
						ti->pod = ui->pod;
					}
				}
			}
		}

		return true;
	}

	void* TypeInfoDataBase::load(const std::filesystem::path& filename)
	{
		std::filesystem::path path(filename);
		if (!path.is_absolute())
			path = get_app_path() / path;

		void* library = nullptr;
		if (path.extension() != L".typeinfo")
		{
			library = load_library(path.c_str());
			path.replace_extension(L".typeinfo");
		}

		auto content = get_file_content(path);
		if (content.empty())
		{
			wprintf(L"typeinfo do not exist: %s\n", path.c_str());
			assert(0);
			return library;
		}
		if (!load_from_string(content, library))
		{
			wprintf(L"typeinfo wrong format: %s\n", path.c_str());
			assert(0);
			return library;
		}

		return library;
	}

	void TypeInfoDataBase::unload(void* library)
	{
		std::vector<uint> invalid_tis;
		for (auto it = typeinfos.begin(); it != typeinfos.end(); it++)
		{
			if (auto ei = it->second->retrive_ei(); ei)
			{
				if (ei->library == library)
					invalid_tis.push_back(it->first);
			}
			if (auto ui = it->second->retrive_ui(); ui)
			{
				if (ui->library == library)
					invalid_tis.push_back(it->first);
			}
		}
		for (auto hash : invalid_tis)
			typeinfos.erase(hash);
		for (auto it = enums.begin(); it != enums.end();)
		{
			if (it->second.library == library)
				it = enums.erase(it);
			else
				it++;
		}
		for (auto it = udts.begin(); it != udts.end();)
		{
			if (it->second.library == library)
				it = udts.erase(it);
			else
				it++;
		}

		free_library(library);
	}

	std::string TypeInfoDataBase::save_to_string()
	{
		pugi::xml_document doc;
		auto doc_root = doc.append_child("typeinfo");

		// get TypeTag ti not using the static interface, because the ti may change
		auto ti_type_tag = TypeInfo::get(TagE, "flame::TypeTag", *this);
		auto write_ti = [&](TypeInfo* ti, pugi::xml_attribute a) {
			a.set_value((ti_type_tag->serialize(&ti->tag) + '@' + ti->name).c_str());
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
				n_enum.append_attribute("source_file").set_value(ei->source_file.string().c_str());
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
					auto name = vi.type->name;
					switch (vi.type->tag)
					{
					case TagAU:
						name = ((TypeInfo_ArrayOfUdt*)vi.type)->ti->name;
					case TagU:
					case TagVU:
						for (auto i = 0; i < sorted_udts.size(); i++)
						{
							if (sorted_udts[i].second->name == name)
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
				n_udt.append_attribute("pod").set_value(ui.second->pod);
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
				if (!ui.second->source_file.empty())
					n_udt.append_attribute("source_file").set_value(ui.second->source_file.string().c_str());
			}

			auto n_datas = doc_root.append_child("datas");
			for (auto& di : datas)
			{
				auto n_data = n_datas.append_child("data");
				n_data.append_attribute("name").set_value(di.second.name.c_str());
				n_data.append_attribute("rva").set_value(di.second.rva);
				write_ti(di.second.type, n_data.append_attribute("type"));
				if (auto str = di.second.metas.to_string(); !str.empty())
					n_data.append_attribute("metas").set_value(str.c_str());
				if (!di.second.source_file.empty())
					n_data.append_attribute("source_file").set_value(di.second.source_file.string().c_str());
			}
		}

		std::stringstream ss;
		doc.save(ss);
		return ss.str();
	}

	void TypeInfoDataBase::save(const std::filesystem::path& filename)
	{
		auto content = save_to_string();
		std::ofstream file(filename);
		file << content;
		file.close();
	}
}
