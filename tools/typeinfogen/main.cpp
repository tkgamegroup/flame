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
		static FLAME_SAL(str_unsigned, "unsigned ");
		auto pos = str.find(str_unsigned.s, 0, str_unsigned.l);
		while (pos != std::string::npos)
		{
			str = str.replace(pos, str_unsigned.l, "u");
			pos = str.find(str_unsigned.s, 0, str_unsigned.l);
		}
	}
	{
		static FLAME_SAL(str_int64, "__int64");
		auto pos = str.find(str_int64.s, 0, str_int64.l);
		while (pos != std::string::npos)
		{
			str = str.replace(pos, str_int64.l, "longlong");
			pos = str.find(str_int64.s, 0, str_int64.l);
		}
	}
	{
		static FLAME_SAL(str_enum, "enum ");
		auto pos = str.find(str_enum.s, 0, str_enum.l);
		while (pos != std::string::npos)
		{
			str = str.replace(pos, str_enum.l, "");
			pos = str.find(str_enum.s, 0, str_enum.l);
		}
	}
	{
		static FLAME_SAL(str_stringa, "flame::String<char>");
		auto pos = str.find(str_stringa.s, 0, str_stringa.l);
		while (pos != std::string::npos)
		{
			str = str.replace(pos, str_stringa.l, "flame::StringA");
			pos = str.find(str_stringa.s, 0, str_stringa.l);
		}
	}
	{
		static FLAME_SAL(str_stringw, "flame::String<wchar_t>");
		auto pos = str.find(str_stringw.s, 0, str_stringw.l);
		while (pos != std::string::npos)
		{
			str = str.replace(pos, str_stringw.l, "flame::StringW");
			pos = str.find(str_stringw.s, 0, str_stringw.l);
		}
	}
	{
		FLAME_SAL(array_str, "flame::Array");
		if (str.compare(0, array_str.l, array_str.s) == 0 && str.size() > array_str.l + 1)
		{
			if (is_array)
				*is_array = true;
			str.erase(str.begin(), str.begin() + array_str.l + 1);
			str.erase(str.end() - 1);
		}
	}
	{
		FLAME_SAL(listenerhub_str, "flame::ListenerHub");
		if (str.compare(0, listenerhub_str.l, listenerhub_str.s) == 0)
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

	return tn_c2a(str);
}

struct TypeInfoDesc
{
	TypeTag tag;
	bool is_array;
	std::string base_name;

	TypeInfoDesc()
	{
	}

	TypeInfoDesc(TypeTag tag, const std::string& base_name, bool is_array = false) :
		tag(tag),
		is_array(is_array),
		base_name(base_name)
	{
	}

	TypeInfo* get()
	{
		return TypeInfo::get(tag, base_name.c_str(), is_array);
	}
};

TypeInfoDesc typeinfo_from_symbol(IDiaSymbol* s_type, uint flags)
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
				name += "longlong";
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
		return TypeInfoDesc((flags & VariableFlagEnumMulti) ? TypeEnumMulti : TypeEnumSingle, format_type(pwname, nullptr));
	case SymTagBaseType:
		return TypeInfoDesc(TypeData, base_type_name(s_type));
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
		return TypeInfoDesc(TypePointer, name, is_array);
	}
	case SymTagUDT:
	{
		s_type->get_name(&pwname);
		auto is_array = false;
		auto name = format_type(pwname, &is_array);
		return TypeInfoDesc(TypeData, name, is_array);
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
	std::filesystem::path module_path(args[1]);
	if (!std::filesystem::exists(module_path))
	{
		printf("typeinfogen: module does not exist: %s\n", module_path.string().c_str());
		return 0;
	}

	std::vector<std::filesystem::path> dependencies;
	auto pdb_path = module_path;
	pdb_path.replace_extension(L".pdb");

	auto arr_dep = get_module_dependencies(module_path.c_str());
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
	printf("generating typeinfo for %s: ", module_path.string().c_str());

	auto last_curr_path = get_curr_path();
	set_curr_path(get_app_path().v);
	for (auto& d : dependencies)
		TypeInfoDatabase::load(d.c_str(), true, false);
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

	auto db = new TypeInfoDatabase(module_path);
	global_typeinfo_databases.push_back(db);

	LONG l;
	ULONG ul;
	ULONGLONG ull;
	DWORD dw;
	wchar_t* pwname;

	std::filesystem::path include_root;
	std::filesystem::path source_root;
	{
		DWORD h;
		auto version_size = GetFileVersionInfoSizeW(module_path.c_str(), &h);
		if (version_size > 0)
		{
			auto version_data = new char[version_size];
			if (GetFileVersionInfoW(module_path.c_str(), h, version_size, version_data))
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

	struct DesiredVariable
	{
		std::string name;
		uint flags;
	};
	struct DesiredFunction
	{
		std::string name;
		std::string code;
	};
	struct DesiredUDT
	{
		std::string name;
		std::string full_name;
		std::string base_name;
		std::vector<DesiredVariable> variables;
		std::vector<DesiredFunction> functions;
	};
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

	IDiaEnumSymbols* _udts;
	global->findChildren(SymTagUDT, NULL, nsNone, &_udts);
	IDiaSymbol* _udt;
	while (SUCCEEDED(_udts->Next(1, &_udt, &ul)) && (ul == 1))
	{
		_udt->get_name(&pwname);
		auto name = w2s(pwname);

		for (auto& du : desired_udts)
		{
			if (du.full_name == name)
			{
				if (!find_udt(FLAME_HASH(name.c_str())))
				{
					_udt->get_length(&ull);
					auto u = db->add_udt(name, ull, du.base_name);

					IDiaEnumSymbols* _variables;
					_udt->findChildren(SymTagData, NULL, nsNone, &_variables);
					IDiaSymbol* _variable;
					while (SUCCEEDED(_variables->Next(1, &_variable, &ul)) && (ul == 1))
					{
						_variable->get_name(&pwname);
						auto name = w2s(pwname);
						for (auto& dv : du.variables)
						{
							if (dv.name == name)
							{
								IDiaSymbol* s_type;
								_variable->get_type(&s_type);

								_variable->get_offset(&l);
								s_type->get_length(&ull);

								auto desc = typeinfo_from_symbol(s_type, dv.flags);
								if (desc.tag == TypeEnumSingle || desc.tag == TypeEnumMulti)
								{
									if (!find_enum(FLAME_HASH(desc.base_name.c_str())))
									{
										auto e = db->add_enum(desc.base_name);

										IDiaEnumSymbols* items;
										s_type->findChildren(SymTagNull, NULL, nsNone, &items);
										IDiaSymbol* item;
										while (SUCCEEDED(items->Next(1, &item, &ul)) && (ul == 1))
										{
											VARIANT variant;
											ZeroMemory(&variant, sizeof(variant));
											item->get_name(&pwname);
											item->get_value(&variant);

											auto item_name = w2s(pwname);
											if (!SUS::ends_with(item_name, "_Max") && !SUS::ends_with(item_name, "_Count"))
												e->add_item(item_name, variant.lVal);

											item->Release();
										}
										items->Release();
									}
								}
								u->add_variable(desc.get(), name, dv.flags, l, ull);

								s_type->Release();

								break;
							}
						}
						_variable->Release();
					}
					_variables->Release();

					IDiaEnumSymbols* _functions;
					_udt->findChildren(SymTagFunction, NULL, nsNone, &_functions);
					IDiaSymbol* _function;
					while (SUCCEEDED(_functions->Next(1, &_function, &ul)) && (ul == 1))
					{
						_function->get_name(&pwname);
						auto name = w2s(pwname);
						for (auto& df : du.functions)
						{
							if (df.name == name)
							{
								if (name == du.name)
									name = "ctor";
								else if (name[0] == '~')
									name = "dtor";

								void* rva;
								TypeInfoDesc ret_type;

								_function->get_relativeVirtualAddress(&dw);
								rva = (void*)dw;
								if (rva)
								{
									IDiaSymbol* s_function_type;
									_function->get_type(&s_function_type);

									IDiaSymbol* s_return_type;
									s_function_type->get_type(&s_return_type);
									ret_type = typeinfo_from_symbol(s_return_type, 0);
									s_return_type->Release();

									auto f = u->add_function(name, rva, ret_type.get());

									IDiaEnumSymbols* s_parameters;
									s_function_type->findChildren(SymTagFunctionArgType, NULL, nsNone, &s_parameters);
									IDiaSymbol* s_parameter;
									while (SUCCEEDED(s_parameters->Next(1, &s_parameter, &ul)) && (ul == 1))
									{
										f->add_parameter(typeinfo_from_symbol(s_parameter, 0).get());

										s_parameter->Release();
									}
									s_parameters->Release();

									s_function_type->Release();

									f->code = df.code;
								}

								break;
							}
						}
						_function->Release();
					}
					_functions->Release();

					FunctionInfo* ctor = nullptr;
					FunctionInfo* dtor = nullptr;
					for (auto f : u->functions)
					{
						if (f->name == "ctor" && f->parameters.s == 0)
							ctor = f;
						else if (f->name == "dtor")
							dtor = f;
						if (ctor && dtor)
							break;
					}
					if (ctor)
					{
						auto library = LoadLibraryW(module_path.c_str());
						if (library)
						{
							auto obj = malloc(u->size);
							memset(obj, 0, u->size);

							cmf(p2f<MF_v_v>((char*)library + (uint)(ctor->rva)), obj);
							for (auto& i : u->variables)
							{
								if (!i->default_value)
									continue;
								auto type = i->type;
								auto tag = type->tag;
								if (!type->is_array && tag != TypePointer && !(i->flags & VariableFlagOutput))
									memcpy(i->default_value, (char*)obj + i->offset, i->size);
							}
							if (dtor)
								cmf(p2f<MF_v_v>((char*)library + (uint)(dtor->rva)), obj);

							free(obj);
							FreeLibrary(library);
						}
					}
				}

				break;
			}
		}
		_udt->Release();
	}
	_udts->Release();

	pugi::xml_document file;
	auto file_root = file.append_child("typeinfo");

	auto n_enums = file_root.append_child("enums");
	for (auto e : db->enums.get_all())
	{
		auto n_enum = n_enums.append_child("enum");
		n_enum.append_attribute("name").set_value(e->name.v);

		auto n_items = n_enum.append_child("items");
		for (auto i : e->items)
		{
			auto n_item = n_items.append_child("item");
			n_item.append_attribute("name").set_value(i->name.v);
			n_item.append_attribute("value").set_value(i->value);
		}
	}

	auto n_udts = file_root.append_child("udts");
	for (auto u : db->udts.get_all())
	{
		auto n_udt = n_udts.append_child("udt");
		n_udt.append_attribute("name").set_value(u->name.v);
		n_udt.append_attribute("size").set_value(u->size);
		n_udt.append_attribute("base_name").set_value(u->base_name.v);

		auto n_items = n_udt.append_child("variables");
		for (auto v : u->variables)
		{
			auto n_variable = n_items.append_child("variable");
			auto type = v->type;
			n_variable.append_attribute("type").set_value(type->name.v);
			n_variable.append_attribute("name").set_value(v->name.v);
			n_variable.append_attribute("flags").set_value(v->flags);
			n_variable.append_attribute("offset").set_value(v->offset);
			n_variable.append_attribute("size").set_value(v->size);
			if (v->default_value)
				n_variable.append_attribute("default_value").set_value(type->serialize(v->default_value).c_str());
		}

		auto n_functions = n_udt.append_child("functions");
		for (auto f : u->functions)
		{
			auto n_function = n_functions.append_child("function");
			n_function.append_attribute("name").set_value(f->name.v);
			n_function.append_attribute("rva").set_value((uint)f->rva);
			n_function.append_attribute("return_type").set_value(f->type->name.v);
			if (f->parameters.s != 0)
			{
				auto n_parameters = n_function.append_child("parameters");
				for (auto p : f->parameters)
					n_parameters.append_child("parameter").append_attribute("type").set_value(p->name.v);
			}
		}
	}

	global_typeinfo_databases.remove(global_typeinfo_databases.s - 1);

	auto typeinfo_path = module_path;
	typeinfo_path.replace_extension(L".typeinfo");
	file.save_file(typeinfo_path.string().c_str());

	auto typeinfo_code_path = module_path;
	typeinfo_code_path.replace_extension(L".typeinfo.code");
	std::ofstream typeinfo_code(typeinfo_code_path);
	for (auto u : db->udts.get_all())
	{
		for (auto f : u->functions)
		{
			if (f->code.s != 0)
			{
				typeinfo_code << "##" << u->name.str() << "::" << f->name.str() <<"\n";
				typeinfo_code << f->code.str();
			}
		}
	}
	typeinfo_code.close();

	delete db;

	printf(" - done\n");

	return 0;
}
