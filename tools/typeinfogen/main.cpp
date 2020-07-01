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
	{
		static auto listenerhub_str = std::string("flame::ListenerHub");
		if (str.compare(0, listenerhub_str.size(), listenerhub_str.c_str()) == 0)
			str = "ListenerHub";
	}

	std::string head;
	std::string tail;
	auto pos_t = str.find('<');
	if (pos_t != std::string::npos)
	{
		head = std::string(str.begin(), str.begin() + pos_t);
		tail = std::string(str.begin() + pos_t, str.end());
	}
	else
		head = str;

	str = head + tail;

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

TagAndName typeinfo_from_symbol(IDiaSymbol* s_type, uint flags)
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
			assert(0);
		}
	};

	s_type->get_symTag(&dw);
	switch (dw)
	{
	case SymTagEnum:
		s_type->get_name(&pwname);
		return TagAndName((flags & VariableFlagEnumMulti) ? TypeEnumMulti : TypeEnumSingle, format_type(pwname, nullptr));
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
			assert(0);
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
		auto ret = typeinfo_from_symbol(s_arg_type, 0);
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

	std::vector<std::filesystem::path> dependencies;
	auto pdb_path = library_path;
	pdb_path.replace_extension(L".pdb");

	auto arr_dep = get_library_dependencies(library_path.c_str());
	for (auto i = 0; i < arr_dep.s; i++)
	{
		auto d = std::filesystem::path(arr_dep[i].str());
		if (SUW::starts_with(d, L"flame_"))
		{
			d.replace_extension(L".typeinfo");
			if (std::filesystem::exists(d))
				dependencies.push_back(d);
		}
	}
	printf("generating typeinfo for %s: ", library_path.string().c_str());

	auto last_curr_path = get_curr_path();
	set_curr_path(get_app_path().v);
	for (auto& d : dependencies)
		TypeInfoDatabase::load(d.c_str());
	set_curr_path(last_curr_path.v);

	if (FAILED(CoInitialize(NULL)))
	{
		printf("com initial failed\n");
		assert(0);
		return 0;
	}

	CComPtr<IDiaDataSource> dia_source;
	if (FAILED(CoCreateInstance(CLSID_DiaSource, NULL, CLSCTX_INPROC_SERVER, __uuidof(IDiaDataSource), (void**)&dia_source)))
	{
		printf("dia not found\n");
		assert(0);
		return 0;
	}
	if (FAILED(dia_source->loadDataFromPdb(pdb_path.c_str())))
	{
		printf("pdb failed to open: %s\n", pdb_path.string().c_str());
		assert(0);
		return 0;
	}
	CComPtr<IDiaSession> session;
	if (FAILED(dia_source->openSession(&session)))
	{
		printf("session failed to open\n");
		assert(0);
		return 0;
	}
	CComPtr<IDiaSymbol> global;
	if (FAILED(session->get_globalScope(&global)))
	{
		printf("failed to get global\n");
		assert(0);
		return 0;
	}

	LONG l;
	ULONG ul;
	ULONGLONG ull;
	DWORD dw;
	wchar_t* pwname;

	std::filesystem::path include_root;
	std::filesystem::path source_root;
	{
		DWORD h;
		auto version_size = GetFileVersionInfoSizeW(library_path.c_str(), &h);
		if (version_size > 0)
		{
			auto version_data = new char[version_size];
			if (GetFileVersionInfoW(library_path.c_str(), h, version_size, version_data))
			{
				void* d; uint s;
				VerQueryValue(version_data, "\\StringFileInfo\\040904b0\\FileDescription", &d, &s);
				auto sp = SUS::split((char*)d, ';');
				include_root = sp[0];
				include_root.make_preferred();
				source_root = sp[1];
				source_root.make_preferred();
			}
			delete[] version_data;
		}
	}
	std::vector<std::filesystem::path> my_sources;
	IDiaEnumSourceFiles* _source_files;
	IDiaSourceFile* _source_file;
	session->findFile(nullptr, nullptr, 0, &_source_files);
	while (SUCCEEDED(_source_files->Next(1, &_source_file, &ul)) && (ul == 1))
	{
		_source_file->get_fileName(&pwname);
		auto fn = std::filesystem::path(pwname);
		auto ext = fn.extension();
		if (ext == L".h" || ext == L".cpp")
		{
			auto my_file = false;
			auto p = fn.parent_path();
			while (p.root_path() != p)
			{
				if (p == include_root || p == source_root)
				{
					my_file = true;
					break;
				}
				p = p.parent_path();
			}
			if (my_file)
				my_sources.push_back(fn);
		}
		_source_file->Release();
	}
	_source_files->Release();

	struct DesiredEnum
	{
		std::string name;
	};
	struct DesiredFunction
	{
		std::string name;
		std::string code;
	};
	struct DesiredVariable
	{
		std::string name;
		uint flags;
	};
	struct DesiredUDT
	{
		std::string name;
		std::string full_name;
		std::string base_name;
		std::vector<DesiredVariable> variables;
		std::vector<DesiredFunction> functions;
	};
	std::vector<DesiredEnum> desired_enums;
	std::vector<DesiredUDT> desired_udts;

	for (auto& src : my_sources)
	{
		std::vector<std::string> lines;
		std::vector<std::pair<int, std::string>> current_namespaces;

		std::ifstream file(src);
		while (!file.eof())
		{
			std::string line;
			std::getline(file, line);
			SUS::trim(line);
			if (!line.empty())
				lines.push_back(line);
		}

		auto braces_level = 0;
		for (auto i = 0; i < lines.size(); i++)
		{
			static std::regex reg_ns(R"(^namespace\s+(\w+))");
			static std::regex reg_R(R"(FLAME_R\((.*)\))");
			static std::regex reg_RV(R"(FLAME_RV\((.*)\))");
			static std::regex reg_RF(R"(FLAME_RF\(([\~\w]+)\))");
			std::smatch res;
			if (std::regex_search(lines[i], res, reg_ns))
				current_namespaces.emplace_back(braces_level, res[1].str());
			else if (std::regex_search(lines[i], res, reg_R))
			{
				DesiredUDT du;

				auto str = res[1].str();
				SUS::remove_spaces(str);
				auto sp = SUS::split(str, ',');
				if (sp.size() > 0)
				{
					auto _sp = SUS::split(sp[0], ':');
					du.name = _sp[0];
					for (auto& ns : current_namespaces)
						du.full_name += ns.second + "::";
					du.full_name += du.name;
					if (_sp.size() > 1)
						du.base_name = _sp[1];
				}
				if (sp.size() > 1)
				{
					auto _sp = SUS::split(sp[1], ':');
					if (_sp.size() == 2 && _sp[0] == "d")
						;
				}

				auto braces_level = 0;
				for (i = i + 1; i < lines.size(); i++)
				{
					if (std::regex_search(lines[i], res, reg_RV))
					{
						auto str = res[1].str();
						SUS::remove_spaces(str);
						auto sp = SUS::split(str, ',');
						DesiredVariable dv;
						dv.flags = 0;
						dv.name = sp[1];
						for (auto i = 2; i < sp.size(); i++)
						{
							if (sp[i] == "i")
								dv.flags |= VariableFlagInput;
							else if (sp[i] == "o")
								dv.flags |= VariableFlagOutput;
							else if (sp[i] == "m")
								dv.flags |= VariableFlagEnumMulti;
						}
						du.variables.push_back(dv);
					}
					else if (std::regex_search(lines[i], res, reg_RF))
					{
						auto str = res[1].str();
						SUS::remove_spaces(str);
						DesiredFunction df;
						df.name = str;

						if (df.name == "bp_update")
						{
							std::vector<std::string> code_lines;
							auto braces_level = 0;
							for (i = i + 1; i < lines.size(); i++)
							{
								auto l = lines[i];
								SUS::trim(l);
								code_lines.push_back(l);

								braces_level += std::count(lines[i].begin(), lines[i].end(), '{');
								braces_level -= std::count(lines[i].begin(), lines[i].end(), '}');
								if (braces_level == 0)
									break;
							}

							if (code_lines.size() >= 2 && code_lines.front() == "{" && code_lines.back() == "}")
							{
								code_lines.erase(code_lines.begin() + 0);
								code_lines.erase(code_lines.end() - 1);
							}
							for (auto& l : code_lines)
								df.code += l + "\n";
						}

						du.functions.push_back(df);
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

	std::unordered_map<uint, uint> saved_enums;
	std::unordered_map<uint, uint> saved_udts;

	auto has_enum = [&](uint h) {
		if (!find_enum(h))
			return false;
		return saved_enums.find(h) != saved_enums.end();
	};
	auto has_udt = [&](uint h) {
		if (!find_udt(h))
			return false;
		return saved_udts.find(h) != saved_udts.end();
	};

	auto library = LoadLibraryW(library_path.c_str());

	auto typeinfo_path = library_path;
	typeinfo_path.replace_extension(L".typeinfo");
	pugi::xml_document file;
	auto file_root = file.append_child("typeinfo");
	pugi::xml_node n_enums;
	pugi::xml_node n_udts;

	auto typeinfo_code_path = library_path;
	typeinfo_code_path.replace_extension(L".typeinfo.code");
	std::ofstream typeinfo_code(typeinfo_code_path);

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
				auto udt_hash = FLAME_HASH(name.c_str());
				if (!has_udt(udt_hash))
				{
					s_udt->get_length(&ull);
					auto udt_size = ull;

					if (!n_udts)
						n_udts = file_root.append_child("udts");
					auto n_udt = n_udts.append_child("udt");
					n_udt.append_attribute("name").set_value(name.c_str());
					n_udt.append_attribute("size").set_value(udt_size);
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
						for (auto& df : du.functions)
						{
							if (df.name == name)
							{
								if (name == du.name)
									name = "ctor";
								else if (name[0] == '~')
									name = "dtor";

								s_function->get_relativeVirtualAddress(&dw);
								auto rva = dw;
								if (rva)
								{
									IDiaSymbol* s_function_type;
									s_function->get_type(&s_function_type);

									IDiaSymbol* s_return_type;
									s_function_type->get_type(&s_return_type);
									auto ret_type = typeinfo_from_symbol(s_return_type, 0);
									s_return_type->Release();

									if (!n_functions)
										n_functions = n_udt.append_child("functions");
									auto n_function = n_functions.append_child("function");
									n_function.append_attribute("name").set_value(name.c_str());
									n_function.append_attribute("rva").set_value(rva);
									n_function.append_attribute("type_tag").set_value(ret_type.tag);
									n_function.append_attribute("type_name").set_value(ret_type.name.c_str());

									pugi::xml_node n_parameters;

									IDiaEnumSymbols* s_parameters;
									s_function_type->findChildren(SymTagFunctionArgType, NULL, nsNone, &s_parameters);
									IDiaSymbol* s_parameter;
									while (SUCCEEDED(s_parameters->Next(1, &s_parameter, &ul)) && (ul == 1))
									{
										auto desc = typeinfo_from_symbol(s_parameter, 0);

										if (!n_parameters)
											n_parameters = n_udt.append_child("functions");
										auto n_parameter = n_parameters.append_child("parameter");
										n_parameter.append_attribute("type_tag").set_value(desc.tag);
										n_parameter.append_attribute("type_name").set_value(desc.name.c_str());

										s_parameter->Release();
									}
									s_parameters->Release();

									s_function_type->Release();

									if (name == "ctor" && !n_parameters)
										ctor = rva;
									else if (name == "dtor")
										dtor = rva;

									if (!df.code.empty())
									{
										typeinfo_code << "##" << du.full_name << "::" << name << "\n";
										typeinfo_code << df.code;
									}
								}

								break;
							}
						}
						s_function->Release();
					}
					s_functions->Release();

					auto obj = malloc(udt_size);
					memset(obj, 0, udt_size);

					if (ctor)
						cmf(p2f<MF_v_v>((char*)library + ctor), obj);

					pugi::xml_node n_variables;

					IDiaEnumSymbols* s_variables;
					s_udt->findChildren(SymTagData, NULL, nsNone, &s_variables);
					IDiaSymbol* s_variable;
					while (SUCCEEDED(s_variables->Next(1, &s_variable, &ul)) && (ul == 1))
					{
						s_variable->get_name(&pwname);
						auto name = w2s(pwname);
						for (auto& dv : du.variables)
						{
							if (dv.name == name)
							{
								IDiaSymbol* s_type;
								s_variable->get_type(&s_type);

								s_variable->get_offset(&l);
								auto offset = l;
								s_type->get_length(&ull);

								auto desc = typeinfo_from_symbol(s_type, dv.flags);
								if (desc.tag == TypeEnumSingle || desc.tag == TypeEnumMulti)
								{
									auto enum_hash = FLAME_HASH(desc.name.c_str());
									if (!has_enum(enum_hash))
									{
										if (!n_enums)
											n_enums = file_root.append_child("enums");
										auto n_enum = n_enums.append_child("enum");
										n_enum.append_attribute("name").set_value(desc.name.c_str());

										auto n_items = n_enum.append_child("items");

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
											if (!SUS::ends_with(item_name, "_Max") && !SUS::ends_with(item_name, "_Count"))
											{
												auto n_item = n_items.append_child("item");
												n_item.append_attribute("name").set_value(item_name.c_str());
												n_item.append_attribute("value").set_value(variant.lVal);
											}

											s_item->Release();
										}
										s_items->Release();
									}
								}

								if (!n_variables)
									n_variables = n_udt.prepend_child("variables");
								auto n_variable = n_variables.append_child("variable");
								n_variable.append_attribute("type_tag").set_value(desc.tag);
								n_variable.append_attribute("type_name").set_value(desc.name.c_str());
								n_variable.append_attribute("name").set_value(name.c_str());
								n_variable.append_attribute("flags").set_value(dv.flags);
								n_variable.append_attribute("offset").set_value(offset);

								if ((desc.tag == TypeEnumSingle || desc.tag == TypeEnumMulti || desc.tag == TypeData) &&
									desc.name != "flame::StringA" && desc.name != "flame::StringW" && !(dv.flags & VariableFlagOutput))
								{
									auto type = TypeInfo::get(desc.tag, desc.name.c_str());
									n_variable.append_attribute("default_value").set_value(type->serialize_s((char*)obj + offset).c_str());
								}

								s_type->Release();

								break;
							}
						}
						s_variable->Release();
					}
					s_variables->Release();

					if (dtor)
						cmf(p2f<MF_v_v>((char*)library + dtor), obj);
					free(obj);
				}

				break;
			}
		}
		s_udt->Release();
	}
	s_udts->Release();

	file.save_file(typeinfo_path.string().c_str());

	typeinfo_code.close();

	FreeLibrary(library);

	printf(" - done\n");

	return 0;
}
