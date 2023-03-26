#include <flame/xml.h>
#include <flame/foundation/typeinfo.h>
#include <flame/foundation/system.h>

#include <Windows.h>
#include <dia2.h>
#include <atlbase.h>

using namespace flame;

TypeInfoDataBase db;

TypeTag parse_vector(std::string& name)
{
	SUS::strip_head_if(name, "std::vector<");
	name = name.substr(0, first_template_argument_pos(name));

	auto is_pointer = false;
	auto is_pair = false;
	auto is_tuple = false;
	if (SUS::strip_head_if(name, "std::unique_ptr<"))
	{
		is_pointer = true;
		name = name.substr(0, first_template_argument_pos(name));
	}
	else if (SUS::strip_head_if(name, "std::pair<"))
	{
		is_pair = true;
		name.pop_back();
		auto args = parse_template_arguments(name);
		name = TypeInfo::format_name(args[0]) + ";" + TypeInfo::format_name(args[1]);
	}
	else if (SUS::strip_head_if(name, "std::tuple<"))
	{
		is_tuple = true;
		name.pop_back();
		auto args = parse_template_arguments(name);
		std::string temp;
		for (auto& a : args)
			temp += TypeInfo::format_name(a) + ';';
		name = temp;
		name.pop_back();
	}

	auto is_enum = SUS::strip_head_if(name, "enum ");
	SUS::strip_head_if(name, "struct ");
	SUS::strip_head_if(name, "class ");
	name = TypeInfo::format_name(name);
	if (is_enum)
		return TagVE;
	if (!is_pointer)
		is_pointer = SUS::strip_tail_if(name, "*") || SUS::strip_tail_if(name, "*__ptr64");

	if (is_pair)
		return TagVR;
	else if (is_tuple)
		return TagVT;
	else
	{
		if (TypeInfo::is_basic_type(name))
		{
			if (is_pointer)
				assert(0);
			else
				return TagVD;
		}
		else
			return is_pointer ? TagVPU : TagVU;
	}
	return TagCount;
}

TypeInfo* typeinfo_from_symbol(IDiaSymbol* s_type)
{
	DWORD dw;
	ULONG ul;
	wchar_t* pwname;
	VARIANT var;

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
			assert(0);
			break;
		case SymTagUDT:
		{
			pointer_type->get_name(&pwname);
			auto name = TypeInfo::format_name(w2s(pwname));
			if (TypeInfo::is_basic_type(name))
				ret = TypeInfo::get(TagPD, name, db);
			else
			{
				if (name.starts_with("std::vector<"))
				{
					name = w2s(pwname);
					auto tag = parse_vector(name);
					switch (tag)
					{
					case TagVE: tag = TagPVE; break;
					case TagVD: tag = TagPVD; break;
					case TagVU: tag = TagPVU; break;
					case TagVR: tag = TagPVR; break;
					case TagVT: tag = TagPVT; break;
					case TagVPU: tag = TagD; name.clear(); break;
					}
					ret = TypeInfo::get(tag, name, db);
				}
				else
					ret = TypeInfo::get(TagPU, name, db);
			}
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
				return TypeInfo::get(parse_vector(name), name, db);
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
	case SymTagArrayType:
	{
		TypeInfo* ret = nullptr;

		std::string extent_str;
		if (s_type->get_count(&dw) == S_OK)
			extent_str = std::format("[{}]", dw);

		IDiaSymbol* array_type;
		s_type->get_type(&array_type);
		array_type->get_symTag(&dw);
		switch (dw)
		{
		case SymTagEnum:
			array_type->get_name(&pwname);
			ret = TypeInfo::get(TagAE, TypeInfo::format_name(w2s(pwname)) + extent_str, db);
			break;
		case SymTagBaseType:
			ret = TypeInfo::get(TagAD, TypeInfo::format_name(base_type_name(array_type)) + extent_str, db);
			break;
		case SymTagUDT:
			array_type->get_name(&pwname);
			auto name = TypeInfo::format_name(w2s(pwname));
			if (TypeInfo::is_basic_type(name))
				ret = TypeInfo::get(TagAD, name + extent_str, db);
			else
				ret = TypeInfo::get(TagAU, name + extent_str, db);
			break;
		}
		assert(ret);
		array_type->Release();
		return ret;
	}
	default:
		assert(0);
	}
}

int main(int argc, char **args)
{
	if (argc < 2)
		goto show_usage;

	goto process;

show_usage:
	printf("usage: typeinfogen <target> [-just_print_all_symbols]\n");
	return 0;

process:
	std::filesystem::path input_path = args[1];
	bool just_print_all_symbols = false;

	if (argc > 2)
	{
		just_print_all_symbols = std::string(args[2]) == "-just_print_all_symbols";
	}

	printf("typeinfogen: %s\n", input_path.string().c_str());
	if (!std::filesystem::exists(input_path))
	{
		printf("target does not exist: %s\n", input_path.string().c_str());
		return 0;
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

	if (!just_print_all_symbols)
	{
		if (std::filesystem::exists(typeinfo_path))
		{
			auto lwt = std::filesystem::last_write_time(typeinfo_path);
			if (lwt > std::filesystem::last_write_time(pdb_path))
			{
				printf("typeinfogen: %s up to date\n", typeinfo_path.string().c_str());
				return 0;
			}
			std::filesystem::remove(typeinfo_path);
		}
	}

	if (input_path.filename() == L"flame_foundation.dll")
	{
		tidb.typeinfos.clear();
		tidb.enums.clear();
		tidb.functions.clear();
		tidb.udts.clear();
		tidb.init_basic_types();
	}
	else
	{
		for (auto& path : get_module_dependencies(input_path))
		{
			auto ti_path = path;
			ti_path.replace_extension(".typeinfo");
			if (path.filename() != L"flame_foundation.dll" && std::filesystem::exists(ti_path))
				tidb.load(ti_path);
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
		std::filesystem::path source_file;

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
				for (auto& s : children)
				{
					if (s.first == str)
					{
						if (metas)
							*metas = s.second;
						break;
					}
				}
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
		for (auto& r : enum_rules)
		{
			if (r.name == n)
				return;
		}
		auto& r = enum_rules.emplace_back();
		r.name = n;
	};
	auto add_udt_rule = [&](const std::string& n) {
		for (auto& r : udt_rules)
		{
			if (r.name == n)
				return;
		}
		auto& r = udt_rules.emplace_back();
		r.name = n;
	};

	auto desc_path = input_path;
	desc_path.replace_extension(L".typedesc");
	if (std::filesystem::exists(desc_path))
	{
		std::filesystem::path sources_path;

		std::ifstream desc(desc_path);
		std::string line;
		std::getline(desc, line);
		sources_path = line;
		while (!desc.eof())
		{
			std::getline(desc, line);
			if (line.empty()) continue;
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

		if (std::filesystem::exists(sources_path))
		{
			std::string line;
			std::smatch res;
			for (auto& it : std::filesystem::recursive_directory_iterator(sources_path))
			{
				auto path = it.path();
				path.make_preferred();
				if (auto ext = path.extension(); (ext == L".h" || ext == L".cpp") &&
					!path.filename().stem().wstring().ends_with(L"_private"))
				{
					std::deque<std::pair<std::string, uint>> name_spaces;
					auto get_name = [&](const std::string& name) {
						std::string ret;
						uint count = name_spaces.size();
						for (auto i = 0; i < count; i++)
						{
							if (i == count - 1 && name == name_spaces[i].first)
								continue;
							ret += name_spaces[i].first + "::";
						}
						ret += name;
						return ret;
					};
					auto brackets_level = 0;
					auto reflect_mark = false;
					std::string reflect_metas;
					std::pair<std::string, uint> name_space_mark = std::make_pair("", 0U);
					Rule* curr_rule = nullptr;
					std::ifstream file(path);
					while (!file.eof())
					{
						std::getline(file, line);
						SUS::ltrim(line);

						if (line.starts_with("//"))
						{
							line = line.substr(2);
							SUS::ltrim(line);
							if (SUS::strip_head_if(line, "Reflect"))
							{
								reflect_mark = true;
								reflect_metas = line;
							}
						}
						else
						{
							if (SUS::strip_head_if(line, "namespace "))
							{
								name_space_mark = std::make_pair(line, brackets_level);
								curr_rule = nullptr;
							}
							else if (SUS::strip_head_if(line, "enum "))
							{
								if (reflect_mark)
								{
									auto& r = enum_rules.emplace_back();
									r.type = Rule::Equal;
									r.name = get_name(line);
									r.source_file = path;
									curr_rule = &r;
								}
							}
							else if (SUS::strip_head_if(line, "struct ") || SUS::strip_head_if(line, "class "))
							{
								SUS::strip_head_if(line, "EXPORT ");
								if (auto pos = line.find_last_of(':'); pos != std::string::npos)
								{
									line.erase(line.begin() + pos, line.end());
									SUS::rtrim(line);
								}
								name_space_mark = std::make_pair(line, brackets_level);

								if (reflect_mark)
								{
									auto& r = udt_rules.emplace_back();
									r.type = Rule::Equal;
									r.name = get_name(line);
									r.children_type = Rule::Equal;
									for (auto& t : SUS::split(reflect_metas))
									{
										if (t == "ctor")
											r.children.emplace_back().first = "ctor";
										else if (t == "any")
											r.children_type = Rule::Any;
									}
									r.source_file = path;
									curr_rule = &r;
								}
							}
							else
							{
								if (reflect_mark)
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
										if (!curr_rule)
										{
											if (!name_spaces.empty())
											{
												auto rule_name = get_name(name_spaces.back().first);
												for (auto& r : udt_rules)
												{
													if (r.name == rule_name)
													{
														curr_rule = &r;
														break;
													}
												}
												if (!curr_rule)
												{
													auto& r = udt_rules.emplace_back();
													r.type = Rule::Equal;
													r.name = get_name(rule_name);
													r.children_type = Rule::Any;
													curr_rule = &r;
												}
											}
										}
										if (curr_rule)
										{
											auto& c = curr_rule->children.emplace_back();
											c.first = res[1].str();
											c.second.from_string(reflect_metas);
										}
									}
								}
							}

							reflect_mark = false;
							reflect_metas.clear();

							if (auto n = std::count(line.begin(), line.end(), '{'); n > 0)
							{
								brackets_level += n;
								if (!name_space_mark.first.empty())
								{
									name_spaces.push_back(name_space_mark);
									name_space_mark = std::make_pair("", 0U);
								}
							}
							if (auto n = std::count(line.begin(), line.end(), '}'); n > 0)
							{
								brackets_level -= n;
								if (!name_spaces.empty() && brackets_level == name_spaces.back().second)
								{
									name_spaces.pop_back();
									curr_rule = nullptr;
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
		return 1;
	}

	CComPtr<IDiaDataSource> dia_source;
	if (FAILED(CoCreateInstance(CLSID_DiaSource, NULL, CLSCTX_INPROC_SERVER, __uuidof(IDiaDataSource), (void**)&dia_source)))
	{
		printf("typeinfogen: dia not found, exit\n");
		return 1;
	}

	if (FAILED(dia_source->loadDataFromPdb(pdb_path.c_str())))
	{
		printf("pdb failed to open: %s\n", pdb_path.string().c_str());
		return 1;
	}

	CComPtr<IDiaSession> dia_session;
	if (FAILED(dia_source->openSession(&dia_session)))
	{
		printf("session failed to open\n");
		return 1;
	}

	CComPtr<IDiaSymbol> dia_global;
	if (FAILED(dia_session->get_globalScope(&dia_global)))
	{
		printf("failed to get global\n");
		return 1;
	}

	BOOL b;
	LONG l;
	ULONG ul;
	ULONGLONG ull;
	DWORD dw;
	wchar_t* pwname;
	VARIANT variant;

	auto library = load_library(input_path);

	std::map<std::string, IDiaSymbol*> dia_enums;
	std::map<std::string, IDiaSymbol*> dia_funcs;
	std::map<std::string, IDiaSymbol*> dia_udts;

	IDiaEnumSymbols* symbols;
	IDiaSymbol* s_obj;

	auto skip_name = [](const std::string& name) {
		return name.starts_with("std::") || name.starts_with("__") || name.starts_with("`")
			|| name.contains("<lambda_")
			|| name.starts_with("glm::") || name.starts_with("ImGui::");
	};

	if (just_print_all_symbols)
		printf("Enums:\n");
	dia_global->findChildren(SymTagEnum, NULL, nsNone, &symbols);
	while (SUCCEEDED(symbols->Next(1, &s_obj, &ul)) && (ul == 1))
	{
		s_obj->get_name(&pwname);
		auto name = w2s(pwname);
		if (!skip_name(name))
		{
			if (just_print_all_symbols)
				printf("%s\n", name.c_str());
			dia_enums[name] = s_obj;
		}
		else
			s_obj->Release();
	}
	symbols->Release();

	if (just_print_all_symbols)
		printf("UDTs:\n");
	dia_global->findChildren(SymTagUDT, NULL, nsNone, &symbols);
	while (SUCCEEDED(symbols->Next(1, &s_obj, &ul)) && (ul == 1))
	{
		s_obj->get_name(&pwname);
		auto name = w2s(pwname);
		if (!skip_name(name))
		{
			if (just_print_all_symbols)
				printf("%s\n", name.c_str());
			dia_udts[name] = s_obj;
		}
		else
			s_obj->Release();
	}
	symbols->Release();

	if (just_print_all_symbols)
		printf("Functions:\n");
	dia_global->findChildren(SymTagFunction, NULL, nsNone, &symbols);
	while (SUCCEEDED(symbols->Next(1, &s_obj, &ul)) && (ul == 1))
	{
		s_obj->get_name(&pwname);
		auto name = w2s(pwname);
		if (!skip_name(name))
		{
			if (just_print_all_symbols)
				printf("%s\n", name.c_str());
			dia_funcs[name] = s_obj;
		}
		else
			s_obj->Release();
	}

	if (just_print_all_symbols)
		return 0;

	for (auto& s_func : dia_funcs)
	{
		if (s_func.first.starts_with("flame::TypeInfo::get"))
		{
			static std::regex reg("^flame::TypeInfo::get<(.+)>$");
			std::smatch res;
			if (std::regex_search(s_func.first, res, reg))
			{
				auto name = res[1].str();
				if (SUS::strip_head_if(name, "enum "))
					add_enum_rule(name);
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
								add_enum_rule(name);
							else if (tag == TagVU)
								add_udt_rule(name);
						}
						else
							add_udt_rule(name);
					}
				}
			}
		}
	}

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

		for (auto& s_enum : dia_enums)
		{
			Rule* er;
			if (!find_enum(sh(s_enum.first.c_str()), db) && need_enum(s_enum.first, &er))
			{
				EnumInfo e;
				e.name = s_enum.first;
				e.source_file = er->source_file;
				std::vector<std::pair<std::string, int>> items;

				IDiaEnumSymbols* symbols;
				s_enum.second->findChildren(SymTagNull, NULL, nsNone, &symbols);
				IDiaSymbol* s_item;
				while (SUCCEEDED(symbols->Next(1, &s_item, &ul)) && (ul == 1))
				{
					s_item->get_name(&pwname);
					ZeroMemory(&variant, sizeof(variant));
					s_item->get_value(&variant);

					auto item_name = w2s(pwname);
					if (!item_name.ends_with("_Max") && !item_name.ends_with("_Count"))
						items.emplace_back(item_name, variant.lVal);

					s_item->Release();
				}
				symbols->Release();

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
					ii.name_hash = sh(ii.name.c_str());
					ii.value = i.second;
				}

				db.enums.emplace(sh(e.name.c_str()), e);
			}
		}

		for (auto& s_udt : dia_udts)
		{
			Rule* ur;
			if (!find_udt(sh(s_udt.first.c_str()), db) && need_udt(s_udt.first, &ur))
			{
				UdtInfo u;
				u.name = s_udt.first;
				u.source_file = ur->source_file;

				s_udt.second->get_length(&ull);
				u.size = ull;

				IDiaEnumSymbols* s_base_classes;
				IDiaSymbol* s_base_class;
				s_udt.second->findChildren(SymTagBaseClass, NULL, nsNone, &s_base_classes);
				if (SUCCEEDED(s_base_classes->Next(1, &s_base_class, &ul)) && (ul == 1))
				{
					s_base_class->get_name(&pwname);
					u.base_class_name = w2s(pwname);
				}

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

				IDiaEnumSymbols* s_functions;
				s_udt.second->findChildren(SymTagFunction, NULL, nsNone, &s_functions);
				IDiaSymbol* s_function;
				while (SUCCEEDED(s_functions->Next(1, &s_function, &ul)) && (ul == 1))
				{
					s_function->get_name(&pwname);
					auto name = w2s(pwname);
					if (u.name.ends_with(name))
						name = "ctor";
					else if (name[0] == '~')
						name = "dtor";
					if (name == "__local_vftable_ctor_closure" || name == "__vecDelDtor")
						continue;

					auto rva = 0; auto voff = -1;
					s_function->get_virtual(&b);
					if (!b)
					{
						if (s_function->get_relativeVirtualAddress(&dw) == S_OK)
							rva = dw;
					}
					else
					{
						if (s_function->get_virtualBaseOffset(&dw) == S_OK)
							voff = dw;
					}

					if (!rva && voff == -1)
						continue;

					IDiaSymbol* s_function_type;
					s_function->get_type(&s_function_type);
					auto return_type = get_return_type(s_function_type);
					auto parameters = get_parameters(s_function_type);

					Metas metas;
					if (ur->pass_child(name, &metas))
					{
						auto& fi = u.functions.emplace_back();
						fi.name = name;
						fi.name_hash = sh(fi.name.c_str());
						fi.rva = rva;
						fi.voff = voff;
						fi.metas = metas;
						fi.library = library;
						fi.metas = metas;

						IDiaSymbol6* s6_function = (IDiaSymbol6*)s_function;
						fi.is_static = false;
						if (s6_function->get_isStaticMemberFunc(&b) == S_OK)
							fi.is_static = b;

						fi.return_type = return_type;
						fi.parameters = parameters;

						if (!rva && voff < 0 && ur->children_type == Rule::Equal && (name != "ctor" && name != "dtor"))
						{
							auto str = std::format("Fatal Error: Cannot find implementation of required function '{}' of udt '{}'\n", name.c_str(), u.name.c_str());
							printf(str.c_str());
							_wassert(s2w(str).c_str(), _CRT_WIDE(__FILE__), (unsigned)(__LINE__));
						}
					}

					s_function_type->Release();
					s_function->Release();
				}
				s_functions->Release();

				IDiaEnumSymbols* s_variables;
				s_udt.second->findChildren(SymTagData, NULL, nsNone, &s_variables);
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

							if (auto it = dia_udts.find(type->name); it != dia_udts.end())
							{
								IDiaEnumSymbols* s_functions;
								it->second->findChildren(SymTagFunction, NULL, nsNone, &s_functions);
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

							dia_global->findChildren(SymTagData, s2w(u.name + "::" + name).c_str(), nsNone, &symbols);
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
								fi.name_hash = sh(fi.name.c_str());
								fi.rva = rva;
								fi.voff = voff;
								fi.return_type = return_type;
								fi.parameters = parameters;
								fi.library = library;
								fi.metas = metas;
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
							vi.name_hash = sh(vi.name.c_str());
							vi.offset = offset;
							vi.metas = metas;
						}
					}
					s_variable->Release();
				}
				s_variables->Release();

				for (auto& fi : u.functions)
				{
					if (fi.name == "ctor" && fi.parameters.empty() && fi.rva)
						fi.name = "dctor";
					if (fi.voff > 0)
						u.is_pod = false;
					if (fi.name_hash == "create"_h && fi.metas.get("static"_h))
						u.is_pod = false;
				}
				for (auto i = 0; i < u.functions.size(); i++)
					u.functions_map.emplace(u.functions[i].name_hash, i);

				for (auto& v : u.variables)
				{
					if (!is_in(v.type->tag, TagE, TagD))
					{
						u.is_pod = false;
						break;
					}
					if (v.type == TypeInfo::get<std::string>() ||
						v.type == TypeInfo::get<std::wstring>() ||
						v.type == TypeInfo::get<std::filesystem::path>())
					{
						u.is_pod = false;
						break;
					}
				}
				for (auto i = 0; i < u.variables.size(); i++)
					u.variables_map.emplace(u.variables[i].name_hash, i);

				void* obj = nullptr;
				if (library)
				{
					if (auto fi = u.find_function("create"_h); fi && is_in(fi->return_type->tag, TagP_Beg, TagP_End))
					{
						if (fi->parameters.empty())
							obj = fi->call<void*>(nullptr);
						else if (fi->parameters.size() == 1 && is_in(fi->parameters[0]->tag, TagP_Beg, TagP_End))
							obj = fi->call<void*>(nullptr, nullptr);
					}
					else
					{
						obj = malloc(u.size);
						if (auto fi = u.find_function("dctor"_h); fi && fi->rva)
							fi->call<void>(obj);
						else
							memset(obj, 0, u.size);
					}
				}

				if (ur->children_type == Rule::Equal)
				{
					for (auto& c : ur->children)
					{
						if (c.first.starts_with("get_"))
						{
							if (auto getter_idx = u.find_function_i(c.first); getter_idx != -1)
							{
								auto name = c.first;
								SUS::strip_head_if(name, "get_");
								if (auto setter_idx = u.find_function_i("set_" + name); setter_idx != -1)
								{
									if (!u.functions[getter_idx].parameters.empty())
										continue;
									if (u.functions[setter_idx].parameters.size() != 1)
										continue;
									auto& a = u.attributes.emplace_back();
									a.ui = &u;
									a.name = name;
									a.name_hash = sh(a.name.c_str());
									a.type = u.functions[getter_idx].return_type;
									a.getter_idx = getter_idx;
									a.setter_idx = setter_idx;
								}
							}
						}
						else if (!c.first.starts_with("set_"))
						{
							if (auto var_idx = u.find_variable_i(c.first); var_idx != -1)
							{
								auto& a = u.attributes.emplace_back();
								a.ui = &u;
								a.name = c.first;
								a.name_hash = sh(a.name.c_str());
								a.type = u.variables[var_idx].type;
								a.var_idx = var_idx;
								a.getter_idx = u.find_function_i("get_" + c.first);
								if (a.getter_idx != -1 && !u.functions[a.getter_idx].parameters.empty())
									a.getter_idx = -1;
								a.setter_idx = u.find_function_i("set_" + c.first);
								if (a.setter_idx != -1 && u.functions[a.setter_idx].parameters.size() != 1)
									a.setter_idx = -1;
							}
						}
					}
					for (auto i = 0; i < u.attributes.size(); i++)
						u.attributes_map.emplace(u.attributes[i].name_hash, i);
				}

				if (obj)
				{
					for (auto& vi : u.variables)
						vi.default_value = vi.type->serialize((char*)obj + vi.offset);
					for (auto& a : u.attributes)
						a.default_value = a.var_idx != -1 ? a.var()->default_value : a.serialize(obj);

					if (auto fi = u.find_function("dtor"_h); fi && fi->rva)
						fi->call<void>(obj);
					free(obj);
				}

				db.udts.emplace(sh(u.name.c_str()), u);
			}
		}

		enum_rules.clear();
		udt_rules.clear();
		for (auto t : referenced_types)
		{
			auto name = t->name;
			switch (t->tag)
			{
			case TagAE:
				name = ((TypeInfo_ArrayOfEnum*)t)->ti->name;
			case TagE:
			case TagVE:
				if (!find_enum(sh(name.c_str()), db))
				{
					add_enum_rule(name);
					need_collect = true;
				}
				break;
			case TagAU:
				name = ((TypeInfo_ArrayOfUdt*)t)->ti->name;
			case TagU:
			case TagVU:
				if (!find_udt(sh(name.c_str()), db))
				{
					add_udt_rule(name);
					need_collect = true;
				}
				break;
			}
		}
	}

	db.save(typeinfo_path);
	free_library(library);

	printf("typeinfogen: %s generated\n", typeinfo_path.string().c_str());

	return 0;
}
