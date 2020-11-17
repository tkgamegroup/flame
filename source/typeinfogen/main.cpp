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
	std::filesystem::path library_path(args[1]);
	if (!std::filesystem::exists(library_path))
	{
		printf("typeinfogen: library does not exist: %s\n", library_path.string().c_str());
		return 0;
	}

	auto pdb_path = library_path;
	pdb_path.replace_extension(L".pdb");
	auto typeinfo_path = library_path;
	typeinfo_path.replace_extension(L".typeinfo");

	std::vector<std::filesystem::path> dirs;

	auto clear = false;
	for (auto i = 2; i < argc; i++)
	{
		auto arg = args[i];
		if (arg[0] == '-')
		{
			switch (arg[1])
			{
			case 'c':
				clear = true;
				break;
			case 'd':
				dirs.push_back(arg + 2);
				break;
			}
		}
	}

	if (clear)
	{
		if (std::filesystem::exists(pdb_path))
			std::filesystem::remove(pdb_path);
		return 0;
	}

	//if (std::filesystem::exists(typeinfo_path) && std::filesystem::last_write_time(typeinfo_path) >= std::filesystem::last_write_time(library_path))
	//{
	//	printf("typeinfo up to date\n");
	//	return 0;
	//}

	for (auto& d : dirs)
		d.make_preferred();

	printf("generating typeinfo for %s: ", library_path.string().c_str());

	std::unordered_map<std::string, uint> enums;
	std::unordered_map<std::string, uint> udts;
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

	LONG l;
	ULONG ul;
	ULONGLONG ull;
	DWORD dw;
	wchar_t* pwname;

	std::vector<std::filesystem::path> files;
	for (auto& d : dirs)
	{
		for (std::filesystem::recursive_directory_iterator end, it(d); it != end; it++)
		{
			if (!std::filesystem::is_directory(it->status()))
			{
				auto p = it->path();
				auto e = p.extension();
				if (e == L".h" || e == L".cpp")
					files.push_back(p);
			}
		}
	}

	struct DesiredEnum
	{
		std::string name;
		std::string full_name;
	};
	struct MemberTarget
	{
		bool include = true;
		std::string name;
		std::regex reg;
		std::vector<std::string> meta;
		std::string code;
	};
	struct DesiredUDT
	{
		bool all = true;
		std::string name;
		std::string full_name;
		std::string base_name;
		std::vector<MemberTarget> targets;

		const MemberTarget* find_target(const std::string& name) const
		{
			for (auto& t : targets)
			{
				if (!t.name.empty())
				{
					if (t.name == name)
						return &t;
				}
				else
				{
					if (std::regex_search(name, t.reg))
						return &t;
				}
			}
			return nullptr;
		}
	};
	std::vector<DesiredEnum> desired_enums;
	std::vector<DesiredUDT> desired_udts;

	for (auto& src : files)
	{
		std::vector<std::string> lines;
		std::vector<std::pair<int, std::string>> current_namespaces;

		std::ifstream file(src);
		while (!file.eof())
		{
			std::string line;
			std::getline(file, line);
			line = SUS::trim(line);
			if (!line.empty())
				lines.push_back(line);
		}

		auto braces_level = 0;
		for (auto i = 0; i < lines.size(); i++)
		{
			static std::regex reg_ns(R"(^namespace\s+(\w+))");
			static std::regex reg_R(R"(^(.*)//\s*R(.*)$)");
			static std::regex reg_E(R"(enum\s(\w+))");
			static std::regex reg_U(R"(struct\s+(\w+)(\s+:\s+(\w+))?)");
			static std::regex reg_V(R"([\w\*]+\s(\w+))");
			static std::regex reg_F(R"(\w+\s(\w+)\s*\(.*\))");
			std::smatch res;
			if (std::regex_search(lines[i], res, reg_ns))
				current_namespaces.emplace_back(braces_level, res[1].str());
			else if (std::regex_search(lines[i], res, reg_R))
			{
				auto str = res[1].str();
				auto meta = SUS::split(SUS::trim(res[2].str()));
				if (std::regex_search(str, res, reg_E))
				{
					DesiredEnum de;

					de.name = res[1].str();
					for (auto& ns : current_namespaces)
						de.full_name += ns.second + "::";
					de.full_name += de.name;

					desired_enums.push_back(de);
				}
				else if (std::regex_search(str, res, reg_U))
				{
					DesiredUDT du;

					du.name = res[1].str();
					for (auto& ns : current_namespaces)
						du.full_name += ns.second + "::";
					du.full_name += du.name;
					if (res[2].matched)
						du.base_name = res[3].str();

					for (auto& t : meta)
					{
						if (t == "~")
							du.all = false;
						else
						{
							MemberTarget mt;
							if (t[0] == '!')
							{
								t.erase(t.begin());
								mt.include = false;
							}
							auto pos = t.find('*');
							if (pos != std::string::npos)
								mt.reg = std::regex(t.substr(0, pos) + "\\w+" + t.substr(pos + 1));
							else
								mt.name = t;
							du.targets.push_back(mt);
						}
					}

					auto braces_level = 0;
					for (i = i + 1; i < lines.size(); i++)
					{
						if (std::regex_search(lines[i], res, reg_R))
						{
							auto str = res[1].str();
							auto meta = SUS::split(SUS::trim(res[2].str()));
							if (std::regex_search(str, res, reg_F))
							{
								MemberTarget ft;
								ft.name = res[1].str();
								ft.meta = meta;
								for (auto& t : meta)
								{
									if (t == "code")
									{
										std::vector<std::string> code_lines;
										auto braces_level = 0;
										for (i = i + 1; i < lines.size(); i++)
										{
											auto l = SUS::trim(lines[i]);
											code_lines.push_back(l);

											braces_level += std::count(l.begin(), l.end(), '{');
											braces_level -= std::count(l.begin(), l.end(), '}');
											if (braces_level == 0)
												break;
										}

										if (code_lines.size() >= 2 && code_lines.front() == "{" && code_lines.back() == "}")
										{
											code_lines.erase(code_lines.begin() + 0);
											code_lines.erase(code_lines.end() - 1);
										}
										ft.code = "\n";
										for (auto& l : code_lines)
											ft.code += "\t\t\t\t\t\t" + l + "\n";
										ft.code += "\t\t\t\t\t";
										break;
									}
								}

								du.targets.push_back(ft);
							}
							else if (std::regex_search(str, res, reg_V))
							{
								MemberTarget vt;
								vt.name = res[1].str();
								vt.meta = meta;
								du.targets.push_back(vt);
							}
						}
						else
						{
							braces_level += std::count(lines[i].begin(), lines[i].end(), '{');
							braces_level -= std::count(lines[i].begin(), lines[i].end(), '}');
							if (braces_level == 0)
								break;
						}
					}
					desired_udts.push_back(du);
				}
			}
			else
			{
				braces_level += std::count(lines[i].begin(), lines[i].end(), '{');
				braces_level -= std::count(lines[i].begin(), lines[i].end(), '}');
				if (!current_namespaces.empty() && braces_level == current_namespaces.back().first)
					current_namespaces.erase(current_namespaces.end() - 1);
			}
		}
		file.close();
	}

	auto library = LoadLibraryW(library_path.c_str());

	pugi::xml_document file;
	auto file_root = file.append_child("typeinfo");
	pugi::xml_node n_enums;
	pugi::xml_node n_udts;

	auto add_enum = [&](const std::string& name, IDiaSymbol* s_type) {
		if (!has_enum(name))
		{
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
		}
	};

	IDiaEnumSymbols* s_enums;
	global->findChildren(SymTagEnum, NULL, nsNone, &s_enums);
	IDiaSymbol* s_enum;
	while (SUCCEEDED(s_enums->Next(1, &s_enum, &ul)) && (ul == 1))
	{
		s_enum->get_name(&pwname);
		auto name = w2s(pwname);
		for (auto& de : desired_enums)
		{
			if (de.full_name == name)
			{
				add_enum(name, s_enum);

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
		auto name = w2s(pwname);

		for (auto& du : desired_udts)
		{
			if (du.full_name == name)
			{
				if (!has_udt(name))
				{
					udts.emplace(name, 0);

					s_udt->get_length(&ull);
					auto udt_size = ull;

					if (!n_udts)
						n_udts = file_root.append_child("udts");
					auto n_udt = n_udts.append_child("udt");
					n_udt.append_attribute("name").set_value(name.c_str());
					n_udt.append_attribute("size").set_value(du.all ? udt_size : 0);
					n_udt.append_attribute("base_name").set_value(du.base_name.c_str());

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
						if (name == du.name)
							name = "ctor";
						else if (name[0] == '~')
							name = "dtor";

						auto ft = du.find_target(name);
						if ((!ft && du.all) || (ft && ft->include))
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
								n_function.append_attribute("rva").set_value((du.all || ft) ? rva : 0);
								n_function.append_attribute("voff").set_value((du.all || ft) ? voff : 0);
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
										add_enum(desc.name, s_type);

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

								if (ft && !ft->code.empty())
									n_function.append_child("code").append_child(pugi::node_pcdata).set_value(ft->code.c_str());
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
					IDiaSymbol* s_variable;
					while (SUCCEEDED(s_variables->Next(1, &s_variable, &ul)) && (ul == 1))
					{
						s_variable->get_name(&pwname);
						auto name = w2s(pwname);
						auto vt = du.find_target(name);
						if ((!vt && du.all) || (vt && vt->include))
						{
							IDiaSymbol* s_type;
							s_variable->get_type(&s_type);

							s_variable->get_offset(&l);
							auto offset = l;
							s_type->get_length(&ull);

							auto desc = typeinfo_from_symbol(s_type);
							if (desc.tag == TypeEnumSingle || desc.tag == TypeEnumMulti)
								add_enum(desc.name, s_type);

							if (!n_variables)
								n_variables = n_udt.prepend_child("variables");
							auto n_variable = n_variables.append_child("variable");
							n_variable.append_attribute("type_tag").set_value(desc.tag);
							n_variable.append_attribute("type_name").set_value(desc.name.c_str());
							n_variable.append_attribute("name").set_value(name.c_str());
							n_variable.append_attribute("offset").set_value((du.all || vt) ? offset : 0);
							auto meta = std::string();
							if (vt)
							{
								for (auto& t : vt->meta)
									meta += t + " ";
								if (!meta.empty())
									meta.erase(meta.end() - 1);
							}
							n_variable.append_attribute("meta").set_value(meta.c_str());

							if (desc.tag != TypePointer)
							{
								auto type = TypeInfo::get(desc.tag, desc.name.c_str());
								if (type)
								{
									std::string str;
									type->serialize(&str, (char*)obj + offset);
									if (!str.empty())
										n_variable.append_attribute("default_value").set_value(str.c_str());
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
