#include <flame/serialize.h>

#include <Windows.h>
#include <dia2.h>
#include <atlbase.h>

namespace flame
{
	void* f_malloc(uint size)
	{
		return malloc(size);
	}

	void* f_realloc(void* p, uint size)
	{
		if (!p)
			return f_malloc(size);
		return realloc(p, size);
	}

	void f_free(void* p)
	{
		free(p);
	}

	void* load_module(const wchar_t* module_name)
	{
		return LoadLibraryW(module_name);
	}

	void* get_module_func(void* module, const char* name)
	{
		return GetProcAddress((HMODULE)module, name);
	}

	void free_module(void* library)
	{
		FreeLibrary((HMODULE)library);
	}

	struct TypeInfoPrivate : TypeInfo
	{
		TypeTag tag;
		bool is_array;
		std::string base_name;
		std::string name;
		uint base_hash;
		uint hash;

		TypeInfoPrivate(TypeTag tag, const std::string& base_name, bool is_array) :
			tag(tag),
			base_name(base_name),
			base_hash(FLAME_HASH(base_name.c_str())),
			is_array(is_array)
		{
			name = make_str(tag, base_name, is_array);
			hash = FLAME_HASH(name.c_str());
		}
	};

	TypeTag TypeInfo::tag() const
	{
		return ((TypeInfoPrivate*)this)->tag;
	}

	bool TypeInfo::is_array() const
	{
		return ((TypeInfoPrivate*)this)->is_array;
	}

	const char* TypeInfo::base_name() const
	{
		return ((TypeInfoPrivate*)this)->base_name.c_str();
	}

	const char* TypeInfo::name() const
	{
		return ((TypeInfoPrivate*)this)->name.c_str();
	}

	uint TypeInfo::base_hash() const
	{
		return ((TypeInfoPrivate*)this)->base_hash;
	}

	uint TypeInfo::hash() const
	{
		return ((TypeInfoPrivate*)this)->hash;
	}

	static std::vector<std::unique_ptr<TypeInfoPrivate>> type_infos;

	const TypeInfo* TypeInfo::get(TypeTag tag, const char* base_name, bool is_array)
	{
		for (auto& i : type_infos)
		{
			if (i->tag == tag && i->base_name == base_name && i->is_array == is_array)
				return i.get();
		}
		auto i = new TypeInfoPrivate(tag, base_name, is_array);
		type_infos.emplace_back(i);
		return i;
	}

	const TypeInfo* TypeInfo::get(const char* str)
	{
		TypeTag tag;
		std::string base_name;
		bool is_array;
		break_str(str, tag, base_name, is_array);
		return TypeInfo::get(tag, base_name.c_str(), is_array);
	}

	struct VariableInfoPrivate : VariableInfo
	{
		TypeInfoPrivate* type;
		std::string name;
		uint name_hash;
		std::string decoration;
		uint offset, size;
		std::string default_value;
	};

	const TypeInfo* VariableInfo::type() const
	{
		return ((VariableInfoPrivate*)this)->type;
	}

	const char* VariableInfo::name() const
	{
		return ((VariableInfoPrivate*)this)->name.c_str();
	}

	uint VariableInfo::name_hash() const
	{
		return ((VariableInfoPrivate*)this)->name_hash;
	}

	uint VariableInfo::offset() const
	{
		return ((VariableInfoPrivate*)this)->offset;
	}

	uint VariableInfo::size() const
	{
		return ((VariableInfoPrivate*)this)->size;
	}

	const char* VariableInfo::decoration() const
	{
		return ((VariableInfoPrivate*)this)->decoration.c_str();
	}

	const char* VariableInfo::default_value() const
	{
		return ((VariableInfoPrivate*)this)->default_value.c_str();
	}

	struct EnumItemPrivate : EnumItem
	{
		std::string name;
		int value;
	};

	const char* EnumItem::name() const
	{
		return ((EnumItemPrivate*)this)->name.c_str();
	}

	int EnumItem::value() const
	{
		return ((EnumItemPrivate*)this)->value;
	}

	struct EnumInfoPrivate : EnumInfo
	{
		TypeinfoDatabase* db;

		std::string name;
		std::vector<std::unique_ptr<EnumItemPrivate>> items;
	};

	TypeinfoDatabase* EnumInfo::db() const
	{
		return ((EnumInfoPrivate*)this)->db;
	}

	const char* EnumInfo::name() const
	{
		return ((EnumInfoPrivate*)this)->name.c_str();
	}

	uint EnumInfo::item_count() const
	{
		return ((EnumInfoPrivate*)this)->items.size();
	}

	EnumItem* EnumInfo::item(int idx) const
	{
		return ((EnumInfoPrivate*)this)->items[idx].get();
	}

	EnumItem* EnumInfo::find_item(const char* _name, int* out_idx) const
	{
		auto name = std::string(_name);
		auto& items = ((EnumInfoPrivate*)this)->items;
		for (auto i = 0; i < items.size(); i++)
		{
			if (items[i]->name == name)
			{
				if (out_idx)
					*out_idx = items[i]->value;
				return items[i].get();
			}
		}
		if (out_idx)
			*out_idx = -1;
		return nullptr;
	}

	EnumItem* EnumInfo::find_item(int value, int* out_idx) const
	{
		auto& items = ((EnumInfoPrivate*)this)->items;
		for (auto i = 0; i < items.size(); i++)
		{
			if (items[i]->value == value)
			{
				if (out_idx)
					*out_idx = i;
				return items[i].get();
			}
		}
		if (out_idx)
			*out_idx = -1;
		return nullptr;
	}

	EnumItem* EnumInfo::add_item(const char* name, int value)
	{
		auto i = new EnumItemPrivate;
		i->name = name;
		i->value = value;
		((EnumInfoPrivate*)this)->items.emplace_back(i);
		return i;
	}

	struct FunctionInfoPrivate : FunctionInfo
	{
		TypeinfoDatabase* db;

		std::string name;
		void* rva;
		TypeInfoPrivate* return_type;
		std::vector<TypeInfoPrivate*> parameter_types;
	};

	TypeinfoDatabase* FunctionInfo::db() const
	{
		return ((FunctionInfoPrivate*)this)->db;
	}

	const char* FunctionInfo::name() const
	{
		return ((FunctionInfoPrivate*)this)->name.c_str();
	}

	void* FunctionInfo::rva() const
	{
		return ((FunctionInfoPrivate*)this)->rva;
	}

	const TypeInfo* FunctionInfo::return_type() const
	{
		return ((FunctionInfoPrivate*)this)->return_type;
	}

	uint FunctionInfo::parameter_count() const
	{
		return ((FunctionInfoPrivate*)this)->parameter_types.size();
	}

	const TypeInfo* FunctionInfo::parameter_type(uint idx) const
	{
		return ((FunctionInfoPrivate*)this)->parameter_types[idx];
	}

	void FunctionInfo::add_parameter(const TypeInfo* type)
	{
		((FunctionInfoPrivate*)this)->parameter_types.push_back((TypeInfoPrivate*)type);
	}

	struct UdtInfoPrivate : UdtInfo
	{
		TypeinfoDatabase* db;

		TypeInfoPrivate* type;
		uint size;
		std::vector<std::unique_ptr<VariableInfoPrivate>> variables;
		std::vector<std::unique_ptr<FunctionInfoPrivate>> functions;

		VariableInfoPrivate* find_variable(const std::string& name, int* out_idx)
		{
			for (auto i = 0; i < variables.size(); i++)
			{
				if (variables[i]->name == name)
				{
					if (out_idx)
						*out_idx = i;
					return variables[i].get();
				}
			}
			if (out_idx)
				*out_idx = -1;
			return nullptr;
		}

		FunctionInfoPrivate* find_function(const std::string& name, int* out_idx)
		{
			for (auto i = 0; i < functions.size(); i++)
			{
				if (functions[i]->name == name)
				{
					if (out_idx)
						*out_idx = i;
					return functions[i].get();
				}
			}
			if (out_idx)
				*out_idx = -1;
			return nullptr;
		}
	};

	TypeinfoDatabase* UdtInfo::db() const
	{
		return ((UdtInfoPrivate*)this)->db;
	}

	const TypeInfo* UdtInfo::type() const
	{
		return ((UdtInfoPrivate*)this)->type;
	}

	uint UdtInfo::size() const
	{
		return ((UdtInfoPrivate*)this)->size;
	}

	uint UdtInfo::variable_count() const
	{
		return ((UdtInfoPrivate*)this)->variables.size();
	}

	VariableInfo* UdtInfo::variable(uint idx) const
	{
		return ((UdtInfoPrivate*)this)->variables[idx].get();
	}

	VariableInfo* UdtInfo::find_variable(const char* name, int* out_idx) const
	{
		return ((UdtInfoPrivate*)this)->find_variable(name, out_idx);
	}

	VariableInfo* UdtInfo::add_variable(const TypeInfo* type, const char* name, const char* decoration, uint offset, uint size)
	{
		auto v = new VariableInfoPrivate;
		v->type = (TypeInfoPrivate*)type;
		v->name = name;
		v->name_hash = FLAME_HASH(name);
		v->decoration = decoration;
		v->offset = offset;
		v->size = size;
		((UdtInfoPrivate*)this)->variables.emplace_back(v);
		return v;
	}

	uint UdtInfo::function_count() const
	{
		return ((UdtInfoPrivate*)this)->functions.size();
	}

	FunctionInfo* UdtInfo::function(uint idx) const
	{
		return ((UdtInfoPrivate*)this)->functions[idx].get();
	}

	FunctionInfo* UdtInfo::find_function(const char* name, int* out_idx) const
	{
		return ((UdtInfoPrivate*)this)->find_function(name, out_idx);
	}

	FunctionInfo* UdtInfo::add_function(const char* name, void* rva, const TypeInfo* return_type)
	{
		auto f = new FunctionInfoPrivate;
		f->return_type = (TypeInfoPrivate*)return_type;
		f->db = db();
		f->name = name;
		f->rva = rva;
		((UdtInfoPrivate*)this)->functions.emplace_back(f);
		return f;
	}

	static std::string format_name(const wchar_t* in, bool* pass_prefix = nullptr, bool* pass_$ = nullptr, std::string* attribute = nullptr)
	{
		static FLAME_SAL(prefix, "flame::");
		static FLAME_SAL(str_unsigned, "unsigned ");
		static FLAME_SAL(str_int64, "__int64");
		static FLAME_SAL(str_enum, "enum ");
		static FLAME_SAL(str_stringa, "String<char>");
		static FLAME_SAL(str_stringw, "String<wchar_t>");

		if (pass_prefix)
			*pass_prefix = false;
		if (pass_$)
			*pass_$ = false;

		auto str = w2s(in);

		if (pass_prefix)
		{
			if (str.compare(0, prefix.l, prefix.s) == 0)
				*pass_prefix = true;
			else
				return "";
		}

		{
			auto pos = str.find(str_unsigned.s, 0, str_unsigned.l);
			while (pos != std::string::npos)
			{
				str = str.replace(pos, str_unsigned.l, "u");
				pos = str.find(str_unsigned.s, 0, str_unsigned.l);
			}
		}
		{
			auto pos = str.find(str_int64.s, 0, str_int64.l);
			while (pos != std::string::npos)
			{
				str = str.replace(pos, str_int64.l, "longlong");
				pos = str.find(str_int64.s, 0, str_int64.l);
			}
		}
		{
			auto pos = str.find(str_enum.s, 0, str_enum.l);
			while (pos != std::string::npos)
			{
				str = str.replace(pos, str_enum.l, "");
				pos = str.find(str_enum.s, 0, str_enum.l);
			}
		}
		{
			static std::string eliminated_template_strs[] = {
				",std::allocator",
				",std::char_traits",
			};
			for (auto& s : eliminated_template_strs)
			{
				size_t pos;
				while ((pos = str.find(s)) != std::string::npos)
				{
					auto v = 0;
					auto l = s.size();
					do
					{
						auto ch = str[pos + l];
						if (ch == '<')
							v++;
						else if (ch == '>')
							v--;
						l++;
					} while (v > 0);
					str = str.replace(pos, l, "");
				}
			}
		}
		{
			auto pos = str.find(str_stringa.s, 0, str_stringa.l);
			while (pos != std::string::npos)
			{
				str = str.replace(pos, str_stringa.l, "StringA");
				pos = str.find(str_stringa.s, 0, str_stringa.l);
			}
		}
		{
			auto pos = str.find(str_stringw.s, 0, str_stringw.l);
			while (pos != std::string::npos)
			{
				str = str.replace(pos, str_stringw.l, "StringW");
				pos = str.find(str_stringw.s, 0, str_stringw.l);
			}
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
		auto pos_$ = head.find('$');
		if (pos_$ != std::string::npos)
		{
			if (pass_$)
				*pass_$ = true;

			if (attribute)
				*attribute = std::string(head.begin() + pos_$ + 1, head.end());
			head.resize(pos_$);
		}
		else if (pass_$)
			return "";

		str = head + tail;

		str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
		str.erase(std::remove(str.begin(), str.end(), '$'), str.end());

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

		const TypeInfo* get()
		{
			return TypeInfo::get(tag, base_name.c_str(), is_array);
		}
	};

	static TypeInfoDesc symbol_to_typeinfo(IDiaSymbol* s_type, const std::string& decoration)
	{
		DWORD dw;
		wchar_t* pwname;

		FLAME_SAL(array_str, "flame::Array");

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
			return TypeInfoDesc(decoration.find('m') != std::string::npos ? TypeEnumMulti : TypeEnumSingle, format_name(pwname));
		case SymTagBaseType:
			return TypeInfoDesc(TypeData, base_type_name(s_type));
		case SymTagPointerType:
		{
			std::string name;
			IDiaSymbol* pointer_type;
			s_type->get_type(&pointer_type);
			pointer_type->get_symTag(&dw);
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
				name = format_name(pwname);
				break;
			}
			pointer_type->Release();
			auto is_array = false;
			if (name.compare(0, array_str.l, array_str.s) == 0 && name.size() > array_str.l + 1)
			{
				is_array = true;
				name.erase(name.begin(), name.begin() + array_str.l + 1);
				name.erase(name.end() - 1);
			}
			return TypeInfoDesc(TypePointer, name, is_array);
		}
		case SymTagUDT:
		{
			s_type->get_name(&pwname);
			auto tag = TypeData;
			auto name = format_name(pwname);
			auto is_array = false;
			if (name.compare(0, array_str.l, array_str.s) == 0 && name.size() > array_str.l + 1)
			{
				is_array = true;
				name.erase(name.begin(), name.begin() + array_str.l + 1);
				name.erase(name.end() - 1);
			}
			return TypeInfoDesc(tag, name, is_array);
		}
		break;
		case SymTagFunctionArgType:
		{
			IDiaSymbol* s_arg_type;
			s_type->get_type(&s_arg_type);
			auto ret = symbol_to_typeinfo(s_arg_type, "");
			s_arg_type->Release();
			return ret;
		}
		}
	}

	struct FunctionDesc
	{
		void* rva;
		TypeInfoDesc ret_type;
		std::vector<TypeInfoDesc> parameters;
	};

	void symbol_to_function(IDiaSymbol* s_function, FunctionDesc& desc)
	{
		DWORD dw;
		ULONG ul;

		s_function->get_relativeVirtualAddress(&dw);
		desc.rva = (void*)dw;
		if (!desc.rva)
			return;

		IDiaSymbol* s_function_type;
		s_function->get_type(&s_function_type);

		IDiaSymbol* s_return_type;
		s_function_type->get_type(&s_return_type);
		desc.ret_type = symbol_to_typeinfo(s_return_type, "");
		s_return_type->Release();

		/*
		if (rva && attribute.find('c') != std::string::npos)
		{
			s_function->get_length(&ull);
			IDiaEnumLineNumbers* lines;

			if (SUCCEEDED(session->findLinesByRVA(dw, (DWORD)ull, &lines)))
			{
				IDiaLineNumber* line;
				DWORD src_file_id = -1;
				std::wstring source_file_name;
				DWORD line_num;

				uint min_line = 1000000;
				uint max_line = 0;

				while (SUCCEEDED(lines->Next(1, &line, &ul)) && (ul == 1))
				{
					if (src_file_id == -1)
					{
						line->get_sourceFileId(&src_file_id);

						BSTR fn;
						IDiaSourceFile* source_file;
						line->get_sourceFile(&source_file);
						source_file->get_fileName(&fn);
						source_file->Release();

						source_file_name = fn;
					}

					line->get_lineNumber(&line_num);
					if (min_line > line_num)
						min_line = line_num;
					if (max_line < line_num)
						max_line = line_num;

					line->Release();
				}

				lines->Release();

				if (max_line > min_line)
					code = w2s(source_file_name) + "#" + std::to_string(min_line) + ":" + std::to_string(max_line);
			}
		}
		*/

		IDiaEnumSymbols* s_parameters;
		s_function_type->findChildren(SymTagFunctionArgType, NULL, nsNone, &s_parameters);
		IDiaSymbol* s_parameter;
		while (SUCCEEDED(s_parameters->Next(1, &s_parameter, &ul)) && (ul == 1))
		{
			desc.parameters.push_back(symbol_to_typeinfo(s_parameter, ""));

			s_parameter->Release();
		}
		s_parameters->Release();

		s_function_type->Release();
	}

	struct TypeinfoDatabasePrivate : TypeinfoDatabase
	{
		void* module;
		std::wstring module_name;

		std::map<uint, std::unique_ptr<EnumInfoPrivate>> enums;
		std::map<uint, std::unique_ptr<UdtInfoPrivate>> udts;
		std::map<uint, std::unique_ptr<FunctionInfoPrivate>> functions;

		TypeinfoDatabasePrivate()
		{
			module = nullptr;
		}

		~TypeinfoDatabasePrivate()
		{
			if (module)
				free_module(module);
		}
	};

	static std::vector<TypeinfoDatabasePrivate*> global_dbs;
	uint extra_global_db_count;
	TypeinfoDatabase* const* extra_global_dbs;

	uint global_db_count()
	{
		return global_dbs.size();
	}

	TypeinfoDatabase* global_db(uint idx)
	{
		return global_dbs[idx];
	}

	void* TypeinfoDatabase::module() const
	{
		return ((TypeinfoDatabasePrivate*)this)->module;
	}

	const wchar_t* TypeinfoDatabase::module_name() const
	{
		return ((TypeinfoDatabasePrivate*)this)->module_name.c_str();
	}

	template <class T, class U>
	Array<T*> get_typeinfo_objects(const std::map<uint, std::unique_ptr<U>>& map)
	{
		auto ret = Array<T*>();
		ret.resize(map.size());
		auto i = 0;
		for (auto it = map.begin(); it != map.end(); it++)
		{
			ret[i] = it->second.get();
			i++;
		}
		return ret;
	}

	template <class T>
	T* find_typeinfo_object(const std::map<uint, std::unique_ptr<T>>& map, uint name_hash)
	{
		auto it = map.find(name_hash);
		if (it == map.end())
			return nullptr;
		return it->second.get();
	}

	Array<EnumInfo*> TypeinfoDatabase::get_enums()
	{
		return get_typeinfo_objects<EnumInfo>(((TypeinfoDatabasePrivate*)this)->enums);
	}

	EnumInfo* TypeinfoDatabase::find_enum(uint name_hash)
	{
		return find_typeinfo_object(((TypeinfoDatabasePrivate*)this)->enums, name_hash);
	}

	EnumInfo* TypeinfoDatabase::add_enum(const char* name)
	{
		auto e = new EnumInfoPrivate;
		e->db = this;
		e->name = name;
		((TypeinfoDatabasePrivate*)this)->enums.emplace(FLAME_HASH(name), e);
		return e;
	}

	Array<FunctionInfo*> TypeinfoDatabase::get_functions()
	{
		return get_typeinfo_objects<FunctionInfo>(((TypeinfoDatabasePrivate*)this)->functions);
	}

	FunctionInfo* TypeinfoDatabase::find_function(uint name_hash)
	{
		return find_typeinfo_object(((TypeinfoDatabasePrivate*)this)->functions, name_hash);
	}

	FunctionInfo* TypeinfoDatabase::add_function(const char* name, void* rva, const TypeInfo* return_type)
	{
		auto f = new FunctionInfoPrivate;
		f->return_type = (TypeInfoPrivate*)return_type;
		f->db = this;
		f->name = name;
		f->rva = rva;
		((TypeinfoDatabasePrivate*)this)->functions.emplace(FLAME_HASH(name), f);
		return f;
	}

	Array<UdtInfo*> TypeinfoDatabase::get_udts()
	{
		return get_typeinfo_objects<UdtInfo>(((TypeinfoDatabasePrivate*)this)->udts);
	}

	UdtInfo* TypeinfoDatabase::find_udt(uint name_hash)
	{
		return find_typeinfo_object(((TypeinfoDatabasePrivate*)this)->udts, name_hash);
	}

	UdtInfo* TypeinfoDatabase::add_udt(const TypeInfo* type, uint size)
	{
		auto u = new UdtInfoPrivate;
		u->type = (TypeInfoPrivate*)type;
		u->db = this;
		u->size = size;
		((TypeinfoDatabasePrivate*)this)->udts.emplace(type->hash(), u);
		return u;
	}

	static void com_init()
	{
		static bool inited = false;
		if (inited)
			return;
		assert(SUCCEEDED(CoInitialize(NULL)));
		inited = true;
	}

	void TypeinfoDatabase::collect(const wchar_t* module_filename, const wchar_t* _pdb_filename)
	{
		auto module_filename_path = std::filesystem::path(module_filename);

		com_init();

		CComPtr<IDiaDataSource> dia_source;
		if (FAILED(CoCreateInstance(CLSID_DiaSource, NULL, CLSCTX_INPROC_SERVER, __uuidof(IDiaDataSource), (void**)&dia_source)))
		{
			printf("dia not found\n");
			assert(0);
			return;
		}
		std::wstring pdb_filename;
		if (_pdb_filename)
			pdb_filename = _pdb_filename;
		else
			pdb_filename = module_filename_path.replace_extension(L".pdb");
		if (FAILED(dia_source->loadDataFromPdb(pdb_filename.c_str())))
		{
			printf("pdb failed to open: %s\n", w2s(pdb_filename).c_str());
			assert(0);
			return;
		}
		CComPtr<IDiaSession> session;
		if (FAILED(dia_source->openSession(&session)))
		{
			printf("session failed to open\n");
			assert(0);
			return;
		}
		CComPtr<IDiaSymbol> global;
		if (FAILED(session->get_globalScope(&global)))
		{
			printf("failed to get global\n");
			assert(0);
			return;
		}

		auto db = new TypeinfoDatabasePrivate;
		db->module_name = module_filename;
		extra_global_db_count = 1;
		extra_global_dbs = (TypeinfoDatabase**)&db;



		LONG l;
		ULONG ul;
		ULONGLONG ull;
		DWORD dw;
		wchar_t* pwname;

		// udts
		IDiaEnumSymbols* _udts;
		global->findChildren(SymTagUDT, NULL, nsNone, &_udts);
		IDiaSymbol* _udt;
		while (SUCCEEDED(_udts->Next(1, &_udt, &ul)) && (ul == 1))
		{
			_udt->get_name(&pwname);
			bool pass_prefix, pass_$;
			auto udt_name = format_name(pwname, &pass_prefix, &pass_$);

			if (pass_prefix && pass_$ && udt_name.find("(unnamed") == std::string::npos && udt_name.find("(lambda_") == std::string::npos)
			{
				auto udt_hash = TypeInfo::get_hash(TypeData, udt_name.c_str());
				if (!::flame::find_udt(udt_hash))
				{
					_udt->get_length(&ull);
					auto u = (UdtInfoPrivate*)db->add_udt(TypeInfo::get(TypeData, udt_name.c_str()), ull);

					IDiaEnumSymbols* _variables;
					_udt->findChildren(SymTagData, NULL, nsNone, &_variables);
					IDiaSymbol* _variable;
					while (SUCCEEDED(_variables->Next(1, &_variable, &ul)) && (ul == 1))
					{
						_variable->get_name(&pwname);
						std::string attribute;
						auto name = format_name(pwname, nullptr, &pass_$, &attribute);
						if (pass_$)
						{
							if (name[0] == '_')
								name.erase(name.begin());

							IDiaSymbol* s_type;
							_variable->get_type(&s_type);

							_variable->get_offset(&l);
							s_type->get_length(&ull);

							auto desc = symbol_to_typeinfo(s_type, attribute);
							if (desc.tag == TypeEnumSingle || desc.tag == TypeEnumMulti)
							{
								auto hash = FLAME_HASH(desc.base_name.c_str());
								if (!::flame::find_enum(hash))
								{
									auto e = db->add_enum(desc.base_name.c_str());

									IDiaEnumSymbols* items;
									s_type->findChildren(SymTagNull, NULL, nsNone, &items);
									IDiaSymbol* item;
									while (SUCCEEDED(items->Next(1, &item, &ul)) && (ul == 1))
									{
										VARIANT v;
										ZeroMemory(&v, sizeof(v));
										item->get_name(&pwname);
										item->get_value(&v);

										auto item_name = w2s(pwname);
										if (!sendswith(item_name, std::string("Max")) && !sendswith(item_name, std::string("Count")))
											e->add_item(item_name.c_str(), v.lVal);

										item->Release();
									}
									items->Release();
								}
							}
							u->add_variable(desc.get(), name.c_str(), attribute.c_str(), l, ull);

							s_type->Release();
						}
						_variable->Release();
					}
					_variables->Release();

					IDiaEnumSymbols* _functions;
					_udt->findChildren(SymTagFunction, NULL, nsNone, &_functions);
					IDiaSymbol* _function;
					auto name_no_namespace = udt_name;
					{
						auto pos = name_no_namespace.find_last_of(':');
						if (pos != std::string::npos)
							name_no_namespace.erase(name_no_namespace.begin(), name_no_namespace.begin() + pos + 1);
					}
					while (SUCCEEDED(_functions->Next(1, &_function, &ul)) && (ul == 1))
					{
						_function->get_name(&pwname);
						auto name = format_name(pwname, nullptr, &pass_$);
						if (pass_$)
						{
							if (name == name_no_namespace)
								name = "ctor";
							else if (name[0] == '~')
								name = "dtor";

							FunctionDesc desc;
							symbol_to_function(_function, desc);
							if (desc.rva)
							{
								auto f = (FunctionInfoPrivate*)u->add_function(name.c_str(), desc.rva, desc.ret_type.get());
								for (auto& p : desc.parameters)
									f->add_parameter(p.get());
							}
						}
						_function->Release();
					}
					_functions->Release();

					FunctionInfoPrivate* ctor = nullptr;
					FunctionInfoPrivate* dtor = nullptr;
					for (auto& f : u->functions)
					{
						if (f->name == "ctor" && f->parameter_types.empty())
							ctor = f.get();
						else if (f->name == "dtor")
							dtor = f.get();
						if (ctor && dtor)
							break;
					}
					if (ctor)
					{
						auto library = LoadLibraryW(module_filename);
						if (library)
						{
							auto obj = malloc(u->size);
							memset(obj, 0, u->size);

							cmf(p2f<MF_v_v>((char*)library + (uint)(ctor->rva)), obj);
							for (auto& i : u->variables)
							{
								auto type = i->type;
								auto tag = type->tag;
								if (!type->is_array && (tag == TypeEnumSingle || tag == TypeEnumMulti || tag == TypeData) &&
									i->decoration.find('o') == std::string::npos)
									i->default_value = type->serialize((char*)obj + i->offset, 1);
							}
							if (dtor)
								cmf(p2f<MF_v_v>((char*)library + (uint)(dtor->rva)), obj);

							free(obj);
							FreeLibrary(library);
						}
					}
				}
			}
			_udt->Release();
		}
		_udts->Release();

		// functions
		IDiaEnumSymbols* _functions;
		global->findChildren(SymTagFunction, NULL, nsNone, &_functions);
		IDiaSymbol* _function;
		while (SUCCEEDED(_functions->Next(1, &_function, &ul)) && (ul == 1))
		{
			_function->get_name(&pwname);
			bool pass_prefix, pass_$;
			std::string attribute;
			auto name = format_name(pwname, &pass_prefix, &pass_$, &attribute);
			if (pass_prefix && pass_$ && attribute.find("::") == std::string::npos /* not a member function */)
			{
				auto hash = FLAME_HASH(name.c_str());
				if (!::flame::find_function(hash))
				{
					FunctionDesc desc;
					symbol_to_function(_function, desc);
					if (desc.rva)
					{
						auto f = db->add_function(name.c_str(), desc.rva, desc.ret_type.get());
						for (auto& p : desc.parameters)
							f->add_parameter(p.get());
					}
				}
			}

			_function->Release();
		}
		_functions->Release();

		pugi::xml_document file;
		auto file_root = file.append_child("typeinfo");

		auto n_enums = file_root.append_child("enums");
		for (auto& _e : db->enums)
		{
			auto e = _e.second.get();

			auto n_enum = n_enums.append_child("enum");
			n_enum.append_attribute("name").set_value(e->name.c_str());

			auto n_items = n_enum.append_child("items");
			for (auto& i : e->items)
			{
				auto n_item = n_items.append_child("item");
				n_item.append_attribute("name").set_value(i->name.c_str());
				n_item.append_attribute("value").set_value(i->value);
			}
		}

		auto n_functions = file_root.append_child("functions");
		for (auto& _f : db->functions)
		{
			auto f = _f.second.get();

			auto n_function = n_functions.append_child("function");
			n_function.append_attribute("name").set_value(f->name.c_str());
			n_function.append_attribute("rva").set_value((uint)f->rva);
			n_function.append_attribute("return_type").set_value(f->return_type->name.c_str());
			if (!f->parameter_types.empty())
			{
				auto n_parameters = n_function.append_child("parameters");
				for (auto& p : f->parameter_types)
					n_parameters.append_child("parameter").append_attribute("type").set_value(p->name.c_str());
			}
		}

		auto n_udts = file_root.append_child("udts");
		for (auto& _u : db->udts)
		{
			auto u = _u.second.get();

			auto n_udt = n_udts.append_child("udt");
			n_udt.append_attribute("name").set_value(u->type->name.c_str());
			n_udt.append_attribute("size").set_value(u->size);

			auto n_items = n_udt.append_child("variables");
			for (auto& v : u->variables)
			{
				auto n_variable = n_items.append_child("variable");
				auto type = v->type;
				n_variable.append_attribute("type").set_value(type->name.c_str());
				n_variable.append_attribute("name").set_value(v->name.c_str());
				n_variable.append_attribute("decoration").set_value(v->decoration.c_str());
				n_variable.append_attribute("offset").set_value(v->offset);
				n_variable.append_attribute("size").set_value(v->size);
				if (!v->default_value.empty())
					n_variable.append_attribute("default_value").set_value(v->default_value.c_str());
			}

			auto n_functions = n_udt.append_child("functions");
			for (auto& f : u->functions)
			{
				auto n_function = n_functions.append_child("function");
				n_function.append_attribute("name").set_value(f->name.c_str());
				n_function.append_attribute("rva").set_value((uint)f->rva);
				n_function.append_attribute("return_type").set_value(f->return_type->name.c_str());
				if (!f->parameter_types.empty())
				{
					auto n_parameters = n_function.append_child("parameters");
					for (auto& p : f->parameter_types)
						n_parameters.append_child("parameter").append_attribute("type").set_value(p->name.c_str());
				}
			}
		}

		file.save_file(module_filename_path.replace_extension(L".typeinfo").c_str());

		extra_global_db_count = 0;
		extra_global_dbs = nullptr;

		TypeinfoDatabase::destroy(db);
	}

	TypeinfoDatabase* TypeinfoDatabase::load(const wchar_t* typeinfo_filename, bool add_to_global, bool load_with_module)
	{
		pugi::xml_document file;
		pugi::xml_node file_root;
		if (!file.load_file(typeinfo_filename) || (file_root = file.first_child()).name() != std::string("typeinfo"))
		{
			assert(0);
			return nullptr;
		}

		auto db = new TypeinfoDatabasePrivate;
		db->module_name = std::filesystem::path(typeinfo_filename).replace_extension(L".dll");
		extra_global_db_count = 1;
		extra_global_dbs = (TypeinfoDatabase**)&db;

		for (auto n_enum : file_root.child("enums"))
		{
			auto e = db->add_enum(n_enum.attribute("name").value());

			for (auto n_item : n_enum.child("items"))
				e->add_item(n_item.attribute("name").value(), n_item.attribute("value").as_int());
		}

		for (auto n_function : file_root.child("functions"))
		{
			auto f = db->add_function(n_function.attribute("name").value(), (void*)n_function.attribute("rva").as_uint(), TypeInfo::get(n_function.attribute("return_type").value()));
			for (auto n_parameter : n_function.child("parameters"))
				f->add_parameter(TypeInfo::get(n_parameter.attribute("type").value()));
		}

		for (auto n_udt : file_root.child("udts"))
		{
			auto u = (UdtInfoPrivate*)db->add_udt(TypeInfo::get(n_udt.attribute("name").value()), n_udt.attribute("size").as_uint());

			for (auto n_variable : n_udt.child("variables"))
			{
				auto v = (VariableInfoPrivate*)u->add_variable(TypeInfo::get(n_variable.attribute("type").value()), n_variable.attribute("name").value(),
					n_variable.attribute("decoration").value(), n_variable.attribute("offset").as_uint(), n_variable.attribute("size").as_uint());
				v->default_value = n_variable.attribute("default_value").value();
			}

			for (auto n_function : n_udt.child("functions"))
			{
				auto f = u->add_function(n_function.attribute("name").value(), (void*)n_function.attribute("rva").as_uint(), TypeInfo::get(n_function.attribute("return_type").value()));
				for (auto n_parameter : n_function.child("parameters"))
					f->add_parameter(TypeInfo::get(n_parameter.attribute("type").value()));
			}
		}

		extra_global_db_count = 0;
		extra_global_dbs = nullptr;

		if (load_with_module)
			db->module = load_module(db->module_name.c_str());
		if (add_to_global)
			global_dbs.push_back(db);

		return db;
	}

	void TypeinfoDatabase::destroy(TypeinfoDatabase* db)
	{
		delete (TypeinfoDatabasePrivate*)db;
	}
}
