#include <flame/xml.h>
#include <flame/foundation/typeinfo.h>
#include <flame/foundation/system.h>

#include <Windows.h>
#include <dia2.h>
#include <atlbase.h>
#include <chrono>

using namespace flame;

TypeInfoDataBase db;

TypeTag parse_vector(std::string& name)
{
	static std::regex reg1("std::vector<([\\w\\s:\\*<>,]+),std::allocator<");
	static std::regex reg2("std::vector<std::unique_ptr<([\\w:]+)");
	std::smatch res;
	auto ok = false;
	auto is_pointer = false;
	if (std::regex_search(name, res, reg2))
	{
		ok = true;
		is_pointer = true;
	}
	else if (std::regex_search(name, res, reg1))
		ok = true;
	if (ok)
	{
		name = res[1].str();
		auto is_enum = SUS::strip_head_if(name, "enum ");
		SUS::strip_head_if(name, "struct ");
		SUS::strip_head_if(name, "class ");
		name = TypeInfo::format_name(name);
		if (is_enum)
			return TagVE;
		if (!is_pointer)
			is_pointer = SUS::strip_tail_if(name, "*") || SUS::strip_tail_if(name, "*__ptr64");
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
		return TypeInfo::get(TagD, "__unsupported__symtag_" + str(dw));
	}
}

void* load_exe_as_dll(const std::filesystem::path& path);

int main(int argc, char **args)
{
	if (argc != 2)
		goto show_usage;

	goto process;

show_usage:
	printf("usage: typeinfogen <target>\n");
	return 0;

process:
	std::filesystem::path input_path = args[1];

	printf("typeinfogen: %s\n", input_path.string().c_str());
	if (!std::filesystem::exists(input_path))
	{
		printf("target does not exist: %s\n", input_path.string().c_str());
		return 0;
	}

	for (auto& path : get_module_dependencies(input_path))
	{
		auto ti_path = path;
		ti_path.replace_extension(".typeinfo");
		if (std::filesystem::exists(ti_path) && input_path != path)
			tidb.load(ti_path);
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
		if (lwt > std::filesystem::last_write_time(pdb_path))
		{
			printf("typeinfogen: %s up to date\n", typeinfo_path.string().c_str());
			return 0;
		}
	}

	struct Rule
	{
		enum Type
		{
			Equal,
			StartsWith,
			EndsWith,
			Contains,
			Any,
			Excepts
		};

		Type type = Equal;
		std::string name;
		Type children_type = Any;
		std::vector<std::pair<std::string, Metas>> children;

		bool pass(const std::string& str)
		{
			switch (type)
			{
			case Equal:
				return name == str;
			case StartsWith:
				return str.starts_with(name);
			case EndsWith:
				return str.ends_with(name);
			case Contains:
				return str.contains(name);
			}
			return false;
		}

		bool pass_child(const std::string& str, Metas* metas = nullptr)
		{
			switch (children_type)
			{
			case Equal:
				for (auto& s : children)
				{
					if (s.first == str)
					{
						if (metas)
							*metas = s.second;
						return true;
					}
				}
				break;
			case Any:
				return true;
			case Excepts:
				for (auto& s : children)
				{
					if (s.first == str)
						return false;
				}
				return true;
			}
			return false;
		}
	};

	std::vector<Rule> enum_rules;
	std::vector<Rule> udt_rules;
	auto need = [&](std::vector<Rule>& rules, const std::string& name, Rule** out_r) {
		for (auto& r : rules)
		{
			if (r.pass(name))
			{
				if (out_r)
					*out_r = &r;
				return true;
			}
		}
		return false;
	};
	auto need_enum = [&](const std::string& name, Rule** out_r = nullptr) {
		return need(enum_rules, name, out_r);
	};
	auto need_udt = [&](const std::string& name, Rule** out_r = nullptr) {
		return need(udt_rules, name, out_r);
	};
	auto add_enum_rule = [&](const std::string& n) {
		auto& r = enum_rules.emplace_back();
		r.name = n;
	};
	auto add_udt_rule = [&](const std::string& n) {
		auto& r = udt_rules.emplace_back();
		r.name = n;
	};

	auto desc_path = input_path;
	desc_path.replace_extension(L".typedesc");
	if (std::filesystem::exists(desc_path))
	{
		std::filesystem::path source_path;

		std::ifstream desc(desc_path);
		std::string line;
		std::getline(desc, line);
		source_path = line;
		while (!desc.eof())
		{
			std::getline(desc, line);
			auto sp = SUS::split(line);
			auto read_rule = [&](Rule& r) {
				if (sp[1] == "equal")
					r.type = Rule::Equal;
				else if (sp[1] == "starts_with")
					r.type = Rule::StartsWith;
				else if (sp[1] == "ends_with")
					r.type = Rule::EndsWith;
				else if (sp[1] == "contains")
					r.type = Rule::Contains;
				r.name = sp[2];
			};
			if (sp[0] == "enum")
			{
				auto& r = enum_rules.emplace_back();
				read_rule(r);
			}
			else if (sp[0] == "udt")
			{
				auto& r = udt_rules.emplace_back();
				read_rule(r);
			}
		}
		desc.close();

		if (std::filesystem::exists(source_path))
		{
			std::string line;
			std::smatch res;
			for (auto& it : std::filesystem::recursive_directory_iterator(source_path))
			{
				if (it.path().extension() == L".h" && 
					!it.path().filename().wstring().ends_with(L"_private"))
				{
					Rule* pr = nullptr;
					std::ifstream file(it.path());
					while (!file.eof())
					{
						std::getline(file, line);
						SUS::ltrim(line);
						if (line.starts_with("///"))
						{
							line = line.substr(3);
							SUS::ltrim(line);
							if (SUS::strip_head_if(line, "Reflect"))
							{
								auto metas = line;

								auto need_ctor = false;

								auto sp = SUS::split(line);
								for (auto& t : sp)
								{
									if (t == "ctor")
										need_ctor = true;
								}

								std::getline(file, line);
								SUS::ltrim(line);
								if (SUS::strip_head_if(line, "enum "))
								{
									auto& r = enum_rules.emplace_back();
									r.type = Rule::EndsWith;
									r.name = line;
									pr = &r;
								}
								else if (SUS::strip_head_if(line, "struct "))
								{
									if (auto pos = line.find_last_of(':'); pos != std::string::npos)
									{
										line.erase(line.begin() + pos, line.end());
										SUS::rtrim(line);
									}
									auto& r = udt_rules.emplace_back();
									r.type = Rule::EndsWith;
									r.name = line;
									r.children_type = Rule::Equal;
									if (need_ctor)
									{
										auto& c = r.children.emplace_back();
										c.first = "ctor";
									}
									pr = &r;
								}
								else if (pr)
								{
									static std::regex reg_var("[\\w*&>]\\s+(\\w+)\\s*(=.*)?;");
									static std::regex reg_fun("\\s+(\\w+)\\s*\\(");
									std::string name;
									if (std::regex_search(line, res, reg_var))
										name = res[1].str();
									else if (std::regex_search(line, res, reg_fun))
										name = res[1].str();
									if (!name.empty())
									{
										auto& c = pr->children.emplace_back();
										c.first = res[1].str();
										c.second.from_string(metas);
									}
								}
							}
						}
					}
					file.close();
				}
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

	auto library = load_library(input_path);

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
				if (SUS::strip_head_if(name, "enum "))
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
								add_udt_rule(name);
							else if (tag == TagVU)
								add_udt_rule(name);
						}
						else
							add_udt_rule(name);
					}
				}
			}
		}
		s_function->Release();
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
			if (!find_enum(sh(enum_name.c_str()), db) && need_enum(enum_name))
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
						auto sh = items[0].first[0];
						for (auto i = 1; i < items.size(); i++)
						{
							if (items[i].first[0] != sh)
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

				db.enums.emplace(sh(e.name.c_str()), e);
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
			Rule* ur;
			if (!find_udt(sh(udt_name.c_str()), db) && need_udt(udt_name, &ur))
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

				auto get_return_type = [&](IDiaSymbol* s_function_type) {
					TypeInfo* ret;
					IDiaSymbol* s_return_type;
					s_function_type->get_type(&s_return_type);
					ret = typeinfo_from_symbol(s_return_type);
					reference_type(ret);
					s_return_type->Release();
					return ret;
				};

				auto get_parameters = [&](IDiaSymbol* s_function_type) {
					std::vector<TypeInfo*> ret;

					IDiaEnumSymbols* s_parameters;
					s_function_type->findChildren(SymTagFunctionArgType, NULL, nsNone, &s_parameters);
					IDiaSymbol* s_parameter;
					while (SUCCEEDED(s_parameters->Next(1, &s_parameter, &ul)) && (ul == 1))
					{
						IDiaSymbol* s_type;
						s_parameter->get_type(&s_type);
						auto ti = typeinfo_from_symbol(s_parameter);
						ret.push_back(ti);
						reference_type(ti);
						s_type->Release();

						s_parameter->Release();
					}
					s_parameters->Release();

					return ret;
				};

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

					Metas metas;
					if (ur->pass_child(name, &metas))
					{
						auto& fi = u.functions.emplace_back();
						fi.name = name;
						fi.rva = rva;
						fi.voff = voff;
						fi.metas = metas;
						fi.library = library;

						IDiaSymbol* s_function_type;
						s_function->get_type(&s_function_type);

						fi.return_type = get_return_type(s_function_type);

						IDiaSymbol6* s6_function = (IDiaSymbol6*)s_function;
						fi.is_static = false;
						if (s6_function->get_isStaticMemberFunc(&b) == S_OK)
							fi.is_static = b;

						fi.parameters = get_parameters(s_function_type);

						s_function_type->Release();

						if (name == "ctor" && fi.parameters.empty() && rva)
							fi.name = "dctor";
					}

					s_function->Release();
				}
				s_functions->Release();

				pugi::xml_node n_variables;

				IDiaEnumSymbols* s_variables;
				s_udt->findChildren(SymTagData, NULL, nsNone, &s_variables);
				IDiaSymbol* s_variable;
				while (SUCCEEDED(s_variables->Next(1, &s_variable, &ul)) && (ul == 1))
				{
					s_variable->get_name(&pwname);
					auto name = w2s(pwname);

					Metas metas;
					if (ur->pass_child(name, &metas))
					{
						IDiaSymbol* s_type;
						s_variable->get_type(&s_type);
						auto type = typeinfo_from_symbol(s_type);
						s_type->Release();

						if (metas.get("static"_h))
						{
							IDiaEnumSymbols* symbols;
							IDiaSymbol* symbol;
							auto voff = -1;
							auto rva = 0U;
							TypeInfo* return_type = nullptr;
							std::vector<TypeInfo*> parameters;

							global->findChildren(SymTagUDT, s2w(type->name).c_str(), nsNone, &symbols);
							if (SUCCEEDED(symbols->Next(1, &symbol, &ul)) && (ul == 1))
							{
								IDiaEnumSymbols* s_functions;
								symbol->findChildren(SymTagFunction, NULL, nsNone, &s_functions);
								IDiaSymbol* s_function;
								while (SUCCEEDED(s_functions->Next(1, &s_function, &ul)) && (ul == 1))
								{
									s_function->get_name(&pwname);
									auto name = w2s(pwname);

									if (name == "operator()" && s_function->get_virtualBaseOffset(&dw) == S_OK)
									{
										voff = dw;

										IDiaSymbol* s_function_type;
										s_function->get_type(&s_function_type);

										return_type = get_return_type(s_function_type);
										parameters = get_parameters(s_function_type);

										s_function_type->Release();
									}

									s_function->Release();
								}
								s_functions->Release();
							}

							global->findChildren(SymTagData, s2w(udt_name + "::" + name).c_str(), nsNone, &symbols);
							if (SUCCEEDED(symbols->Next(1, &symbol, &ul)) && (ul == 1))
							{
								if (symbol->get_relativeVirtualAddress(&dw) == S_OK)
									rva = dw;

								symbol->Release();
							}
							symbols->Release();

							if (rva && voff != -1)
							{
								auto& fi = u.functions.emplace_back();
								fi.name = name;
								fi.rva = rva;
								fi.voff = voff;
								fi.return_type = return_type;
								fi.parameters = parameters;
								fi.library = library;
							}

							continue;
						}
						else
						{
							uint offset;
							s_variable->get_offset(&l);
							offset = l;

							auto& vi = u.variables.emplace_back();
							vi.type = type;
							reference_type(vi.type);
							vi.name = name;
							vi.offset = offset;
							vi.metas = metas;
						}
					}
					s_variable->Release();
				}
				s_variables->Release();

				void* obj = nullptr;
				if (library)
				{
					if (auto fi = u.find_function("create"); fi && is_in(fi->return_type->tag, TagP_Beg, TagP_End))
					{
						if (fi->parameters.empty())
							obj = fi->call<void*>(nullptr);
						else if (fi->parameters.size() == 1 && is_in(fi->parameters[0]->tag, TagP_Beg, TagP_End))
							obj = fi->call<void*>(nullptr, nullptr);
					}
					else
					{
						obj = malloc(udt_size);
						if (auto fi = u.find_function("dctor"); fi)
							fi->call<void>(obj);
						else
							memset(obj, 0, udt_size);
					}
				}

				if (ur->children_type == Rule::Equal)
				{
					for (auto& c : ur->children)
					{
						if (c.first.starts_with("get_"))
						{
							auto getter_idx = u.find_function_i(c.first);
							if (getter_idx != -1)
							{
								auto name = c.first;
								SUS::strip_head_if(name, "get_");
								auto setter_idx = u.find_function_i("set_" + name);
								if (setter_idx != -1)
								{
									auto& a = u.attributes.emplace_back();
									a.ui = &u;
									a.name = name;
									a.type = u.functions[getter_idx].return_type;
									a.getter_idx = getter_idx;
									a.setter_idx = setter_idx;
								}
							}
						}
						else if (!c.first.starts_with("set_"))
						{
							auto var_idx = u.find_variable_i(c.first);
							if (var_idx != -1)
							{
								auto& a = u.attributes.emplace_back();
								a.ui = &u;
								a.name = c.first;
								a.type = u.variables[var_idx].type;
								a.var_idx = var_idx;
								a.getter_idx = u.find_function_i("get_" + c.first);
								a.setter_idx = u.find_function_i("set_" + c.first);
							}
						}
					}
				}

				if (obj)
				{
					for (auto& vi : u.variables)
						vi.default_value = vi.type->serialize((char*)obj + vi.offset);
					for (auto& a : u.attributes)
						a.default_value = a.var_idx != -1 ? a.var()->default_value : a.serialize(obj);

					if (auto fi = u.find_function("dtor"); fi)
						fi->call<void>(obj);
					free(obj);
				}

				db.udts.emplace(sh(u.name.c_str()), u);
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
				if (!find_enum(sh(t->name.c_str()), db))
				{
					add_enum_rule(t->name);

					need_collect = true;
				}
				break;
			case TagU:
			case TagVU:
				if (!find_udt(sh(t->name.c_str()), db))
				{
					add_udt_rule(t->name);

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
