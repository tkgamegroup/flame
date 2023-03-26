#pragma once

#include "foundation.h"

namespace flame
{
	/*
	*	E - Enum
	*	D - Data
	*	F - Function
	*	U - Udt
	*	R - std::pair
	*	T - std::tuple
	*	A - Array
	*	P - Pointer
	*	V - Vector
	*/
	enum TypeTag
	{
		TagE,
		TagD,
		TagF,
		TagU,
		TagR,
		TagT,
		TagPE,
		TagPD,
		TagPU,
		TagPR,
		TagPT,
		TagPVE,
		TagPVD,
		TagPVU,
		TagPVR,
		TagPVT,
		TagAE,
		TagAD,
		TagAU,
		TagVE,
		TagVD,
		TagVU,
		TagVR,
		TagVT,
		TagVPU,

		TagP_Beg = TagPE,
		TagP_End = TagPVT,
		TagA_Beg = TagAE,
		TagA_End = TagAU,
		TagV_Beg = TagVE,
		TagV_End = TagVPU,

		TagCount
	};

	enum DataType
	{
		DataVoid,
		DataBool,
		DataChar,
		DataInt,
		DataShort,
		DataLong, // long long
		DataFloat,
		DataString,
		DataWString,
		DataPath
	};

	template<typename T>
	concept basic_type = basic_std_type<T> || basic_math_type<T>;

	template<typename T>
	concept pointer_of_enum_type = pointer_type<T> && enum_type<std::remove_pointer_t<T>>;

	template<typename T>
	concept pointer_of_data_type = pointer_type<T> && basic_type<std::remove_pointer_t<T>>;

	template<typename T>
	concept pointer_of_udt_type = pointer_type<T> && !basic_type<std::remove_pointer_t<T>>;

	template<typename T>
	concept array_of_enum_type = array_type<T> && enum_type<std::remove_extent_t<T>>;

	template<typename T>
	concept array_of_data_type = array_type<T> && basic_type<std::remove_extent_t<T>>;

	template<typename T>
	concept array_of_udt_type = array_type<T> && !basic_type<std::remove_extent_t<T>>;

	template<typename T>
	concept vector_of_enum_type = vector_type<T> && enum_type<typename T::value_type>;

	template<typename T>
	concept vector_of_data_type = vector_type<T> && basic_type<typename T::value_type>;

	template<typename T>
	concept vector_of_udt_type = vector_type<T> && !basic_type<typename T::value_type> && !pair_type<typename T::value_type> && !tuple_type<typename T::value_type>;

	template<typename T>
	concept vector_of_pair_type = vector_type<T> && pair_type<typename T::value_type>;

	template<typename T>
	concept vector_of_tuple_type = vector_type<T> && tuple_type<typename T::value_type>;

	template<typename T>
	concept vector_of_pointer_of_udt_type = vector_type<T> && pointer_of_udt_type<typename T::value_type>;

	FLAME_FOUNDATION_API extern TypeInfoDataBase& tidb;

	inline int first_template_argument_pos(std::string_view name)
	{
		auto p = 0, lv = 0;
		while (p < name.size())
		{
			auto ch = name[p];
			if (ch == ',' && lv == 0)
				return p;
			else if (p == (int)name.size() - 1 && lv == 0)
				return p + 1;
			else if (ch == '<')
				lv++;
			else if (ch == '>')
				lv--;
			p++;
		}
		return -1;
	}

	inline std::vector<std::string_view> parse_template_arguments(std::string_view name)
	{
		std::vector<std::string_view> ret;
		auto off = 0;
		while (off < name.size())
		{
			auto n = first_template_argument_pos({ name.begin() + off, name.end() });
			if (n == -1)
			{
				ret.push_back(name.substr(off));
				break;
			}
			ret.push_back(name.substr(off, n));
			off += n + 1;
		}
		return ret;
	}

	struct TypeInfo
	{
		TypeTag tag;
		std::string name;
		uint size;

		inline static std::string format_name(std::string_view name)
		{
			auto ret = std::string(name);

			SUS::replace_all(ret, "enum ", "");
			SUS::replace_all(ret, "struct ", "");
			SUS::replace_all(ret, "class ", "");
			SUS::replace_all(ret, "unsigned ", "u");
			SUS::replace_all(ret, "__int64 ", "int64");
			SUS::replace_all(ret, "Private", "");
			SUS::strip_char(ret, ' ');

			if (SUS::strip_head_if(ret, "glm::vec<2,int,0>"))
				ret = "glm::ivec2" + ret;
			else if (SUS::strip_head_if(ret, "glm::vec<3,int,0>"))
				ret = "glm::ivec3" + ret;
			else if (SUS::strip_head_if(ret, "glm::vec<4,int,0>"))
				ret = "glm::ivec4" + ret;
			else if (SUS::strip_head_if(ret, "glm::vec<2,uint,0>"))
				ret = "glm::uvec2" + ret;
			else if (SUS::strip_head_if(ret, "glm::vec<3,uint,0>"))
				ret = "glm::uvec3" + ret;
			else if (SUS::strip_head_if(ret, "glm::vec<4,uint,0>"))
				ret = "glm::uvec4" + ret;
			else if (SUS::strip_head_if(ret, "glm::vec<2,uchar,0>"))
				ret = "glm::cvec2" + ret;
			else if (SUS::strip_head_if(ret, "glm::vec<3,uchar,0>"))
				ret = "glm::cvec3" + ret;
			else if (SUS::strip_head_if(ret, "glm::vec<4,uchar,0>"))
				ret = "glm::cvec4" + ret;
			else if (SUS::strip_head_if(ret, "glm::vec<2,float,0>"))
				ret = "glm::vec2" + ret;
			else if (SUS::strip_head_if(ret, "glm::vec<3,float,0>"))
				ret = "glm::vec3" + ret;
			else if (SUS::strip_head_if(ret, "glm::vec<4,float,0>"))
				ret = "glm::vec4" + ret;
			else if (SUS::strip_head_if(ret, "glm::mat<2,2,float,0>"))
				ret = "glm::mat2" + ret;
			else if (SUS::strip_head_if(ret, "glm::mat<3,3,float,0>"))
				ret = "glm::mat3" + ret;
			else if (SUS::strip_head_if(ret, "glm::mat<4,4,float,0>"))
				ret = "glm::mat4" + ret;
			else if (SUS::strip_head_if(ret, "glm::qua<float,0>"))
				ret = "glm::quat" + ret;
			else if (SUS::strip_head_if(ret, "std::basic_string<char,std::char_traits<char>,std::allocator<char>>"))
				ret = "std::string" + ret;
			else if (SUS::strip_head_if(ret, "std::basic_string<wchar_t,std::char_traits<wchar_t>,std::allocator<wchar_t>>"))
				ret = "std::wstring" + ret;
			else if (SUS::strip_head_if(ret, "std::pair<") || SUS::strip_head_if(ret, "std::tuple<"))
			{
				ret.pop_back();
				auto args = parse_template_arguments(ret);
				std::string temp;
				for (auto& a : args)
					temp += TypeInfo::format_name(a) + ';';
				ret = temp;
				ret.pop_back();
			}

			return ret;
		}

		inline static bool is_basic_type(std::string_view name);

		TypeInfo(TypeTag tag, std::string_view name, uint size) :
			tag(tag),
			name(name),
			size(size)
		{
		}

		virtual ~TypeInfo() {}

		virtual void* create(void* p = nullptr) const { if (!p) p = malloc(size); return p; }
		virtual void destroy(void* p, bool free_memory = true) const { if (free_memory) free(p); }
		virtual void copy(void* dst, const void* src) const { memcpy(dst, src ? src : get_v(), size); }
		virtual bool compare(const void* d1, const void* d2) const { return memcmp(d1, d2, size) == 0; }
		virtual std::string serialize(const void* p) const { return ""; }
		virtual void unserialize(const std::string& str, void* p) const {}

		virtual void* get_v() const { return nullptr; };
		virtual TypeInfo* get_wrapped() const { return nullptr; }
		virtual void call_getter(const FunctionInfo* fi, void* obj, void* dst) const {};
		virtual void call_setter(const FunctionInfo* fi, void* obj, void* src) const {};

		inline void* get_value(void* obj, int offset, const FunctionInfo* getter, bool use_copy = false) const
		{
			if (getter)
			{
				call_getter(getter, obj, nullptr);
				return get_v();
			}
			auto p = (char*)obj + offset;
			if (use_copy)
			{
				auto v = get_v();
				copy(v, p);
				return v;
			}
			return p;
		}

		inline void set_value(void* obj, int offset, const FunctionInfo* setter, void* src) const
		{
			if (setter)
				call_setter(setter, obj, src);
			else
				copy((char*)obj + offset, src);
		}

		inline static uint get_hash(TypeTag tag, std::string_view name)
		{
			auto ret = sh(name.data());
			ret ^= std::hash<uint>()(tag);
			return ret;
		}

		FLAME_FOUNDATION_API static TypeInfo* get(TypeTag tag, const std::string& name, TypeInfoDataBase& db = tidb);
		inline static				TypeInfo* get(const std::string& name, TypeInfoDataBase& db = tidb);
		FLAME_FOUNDATION_API static TypeInfo* void_type;

		template<enum_type T>
		static TypeInfo* get(TypeInfoDataBase& db = tidb)
		{
			static auto ret = get(TagE, format_name(typeid(T).name()), db);
			return ret;
		}

		template<basic_type T>
		static TypeInfo* get(TypeInfoDataBase& db = tidb)
		{
			static auto ret = get(TagD, format_name(typeid(T).name()), db);
			return ret;
		}

		template<typename T>
		static TypeInfo* get(TypeInfoDataBase& db = tidb)
		{
			static auto ret = get(TagU, format_name(typeid(T).name()), db);
			return ret;
		}

		template<pointer_of_enum_type T>
		static TypeInfo* get(TypeInfoDataBase& db = tidb)
		{
			static auto ret = get(TagPE, format_name(typeid(std::remove_pointer_t<T>).name()), db);
			return ret;
		}

		template<pointer_of_data_type T>
		static TypeInfo* get(TypeInfoDataBase& db = tidb)
		{
			static auto ret = get(TagPD, format_name(typeid(std::remove_pointer_t<T>).name()), db);
			return ret;
		}

		template<pointer_of_udt_type T>
		static TypeInfo* get(TypeInfoDataBase& db = tidb)
		{
			static auto ret = get(TagPU, format_name(typeid(std::remove_pointer_t<T>).name()), db);
			return ret;
		}

		template<array_of_enum_type T>
		static TypeInfo* get(TypeInfoDataBase& db = tidb)
		{
			static auto ret = get(TagAE, format_name(typeid(std::remove_extent_t<T>).name()) +
				std::format("[{}]", std::extent_v<T>), db);
			return ret;
		}

		template<array_of_data_type T>
		static TypeInfo* get(TypeInfoDataBase& db = tidb)
		{
			static auto ret = get(TagAD, format_name(typeid(std::remove_extent_t<T>).name()) +
				std::format("[{}]", std::extent_v<T>), db);
			return ret;
		}

		template<array_of_udt_type T>
		static TypeInfo* get(TypeInfoDataBase& db = tidb)
		{
			static auto ret = get(TagAU, format_name(typeid(std::remove_extent_t<T>).name()) +
				std::format("[{}]", std::extent_v<T>), db);
			return ret;
		}

		template<vector_of_enum_type T>
		static TypeInfo* get(TypeInfoDataBase& db = tidb)
		{
			static auto ret = get(TagVE, format_name(typeid(typename T::value_type).name()), db);
			return ret;
		}

		template<vector_of_data_type T>
		static TypeInfo* get(TypeInfoDataBase& db = tidb)
		{
			static auto ret = get(TagVD, format_name(typeid(typename T::value_type).name()), db);
			return ret;
		}

		template<vector_of_udt_type T>
		static TypeInfo* get(TypeInfoDataBase& db = tidb)
		{
			static auto ret = get(TagVU, format_name(typeid(typename T::value_type).name()), db);
			return ret;
		}

		template<vector_of_pair_type T>
		static TypeInfo* get(TypeInfoDataBase& db = tidb)
		{
			static auto ret = get(TagVR, format_name(typeid(typename T::value_type).name()), db);
			return ret;
		}

		template<vector_of_tuple_type T>
		static TypeInfo* get(TypeInfoDataBase& db = tidb)
		{
			static auto ret = get(TagVT, format_name(typeid(typename T::value_type).name()), db);
			return ret;
		}

		template<vector_of_pointer_of_udt_type T>
		static TypeInfo* get(TypeInfoDataBase& db = tidb)
		{
			static auto ret = get(TagVPU, format_name(typeid(std::remove_pointer_t<typename T::value_type>).name()), db);
			return ret;
		}

		template<typename T>
		inline static std::string serialize_t(const T& v, TypeInfoDataBase& db = tidb)
		{
			return get<T>(db)->serialize(&v);
		}

		template<typename T>
		inline static void unserialize_t(const std::string& str, T& v, TypeInfoDataBase& db = tidb)
		{
			return get<T>(db)->unserialize(str, &v);
		}

		inline EnumInfo* retrive_ei();
		inline UdtInfo* retrive_ui();
	};

	struct Metas
	{
		std::unordered_map<uint, std::string> items;

		void from_string(const std::string& str)
		{
			for (auto& t : SUS::split(SUS::get_trimed(str)))
			{
				auto sp = SUS::split(t, '=');
				uint key;
				if (isdigit(sp[0][0]))
					key = s2t<uint>(sp[0]);
				else
					key = sh(sp[0].c_str());
				items.emplace(key, sp.size() > 1 ? sp[1] : "");
			}
		}

		std::string to_string()
		{
			std::string ret;
			for (auto& i : items)
			{
				if (!ret.empty())
					ret += ' ';
				ret += str(i.first);
				if (!i.second.empty())
					ret += "=" + i.second;
			}
			return ret;
		}

		inline bool get(uint h, std::string* value = nullptr) const
		{
			auto it = items.find(h);
			if (it == items.end())
				return false;
			if (value)
				*value = it->second;
			return true;
		}
	};

	struct EnumItemInfo
	{
		EnumInfo* ei = nullptr;
		std::string name;
		uint name_hash;
		int value = -1;
	};

	struct EnumInfo
	{
		TypeInfoDataBase* db = nullptr;
		std::string name;
		uint name_hash;
		bool is_flags;
		std::vector<EnumItemInfo> items;
		void* library = nullptr;
		std::filesystem::path source_file;

		inline const EnumItemInfo* find_item(std::string_view name) const
		{
			for (auto& i : items)
			{
				if (i.name == name)
					return &i;
			}
			return nullptr;
		}

		inline const EnumItemInfo* find_item(int value) const
		{
			for (auto& i : items)
			{
				if (i.value == value)
					return &i;
			}
			return nullptr;
		}
	};

	struct FunctionInfo
	{
		TypeInfoDataBase* db = nullptr;
		UdtInfo* ui = nullptr;
		std::string name;
		uint name_hash;
		uint rva = 0;
		int voff = -1;
		bool is_static = false;
		TypeInfo* return_type = nullptr;
		std::vector<TypeInfo*> parameters;
		std::string code;
		Metas metas;
		void* library = nullptr;
		std::filesystem::path source_file;

		inline bool check(TypeInfo* ret, const std::initializer_list<TypeInfo*>& parms) const
		{
			if (return_type != ret || parameters.size() != parms.size())
				return false;
			auto i = 0;
			for (auto t : parms)
			{
				if (parameters[i] != t)
					return false;
				i++;
			}
			return true;
		}

		template<typename R, typename... Args>
		R call(void* obj, Args ...args) const
		{
			void* addr = nullptr;
			if (rva)
			{
				addr = (char*)library + rva;
				if (voff != -1)
					obj = *(void**)addr;
			}
			if (voff != -1 && obj)
			{
				auto vtbl = *(void**)obj;
				addr = ((void**)vtbl)[voff / 8];
			}
			if (obj)
			{
				struct T {};
				return ((*(T*)obj).*(a2f<R(T::*)(Args...)>(addr)))(args...);
			}
			return ((R(*)(Args...))addr)(args...);
		}
	};

	struct VariableInfo
	{
		UdtInfo* ui = nullptr;
		std::string name;
		uint name_hash;
		TypeInfo* type = nullptr;
		uint offset = 0;
		std::string default_value;
		Metas metas;
	};

	struct Attribute
	{
		UdtInfo* ui = nullptr;
		std::string name;
		uint name_hash;
		TypeInfo* type = nullptr;
		int var_idx = -1;
		int getter_idx = -1;
		int setter_idx = -1;
		std::string default_value;

		inline VariableInfo* var() const;
		inline int var_off() const;
		inline void* get_value(void* obj, bool use_copy = false) const;
		inline void set_value(void* obj, void* src = nullptr) const;
		inline std::string serialize(void* obj) const;
	};

	struct UdtInfo
	{
		TypeInfoDataBase* db = nullptr;
		std::string name;
		uint name_hash;
		uint size = 0;
		std::string base_class_name;
		bool is_pod = true;
		std::vector<VariableInfo> variables;
		std::vector<FunctionInfo> functions;
		std::vector<Attribute> attributes;
		std::unordered_map<uint, uint> variables_map;
		std::unordered_map<uint, uint> functions_map;
		std::unordered_map<uint, uint> attributes_map;
		void* library = nullptr;
		std::filesystem::path source_file;

		inline int find_variable_i(std::string_view name) const
		{
			for (auto i = 0; i < variables.size(); i++)
			{
				if (variables[i].name == name)
					return i;
			}
			return -1;
		}

		inline const VariableInfo* find_variable(std::string_view name) const
		{
			auto idx = find_variable_i(name);
			return idx == -1 ? nullptr : &variables[idx];
		}

		inline const VariableInfo* find_variable(uint name_hash) const
		{
			auto it = variables_map.find(name_hash);
			if (it == variables_map.end())
				return nullptr;
			return &variables[it->second];
		}

		template <class T>
		T& var_addr(void* obj, uint name_hash) const
		{
			return *(T*)((char*)obj + find_variable(name_hash)->offset);
		}

		inline int find_function_i(std::string_view name) const
		{
			for (auto i = 0; i < functions.size(); i++)
			{
				if (functions[i].name == name)
					return i;
			}
			return -1;
		}

		inline const FunctionInfo* find_function(std::string_view name) const
		{
			auto idx = find_function_i(name);
			return idx == -1 ? nullptr : &functions[idx];
		}

		inline const FunctionInfo* find_function(uint name_hash) const
		{
			auto it = functions_map.find(name_hash);
			if (it == functions_map.end())
				return nullptr;
			return &functions[it->second];
		}

		inline const Attribute* find_attribute(std::string_view name) const
		{
			for (auto& a : attributes)
			{
				if (a.name == name)
					return &a;
			}
			return nullptr;
		}

		inline const Attribute* find_attribute(uint name_hash) const
		{
			auto it = attributes_map.find(name_hash);
			if (it == attributes_map.end())
				return nullptr;
			return &attributes[it->second];
		}

		FLAME_FOUNDATION_API void* create_object(void* p = nullptr) const;
		FLAME_FOUNDATION_API void destroy_object(void* p, bool free_memory = true) const;
		FLAME_FOUNDATION_API void copy_object(void* dst, const void* src) const;

		// create a new type, and turn all pointer types(including vector of pointer ect.) into std::sting, so that you can use this type
		// to read/edit from file. The new type will NOT add to database, when you don't need it anymore, just delete it.
		FLAME_FOUNDATION_API UdtInfo* transform_to_serializable() const;
	};

	VariableInfo* Attribute::var() const
	{
		return var_idx == -1 ? nullptr : &ui->variables[var_idx];
	}

	int Attribute::var_off() const
	{
		auto vi = var();
		return vi ? vi->offset : -1;
	}

	void* Attribute::get_value(void* obj, bool use_copy) const
	{
		return type->get_value(obj, var_off(), getter_idx != -1 ? &ui->functions[getter_idx] : nullptr, use_copy);
	}

	void Attribute::set_value(void* obj, void* src) const
	{
		type->set_value(obj, var_off(), setter_idx != -1 ? &ui->functions[setter_idx] : nullptr, src);
	}

	std::string Attribute::serialize(void* obj) const
	{
		return type->serialize(get_value(obj));
	}

	struct TypeInfoDataBase
	{
		FLAME_FOUNDATION_API TypeInfoDataBase();

		std::map<uint, std::unique_ptr<TypeInfo>> typeinfos;

		std::map<uint, EnumInfo> enums;
		std::map<uint, FunctionInfo> functions;
		std::map<uint, UdtInfo> udts;

		TypeInfoDataBase& operator=(TypeInfoDataBase&& oth)
		{
			typeinfos = std::move(oth.typeinfos);
			enums = std::move(oth.enums);
			for (auto& ei : enums)
				ei.second.db = this;
			functions = std::move(oth.functions);
			for (auto& fi : functions)
				fi.second.db = this;
			udts = std::move(oth.udts);
			for (auto& ui : udts)
				ui.second.db = this;
			return *this;
		}

		FLAME_FOUNDATION_API void init_basic_types();
		FLAME_FOUNDATION_API bool load_from_string(const std::string& content, void* library = nullptr);
		FLAME_FOUNDATION_API void* load(const std::filesystem::path& filename); // return the loaded library
		FLAME_FOUNDATION_API void unload(void* library);
		FLAME_FOUNDATION_API std::string save_to_string();
		FLAME_FOUNDATION_API void save(const std::filesystem::path& filename);
	};

	inline EnumInfo* find_enum(uint hash, TypeInfoDataBase& db = tidb)
	{
		auto it = db.enums.find(hash);
		if (it != db.enums.end())
			return &it->second;
		if (&db != &tidb)
		{
			it = tidb.enums.find(hash);
			if (it != tidb.enums.end())
				return &it->second;
		}
		return nullptr;
	}

	inline UdtInfo* find_udt(uint hash, TypeInfoDataBase& db = tidb)
	{
		auto it = db.udts.find(hash);
		if (it != db.udts.end())
			return &it->second;
		if (&db != &tidb)
		{
			it = tidb.udts.find(hash);
			if (it != tidb.udts.end())
				return &it->second;
		}
		return nullptr;
	}

	inline bool TypeInfo::is_basic_type(std::string_view name)
	{
		return tidb.typeinfos.find(get_hash(TagD, name)) != tidb.typeinfos.end();
	}

	inline TypeInfo* TypeInfo::get(const std::string& name, TypeInfoDataBase& db)
	{
		auto name_hash = sh(name.c_str());
		if (auto ei = find_enum(name_hash); ei)
			return TypeInfo::get(TagE, name, db);
		if (auto ui = find_udt(name_hash); ui)
			return TypeInfo::get(TagU, name, db);
		return TypeInfo::get(TagD, name, db);
	}

	struct TypeInfo_Enum : TypeInfo
	{
		EnumInfo* ei = nullptr;

		thread_local static int v;

		TypeInfo_Enum(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo(TagE, base_name, sizeof(int))
		{
			ei = find_enum(sh(name.c_str()), db);
		}

		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type->tag == TagE);
			if (!dst) dst = &v;
			*(int*)dst = fi->call<int>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->return_type == TypeInfo::void_type && fi->parameters.size() == 1 && fi->parameters[0]->tag == TagE);
			if (!src) src = &v;
			fi->call<void>(obj, *(int*)src);
		}
	};

	struct TypeInfo_EnumSingle : TypeInfo_Enum
	{
		TypeInfo_EnumSingle(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo_Enum(base_name, db)
		{
		}

		std::string serialize(const void* p) const override
		{
			if (!p) p = &v;
			return ei->find_item(*(int*)p)->name;
		}
		void unserialize(const std::string& str, void* p) const override
		{
			if (!p) p = &v;
			*(int*)p = ei->find_item(str)->value;
		}
	};

	struct TypeInfo_EnumMulti : TypeInfo_Enum
	{
		TypeInfo_EnumMulti(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo_Enum(base_name, db)
		{
		}

		std::string serialize(const void* p) const override
		{
			std::string ret;
			if (!p) p = &v;
			auto vv = *(int*)p;
			for (auto i = 0; i < ei->items.size(); i++)
			{
				if ((vv & 1) == 1)
				{
					if (!ret.empty())
						ret += '|';
					ret += ei->find_item(1 << i)->name;
				}
				vv >>= 1;
			}
			return ret;
		}
		void unserialize(const std::string& str, void* p) const override
		{
			auto vv = 0;
			auto sp = SUS::split(str, '|');
			for (auto& t : sp)
				vv |= ei->find_item(t)->value;
			if (!p) p = &v;
			*(int*)p = vv;
		}
	};

	struct TypeInfo_Data : TypeInfo
	{
		DataType data_type = DataVoid;
		bool is_signed = true;
		uint vec_size = 1;
		uint col_size = 1;

		TypeInfo_Data(std::string_view name, uint _size) :
			TypeInfo(TagD, name, _size)
		{
			static std::regex reg(R"(Dummy_(\d+)$)");
			std::smatch res;
			auto str = std::string(name);
			std::regex_search(str, res, reg);
			if (res.size() > 1)
			{
				size = stoi(res[1].str());
			}
		}
	};

	struct TypeInfo_void : TypeInfo_Data
	{
		TypeInfo_void() :
			TypeInfo_Data("void", 0)
		{
			data_type = DataVoid;
		}
	};

	struct TypeInfo_bool : TypeInfo_Data
	{
		thread_local static bool v;

		TypeInfo_bool() :
			TypeInfo_Data("bool", sizeof(bool))
		{
			data_type = DataBool;
		}

		std::string serialize(const void* p) const override
		{
			if (!p) p = &v;
			return *(bool*)p ? "true" : "false";
		}
		void unserialize(const std::string& str, void* p) const override
		{
			if (!p) p = &v;
			if (str == "false")
				*(bool*)p = false;
			else if (str == "true")
				*(bool*)p = true;
			else
				*(bool*)p = s2t<int>(str) != 0;
		}
		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(bool*)dst = fi->call<bool>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { (TypeInfo* const)this }));
			if (!src) src = &v;
			fi->call<void>(obj, *(bool*)src);
		}
	};

	struct TypeInfo_char : TypeInfo_Data
	{
		thread_local static char v;

		TypeInfo_char() :
			TypeInfo_Data("char", sizeof(char))
		{
			data_type = DataChar;
		}

		std::string serialize(const void* p) const override
		{
			if (!p) p = &v;
			return str(*(char*)p);
		}
		void unserialize(const std::string& str, void* p) const override
		{
			if (!p) p = &v;
			*(char*)p = s2t<char>(str);
		}
		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(char*)dst = fi->call<char>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { (TypeInfo* const)this }));
			if (!src) src = &v;
			fi->call<void>(obj, *(char*)src);
		}
	};

	struct TypeInfo_uchar : TypeInfo_Data
	{
		thread_local static uchar v;

		TypeInfo_uchar() :
			TypeInfo_Data("uchar", sizeof(uchar))
		{
			data_type = DataChar;
			is_signed = false;
		}

		std::string serialize(const void* p) const override
		{
			if (!p) p = &v;
			return str(*(uchar*)p);
		}
		void unserialize(const std::string& str, void* p) const override
		{
			if (!p) p = &v;
			*(uchar*)p = s2t<uchar>(str);
		}
		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(uchar*)dst = fi->call<uchar>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { (TypeInfo* const)this }));
			if (!src) src = &v;
			fi->call<void>(obj, *(uchar*)src);
		}
	};

	struct TypeInfo_short : TypeInfo_Data
	{
		thread_local static short v;

		TypeInfo_short() :
			TypeInfo_Data("short", sizeof(short))
		{
			data_type = DataShort;
		}

		std::string serialize(const void* p) const override
		{
			if (!p) p = &v;
			return str(*(short*)p);
		}
		void unserialize(const std::string& str, void* p) const override
		{
			if (!p) p = &v;
			*(short*)p = s2t<short>(str);
		}
		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(short*)dst = fi->call<short>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { (TypeInfo* const)this }));
			if (!src) src = &v;
			fi->call<void>(obj, *(short*)src);
		}
	};

	struct TypeInfo_ushort : TypeInfo_Data
	{
		thread_local static ushort v;

		TypeInfo_ushort() :
			TypeInfo_Data("ushort", sizeof(ushort))
		{
			data_type = DataShort;
			is_signed = false;
		}

		std::string serialize(const void* p) const override
		{
			if (!p) p = &v;
			return str(*(ushort*)p);
		}
		void unserialize(const std::string& str, void* p) const override
		{
			if (!p) p = &v;
			*(ushort*)p = s2t<ushort>(str);
		}
		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(ushort*)dst = fi->call<ushort>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { (TypeInfo* const)this }));
			if (!src) src = &v;
			fi->call<void>(obj, *(ushort*)src);
		}
	};

	struct TypeInfo_int : TypeInfo_Data
	{
		thread_local static int v;

		TypeInfo_int() :
			TypeInfo_Data("int", sizeof(int))
		{
			data_type = DataInt;
		}

		std::string serialize(const void* p) const override
		{
			if (!p) p = &v;
			return str(*(int*)p);
		}
		void unserialize(const std::string& str, void* p) const override
		{
			if (!p) p = &v;
			*(int*)p = s2t<int>(str);
		}
		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(int*)dst = fi->call<int>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { (TypeInfo* const)this }));
			if (!src) src = &v;
			fi->call<void>(obj, *(int*)src);
		}
	};

	struct TypeInfo_uint : TypeInfo_Data
	{
		thread_local static uint v;

		TypeInfo_uint() :
			TypeInfo_Data("uint", sizeof(uint))
		{
			data_type = DataInt;
			is_signed = false;
		}

		std::string serialize(const void* p) const override
		{
			if (!p) p = &v;
			return str(*(uint*)p);
		}
		void unserialize(const std::string& str, void* p) const override
		{
			if (!p) p = &v;
			*(uint*)p = s2t<uint>(str);
		}
		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(uint*)dst = fi->call<uint>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { (TypeInfo* const)this }));
			if (!src) src = &v;
			fi->call<void>(obj, *(uint*)src);
		}
	};

	struct TypeInfo_int64 : TypeInfo_Data
	{
		thread_local static int64 v;

		TypeInfo_int64() :
			TypeInfo_Data("int64", sizeof(int64))
		{
			data_type = DataLong;
		}

		std::string serialize(const void* p) const override
		{
			if (!p) p = &v;
			return str(*(int64*)p);
		}
		void unserialize(const std::string& str, void* p) const override
		{
			if (!p) p = &v;
			*(int64*)p = s2t<int64>(str);
		}
		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(int64*)dst = fi->call<int64>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { (TypeInfo* const)this }));
			if (!src) src = &v;
			fi->call<void>(obj, *(int64*)src);
		}
	};

	struct TypeInfo_uint64 : TypeInfo_Data
	{
		thread_local static uint64 v;

		TypeInfo_uint64() :
			TypeInfo_Data("uint64", sizeof(uint64))
		{
			data_type = DataLong;
			is_signed = false;
		}

		std::string serialize(const void* p) const override
		{
			if (!p) p = &v;
			return str(*(uint64*)p);
		}
		void unserialize(const std::string& str, void* p) const override
		{
			if (!p) p = &v;
			*(uint64*)p = s2t<uint64>(str);
		}
		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(uint64*)dst = fi->call<uint64>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { (TypeInfo* const)this }));
			if (!src) src = &v;
			fi->call<void>(obj, *(uint64*)src);
		}
	};

	struct TypeInfo_float : TypeInfo_Data
	{
		thread_local static float v;

		TypeInfo_float() :
			TypeInfo_Data("float", sizeof(float))
		{
			data_type = DataFloat;
		}

		std::string serialize(const void* p) const override
		{
			if (!p) p = &v;
			return str(*(float*)p);
		}
		void unserialize(const std::string& str, void* p) const override
		{
			if (!p) p = &v;
			*(float*)p = s2t<float>(str);
		}
		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(float*)dst = fi->call<float>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { (TypeInfo* const)this }));
			if (!src) src = &v;
			fi->call<void>(obj, *(float*)src);
		}
	};

	struct TypeInfo_cvec2 : TypeInfo_Data
	{
		thread_local static cvec2 v;

		TypeInfo_cvec2() :
			TypeInfo_Data("glm::cvec2", sizeof(cvec2))
		{
			data_type = DataChar;
			is_signed = false;
			vec_size = 2;
		}

		std::string serialize(const void* p) const override
		{
			if (!p) p = &v;
			return str(*(cvec2*)p);
		}
		void unserialize(const std::string& str, void* p) const override
		{
			if (!p) p = &v;
			*(cvec2*)p = s2t<2, uchar>(str);
		}
		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(cvec2*)dst = fi->call<cvec2>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { TypeInfo::get<cvec2*>() }));
			if (!src) src = &v;
			fi->call<void, const cvec2&>(obj, *(cvec2*)src);
		}
	};

	struct TypeInfo_cvec3 : TypeInfo_Data
	{
		thread_local static cvec3 v;

		TypeInfo_cvec3() :
			TypeInfo_Data("glm::cvec3", sizeof(cvec3))
		{
			data_type = DataChar;
			is_signed = false;
			vec_size = 3;
		}

		std::string serialize(const void* p) const override
		{
			if (!p) p = &v;
			return str(*(cvec3*)p);
		}
		void unserialize(const std::string& str, void* p) const override
		{
			if (!p) p = &v;
			*(cvec3*)p = s2t<3, uchar>(str);
		}
		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(cvec3*)dst = fi->call<cvec3>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { TypeInfo::get<cvec3*>() }));
			if (!src) src = &v;
			fi->call<void, const cvec3&>(obj, *(cvec3*)src);
		}
	};

	struct TypeInfo_cvec4 : TypeInfo_Data
	{
		thread_local static cvec4 v;

		TypeInfo_cvec4() :
			TypeInfo_Data("glm::cvec4", sizeof(cvec4))
		{
			data_type = DataChar;
			is_signed = false;
			vec_size = 4;
		}

		std::string serialize(const void* p) const override
		{
			if (!p) p = &v;
			return str(*(cvec4*)p);
		}
		void unserialize(const std::string& str, void* p) const override
		{
			if (!p) p = &v;
			*(cvec4*)p = s2t<4, uchar>(str);
		}
		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(cvec4*)dst = fi->call<cvec4>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { TypeInfo::get<cvec4*>() }));
			if (!src) src = &v;
			fi->call<void, const cvec4&>(obj, *(cvec4*)src);
		}
	};

	struct TypeInfo_ivec2 : TypeInfo_Data
	{
		thread_local static ivec2 v;

		TypeInfo_ivec2() :
			TypeInfo_Data("glm::ivec2", sizeof(ivec2))
		{
			data_type = DataInt;
			vec_size = 2;
		}

		std::string serialize(const void* p) const override
		{
			if (!p) p = &v;
			return str(*(ivec2*)p);
		}
		void unserialize(const std::string& str, void* p) const override
		{
			if (!p) p = &v;
			*(ivec2*)p = s2t<2, int>(str);
		}
		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(ivec2*)dst = fi->call<ivec2>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { TypeInfo::get<ivec2*>() }));
			if (!src) src = &v;
			fi->call<void, const ivec2&>(obj, *(ivec2*)src);
		}
	};

	struct TypeInfo_ivec3 : TypeInfo_Data
	{
		thread_local static ivec3 v;

		TypeInfo_ivec3() :
			TypeInfo_Data("glm::ivec3", sizeof(ivec3))
		{
			data_type = DataInt;
			vec_size = 3;
		}

		std::string serialize(const void* p) const override
		{
			if (!p) p = &v;
			return str(*(ivec3*)p);
		}
		void unserialize(const std::string& str, void* p) const override
		{
			if (!p) p = &v;
			*(ivec3*)p = s2t<3, int>(str);
		}
		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(ivec3*)dst = fi->call<ivec3>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { TypeInfo::get<ivec3*>() }));
			if (!src) src = &v;
			fi->call<void, const ivec3&>(obj, *(ivec3*)src);
		}
	};

	struct TypeInfo_ivec4 : TypeInfo_Data
	{
		thread_local static ivec4 v;

		TypeInfo_ivec4() :
			TypeInfo_Data("glm::ivec4", sizeof(ivec4))
		{
			data_type = DataInt;
			vec_size = 4;
		}

		std::string serialize(const void* p) const override
		{
			if (!p) p = &v;
			return str(*(ivec4*)p);
		}
		void unserialize(const std::string& str, void* p) const override
		{
			if (!p) p = &v;
			*(ivec4*)p = s2t<4, int>(str);
		}
		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(ivec4*)dst = fi->call<ivec4>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { TypeInfo::get<ivec4*>() }));
			if (!src) src = &v;
			fi->call<void, const ivec4&>(obj, *(ivec4*)src);
		}
	};

	struct TypeInfo_uvec2 : TypeInfo_Data
	{
		thread_local static uvec2 v;

		TypeInfo_uvec2() :
			TypeInfo_Data("glm::uvec2", sizeof(uvec2))
		{
			data_type = DataInt;
			is_signed = false;
			vec_size = 2;
		}

		std::string serialize(const void* p) const override
		{
			if (!p) p = &v;
			return str(*(uvec2*)p);
		}
		void unserialize(const std::string& str, void* p) const override
		{
			if (!p) p = &v;
			*(uvec2*)p = s2t<2, uint>(str);
		}
		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(uvec2*)dst = fi->call<uvec2>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { TypeInfo::get<uvec2*>() }));
			if (!src) src = &v;
			fi->call<void, const uvec2&>(obj, *(uvec2*)src);
		}
	};

	struct TypeInfo_uvec3 : TypeInfo_Data
	{
		thread_local static uvec3 v;

		TypeInfo_uvec3() :
			TypeInfo_Data("glm::uvec3", sizeof(uvec3))
		{
			data_type = DataInt;
			is_signed = false;
			vec_size = 3;
		}

		std::string serialize(const void* p) const override
		{
			if (!p) p = &v;
			return str(*(uvec3*)p);
		}
		void unserialize(const std::string& str, void* p) const override
		{
			if (!p) p = &v;
			*(uvec3*)p = s2t<3, uint>(str);
		}
		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(uvec3*)dst = fi->call<uvec3>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { TypeInfo::get<uvec3*>() }));
			if (!src) src = &v;
			fi->call<void, const uvec3&>(obj, *(uvec3*)src);
		}
	};

	struct TypeInfo_uvec4 : TypeInfo_Data
	{
		thread_local static uvec4 v;

		TypeInfo_uvec4() :
			TypeInfo_Data("glm::uvec4", sizeof(uvec4))
		{
			data_type = DataInt;
			is_signed = false;
			vec_size = 4;
		}

		std::string serialize(const void* p) const override
		{
			if (!p) p = &v;
			return str(*(uvec4*)p);
		}
		void unserialize(const std::string& str, void* p) const override
		{
			if (!p) p = &v;
			*(uvec4*)p = s2t<4, uint>(str);
		}
		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(uvec4*)dst = fi->call<uvec4>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { TypeInfo::get<uvec4*>() }));
			if (!src) src = &v;
			fi->call<void, const uvec4&>(obj, *(uvec4*)src);
		}
	};

	struct TypeInfo_vec2 : TypeInfo_Data
	{
		thread_local static vec2 v;

		TypeInfo_vec2() :
			TypeInfo_Data("glm::vec2", sizeof(vec2))
		{
			data_type = DataFloat;
			vec_size = 2;
		}

		std::string serialize(const void* p) const override
		{
			if (!p) p = &v;
			return str(*(vec2*)p);
		}
		void unserialize(const std::string& str, void* p) const override
		{
			if (!p) p = &v;
			*(vec2*)p = s2t<2, float>(str);
		}
		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(vec2*)dst = fi->call<vec2>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { TypeInfo::get<vec2*>() }));
			if (!src) src = &v;
			fi->call<void, const vec2&>(obj, *(vec2*)src);
		}
	};

	struct TypeInfo_vec3 : TypeInfo_Data
	{
		thread_local static vec3 v;

		TypeInfo_vec3() :
			TypeInfo_Data("glm::vec3", sizeof(vec3))
		{
			data_type = DataFloat;
			vec_size = 3;
		}

		std::string serialize(const void* p) const override
		{
			if (!p) p = &v;
			return str(*(vec3*)p);
		}
		void unserialize(const std::string& str, void* p) const override
		{
			if (!p) p = &v;
			*(vec3*)p = s2t<3, float>(str);
		}
		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(vec3*)dst = fi->call<vec3>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { TypeInfo::get<vec3*>() }));
			if (!src) src = &v;
			fi->call<void, const vec3&>(obj, *(vec3*)src);
		}
	};

	struct TypeInfo_vec4 : TypeInfo_Data
	{
		thread_local static vec4 v;

		TypeInfo_vec4() :
			TypeInfo_Data("glm::vec4", sizeof(vec4))
		{
			data_type = DataFloat;
			vec_size = 4;
		}

		std::string serialize(const void* p) const override
		{
			if (!p) p = &v;
			return str(*(vec4*)p);
		}
		void unserialize(const std::string& str, void* p) const override
		{
			if (!p) p = &v;
			*(vec4*)p = s2t<4, float>(str);
		}
		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(vec4*)dst = fi->call<vec4>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { TypeInfo::get<vec4*>() }));
			if (!src) src = &v;
			fi->call<void, const vec4&>(obj, *(vec4*)src);
		}
	};

	struct TypeInfo_mat2 : TypeInfo_Data
	{
		thread_local static mat2 v;

		TypeInfo_mat2() :
			TypeInfo_Data("glm::mat2", sizeof(mat2))
		{
			data_type = DataFloat;
			vec_size = 2;
			col_size = 2;
		}

		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(mat2*)dst = fi->call<mat2>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { TypeInfo::get<mat2*>() }));
			if (!src) src = &v;
			fi->call<void, const mat2&>(obj, *(mat2*)src);
		}
	};

	struct TypeInfo_mat3 : TypeInfo_Data
	{
		thread_local static mat3 v;

		TypeInfo_mat3() :
			TypeInfo_Data("glm::mat3", sizeof(mat3))
		{
			data_type = DataFloat;
			vec_size = 3;
			col_size = 3;
		}

		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(mat3*)dst = fi->call<mat3>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { TypeInfo::get<mat3*>() }));
			if (!src) src = &v;
			fi->call<void, const mat3&>(obj, *(mat3*)src);
		}
	};

	struct TypeInfo_mat4 : TypeInfo_Data
	{
		thread_local static mat4 v;

		TypeInfo_mat4() :
			TypeInfo_Data("glm::mat4", sizeof(mat4))
		{
			data_type = DataFloat;
			vec_size = 4;
			col_size = 4;
		}

		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(mat4*)dst = fi->call<mat4>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { TypeInfo::get<mat4*>() }));
			if (!src) src = &v;
			fi->call<void, const mat4&>(obj, *(mat4*)src);
		}
	};

	struct TypeInfo_quat : TypeInfo_Data
	{
		thread_local static quat v;

		TypeInfo_quat() :
			TypeInfo_Data("glm::quat", sizeof(quat))
		{
		}

		std::string serialize(const void* p) const override
		{
			if (!p) p = &v;
			return str(*(vec4*)p);
		}
		void unserialize(const std::string& str, void* p) const override
		{
			if (!p) p = &v;
			*(vec4*)p = s2t<4, float>(str);
		}

		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(quat*)dst = fi->call<quat>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { TypeInfo::get<quat*>() }));
			if (!src) src = &v;
			fi->call<void, const quat&>(obj, *(quat*)src);
		}
	};

	struct TypeInfo_string : TypeInfo_Data
	{
		thread_local static std::string v;

		TypeInfo_string() :
			TypeInfo_Data("std::string", sizeof(std::string))
		{
			data_type = DataString;
		}

		void* create(void* p = nullptr) const override
		{
			if (!p)
				return new std::string;
			new(p) std::string;
			return p;
		}
		void destroy(void* p, bool free_memory = true) const override
		{
			((std::string*)p)->~basic_string();
			if (free_memory)
				free(p);
		}
		void copy(void* dst, const void* src) const override
		{
			*(std::string*)dst = *(src ? (std::string*)src : &v);
		}
		bool compare(const void* d1, const void* d2) const override
		{
			return *(std::string*)d1 == *(std::string*)d2;
		}
		std::string serialize(const void* p) const override
		{
			if (!p) p = &v;
			return *(std::string*)p;
		}
		void unserialize(const std::string& str, void* p) const override
		{
			if (!p) p = &v;
			*(std::string*)p = str;
		}
		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(std::string*)dst = fi->call<std::string>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { TypeInfo::get<std::string*>() }));
			if (!src) src = &v;
			fi->call<void, const std::string&>(obj, *(std::string*)src);
		}
	};

	struct TypeInfo_wstring : TypeInfo_Data
	{
		thread_local static std::wstring v;

		TypeInfo_wstring() :
			TypeInfo_Data("std::wstring", sizeof(std::string))
		{
			data_type = DataWString;
		}

		void* create(void* p = nullptr) const override
		{
			if (!p)
				return new std::wstring;
			new(p) std::wstring;
			return p;
		}
		void destroy(void* p, bool free_memory = true) const override
		{
			((std::wstring*)p)->~basic_string();
			if (free_memory)
				free(p);
		}
		void copy(void* dst, const void* src) const override
		{
			*(std::wstring*)dst = *(src ? (std::wstring*)src : &v);
		}
		bool compare(const void* d1, const void* d2) const override
		{
			return *(std::wstring*)d1 == *(std::wstring*)d2;
		}
		std::string serialize(const void* p) const override
		{
			if (!p) p = &v;
			return w2s(*(std::wstring*)p);
		}
		void unserialize(const std::string& str, void* p) const override
		{
			if (!p) p = &v;
			*(std::wstring*)p = s2w(str);
		}
		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(std::wstring*)dst = fi->call<std::wstring>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { TypeInfo::get<std::wstring*>() }));
			if (!src) src = &v;
			fi->call<void, const std::wstring&>(obj, *(std::wstring*)src);
		}
	};

	struct TypeInfo_path : TypeInfo_Data
	{
		thread_local static std::filesystem::path v;

		TypeInfo_path() :
			TypeInfo_Data("std::filesystem::path", sizeof(std::filesystem::path))
		{
			data_type = DataPath;
		}

		void* create(void* p = nullptr) const override
		{
			if (!p)
				return new std::filesystem::path;
			new(p) std::filesystem::path;
			return p;
		}
		void destroy(void* p, bool free_memory = true) const override
		{
			((std::filesystem::path*)p)->~path();
			if (free_memory)
				free(p);
		}
		void copy(void* dst, const void* src) const override
		{
			*(std::filesystem::path*)dst = *(src ? (std::filesystem::path*)src : &v);
		}
		bool compare(const void* d1, const void* d2) const override
		{
			return *(std::filesystem::path*)d1 == *(std::filesystem::path*)d2;
		}
		std::string serialize(const void* p) const override
		{
			if (!p) p = &v;
			return (*(std::filesystem::path*)p).string();
		}
		void unserialize(const std::string& str, void* p) const override
		{
			if (!p) p = &v;
			*(std::filesystem::path*)p = str;
		}
		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(std::filesystem::path*)dst = fi->call<std::filesystem::path>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { TypeInfo::get<std::filesystem::path*>() }));
			if (!src) src = &v;
			fi->call<void, const std::filesystem::path&>(obj, *(std::filesystem::path*)src);
		}
	};

	struct TypeInfo_Rect : TypeInfo_Data
	{
		thread_local static Rect v;

		TypeInfo_Rect() :
			TypeInfo_Data("flame::Rect", sizeof(Rect))
		{
			data_type = DataFloat;
			vec_size = 2;
			col_size = 2;
		}

		std::string serialize(const void* p) const override
		{
			if (!p) p = &v;
			return str(*(vec4*)p);
		}
		void unserialize(const std::string& str, void* p) const override
		{
			if (!p) p = &v;
			*(vec4*)p = s2t<4, float>(str);
		}
		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(Rect*)dst = fi->call<Rect>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { TypeInfo::get<Rect*>() }));
			if (!src) src = &v;
			fi->call<void, const Rect&>(obj, *(Rect*)src);
		}
	};

	struct TypeInfo_AABB : TypeInfo_Data
	{
		thread_local static AABB v;

		TypeInfo_AABB() :
			TypeInfo_Data("flame::AABB", sizeof(AABB))
		{
			data_type = DataFloat;
			vec_size = 3;
			col_size = 2;
		}

		std::string serialize(const void* p) const override
		{
			if (!p) p = &v;
			return str(*(mat2x3*)p);
		}
		void unserialize(const std::string& str, void* p) const override
		{
			if (!p) p = &v;
			*(mat2x3*)p = s2t<2, 3, float>(str);
		}
		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(AABB*)dst = fi->call<AABB>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { TypeInfo::get<AABB*>() }));
			if (!src) src = &v;
			fi->call<void, const AABB&>(obj, *(AABB*)src);
		}
	};

	struct TypeInfo_Plane : TypeInfo_Data
	{
		thread_local static Plane v;

		TypeInfo_Plane() :
			TypeInfo_Data("flame::Plane", sizeof(Plane))
		{
			data_type = DataFloat;
			vec_size = 4;
		}

		std::string serialize(const void* p) const override
		{
			if (!p) p = &v;
			return str(*(vec4*)p);
		}
		void unserialize(const std::string& str, void* p) const override
		{
			if (!p) p = &v;
			*(vec4*)p = s2t<4, float>(str);
		}
		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(Plane*)dst = fi->call<Plane>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { TypeInfo::get<Plane*>() }));
			if (!src) src = &v;
			fi->call<void, const Plane&>(obj, *(Plane*)src);
		}
	};

	struct TypeInfo_Frustum : TypeInfo_Data
	{
		thread_local static Frustum v;

		TypeInfo_Frustum() :
			TypeInfo_Data("flame::Frustum", sizeof(Plane))
		{
		}

		void* get_v() const override
		{
			return &v;
		}
		void call_getter(const FunctionInfo* fi, void* obj, void* dst) const override
		{
			assert(fi->return_type == this);
			if (!dst) dst = &v;
			*(Frustum*)dst = fi->call<Frustum>(obj);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->check(TypeInfo::void_type, { TypeInfo::get<Frustum*>() }));
			if (!src) src = &v;
			fi->call<void, const Frustum&>(obj, *(Frustum*)src);
		}
	};

	struct TypeInfo_Udt : TypeInfo
	{
		UdtInfo* ui = nullptr;

		TypeInfo_Udt(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo(TagU, base_name, 0)
		{
			ui = find_udt(sh(name.c_str()), db);
			if (ui)
				size = ui->size;
		}

		void* create(void* p = nullptr) const override
		{
			return ui->create_object(p);
		}
		void destroy(void* p, bool free_memory = true) const override
		{
			ui->destroy_object(p, free_memory);
		}
		void copy(void* dst, const void* src) const override
		{
			ui->copy_object(dst, src);
		}
	};

	struct TypeInfo_Pair : TypeInfo
	{
		TypeInfo* ti1 = nullptr;
		TypeInfo* ti2 = nullptr;

		TypeInfo_Pair(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo(TagR, base_name, 0)
		{
			auto sp = SUS::split(name, ';');
			assert(sp.size() == 2);
			ti1 = get(sp[0], db);
			ti2 = get(sp[1], db);
			size = ti1->size + ti2->size;
		}

		void* first(const void* p) const
		{
			return (char*)p + 0;
		}
		void* second(const void* p) const
		{
			return (char*)p + ti1->size;
		}

		void* create(void* p = nullptr) const override
		{
			if (!p)
				p = malloc(size);
			ti1->create(first(p));
			ti2->create(second(p));
			return p;
		}
		void destroy(void* p, bool free_memory = true) const override
		{
			ti1->destroy(first(p), false);
			ti2->destroy(second(p), false);
			if (free_memory)
				free(p);
		}
		void copy(void* dst, const void* src) const override
		{
			ti1->copy(first(dst), first(src));
			ti2->copy(second(dst), second(src));
		}
	};

	struct TypeInfo_Tuple : TypeInfo
	{
		std::vector<std::pair<TypeInfo*, uint>> tis;

		TypeInfo_Tuple(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo(TagT, base_name, 0)
		{
			auto sp = SUS::split(name, ';');
			auto align = 4U;
			for (auto& t : sp)
			{
				auto ti = get(t, db);
				if (ti == TypeInfo::get<std::string>())
					align = 8U;
				else if (ti == TypeInfo::get<std::wstring>())
					align = 8U;
				else if (ti == TypeInfo::get<std::filesystem::path>())
					align = 8U;
				tis.push_back(std::make_pair(ti, 0));
			}
			size = 0;
			for (auto i = (int)tis.size() - 1; i >= 0; i--)
			{
				auto& t = tis[i];
				t.second = size;
				size += max(t.first->size, align);
			}
		}

		void* create(void* p = nullptr) const override
		{
			if (!p)
				p = malloc(size);
			for (auto& t : tis)
				t.first->create((char*)p + t.second);
			return p;
		}
		void destroy(void* p, bool free_memory = true) const override
		{
			for (auto& t : tis)
				t.first->destroy((char*)p + t.second, false);
			if (free_memory)
				free(p);
		}
		void copy(void* dst, const void* src) const override
		{
			for (auto& t : tis)
				t.first->copy((char*)dst + t.second, (char*)src + t.second);
		}
	};

	struct TypeInfo_PointerOfEnum : TypeInfo
	{
		TypeInfo_Enum* ti = nullptr;

		TypeInfo_PointerOfEnum(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo(TagPE, base_name, sizeof(void*))
		{
			ti = (TypeInfo_Enum*)get(TagE, name, db);
		}

		TypeInfo* get_wrapped() const override
		{
			return ti;
		}
	};

	struct TypeInfo_PointerOfData : TypeInfo
	{
		TypeInfo_Data* ti = nullptr;

		TypeInfo_PointerOfData(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo(TagPD, base_name, sizeof(void*))
		{
			ti = (TypeInfo_Data*)get(TagD, name, db);
		}

		TypeInfo* get_wrapped() const override
		{
			return ti;
		}
	};

	struct TypeInfo_PointerOfUdt : TypeInfo
	{
		TypeInfo_Udt* ti = nullptr;

		TypeInfo_PointerOfUdt(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo(TagPU, base_name, sizeof(void*))
		{
			ti = (TypeInfo_Udt*)get(TagU, name, db);
		}

		TypeInfo* get_wrapped() const override
		{
			return ti;
		}
	};

	struct TypeInfo_VectorOfEnum : TypeInfo
	{
		TypeInfo_Enum* ti = nullptr;

		TypeInfo_VectorOfEnum(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo(TagVE, base_name, sizeof(std::vector<int>))
		{
			ti = (TypeInfo_Enum*)get(TagE, name, db);
		}

		void* create(void* p = nullptr) const override
		{
			if (!p)
				return new std::vector<int>();
			new(p) std::vector<char>();
			return p;
		}
		void copy(void* dst, const void* src) const override
		{
			auto& dst_vec = *(std::vector<int>*)dst;
			auto& src_vec = *(std::vector<int>*)src;
			dst_vec = src_vec;
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->return_type == TypeInfo::void_type && fi->parameters.size() == 1 && fi->parameters[0]->tag == TagPVE);
			fi->call<void, const std::vector<int>&>(obj, *(std::vector<int>*)src);
		}

		TypeInfo* get_wrapped() const override
		{
			return ti;
		}
	};

	inline void copy_npod_vector(void* dst, const void* src, TypeInfo* ti)
	{
		auto& dst_vec = *(std::vector<char>*)dst;
		auto& src_vec = *(std::vector<char>*)src;
		auto old_len = dst_vec.size() / ti->size;
		auto new_len = src_vec.size() / ti->size;
		for (auto i = new_len; i < old_len; i++)
			ti->destroy((char*)dst_vec.data() + i * ti->size, false);
		dst_vec.resize(new_len * ti->size);
		for (auto i = old_len; i < new_len; i++)
			ti->create((char*)dst_vec.data() + i * ti->size);
		for (auto i = 0; i < new_len; i++)
			ti->copy((char*)dst_vec.data() + i * ti->size, (char*)src_vec.data() + i * ti->size);
	}

	struct TypeInfo_VectorOfData : TypeInfo
	{
		TypeInfo_Data* ti = nullptr;

		TypeInfo_VectorOfData(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo(TagVD, base_name, sizeof(std::vector<int>))
		{
			ti = (TypeInfo_Data*)get(TagD, name, db);
		}

		void* create(void* p = nullptr) const override
		{
			if (!p)
				return new std::vector<int>();
			new(p) std::vector<char>();
			return p;
		}
		void copy(void* dst, const void* src) const override
		{
			copy_npod_vector(dst, src, ti);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->return_type == TypeInfo::void_type && fi->parameters.size() == 1 && fi->parameters[0]->tag == TagPVD);
			fi->call<void, const std::vector<int>&>(obj, *(std::vector<int>*)src);
		}

		TypeInfo* get_wrapped() const override
		{
			return ti;
		}
	};

	struct TypeInfo_VectorOfUdt : TypeInfo
	{
		TypeInfo_Udt* ti = nullptr;

		TypeInfo_VectorOfUdt(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo(TagVU, base_name, sizeof(std::vector<int>))
		{
			ti = (TypeInfo_Udt*)get(TagU, name, db);
		}

		void* create(void* p = nullptr) const override
		{
			if (!p)
				return new std::vector<int>();
			new(p) std::vector<char>();
			return p;
		}
		void copy(void* dst, const void* src) const override
		{
			copy_npod_vector(dst, src, ti);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->return_type == TypeInfo::void_type && fi->parameters.size() == 1 && fi->parameters[0]->tag == TagPVU);
			fi->call<void, const std::vector<int>&>(obj, *(std::vector<int>*)src);
		}

		TypeInfo* get_wrapped() const override
		{
			return ti;
		}
	};

	struct TypeInfo_VectorOfPair : TypeInfo
	{
		TypeInfo_Pair* ti = nullptr;

		TypeInfo_VectorOfPair(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo(TagVR, base_name, sizeof(std::vector<int>))
		{
			ti = (TypeInfo_Pair*)get(TagR, name, db);
		}

		void* create(void* p = nullptr) const override
		{
			if (!p)
				return new std::vector<int>();
			new(p) std::vector<char>();
			return p;
		}
		void copy(void* dst, const void* src) const override
		{
			copy_npod_vector(dst, src, ti);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->return_type == TypeInfo::void_type && fi->parameters.size() == 1 && fi->parameters[0]->tag == TagPVR);
			fi->call<void, const std::vector<int>&>(obj, *(std::vector<int>*)src);
		}

		TypeInfo* get_wrapped() const override
		{
			return ti;
		}
	};

	struct TypeInfo_VectorOfTuple : TypeInfo
	{
		TypeInfo_Tuple* ti = nullptr;

		TypeInfo_VectorOfTuple(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo(TagVT, base_name, sizeof(std::vector<int>))
		{
			ti = (TypeInfo_Tuple*)get(TagT, name, db);
		}

		void* create(void* p = nullptr) const override
		{
			if (!p)
				return new std::vector<int>();
			new(p) std::vector<char>();
			return p;
		}
		void copy(void* dst, const void* src) const override
		{
			copy_npod_vector(dst, src, ti);
		}
		void call_setter(const FunctionInfo* fi, void* obj, void* src) const override
		{
			assert(fi->return_type == TypeInfo::void_type && fi->parameters.size() == 1 && fi->parameters[0]->tag == TagPVT);
			fi->call<void, const std::vector<int>&>(obj, *(std::vector<int>*)src);
		}

		TypeInfo* get_wrapped() const override
		{
			return ti;
		}
	};

	struct TypeInfo_VectorOfPointerOfUdt : TypeInfo
	{
		TypeInfo_PointerOfUdt* ti = nullptr;

		TypeInfo_VectorOfPointerOfUdt(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo(TagVPU, base_name, sizeof(std::vector<int>))
		{
			ti = (TypeInfo_PointerOfUdt*)get(TagPU, name, db);
		}

		void* create(void* p = nullptr) const override
		{
			if (!p)
				return new std::vector<int>();
			new(p) std::vector<char>();
			return p;
		}

		TypeInfo* get_wrapped() const override
		{
			return ti;
		}
	};

	struct TypeInfo_PointerOfVectorOfEnum : TypeInfo
	{
		TypeInfo_VectorOfEnum* ti = nullptr;

		TypeInfo_PointerOfVectorOfEnum(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo(TagPVE, base_name, sizeof(void*))
		{
			ti = (TypeInfo_VectorOfEnum*)get(TagVE, name, db);
		}

		TypeInfo* get_wrapped() const override
		{
			return ti;
		}
	};

	struct TypeInfo_PointerOfVectorOfData : TypeInfo
	{
		TypeInfo_VectorOfData* ti = nullptr;

		TypeInfo_PointerOfVectorOfData(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo(TagPVD, base_name, sizeof(void*))
		{
			ti = (TypeInfo_VectorOfData*)get(TagVD, name, db);
		}

		TypeInfo* get_wrapped() const override
		{
			return ti;
		}
	};

	struct TypeInfo_PointerOfVectorOfUdt : TypeInfo
	{
		TypeInfo_VectorOfUdt* ti = nullptr;

		TypeInfo_PointerOfVectorOfUdt(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo(TagPVU, base_name, sizeof(void*))
		{
			ti = (TypeInfo_VectorOfUdt*)get(TagVU, name, db);
		}

		TypeInfo* get_wrapped() const override
		{
			return ti;
		}
	};

	struct TypeInfo_PointerOfVectorOfPair : TypeInfo
	{
		TypeInfo_VectorOfPair* ti = nullptr;

		TypeInfo_PointerOfVectorOfPair(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo(TagPVR, base_name, sizeof(void*))
		{
			ti = (TypeInfo_VectorOfPair*)get(TagVR, name, db);
		}

		TypeInfo* get_wrapped() const override
		{
			return ti;
		}
	};

	struct TypeInfo_PointerOfVectorOfTuple : TypeInfo
	{
		TypeInfo_VectorOfTuple* ti = nullptr;

		TypeInfo_PointerOfVectorOfTuple(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo(TagPVT, base_name, sizeof(void*))
		{
			ti = (TypeInfo_VectorOfTuple*)get(TagVT, name, db);
		}

		TypeInfo* get_wrapped() const override
		{
			return ti;
		}
	};

	struct TypeInfo_Array : TypeInfo
	{
		uint extent = 0;
		uint stride = 0;

		TypeInfo_Array(TypeTag tag, std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo(tag, "", 0)
		{
			static std::regex reg(R"((.*)\[(\d+)\](:\d+)?$)");
			std::smatch res;
			auto str = std::string(base_name);
			std::regex_search(str, res, reg);
			if (res.size() > 2)
			{
				name = res[1].str();
				extent = stoi(res[2].str());
				if (res[3].matched)
					stride = stoi(res[3].str());
			}
		}
	};

	struct TypeInfo_ArrayOfEnum : TypeInfo_Array
	{
		TypeInfo_Enum* ti = nullptr;

		TypeInfo_ArrayOfEnum(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo_Array(TagAE, base_name, db)
		{
			ti = (TypeInfo_Enum*)get(TagE, name, db);
			name = base_name;
			if (stride == 0)
				stride = ti->size;
			size = stride * extent;
		}

		TypeInfo* get_wrapped() const override
		{ 
			return ti;
		}
	};

	struct TypeInfo_ArrayOfData : TypeInfo_Array
	{
		TypeInfo_Data* ti = nullptr;

		TypeInfo_ArrayOfData(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo_Array(TagAD, base_name, db)
		{
			ti = (TypeInfo_Data*)get(TagD, name, db);
			name = base_name;
			if (stride == 0)
				stride = ti->size;
			size = stride * extent;
		}

		TypeInfo* get_wrapped() const override
		{
			return ti;
		}
	};

	struct TypeInfo_ArrayOfUdt : TypeInfo_Array
	{
		TypeInfo_Udt* ti = nullptr;

		TypeInfo_ArrayOfUdt(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo_Array(TagAU, base_name, db)
		{
			ti = (TypeInfo_Udt*)get(TagU, name, db);
			name = base_name;
			if (stride == 0)
				stride = ti->size;
			size = stride * extent;
		}

		TypeInfo* get_wrapped() const override
		{
			return ti;
		}
	};

	EnumInfo* TypeInfo::retrive_ei()
	{
		switch (tag)
		{
		case TagE:
			return ((TypeInfo_Enum*)this)->ei;
		case TagPE:
			return ((TypeInfo_PointerOfEnum*)this)->ti->ei;
		case TagVE:
			return ((TypeInfo_VectorOfEnum*)this)->ti->ei;
		}
		return nullptr;
	}

	UdtInfo* TypeInfo::retrive_ui()
	{
		switch (tag)
		{
		case TagU:
			return ((TypeInfo_Udt*)this)->ui;
		case TagPU:
			return ((TypeInfo_PointerOfUdt*)this)->ti->ui;
		case TagAU:
			return ((TypeInfo_ArrayOfUdt*)this)->ti->ui;
		case TagVU:
			return ((TypeInfo_VectorOfUdt*)this)->ti->ui;
		case TagVPU:
			return ((TypeInfo_VectorOfPointerOfUdt*)this)->ti->ti->ui;
		}
		return nullptr;
	}

	using TI_E = TypeInfo_Enum;
	using TI_D = TypeInfo_Data;
	using TI_A = TypeInfo_Array;

	struct VirtualObject
	{
		TypeInfo*	type = nullptr;
		char*		data = nullptr;

		template<class T>
		T& as()
		{
			assert(type->tag == TagD || type->tag == TagE);
			return *(T*)data;
		}

		VirtualObject child(uint hash)
		{
			assert(type->tag == TagU);
			auto ti = (TypeInfo_Udt*)type;
			auto vi = hash ? ti->ui->find_variable(hash) : &ti->ui->variables[0];
			assert(vi);
			VirtualObject ret;
			ret.type = vi->type;
			ret.data = data + vi->offset;
			return ret;
		}

		template<class T = TypeInfo>
		T* child_type(uint hash)
		{
			assert(type->tag == TagU);
			auto ti = (TypeInfo_Udt*)type;
			auto vi = ti->ui->find_variable(hash);
			assert(vi);
			return (T*)vi->type;
		}

		VirtualObject item(uint idx)
		{
			assert(is_in(type->tag, TagA_Beg, TagA_End));
			auto ti = (TypeInfo_Array*)type;
			assert(idx < ti->extent);
			VirtualObject ret;
			ret.type = ti->get_wrapped();
			ret.data = data + idx * ti->stride;
			return ret;
		}

		uint offset(const VirtualObject& oth)
		{
			return uint(oth.data - data);
		}

		void create(void* p = nullptr)
		{
			assert(!data && type);
			data = (char*)type->create(p);
		}

		void destroy()
		{
			assert(data && type);
			type->destroy(data);
			data = nullptr;
		}
	};

	template<class BaseType>
	struct VirtualUdt : VirtualObject
	{
		~VirtualUdt()
		{
			if (data)
				destroy();
		}

		BaseType& value()
		{
			return *(BaseType*)data;
		}

		void create(UdtInfo* ui)
		{
			assert(!data);
			type = TypeInfo::get(TagU, ui->name, *ui->db);
			data = (char*)ui->create_object();
		}
	};
}
