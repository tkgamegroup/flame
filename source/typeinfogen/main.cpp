#include <flame/serialize.h>
#include <flame/foundation/typeinfo.h>

#include <Windows.h>
#include <dia2.h>
#include <atlbase.h>

using namespace flame;

std::string format_type(const wchar_t* in)
{
	auto str = w2s(in);

	{
		static auto reg = std::regex(R"(enum )");
		str = std::regex_replace(str, reg, "");
	}
	{
		static auto reg = std::regex(R"(unsigned )");
		str = std::regex_replace(str, reg, "u");
	}
	{
		static auto reg = std::regex(R"(__int64 )");
		str = std::regex_replace(str, reg, "int64");
	}

	SUS::remove_ch(str, ' ');

	return str;
}

struct TagAndName
{
	TypeTag tag;
	std::string name;

	TagAndName(TypeTag t, const std::string& n) :
		tag(t),
		name(n)
	{
	}
};

TagAndName typeinfo_from_symbol(IDiaSymbol* s_type)
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
		case btVoid:
			return "void";
		case btChar:
			return "char";
		case btWChar:
			return "wchar_t";
		case btBool:
			return "bool";
		case btUInt:
			name = "u";
		case btInt:
			switch (len)
			{
			case 1:
				name += "char";
				return name;
			case 2:
				name += "short";
				return name;
			case 4:
				name += "int";
				return name;
			case 8:
				name += "int64";
				return name;
			}
			break;
		case btFloat:
			switch (len)
			{
			case 4:
				return "float";
			case 8:
				return "double";
			}
			break;
		default:
			fassert(0);
		}
	};

	s_type->get_symTag(&dw);
	switch (dw)
	{
	case SymTagEnum:
	{
		s_type->get_name(&pwname);
		auto name = format_type(pwname);
		return TagAndName(name.ends_with("Flags") ? TypeEnumMulti : TypeEnumSingle, name);
	}
	case SymTagBaseType:
		return TagAndName(TypeData, base_type_name(s_type));
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

			break;
		case SymTagUDT:
			pointer_type->get_name(&pwname);
			name = format_type(pwname);
			break;
		}
		pointer_type->Release();
		return TagAndName(TypePointer, name);
	}
	case SymTagUDT:
	{
		s_type->get_name(&pwname);
		auto name = format_type(pwname);
		return TagAndName(TypeData, name);
	}
	case SymTagFunctionArgType:
	{
		IDiaSymbol* s_arg_type;
		s_type->get_type(&s_arg_type);
		auto ret = typeinfo_from_symbol(s_arg_type);
		s_arg_type->Release();
		return ret;
	}
	}
}

int main(int argc, char **args)
{
	std::string input;
	std::string desc;
	auto ap = pack_args(argc, args);
	if (!ap.get_item("-i", input) || !ap.get_item("-d", desc))
		goto show_usage;

	goto process;

show_usage:
	printf("usage: typeinfogen -i filename -d filename\n"
		"-i: specify the target executable\n"
		"-d: specify the desc file, which contains the reflect rules\n");
	return 0;

process:

	auto executable_path = std::filesystem::path(input);
	if (!std::filesystem::exists(executable_path))
	{
		printf("executable does not exist: %s\n", executable_path.string().c_str());
		return 0;
	}

	auto pdb_path = executable_path;
	pdb_path.replace_extension(L".pdb");

	if (ap.has("-rm"))
	{
		if (std::filesystem::exists(pdb_path))
			std::filesystem::remove(pdb_path);
		return 0;
	}

	struct EnumRule
	{
		std::regex name;
		std::regex exclude;

		bool pass(const std::string& _name) const
		{
			if (std::regex_search(_name, name))
				return exclude._Empty() || !std::regex_search(_name, exclude);
			return false;
		}
	};
	struct UdtRule
	{
		struct Item
		{
			TypeTag type;
			std::regex name;
		};

		std::regex name;
		std::regex exclude;
		std::vector<Item> includes;
		std::vector<Item> excludes;

		bool pass(const std::string& _name) const
		{
			if (std::regex_search(_name, name))
				return exclude._Empty() || !std::regex_search(_name, exclude);
			return false;
		}

		bool pass_item(TypeTag type, const std::string& name) const
		{
			auto ok = false;
			for (auto& i : includes)
			{
				if (i.type == type && std::regex_search(name, i.name))
				{
					ok = true;
					break;
				}
			}
			if (ok)
			{
				for (auto& e : excludes)
				{
					if (e.type == type && std::regex_search(name, e.name))
					{
						ok = false;
						break;
					}
				}
			}
			return ok;
		}
	};
	std::vector<EnumRule> enum_rules;
	std::vector<UdtRule> udt_rules;

	auto desc_path = std::filesystem::path(desc);
	pugi::xml_document desc_doc;
	pugi::xml_node desc_root;
	if (!desc_doc.load_file(desc_path.c_str()) || (desc_root = desc_doc.first_child()).name() != std::string("desc"))
	{
		printf("desc does not exist or wrong format: %s\n", desc_path.string().c_str());
		return 0;
	}

	for (auto n_rule : desc_root.child("enums"))
	{
		EnumRule er;
		er.name = n_rule.attribute("name").value();
		auto str = std::string(n_rule.attribute("exclude").value());
		if (!str.empty())
			er.exclude = str;
		enum_rules.push_back(er);
	}
	for (auto n_rule : desc_root.child("udts"))
	{
		UdtRule ur;
		ur.name = n_rule.attribute("name").value();
		auto str = std::string(n_rule.attribute("exclude").value());
		if (!str.empty())
			ur.exclude = str;
		for (auto n_i : n_rule.child("includes"))
		{
			UdtRule::Item item;
			auto t = std::string(n_i.attribute("type").value());
			item.type = t == "function" ? TypeFunction : TypeData;
			item.name = n_i.attribute("name").value();
			ur.includes.push_back(item);
		}
		for (auto n_e : n_rule.child("excludes"))
		{
			UdtRule::Item item;
			auto t = std::string(n_e.attribute("type").value());
			item.type = t == "function" ? TypeFunction : TypeData;
			item.name = n_e.attribute("name").value();
			ur.excludes.push_back(item);
		}
		udt_rules.push_back(ur);
	}

	auto typeinfo_path = executable_path;
	typeinfo_path.replace_extension(L".typeinfo");

	if (std::filesystem::exists(typeinfo_path))
	{
		auto lwt = std::filesystem::last_write_time(typeinfo_path);
		if (lwt > std::filesystem::last_write_time(pdb_path) && lwt > std::filesystem::last_write_time(desc_path))
		{
			printf("typeinfo up to date\n");
			return 0;
		}
	}

	printf("generating typeinfo for %s: ", executable_path.string().c_str());

	if (FAILED(CoInitialize(NULL)))
	{
		printf("com initial failed\n");
		fassert(0);
		return 0;
	}

	CComPtr<IDiaDataSource> dia_source;
	if (FAILED(CoCreateInstance(CLSID_DiaSource, NULL, CLSCTX_INPROC_SERVER, __uuidof(IDiaDataSource), (void**)&dia_source)))
	{
		printf("dia not found\n");
		fassert(0);
		return 0;
	}
	if (FAILED(dia_source->loadDataFromPdb(pdb_path.c_str())))
	{
		printf("pdb failed to open: %s\n", pdb_path.string().c_str());
		fassert(0);
		return 0;
	}
	CComPtr<IDiaSession> session;
	if (FAILED(dia_source->openSession(&session)))
	{
		printf("session failed to open\n");
		fassert(0);
		return 0;
	}
	CComPtr<IDiaSymbol> global;
	if (FAILED(session->get_globalScope(&global)))
	{
		printf("failed to get global\n");
		fassert(0);
		return 0;
	}

	BOOL b;
	LONG l;
	ULONG ul;
	ULONGLONG ull;
	DWORD dw;
	wchar_t* pwname;

	auto library = Library::load(executable_path.c_str(), false);
	auto library_address = library->get_address();
	auto db = TypeInfoDataBase::create();

	auto new_enum = [&](const std::string& name, IDiaSymbol* s_type) {
		if (find_enum(name.c_str(), db))
			return;

		std::vector<std::pair<std::string, int>> items;

		IDiaEnumSymbols* s_items;
		s_type->findChildren(SymTagNull, NULL, nsNone, &s_items);
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

		auto e = add_enum(name.c_str(), db);
		for (auto& i : items)
			e->add_item(i.first.c_str(), i.second);
	};

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
	}

	IDiaEnumSymbols* s_udts;
	global->findChildren(SymTagUDT, NULL, nsNone, &s_udts);
	IDiaSymbol* s_udt;
	while (SUCCEEDED(s_udts->Next(1, &s_udt, &ul)) && (ul == 1))
	{
		s_udt->get_name(&pwname);
		auto udt_name = w2s(pwname);

		for (auto& ur : udt_rules)
		{
			if (ur.pass(udt_name))
			{
				if (!find_udt(udt_name.c_str(), db))
				{
					s_udt->get_length(&ull);
					auto udt_size = ull;
					std::string base_name;

					IDiaEnumSymbols* s_base_classes;
					IDiaSymbol* s_base_class;
					s_udt->findChildren(SymTagBaseClass, NULL, nsNone, &s_base_classes);
					if (SUCCEEDED(s_base_classes->Next(1, &s_base_class, &ul)) && (ul == 1))
					{
						s_base_class->get_name(&pwname);
						base_name = w2s(pwname);
					}

					auto u = add_udt(udt_name.c_str(), udt_size, base_name.c_str(), db);

					DWORD ctor = 0;
					DWORD dtor = 0;

					pugi::xml_node n_functions;

					IDiaEnumSymbols* s_functions;
					s_udt->findChildren(SymTagFunction, NULL, nsNone, &s_functions);
					IDiaSymbol* s_function;
					while (SUCCEEDED(s_functions->Next(1, &s_function, &ul)) && (ul == 1))
					{
						s_function->get_name(&pwname);
						auto name = w2s(pwname);
						if (name.size() <= udt_name.size() && udt_name.compare(udt_name.size() - name.size(), name.size(), name) == 0)
							name = "ctor";
						else if (name[0] == '~')
							name = "dtor";

						if (ur.pass_item(TypeFunction, name))
						{
							s_function->get_relativeVirtualAddress(&dw);
							auto rva = dw;

							s_function->get_virtualBaseOffset(&dw);
							auto voff = dw;

							if (rva || voff != 0)
							{
								IDiaSymbol* s_function_type;
								s_function->get_type(&s_function_type);

								IDiaSymbol* s_return_type;
								s_function_type->get_type(&s_return_type);
								auto type_desc = typeinfo_from_symbol(s_return_type);
								s_return_type->Release();

								auto fi = u->add_function(name.c_str(), rva, voff, TypeInfo::get(type_desc.tag, type_desc.name.c_str(), db));

								IDiaEnumSymbols* s_parameters;
								s_function_type->findChildren(SymTagFunctionArgType, NULL, nsNone, &s_parameters);
								IDiaSymbol* s_parameter;
								while (SUCCEEDED(s_parameters->Next(1, &s_parameter, &ul)) && (ul == 1))
								{
									IDiaSymbol* s_type;
									s_parameter->get_type(&s_type);

									auto type_desc = typeinfo_from_symbol(s_parameter);
									if (type_desc.tag == TypeEnumSingle || type_desc.tag == TypeEnumMulti)
										new_enum(type_desc.name, s_type);

									fi->add_parameter(TypeInfo::get(type_desc.tag, type_desc.name.c_str(), db));

									s_type->Release();

									s_parameter->Release();
								}
								s_parameters->Release();

								s_function_type->Release();

								if (name == "ctor" && fi->get_parameters_count() == 0)
									ctor = rva;
								else if (name == "dtor")
									dtor = rva;
							}
						}
						s_function->Release();
					}
					s_functions->Release();

					auto obj = malloc(udt_size);
					memset(obj, 0, udt_size);
					if (ctor)
						a2f<void(*)(void*)>((char*)library_address + ctor)(obj);

					pugi::xml_node n_variables;

					IDiaEnumSymbols* s_variables;
					s_udt->findChildren(SymTagData, NULL, nsNone, &s_variables);
					auto is_virtual = false;
					if (SUCCEEDED(s_udt->get_virtualTableShapeId(&dw)))
						is_virtual = true;
					IDiaSymbol* s_variable;
					while (SUCCEEDED(s_variables->Next(1, &s_variable, &ul)) && (ul == 1))
					{
						s_variable->get_name(&pwname);
						auto name = w2s(pwname);
						if (ur.pass_item(TypeData, name))
						{
							IDiaSymbol* s_type;
							s_variable->get_type(&s_type);

							s_variable->get_offset(&l);
							auto offset = l;

							if (is_virtual && offset != 0)
							{
								auto type_desc = typeinfo_from_symbol(s_type);
								if (type_desc.name.starts_with("flame::"))
									type_desc.name = SUS::cut_tail_if(type_desc.name, "Private");
								if (type_desc.tag == TypeEnumSingle || type_desc.tag == TypeEnumMulti)
									new_enum(type_desc.name, s_type);
								auto type = TypeInfo::get(type_desc.tag, type_desc.name.c_str(), db);
								std::string default_value;

								if (type_desc.tag != TypePointer)
								{
									if (type)
									{
										type->serialize((char*)obj + offset, &default_value, [](void* _str, uint size) {
											auto& str = *(std::string*)_str;
											str.resize(size);
											return str.data();
										});
									}
								}

								u->add_variable(type, name.c_str(), offset,
									1, 0, default_value.c_str(), "");
							}

							s_type->Release();
						}
						s_variable->Release();
					}
					s_variables->Release();

					if (dtor)
						a2f<void(*)(void*)>((char*)library_address + dtor)(obj);
					free(obj);
				}

				break;
			}
		}
		s_udt->Release();
	}
	s_udts->Release();

	save_typeinfo(typeinfo_path.c_str(), db);

	library->release();
	db->release();

	printf(" - done\n");

	return 0;
}
