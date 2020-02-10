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

	StringW get_curr_path()
	{
		wchar_t buf[260];
		GetCurrentDirectoryW(sizeof(buf), buf);
		return StringW(buf);
	}

	StringW get_app_path()
	{
		wchar_t buf[260];
		GetModuleFileNameW(nullptr, buf, sizeof(buf));
		return StringW(std::filesystem::path(buf).parent_path().wstring());
	}

	void set_curr_path(const wchar_t* p)
	{
		SetCurrentDirectoryW(p);
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
		uint flags;
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

	uint VariableInfo::flags() const
	{
		return ((VariableInfoPrivate*)this)->flags;
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

	VariableInfo* UdtInfo::add_variable(const TypeInfo* type, const char* name, uint flags, uint offset, uint size)
	{
		auto v = new VariableInfoPrivate;
		v->type = (TypeInfoPrivate*)type;
		v->name = name;
		v->name_hash = FLAME_HASH(name);
		v->flags = flags;
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

	static std::string format_type(const wchar_t* in, bool* is_array)
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

		const TypeInfo* get()
		{
			return TypeInfo::get(tag, base_name.c_str(), is_array);
		}
	};

	static TypeInfoDesc typeinfo_from_symbol(IDiaSymbol* s_type, uint flags)
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
			auto tag = TypeData;
			auto is_array = false;
			auto name = format_type(pwname, &is_array);
			return TypeInfoDesc(tag, name, is_array);
		}
		break;
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

	struct TypeinfoDatabasePrivate : TypeinfoDatabase
	{
		void* module;
		std::wstring module_name;

		std::map<uint, std::unique_ptr<EnumInfoPrivate>> enums;
		std::map<uint, std::unique_ptr<UdtInfoPrivate>> udts;

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
		std::filesystem::path pdb_filename;
		if (_pdb_filename)
			pdb_filename = module_filename_path.parent_path() / _pdb_filename;
		else
			pdb_filename = module_filename_path;
		pdb_filename.replace_extension(L".pdb");
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

		std::filesystem::path source_root;
		{
			DWORD h;
			auto version_size = GetFileVersionInfoSizeW(module_filename, &h);
			if (version_size > 0)
			{
				auto version_data = new char[version_size];
				if (GetFileVersionInfoW(module_filename, h, version_size, version_data))
				{
					void* d; uint s;
					VerQueryValue(version_data, "\\StringFileInfo\\040904b0\\FileDescription", &d, &s);
					source_root = (char*)d;
					source_root.make_preferred();
				}
				delete[] version_data;
			}
		}
		std::vector<std::filesystem::path> my_cpps;
		IDiaEnumSourceFiles* _source_files;
		IDiaSourceFile* _source_file;
		session->findFile(nullptr, nullptr, 0, &_source_files);
		while (SUCCEEDED(_source_files->Next(1, &_source_file, &ul)) && (ul == 1))
		{
			_source_file->get_fileName(&pwname);
			auto fn = std::filesystem::path(pwname);
			if (fn.extension() == L".cpp")
			{
				auto my_file = false;
				auto p = fn.parent_path();
				while (p.root_path() != p)
				{
					if (p == source_root)
					{
						my_file = true;
						break;
					}
					p = p.parent_path();
				}
				if (my_file)
					my_cpps.push_back(fn);
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
		};
		struct DesiredUDT
		{
			std::string name;
			std::string full_name;
			std::vector<DesiredVariable> variables;
			std::vector<DesiredFunction> functions;
		};
		std::vector<DesiredUDT> desired_udts;

		for (auto& cpp : my_cpps)
		{
			std::ifstream file(cpp);
			while (!file.eof())
			{
				std::string line;
				std::getline(file, line);
				static std::regex reg_R(R"(\sR\((.*)\))");
				static std::regex reg_RV(R"(\sRV\((.*)\))");
				static std::regex reg_RF(R"(\sRF\((\w+)\))");
				std::smatch res;
				if (std::regex_search(line, res, reg_R))
				{
					auto str = res[1].str();
					SUS::remove_spaces(str);
					auto sp = SUS::split(str, ',');
					DesiredUDT du;
					du.name = sp[0];
					for (auto i = 1; i < sp.size(); i++)
						du.full_name += sp[i] + "::";
					du.full_name += du.name;

					std::vector<std::string> udt_lines;
					auto braces_level = 0;
					while (!file.eof())
					{
						std::getline(file, line);
						udt_lines.push_back(line);
						for (auto& ch : line)
						{
							if (ch == '{')
								braces_level++;
							else if (ch == '}')
								braces_level--;
						}
						if (braces_level == 0)
							break;
					}

					for (auto& line : udt_lines)
					{
						if (std::regex_search(line, res, reg_RV))
						{
							auto str = res[1].str();
							SUS::remove_spaces(str);
							auto sp = SUS::split(str, ',');
							DesiredVariable v;
							v.flags = 0;
							v.name = sp[1];
							if (sp.size() > 2)
							{
								auto io = sp[2][0];
								if (io == 'i')
									v.flags |= VariableFlagInput;
								else if (io == 'o')
									v.flags |= VariableFlagOutput;
							}
							for (auto i = 3; i < sp.size(); i++)
							{
								if (sp[i] == "m")
									v.flags |= VariableFlagEnumMulti;
							}
							du.variables.push_back(v);
						}
						else if (std::regex_search(line, res, reg_RF))
						{
							auto str = res[1].str();
							SUS::remove_spaces(str);
							DesiredFunction f;
							f.name = str;
							du.functions.push_back(f);
						}
					}
					desired_udts.push_back(du);
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
					auto hash = TypeInfo::get_hash(TypeData, name.c_str());
					if (!::flame::find_udt(hash))
					{
						_udt->get_length(&ull);
						auto u = (UdtInfoPrivate*)db->add_udt(TypeInfo::get(TypeData, name.c_str()), ull);

						IDiaEnumSymbols* _variables;
						_udt->findChildren(SymTagData, NULL, nsNone, &_variables);
						IDiaSymbol* _variable;
						while (SUCCEEDED(_variables->Next(1, &_variable, &ul)) && (ul == 1))
						{
							_variable->get_name(&pwname);
							auto name = w2s(pwname);
							for (auto& v : du.variables)
							{
								if (v.name == name)
								{
									IDiaSymbol* s_type;
									_variable->get_type(&s_type);

									_variable->get_offset(&l);
									s_type->get_length(&ull);

									auto desc = typeinfo_from_symbol(s_type, v.flags);
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
												if (!SUS::ends_with(item_name, "Max") && !SUS::ends_with(item_name, "Count"))
													e->add_item(item_name.c_str(), v.lVal);

												item->Release();
											}
											items->Release();
										}
									}
									u->add_variable(desc.get(), name.c_str(), v.flags, l, ull);

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
							for (auto& f : du.functions)
							{
								if (f.name == name)
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

										auto f = (FunctionInfoPrivate*)u->add_function(name.c_str(), rva, ret_type.get());

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
									}

									break;
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
									if (!type->is_array && (tag == TypeEnumSingle || tag == TypeEnumMulti || tag == TypeData) && !(i->flags | VariableFlagOutput))
										i->default_value = type->serialize((char*)obj + i->offset, 1);
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
				n_variable.append_attribute("flags").set_value(v->flags);
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

		for (auto n_udt : file_root.child("udts"))
		{
			auto u = (UdtInfoPrivate*)db->add_udt(TypeInfo::get(n_udt.attribute("name").value()), n_udt.attribute("size").as_uint());

			for (auto n_variable : n_udt.child("variables"))
			{
				auto v = (VariableInfoPrivate*)u->add_variable(TypeInfo::get(n_variable.attribute("type").value()), n_variable.attribute("name").value(),
					n_variable.attribute("flags").as_uint(), n_variable.attribute("offset").as_uint(), n_variable.attribute("size").as_uint());
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
