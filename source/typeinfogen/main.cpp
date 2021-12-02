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
		return TypeInfo::get(TypeInfo::format(TagEnum, w2s(pwname)), db);
	}
	case SymTagBaseType:
		return TypeInfo::get(TagData, base_type_name(s_type));
	case SymTagPointerType:
	{
		std::string name;
		IDiaSymbol* pointer_type;
		s_type->get_type(&pointer_type);
		pointer_type->get_symTag(&dw);
		switch (dw)
		{
		case SymTagEnum:
			name = "int";
			break;
		case SymTagBaseType:
			name = base_type_name(pointer_type);
			break;
		case SymTagPointerType:
			name = "complex_pointer";
			break;
		case SymTagUDT:
			pointer_type->get_name(&pwname);
			name = TypeInfo::format(TagData, w2s(pwname)).second;
			break;
		}
		pointer_type->Release();
		return TypeInfo::get(TagPointer, name);
	}
	case SymTagUDT:
	{
		s_type->get_name(&pwname);
		return TypeInfo::get(TypeInfo::format(TagData, w2s(pwname)), db);
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
		return TypeInfo::get(TagData, "__unsupported__symtag_" + std::to_string(dw));
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
		tidb.load_typeinfo(foundation_path);
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
	for (auto& i : ap.get_items("-enum"))
	{
		auto& r = enum_rules.emplace_back();
		r.name = "^" + i + "$";
	}
	for (auto& i : ap.get_items("-udt"))
	{
		auto& r = udt_rules.emplace_back();
		r.name = "^" + i + "$";
		auto& dr = r.items.emplace_back();
		dr.type = TagData;
		dr.name = "^[\\w:]+$";
		auto& fr = r.items.emplace_back();
		fr.type = TagFunction;
		fr.name = "^[\\w:~]+$";
	}
	if (!desc_path.empty())
	{
		pugi::xml_document desc_doc;
		pugi::xml_node desc_root;
		if (!desc_doc.load_file(desc_path.c_str()) || (desc_root = desc_doc.first_child()).name() != std::string("desc"))
		{
			printf("desc does not exist or wrong format: %s\n", desc_path.string().c_str());
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
				item.type = std::string(n_i.attribute("type").value()) == "function" ? TagFunction : TagData;
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

	auto new_enum = [&](const std::string& enum_name, IDiaSymbol* s_enum) {
		if (find_enum(enum_name, db))
			return;

		auto& e = db.enums.emplace(enum_name, EnumInfo()).first->second;
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
	};

	auto new_udt = [&](const std::string& udt_name, IDiaSymbol* s_udt, UdtRule* ur) {
		if (find_udt(udt_name, db))
			return;

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

		auto& u = db.udts.emplace(udt_name, UdtInfo()).first->second;
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

			std::string metas;
			if (!ur || ur->pass_item(TagFunction, name, &metas))
			{
				auto rva = 0;
				auto voff = -1;
				if (s_function->get_relativeVirtualAddress(&dw) == S_OK)
					rva = dw;
				else if (s_function->get_virtualBaseOffset(&dw) == S_OK)
					voff = dw;

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
						fi.parameters.push_back(typeinfo_from_symbol(s_parameter));
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
			if (!ur || ur->pass_item(TagData, name, &metas))
			{
				IDiaSymbol* s_type;
				s_variable->get_type(&s_type);

				s_variable->get_offset(&l);
				auto offset = l;

				auto& vi = u.variables.emplace_back();
				vi.type = typeinfo_from_symbol(s_type);
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
	};

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
				auto str = res[1].str();
				if (SUS::cut_head_if(str, "enum "))
				{
					auto& r = enum_rules.emplace_back();
					r.name = "^" + str + "$";
				}
				else
				{
					auto tagname = TypeInfo::format(TagData, str);
					if (tagname.first != TagData)
					{
						auto& r = udt_rules.emplace_back();
						r.name = "^" + tagname.second + "$";
						auto& dr = r.items.emplace_back();
						dr.type = TagData;
						dr.name = "^[\\w:]+$";
						auto& fr = r.items.emplace_back();
						fr.type = TagFunction;
						fr.name = "^[\\w:~]+$";
					}
				}
			}
		}
	}
	s_functions->Release();

	IDiaEnumSymbols* s_enums;
	global->findChildren(SymTagEnum, NULL, nsNone, &s_enums);
	IDiaSymbol* s_enum;
	while (SUCCEEDED(s_enums->Next(1, &s_enum, &ul)) && (ul == 1))
	{
		s_enum->get_name(&pwname);
		auto name = w2s(pwname);
		for (auto& er : enum_rules)
		{
			if (er.pass(name))
			{
				new_enum(name, s_enum);
				break;
			}
		}
		s_enum->Release();
	}

	IDiaEnumSymbols* s_udts;
	global->findChildren(SymTagUDT, NULL, nsNone, &s_udts);
	IDiaSymbol* s_udt;
	while (SUCCEEDED(s_udts->Next(1, &s_udt, &ul)) && (ul == 1))
	{
		s_udt->get_name(&pwname);
		auto name = w2s(pwname);
		for (auto& ur : udt_rules)
		{
			if (ur.pass(name))
			{
				new_udt(name, s_udt, &ur);
				break;
			}
		}
		s_udt->Release();
	}
	s_udts->Release();

	db.save_typeinfo(typeinfo_path);

	printf("typeinfogen: %s generated\n", typeinfo_path.string().c_str());

	return 0;
}
