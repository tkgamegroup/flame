#include <flame/serialize.h>
#include <flame/foundation/typeinfo.h>

#include <Windows.h>
#include <dia2.h>
#include <atlbase.h>

using namespace flame;

std::string format_type(const wchar_t* in, bool* is_array)
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
	{
		static auto reg = std::regex(R"(flame::String<char>)");
		str = std::regex_replace(str, reg, "flame::StringA");
	}
	{
		static auto reg = std::regex(R"(flame::String<wchar_t>)");
		str = std::regex_replace(str, reg, "flame::StringW");
	}
	{
		static auto array_str = std::string("flame::Array");
		if (str.compare(0, array_str.size(), array_str.c_str()) == 0 && str.size() > array_str.size() + 1)
		{
			if (is_array)
				*is_array = true;
			str.erase(str.begin(), str.begin() + array_str.size() + 1);
			str.erase(str.end() - 1);
		}
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
		auto name = format_type(pwname, nullptr);
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
		auto is_array = false;
		switch (dw)
		{
		case SymTagBaseType:
			name = base_type_name(pointer_type);
			break;
		case SymTagPointerType:
			fassert(0);
			break;
		case SymTagUDT:
			pointer_type->get_name(&pwname);
			name = format_type(pwname, &is_array);
			break;
		}
		pointer_type->Release();
		return TagAndName(is_array ? TypeArrayOfPointer : TypePointer, name);
	}
	case SymTagUDT:
	{
		s_type->get_name(&pwname);
		auto is_array = false;
		auto name = format_type(pwname, &is_array);
		return TagAndName(is_array ? TypeArrayOfData : TypeData, name);
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

	std::map<std::string, uint> enums;
	std::map<std::string, uint> udts;
	auto has_enum = [&](const std::string& n) {
		return enums.find(n) != enums.end();
	};
	auto has_udt = [&](const std::string& n) {
		return udts.find(n) != udts.end();
	};

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

	auto library = LoadLibraryW(executable_path.c_str());

	pugi::xml_document file;
	auto file_root = file.append_child("typeinfo");
	pugi::xml_node n_enums;
	pugi::xml_node n_udts;

	auto new_enum = [&](const std::string& name, IDiaSymbol* s_type) {
		if (has_enum(name))
			return;

		enums.emplace(name, 0);

		if (!n_enums)
			n_enums = file_root.append_child("enums");
		auto n_enum = n_enums.append_child("enum");
		n_enum.append_attribute("name").set_value(name.c_str());

		auto n_items = n_enum.append_child("items");

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

		for (auto& i : items)
		{
			auto n_item = n_items.append_child("item");
			n_item.append_attribute("name").set_value(i.first.c_str());
			n_item.append_attribute("value").set_value(i.second);
		}

		{
			std::vector<char*> names;
			std::vector<int> values;
			names.resize(items.size());
			values.resize(items.size());
			for (auto i = 0; i < items.size(); i++)
			{
				names[i] = (char*)items[i].first.c_str();
				values[i] = items[i].second;
			}
			add_enum(name.c_str(), items.size(), names.data(), values.data());
		}
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
				if (!has_udt(udt_name))
				{
					udts.emplace(udt_name, 0);

					s_udt->get_length(&ull);
					auto udt_size = ull;

					if (!n_udts)
						n_udts = file_root.append_child("udts");
					auto n_udt = n_udts.append_child("udt");
					n_udt.append_attribute("name").set_value(udt_name.c_str());
					n_udt.append_attribute("size").set_value(udt_size);

					IDiaEnumSymbols* s_base_classes;
					IDiaSymbol* s_base_class;
					s_udt->findChildren(SymTagBaseClass, NULL, nsNone, &s_base_classes);
					if (SUCCEEDED(s_base_classes->Next(1, &s_base_class, &ul)) && (ul == 1))
					{
						s_base_class->get_name(&pwname);
						n_udt.append_attribute("base_name").set_value(w2s(pwname).c_str());
					}

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
								auto ret_type = typeinfo_from_symbol(s_return_type);
								s_return_type->Release();

								if (!n_functions)
									n_functions = n_udt.append_child("functions");
								auto n_function = n_functions.append_child("function");
								n_function.append_attribute("name").set_value(name.c_str());
								n_function.append_attribute("rva").set_value(rva);
								n_function.append_attribute("voff").set_value(voff);
								n_function.append_attribute("type_tag").set_value(ret_type.tag);
								n_function.append_attribute("type_name").set_value(ret_type.name.c_str());

								pugi::xml_node n_parameters;

								IDiaEnumSymbols* s_parameters;
								s_function_type->findChildren(SymTagFunctionArgType, NULL, nsNone, &s_parameters);
								IDiaSymbol* s_parameter;
								while (SUCCEEDED(s_parameters->Next(1, &s_parameter, &ul)) && (ul == 1))
								{
									IDiaSymbol* s_type;
									s_parameter->get_type(&s_type);

									auto desc = typeinfo_from_symbol(s_parameter);
									if (desc.tag == TypeEnumSingle || desc.tag == TypeEnumMulti)
										new_enum(desc.name, s_type);

									if (!n_parameters)
										n_parameters = n_function.append_child("parameters");
									auto n_parameter = n_parameters.append_child("parameter");
									n_parameter.append_attribute("type_tag").set_value(desc.tag);
									n_parameter.append_attribute("type_name").set_value(desc.name.c_str());

									s_type->Release();

									s_parameter->Release();
								}
								s_parameters->Release();

								s_function_type->Release();

								if (name == "ctor" && !n_parameters)
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
						a2f<void(*)(void*)>((char*)library + ctor)(obj);

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
								auto desc = typeinfo_from_symbol(s_type);
								if (desc.name.starts_with("flame::"))
									desc.name = SUS::cut_tail_if(desc.name, "Private");
								if (desc.tag == TypeEnumSingle || desc.tag == TypeEnumMulti)
									new_enum(desc.name, s_type);

								if (!n_variables)
									n_variables = n_udt.prepend_child("variables");
								auto n_variable = n_variables.append_child("variable");
								n_variable.append_attribute("type_tag").set_value(desc.tag);
								n_variable.append_attribute("type_name").set_value(desc.name.c_str());
								n_variable.append_attribute("name").set_value(name.c_str());
								n_variable.append_attribute("offset").set_value(offset);

								if (desc.tag != TypePointer)
								{
									auto type = TypeInfo::get(desc.tag, desc.name.c_str());
									if (type)
									{
										std::string str;
										type->serialize((char*)obj + offset, &str, [](void* _str, uint size) {
											auto& str = *(std::string*)_str;
											str.resize(size);
											return str.data();
										});
										if (!str.empty())
											n_variable.append_attribute("default_value").set_value(str.c_str());
									}
								}
							}

							s_type->Release();
						}
						s_variable->Release();
					}
					s_variables->Release();

					if (dtor)
						a2f<void(*)(void*)>((char*)library + dtor)(obj);
					free(obj);
				}

				break;
			}
		}
		s_udt->Release();
	}
	s_udts->Release();

	file.save_file(typeinfo_path.string().c_str());

	FreeLibrary(library);

	printf(" - done\n");

	return 0;
}
