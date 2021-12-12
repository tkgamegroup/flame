#include <flame/xml.h>
#include <flame/foundation/typeinfo.h>
#include <flame/foundation/system.h>

#include <Windows.h>
#include <dia2.h>
#include <atlbase.h>
#include <chrono>

#include <boost/regex.hpp>

using namespace flame;

TypeInfoDataBase db;

TypeTag parse_vector(std::string& name)
{
	static std::regex reg("std::vector<([\\w\\s:\\*<>]+),");
	std::smatch res;
	if (std::regex_search(name, res, reg))
	{
		name = res[1].str();
		auto is_enum = SUS::cut_head_if(name, "enum ");
		name = TypeInfo::format_name(name);
		if (is_enum)
			return TagVE;
		auto is_pointer = SUS::cut_tail_if(name, "*") || SUS::cut_tail_if(name, "*__ptr64");
		if (TypeInfo::is_basic_type(name))
			return is_pointer ? TagVPD : TagVD;
		else
			return is_pointer ? TagVPU : TagVU;
	}
	assert(0);
	return TagCount;
}

TypeInfo* typeinfo_from_symbol(IDiaSymbol* s_type)
{
	DWORD dw;
	wchar_t* pwname;

	auto base_type_name = [](IDiaSymbol* s)->std::string {
		DWORD baseType;
		s->get_baseType(&baseType);
		ULONGLONG len;
		s->get_length(&len);
		std::string name;
		switch (baseType)
		{
		case btVoid: return "void";
		case btChar: return "char";
		case btWChar: return "wchar_t";
		case btBool: return "bool";
		case btUInt:
		case btULong: name = "u";
		case btInt:
		case btLong:
			switch (len)
			{
			case 1: name += "char"; return name;
			case 2: name += "short"; return name;
			case 4: name += "int"; return name;
			case 8: name += "int64"; return name;
			}
			break;
		case btFloat:
			switch (len)
			{
			case 4: return "float";
			case 8: return "double";
			}
			break;
		default: assert(0);
		}
	};

	s_type->get_symTag(&dw);
	switch (dw)
	{
	case SymTagEnum:
	{
		s_type->get_name(&pwname);
		return TypeInfo::get(TagE, TypeInfo::format_name(w2s(pwname)), db);
	}
	case SymTagBaseType:
		return TypeInfo::get(TagD, base_type_name(s_type));
	case SymTagPointerType:
	{
		std::string name;
		TypeInfo* ret = nullptr;
		IDiaSymbol* pointer_type;
		s_type->get_type(&pointer_type);
		pointer_type->get_symTag(&dw);
		switch (dw)
		{
		case SymTagEnum:
			pointer_type->get_name(&pwname);
			ret = TypeInfo::get(TagPE, TypeInfo::format_name(w2s(pwname)), db);
			break;
		case SymTagBaseType:
			ret = TypeInfo::get(TagPD, TypeInfo::format_name(base_type_name(pointer_type)), db);
			break;
		case SymTagPointerType:
			break;
		case SymTagUDT:
		{
			pointer_type->get_name(&pwname);
			auto name = TypeInfo::format_name(w2s(pwname));
			if (TypeInfo::is_basic_type(name))
				ret = TypeInfo::get(TagPD, name, db);
			else
				ret = TypeInfo::get(TagPU, name, db);
		}
			break;
		}
		assert(ret);
		pointer_type->Release();
		return ret;
	}
	case SymTagUDT:
	{
		s_type->get_name(&pwname);
		auto name = TypeInfo::format_name(w2s(pwname));
		if (TypeInfo::is_basic_type(name))
			return TypeInfo::get(TagD, name, db);
		else
		{
			if (name.starts_with("std::vector<"))
			{
				name = w2s(pwname);
				auto tag = parse_vector(name);
				return TypeInfo::get(tag, name, db);
			}
			return TypeInfo::get(TagU, name, db);
		}
	}
	case SymTagFunctionArgType:
	{
		IDiaSymbol* s_arg_type;
		s_type->get_type(&s_arg_type);
		auto ret = typeinfo_from_symbol(s_arg_type);
		s_arg_type->Release();
		return ret;
	}
	default:
		return TypeInfo::get(TagD, "__unsupported__symtag_" + std::to_string(dw));
	}
}

void* load_exe_as_dll(const std::filesystem::path& path);

int main(int argc, char **args)
{
	auto ap = parse_args(argc, args);
	auto input_path = std::filesystem::path(ap.get_item("-i"));
	auto desc_path = std::filesystem::path(ap.get_item("-d"));
	if (input_path.empty())
		goto show_usage;

	goto process;

show_usage:
	printf("usage: typeinfogen -i <target> -e <rule1> [<rule2>...] -u <rule1> [<rule2>...] -f <rule1> [<rule2>...] [-d <desc file>]\n");
	return 0;

process:
	printf("typeinfogen: %s\n", input_path.string().c_str());
	if (!std::filesystem::exists(input_path))
	{
		printf("target does not exist: %s\n", input_path.string().c_str());
		return 0;
	}

	auto foundation_path = get_app_path(false) / L"flame_foundation.dll";
	if (input_path != foundation_path)
	{
		foundation_path.replace_extension(L".typeinfo");
		tidb.load(foundation_path);
	}

	auto pdb_path = input_path;
	pdb_path.replace_extension(L".pdb");
	if (!std::filesystem::exists(pdb_path))
	{
		printf("pdb does not exist: %s\n", pdb_path.string().c_str());
		return 0;
	}

	auto typeinfo_path = input_path;
	typeinfo_path.replace_extension(L".typeinfo");

	if (std::filesystem::exists(typeinfo_path))
	{
		auto lwt = std::filesystem::last_write_time(typeinfo_path);
		if (lwt > std::filesystem::last_write_time(pdb_path) && (!desc_path.empty() && lwt > std::filesystem::last_write_time(desc_path)))
		{
			printf("typeinfogen: %s up to date\n", typeinfo_path.string().c_str());
			return 0;
		}
	}

	struct EnumRule
	{
		boost::regex name;

		bool pass(const std::string& str) const
		{
			if (boost::regex_search(str, name))
				return true;
			return false;
		}
	};
	struct UdtRule
	{
		struct Item
		{
			TypeTag type;
			boost::regex name;
			std::string metas;
		};

		boost::regex name;
		std::vector<Item> items;

		bool pass(const std::string& str) const
		{
			if (boost::regex_search(str, name))
				return true;
			return false;
		}

		bool pass_item(TypeTag type, const std::string& name, std::string* metas = nullptr) const
		{
			for (auto& i : items)
			{
				if (i.type == type && boost::regex_search(name, i.name))
				{
					if (metas)
						*metas = i.metas;
					return true;
				}
			}
			return false;
		}
	};
	std::vector<EnumRule> enum_rules;
	std::vector<UdtRule> udt_rules;
	auto pass_enum = [&](const std::string& name) {
		for (auto& r : enum_rules)
		{
			if (r.pass(name))
				return true;
		}
		return false;
	};
	auto pass_udt = [&](const std::string& name, UdtRule*& out_r) {
		for (auto& r : udt_rules)
		{
			if (r.pass(name))
			{
				out_r = &r;
				return true;
			}
		}
		return false;
	};
	auto add_named_enum_rule = [&](const std::string& n) {
		auto& r = enum_rules.emplace_back();
		r.name = "^" + n + "$";
	};
	auto add_named_udt_rule = [&](const std::string& n) {
		auto& r = udt_rules.emplace_back();
		r.name = "^" + n + "$";
		auto& dr = r.items.emplace_back();
		dr.type = TagD;
		dr.name = "^[\\w:]+$";
		auto& fr1 = r.items.emplace_back();
		fr1.type = TagF;
		fr1.name = "^ctor$";
		auto& fr2 = r.items.emplace_back();
		fr2.type = TagF;
		fr2.name = "^dtor$";
	};
	for (auto& i : ap.get_items("-enum"))
		add_named_enum_rule(i);
	for (auto& i : ap.get_items("-udt"))
		add_named_udt_rule(i);
	if (!desc_path.empty())
	{
		pugi::xml_document desc_doc;
		pugi::xml_node desc_root;
		if (!desc_doc.load_file(desc_path.c_str()) || (desc_root = desc_doc.first_child()).name() != std::string("desc"))
		{
			printf("desc does not exist or wrong format_name: %s\n", desc_path.string().c_str());
			return 0;
		}

		for (auto n_rule : desc_root.child("enums"))
		{
			auto& er = enum_rules.emplace_back();
			er.name = n_rule.attribute("name").value();
		}
		for (auto n_rule : desc_root.child("udts"))
		{
			auto& ur = udt_rules.emplace_back();
			ur.name = n_rule.attribute("name").value();
			for (auto n_i : n_rule)
			{
				auto& item = ur.items.emplace_back();
				item.type = std::string(n_i.attribute("type").value()) == "function" ? TagF : TagD;
				item.name = n_i.attribute("name").value();
				item.metas = n_i.attribute("metas").value();
			}
		}
	}

	if (FAILED(CoInitialize(NULL)))
	{
		printf("typeinfogen: com initial failed, exit\n");
		assert(0);
		return 1;
	}

	CComPtr<IDiaDataSource> dia_source;
	if (FAILED(CoCreateInstance(CLSID_DiaSource, NULL, CLSCTX_INPROC_SERVER, __uuidof(IDiaDataSource), (void**)&dia_source)))
	{
		printf("typeinfogen: dia not found, exit\n");
		assert(0);
		return 1;
	}

	if (FAILED(dia_source->loadDataFromPdb(pdb_path.c_str())))
	{
		printf("pdb failed to open: %s\n", pdb_path.string().c_str());
		assert(0);
		return 1;
	}

	CComPtr<IDiaSession> session;
	if (FAILED(dia_source->openSession(&session)))
	{
		printf("session failed to open\n");
		assert(0);
		return 1;
	}

	CComPtr<IDiaSymbol> global;
	if (FAILED(session->get_globalScope(&global)))
	{
		printf("failed to get global\n");
		assert(0);
		return 1;
	}

	BOOL b;
	LONG l;
	ULONG ul;
	ULONGLONG ull;
	DWORD dw;
	wchar_t* pwname;

	HMODULE library = nullptr;
	if (input_path.extension() == L".exe")
		library = (HMODULE)load_exe_as_dll(input_path.c_str());
	else
		library = LoadLibraryW(input_path.c_str());

	IDiaEnumSymbols* s_functions;
	global->findChildren(SymTagFunction, NULL, nsNone, &s_functions);
	IDiaSymbol* s_function;
	while (SUCCEEDED(s_functions->Next(1, &s_function, &ul)) && (ul == 1))
	{
		s_function->get_name(&pwname);
		auto name = w2s(pwname);
		if (name.starts_with("flame::TypeInfo::get"))
		{
			static std::regex reg("^flame::TypeInfo::get<([\\w\\s:<>,]+)>$");
			std::smatch res;
			if (std::regex_search(name, res, reg))
			{
				auto name = res[1].str();
				if (SUS::cut_head_if(name, "enum "))
				{
					auto& r = enum_rules.emplace_back();
					r.name = "^" + name + "$";
				}
				else
				{
					name = TypeInfo::format_name(name);
					if (!TypeInfo::is_basic_type(name))
					{
						if (name.starts_with("std::vector<"))
						{
							name = res[1].str();
							auto tag = parse_vector(name);
							if (tag == TagVE)
								add_named_udt_rule(name);
							else if (tag == TagVU)
								add_named_udt_rule(name);
						}
						else
							add_named_udt_rule(name);
					}
				}
			}
		}
	}
	s_functions->Release();

	auto need_collect = true;
	while (need_collect)
	{
		need_collect = false;
		std::vector<TypeInfo*> referenced_types;
		auto reference_type = [&](TypeInfo* ti) {
			if (ti->tag == TagD)
				return;
			if (ti->tag >= TagP_Beg && ti->tag <= TagP_End)
				return;
			if (std::find(referenced_types.begin(), referenced_types.end(), ti) == referenced_types.end())
				referenced_types.push_back(ti);
		};

		IDiaEnumSymbols* s_enums;
		global->findChildren(SymTagEnum, NULL, nsNone, &s_enums);
		IDiaSymbol* s_enum;
		while (SUCCEEDED(s_enums->Next(1, &s_enum, &ul)) && (ul == 1))
		{
			s_enum->get_name(&pwname);
			auto enum_name = w2s(pwname);
			if (!find_enum(enum_name, db) && pass_enum(enum_name))
			{
				EnumInfo e;
				e.name = enum_name;
				std::vector<std::pair<std::string, int>> items;

				IDiaEnumSymbols* s_items;
				s_enum->findChildren(SymTagNull, NULL, nsNone, &s_items);
				IDiaSymbol* s_item;
				while (SUCCEEDED(s_items->Next(1, &s_item, &ul)) && (ul == 1))
				{
					VARIANT variant;
					ZeroMemory(&variant, sizeof(variant));
					s_item->get_name(&pwname);
					s_item->get_value(&variant);

					auto item_name = w2s(pwname);
					if (!item_name.ends_with("_Max") && !item_name.ends_with("_Count"))
						items.emplace_back(item_name, variant.lVal);

					s_item->Release();
				}
				s_items->Release();

				if (items.size() >= 2)
				{
					while (true)
					{
						auto same = true;
						auto ch = items[0].first[0];
						for (auto i = 1; i < items.size(); i++)
						{
							if (items[i].first[0] != ch)
							{
								same = false;
								break;
							}
						}
						if (same)
						{
							for (auto& i : items)
								i.first.erase(i.first.begin());
						}
						else
							break;
					}
				}

				for (auto& i : items)
				{
					auto& ii = e.items.emplace_back();
					ii.name = i.first;
					ii.value = i.second;
				}

				db.enums.emplace(e.name, e);
			}
			s_enum->Release();
		}
		s_enums->Release();

		IDiaEnumSymbols* s_udts;
		global->findChildren(SymTagUDT, NULL, nsNone, &s_udts);
		IDiaSymbol* s_udt;
		while (SUCCEEDED(s_udts->Next(1, &s_udt, &ul)) && (ul == 1))
		{
			s_udt->get_name(&pwname);
			auto udt_name = w2s(pwname);
			UdtRule* ur;
			if (!find_udt(udt_name, db) && pass_udt(udt_name, ur))
			{
				s_udt->get_length(&ull);
				auto udt_size = ull;
				std::string base_class_name;

				IDiaEnumSymbols* s_base_classes;
				IDiaSymbol* s_base_class;
				s_udt->findChildren(SymTagBaseClass, NULL, nsNone, &s_base_classes);
				if (SUCCEEDED(s_base_classes->Next(1, &s_base_class, &ul)) && (ul == 1))
				{
					s_base_class->get_name(&pwname);
					base_class_name = w2s(pwname);
				}

				UdtInfo u;
				u.name = udt_name;
				u.size = udt_size;
				u.base_class_name = base_class_name;

				DWORD ctor = 0;
				DWORD dtor = 0;

				IDiaEnumSymbols* s_all;
				s_udt->findChildren(SymTagNull, NULL, nsNone, &s_all);
				IDiaSymbol* s_obj;
				while (SUCCEEDED(s_all->Next(1, &s_obj, &ul)) && (ul == 1))
				{
					s_obj->get_name(&pwname);
					auto name = w2s(pwname);
					s_obj->Release();
				}
				s_all->Release();

				pugi::xml_node n_functions;

				IDiaEnumSymbols* s_functions;
				s_udt->findChildren(SymTagFunction, NULL, nsNone, &s_functions);
				IDiaSymbol* s_function;
				while (SUCCEEDED(s_functions->Next(1, &s_function, &ul)) && (ul == 1))
				{
					s_function->get_name(&pwname);
					auto name = w2s(pwname);
					if (udt_name.ends_with(name))
						name = "ctor";
					else if (name[0] == '~')
						name = "dtor";
					if (name == "__local_vftable_ctor_closure" || name == "__vecDelDtor")
						continue;

					auto rva = 0;
					auto voff = -1;
					if (s_function->get_relativeVirtualAddress(&dw) == S_OK)
						rva = dw;
					else if (s_function->get_virtualBaseOffset(&dw) == S_OK)
						voff = dw;

					if (!rva && voff == -1)
						continue;

					std::string metas;
					if (ur->pass_item(TagF, name, &metas))
					{
						if (rva || voff != -1)
						{
							auto& fi = u.functions.emplace_back();
							fi.name = name;
							fi.rva = rva;
							fi.voff = voff;

							IDiaSymbol* s_function_type;
							s_function->get_type(&s_function_type);

							IDiaSymbol* s_return_type;
							s_function_type->get_type(&s_return_type);
							fi.return_type = typeinfo_from_symbol(s_return_type);
							reference_type(fi.return_type);
							s_return_type->Release();

							IDiaSymbol6* s6_function = (IDiaSymbol6*)s_function;
							fi.is_static = false;
							if (s6_function->get_isStaticMemberFunc(&b) == S_OK)
								fi.is_static = b;

							fi.metas.from_string(metas);

							IDiaEnumSymbols* s_parameters;
							s_function_type->findChildren(SymTagFunctionArgType, NULL, nsNone, &s_parameters);
							IDiaSymbol* s_parameter;
							while (SUCCEEDED(s_parameters->Next(1, &s_parameter, &ul)) && (ul == 1))
							{
								IDiaSymbol* s_type;
								s_parameter->get_type(&s_type);
								auto ti = typeinfo_from_symbol(s_parameter);
								fi.parameters.push_back(ti);
								reference_type(ti);
								s_type->Release();

								s_parameter->Release();
							}
							s_parameters->Release();

							s_function_type->Release();

							if (name == "ctor" && fi.parameters.empty())
								ctor = rva;
							else if (name == "dtor")
								dtor = rva;
						}
					}

					s_function->Release();
				}
				s_functions->Release();

				void* obj = nullptr;
				if (ctor && library)
				{
					obj = malloc(udt_size);
					memset(obj, 0, udt_size);
					a2f<void(*)(void*)>((char*)library + ctor)(obj);
				}

				pugi::xml_node n_variables;

				IDiaEnumSymbols* s_variables;
				s_udt->findChildren(SymTagData, NULL, nsNone, &s_variables);
				IDiaSymbol* s_variable;
				while (SUCCEEDED(s_variables->Next(1, &s_variable, &ul)) && (ul == 1))
				{
					s_variable->get_name(&pwname);
					auto name = w2s(pwname);
					std::string metas;
					if (ur->pass_item(TagD, name, &metas))
					{
						IDiaSymbol* s_type;
						s_variable->get_type(&s_type);

						s_variable->get_offset(&l);
						auto offset = l;

						auto& vi = u.variables.emplace_back();
						vi.type = typeinfo_from_symbol(s_type);
						reference_type(vi.type);
						vi.name = name;
						vi.offset = offset;
						vi.array_size = 0;
						vi.array_stride = 0;
						vi.default_value = obj ? vi.type->serialize((char*)obj + offset) : "";
						vi.metas.from_string(metas);

						s_type->Release();
					}
					s_variable->Release();
				}
				s_variables->Release();

				if (obj)
				{
					if (dtor)
						a2f<void(*)(void*)>((char*)library + dtor)(obj);
					free(obj);
				}

				db.udts.emplace(u.name, u);
			}
			s_udt->Release();
		}
		s_udts->Release();

		enum_rules.clear();
		udt_rules.clear();
		for (auto t : referenced_types)
		{
			switch (t->tag)
			{
			case TagE:
			case TagVE:
				if (!find_enum(t->name, db))
				{
					add_named_enum_rule(t->name);

					need_collect = true;
				}
				break;
			case TagU:
			case TagVU:
				if (!find_udt(t->name, db))
				{
					add_named_udt_rule(t->name);

					need_collect = true;
				}
				break;
			}
		}
	}

	db.save(typeinfo_path);

	printf("typeinfogen: %s generated\n", typeinfo_path.string().c_str());

	return 0;
}
