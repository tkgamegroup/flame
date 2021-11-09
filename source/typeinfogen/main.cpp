#include "../xml.h"
#include <flame/foundation/typeinfo.h>
#include <flame/foundation/system.h>

#include <Windows.h>
#include <dia2.h>
#include <atlbase.h>
#include <chrono>

#include <boost/regex.hpp>

using namespace flame;

std::string format_type(const wchar_t* in)
{
	auto str = w2s(in);

	SUS::replace_all(str, "enum ", "");
	SUS::replace_all(str, "unsigned ", "u");
	SUS::replace_all(str, "__int64 ", "int64");

	SUS::replace_all(str, "Private", "");

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
			name = "complex_pointer";
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
	auto ap = parse_args(argc, args);
	auto input = ap.get_item("-i");
	auto desc = ap.get_item("-d");
	if (input.empty() || desc.empty())
		goto show_usage;

	goto process;

show_usage:
	printf("usage: typeinfogen -i filename -d filename\n"
		"-i: specify the target executable\n"
		"-d: specify the desc file, which contains the reflect rules\n");
	return 0;

process:
	auto target_path = std::filesystem::path(input);
	printf("typeinfogen: %s\n", target_path.string().c_str());
	if (!std::filesystem::exists(target_path))
	{
		printf("target does not exist: %s\n", target_path.string().c_str());
		return 0;
	}

	wchar_t app_path[260];
	get_app_path(app_path, false);
	auto foundation_path = std::filesystem::path(app_path) / L"flame_foundation.dll";
	if (target_path != foundation_path)
	{
		foundation_path.replace_extension(L".typeinfo");
		load_typeinfo(foundation_path.c_str());
	}

	auto pdb_path = target_path;
	pdb_path.replace_extension(L".pdb");
	if (!std::filesystem::exists(pdb_path))
	{
		printf("pdb does not exist: %s\n", pdb_path.string().c_str());
		return 0;
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
		enum_rules.push_back(er);
	}
	for (auto n_rule : desc_root.child("udts"))
	{
		UdtRule ur;
		auto wtf = n_rule.attribute("name").value();
		ur.name = wtf;
		for (auto n_i : n_rule)
		{
			UdtRule::Item item;
			auto t = std::string(n_i.attribute("type").value());
			item.type = t == "function" ? TypeFunction : TypeData;
			auto wtf = n_i.attribute("name").value();
			item.name = wtf;
			item.metas = n_i.attribute("metas").value();
			ur.items.push_back(item);
		}
		udt_rules.push_back(ur);
	}

	auto typeinfo_path = target_path;
	typeinfo_path.replace_extension(L".typeinfo");

	if (std::filesystem::exists(typeinfo_path))
	{
		auto lwt = std::filesystem::last_write_time(typeinfo_path);
		if (lwt > std::filesystem::last_write_time(pdb_path) && lwt > std::filesystem::last_write_time(desc_path))
		{
			printf("typeinfogen: %s up to date\n", typeinfo_path.string().c_str());
			return 0;
		}
	}

	if (FAILED(CoInitialize(NULL)))
	{
		printf("typeinfogen: com initial failed, exit\n");
		fassert(0);
		return 1;
	}

	CComPtr<IDiaDataSource> dia_source;
	if (FAILED(CoCreateInstance(CLSID_DiaSource, NULL, CLSCTX_INPROC_SERVER, __uuidof(IDiaDataSource), (void**)&dia_source)))
	{
		printf("typeinfogen: dia not found, exit\n");
		fassert(0);
		return 1;
	}

	if (FAILED(dia_source->loadDataFromPdb(pdb_path.c_str())))
	{
		printf("pdb failed to open: %s\n", pdb_path.string().c_str());
		fassert(0);
		return 1;
	}

	CComPtr<IDiaSession> session;
	if (FAILED(dia_source->openSession(&session)))
	{
		printf("session failed to open\n");
		fassert(0);
		return 1;
	}

	CComPtr<IDiaSymbol> global;
	if (FAILED(session->get_globalScope(&global)))
	{
		printf("failed to get global\n");
		fassert(0);
		return 1;
	}

	BOOL b;
	LONG l;
	ULONG ul;
	ULONGLONG ull;
	DWORD dw;
	wchar_t* pwname;

	auto library = LoadLibraryW(target_path.c_str());
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

	std::vector<std::string> function_types;
	auto add_function_type = [&](const std::string& n) {
		for (auto& t : function_types)
		{
			if (t == n)
				return;
		}
		function_types.push_back(n);
	};

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
						if (name == "__local_vftable_ctor_closure" || name == "__vecDelDtor")
							continue;

						std::string metas;
						if (ur.pass_item(TypeFunction, name, &metas))
						{
							auto rva = 0;
							auto voff = -1;
							if (s_function->get_relativeVirtualAddress(&dw) == S_OK)
								rva = dw;
							else if (s_function->get_virtualBaseOffset(&dw) == S_OK)
								voff = dw;

							if (rva || voff != -1)
							{
								IDiaSymbol* s_function_type;
								s_function->get_type(&s_function_type);

								IDiaSymbol* s_return_type;
								s_function_type->get_type(&s_return_type);
								auto type_desc = typeinfo_from_symbol(s_return_type);
								s_return_type->Release();

								IDiaSymbol6* s6_function = (IDiaSymbol6*)s_function;
								auto is_static = false;
								if (s6_function->get_isStaticMemberFunc(&b) == S_OK)
									is_static = b;

								auto fi = u->add_function(name.c_str(), rva, voff, is_static, TypeInfo::get(type_desc.tag, type_desc.name, db), metas.c_str());

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

								add_function_type(fi->get_full_name());

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
						std::string metas;
						if (ur.pass_item(TypeData, name, &metas))
						{
							IDiaSymbol* s_type;
							s_variable->get_type(&s_type);

							s_variable->get_offset(&l);
							auto offset = l;

							if (is_virtual && offset != 0)
							{
								auto type_desc = typeinfo_from_symbol(s_type);
								if (type_desc.name.starts_with("flame::"))
									SUS::cut_tail_if(type_desc.name, "Private");
								if (type_desc.tag == TypeEnumSingle || type_desc.tag == TypeEnumMulti)
									new_enum(type_desc.name, s_type);

								auto type = TypeInfo::get(type_desc.tag, type_desc.name, db);
								u->add_variable(type, name.c_str(), offset, 1, 0, (char*)obj + offset, metas.c_str());
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

	save_typeinfo(typeinfo_path.c_str(), db);

	FreeLibrary(library);
	db->release();

	printf("typeinfogen: %s generated\n", typeinfo_path.string().c_str());

	// compile callers
	{
		auto cpp_path = typeinfo_path;
		cpp_path.replace_filename(typeinfo_path.filename().stem().wstring() + L"_callers");
		cpp_path.replace_extension(L".cpp");
		std::ofstream cpp(cpp_path);
		cpp << "typedef void		V;\n";
		cpp << "typedef void*		P;\n";
		cpp << "typedef int			I;\n";
		cpp << "typedef bool		B;\n";
		cpp << "typedef char		C;\n";
		cpp << "typedef wchar_t		W;\n";
		cpp << "typedef long long	L;\n";
		cpp << "typedef float		F;\n";
		cpp << "typedef struct { char a; char b; }						C2;\n";
		cpp << "typedef struct { char a; char b; char c; }				C3;\n";
		cpp << "typedef struct { char a; char b; char c; char d; }		C4;\n";
		cpp << "typedef struct { int a; int b; }						I2;\n";
		cpp << "typedef struct { int a; int b; int c; }					I3;\n";
		cpp << "typedef struct { int a; int b; int c; int d; }			I4;\n";
		cpp << "typedef struct { float a; float b; }					F2;\n";
		cpp << "typedef struct { float a; float b; float c; }			F3;\n";
		cpp << "typedef struct { float a; float b; float c; float d; }	F4;\n";
		cpp << "typedef struct { F3 a; F3 b; }							F6;\n";
		cpp << "typedef struct { F3 a; F3 b; F3 c; }					F9;\n";
		cpp << "typedef struct { F4 a; F4 b; F4 c; F4 d; }				F16;\n";
		cpp << "typedef struct { F4 a; F4 b; F4 c; F4 d; F4 e; F4 f; }	F24;\n";
		cpp << "typedef struct { }										dummy;\n";
		cpp << "template <class F> F a2f(void* p) { union{ void*p; F f; } cvt; cvt.p = p; return cvt.f; }\n";
		cpp << "\n";
		std::sort(function_types.begin(), function_types.end());
		for (auto& t : function_types)
		{
			auto sp = SUS::split(t, '_');

			auto comma = false;
			cpp << "void " << t << "(void* address, void* obj, void* ret, void** ps)\n";
			cpp << "{\n";

			// function typedef
			cpp << "\ttypedef " << sp[1];
			if (sp[0] == "m")
				cpp << "(dummy::*func)(";
			else
				cpp << "(*func)(";
			for (auto i = 2; i < sp.size(); i++)
			{
				if (comma)
					cpp << ", ";
				cpp << sp[i];
				comma = true;
			}
			cpp << ");\n";

			// function call
			cpp << "\t";
			if (sp[1] != "V")
				cpp << "*(" << sp[1] << "*)ret = ";
			comma = false;
			if (sp[0] == "m")
				cpp << "(*(dummy*)obj.*a2f<func>(address))(";
			else
				cpp << "((func)address)(";
			for (auto i = 2; i < sp.size(); i++)
			{
				if (comma)
					cpp << ", ";
				cpp << "*(" << sp[i] << "*)ps[" << std::to_string(i - 2) << "]";
				comma = true;
			}
			cpp << ");\n";

			cpp << "}\n\n";
		}
		cpp << "extern \"C\" __declspec(dllexport) void get_callers(void(*callback)(const char* name, void(*func)(void*, void*, void*, void**)))\n{\n";
		for (auto& t : function_types)
			cpp << "\tcallback(\"" << t << "\", " << t << ");\n";
		cpp << "}\n";
		cpp.close();

		std::filesystem::current_path(cpp_path.parent_path());

		std::wstring compile_command(L"\"");
		compile_command += s2w(VS_LOCATION);
		compile_command += L"/VC/Auxiliary/Build/vcvars64.bat\" & cl -LD -MD -EHsc -Zi ";
		compile_command += cpp_path.wstring();
		std::string output;
		output.reserve(1024 * 1024);
		exec(nullptr, (wchar_t*)compile_command.c_str(), output.data());
	}

	return 0;
}
