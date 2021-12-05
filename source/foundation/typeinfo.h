#pragma once

#include "foundation.h"

namespace flame
{
	/*
	*	E - Enum
	*	D - Data
	*	F - Function
	*	U - Udt
	*	P - Pointer
	*	V - Vector
	*/
	enum TypeTag
	{
		TagE,
		TagD,
		TagF,
		TagU,
		TagPE,
		TagPD,
		TagPU,
		TagVE,
		TagVD,
		TagVU,
		TagVPE,
		TagVPD,
		TagVPU,

		TagCount
	};

	enum DataType
	{
		DataVoid,
		DataBoolean,
		DataInteger,
		DataFloat,
		DataChar,
		DataWideChar
	};

	enum Meta
	{
		MetaBpInput,
		MetaBpOutput,
		MetaSecondaryAttribute
	};

	FLAME_FOUNDATION_EXPORTS extern TypeInfoDataBase& tidb;

	struct TypeInfo
	{
		TypeTag tag;
		std::string name;
		uint hash;
		uint size;

		inline static std::string format_name(std::string_view name)
		{
			auto ret = std::string(name);

			SUS::cut_head_if(ret, "enum ");
			SUS::cut_head_if(ret, "struct ");
			SUS::cut_head_if(ret, "class ");
			SUS::replace_all(ret, "unsigned ", "u");
			SUS::replace_all(ret, "__int64 ", "int64");
			SUS::replace_all(ret, "Private", "");
			SUS::remove_char(ret, ' ');

			if (ret.starts_with("glm::"))
			{
				if (ret == "glm::vec<2,int,0>")
					ret = "glm::ivec2";
				else if (ret == "glm::vec<3,int,0>")
					ret = "glm::ivec3";
				else if (ret == "glm::vec<4,int,0>")
					ret = "glm::ivec4";
				else if (ret == "glm::vec<2,uint,0>")
					ret = "glm::uvec2";
				else if (ret == "glm::vec<3,uint,0>")
					ret = "glm::uvec3";
				else if (ret == "glm::vec<4,uint,0>")
					ret = "glm::uvec4";
				else if (ret == "glm::vec<2,uchar,0>")
					ret = "glm::cvec2";
				else if (ret == "glm::vec<3,uchar,0>")
					ret = "glm::cvec3";
				else if (ret == "glm::vec<4,uchar,0>")
					ret = "glm::cvec4";
				else if (ret == "glm::vec<2,float,0>")
					ret = "glm::vec2";
				else if (ret == "glm::vec<3,float,0>")
					ret = "glm::vec3";
				else if (ret == "glm::vec<4,float,0>")
					ret = "glm::vec4";
				else if (ret == "glm::mat<2,2,float,0>")
					ret = "glm::mat2";
				else if (ret == "glm::mat<3,3,float,0>")
					ret = "glm::mat3";
				else if (ret == "glm::mat<4,4,float,0>")
					ret = "glm::mat4";
				else if (ret == "glm::qua<float,0>")
					ret = "glm::quat";
				else
					assert(0);
			}
			else if (ret.starts_with("std::basic_string<char"))
				ret = "std::string";
			else if (ret.starts_with("std::basic_string<wchar_t"))
				ret = "std::wstring";
			//else if (ret.starts_with("std::vector<"))
			//{
			//	static std::regex reg("std::vector<([\\w:\\*<>]+),");
			//	std::smatch res;
			//	if (std::regex_search(ret, res, reg))
			//	{
			//		auto str = format(TagD, res[1].str()).second;
			//		if (tag == TagD)
			//		{
			//			ret.first = TagVector;
			//			ret = str;
			//		}
			//		else
			//			ret = "std::vector<" + str + ">";
			//	}
			//}

			return ret;
		}

		inline static bool is_basic_type(std::string_view name);

		TypeInfo(TypeTag tag, std::string_view _name, uint size) :
			tag(tag),
			name(_name),
			size(size)
		{
			hash = get_hash(tag, name);
		}

		virtual ~TypeInfo() {}

		virtual void* create() const { return malloc(size); }
		virtual void destroy(void* p) const { free(p); }
		virtual void copy(void* dst, const void* src) const { memcpy(dst, src, size); }
		virtual bool compare(const void* d1, const void* d2) const { return memcmp(d1, d2, size) == 0; }
		virtual std::string serialize(const void* p) const { return ""; }
		virtual void unserialize(const std::string& str, void* p) const {}

		inline static uint get_hash(TypeTag tag, std::string_view name)
		{
			auto ret = ch(name.data());
			ret ^= std::hash<uint>()(tag);
			return ret;
		}

		FLAME_FOUNDATION_EXPORTS static TypeInfo* get(TypeTag tag, const std::string& name, TypeInfoDataBase& db = tidb);

		template<enum_type T>
		static TypeInfo* get(TypeInfoDataBase& db = tidb)
		{
			static auto ret = get(TagE, format_name(typeid(T).name()), db);
			return ret;
		}

		template<not_enum_type T>
		static TypeInfo* get(TypeInfoDataBase& db = tidb)
		{
			static auto ret = get(format(TagD, typeid(T).name()), db);
			return ret;
		}

		template <class T>
		inline static std::string serialize_t(T* v, TypeInfoDataBase& db = tidb)
		{
			return get<T>(db)->serialize(v);
		}

		template <class T>
		inline static void unserialize_t(const std::string& str, T* v, TypeInfoDataBase& db = tidb)
		{
			return get<T>(db)->unserialize(str, v);
		}
	};

	struct Metas
	{
		std::vector<std::pair<Meta, LightCommonValue>> d;

		void from_string(const std::string& str, TypeInfoDataBase& db = tidb);
		std::string to_string(TypeInfoDataBase& db = tidb) const;

		inline bool get(Meta m, LightCommonValue* v) const
		{
			for (auto& i : d)
			{
				if (i.first == m)
				{
					if (v)
						*v = i.second;
					return true;
				}
			}
			return false;
		}
	};

	struct EnumItemInfo
	{
		EnumInfo* ei;
		std::string name;
		int value;
	};

	struct EnumInfo
	{
		std::string name;
		std::vector<EnumItemInfo> items;

		inline EnumItemInfo* find_item(std::string_view name) const
		{
			for (auto& i : items)
			{
				if (i.name == name)
					return (EnumItemInfo*)&i;
			}
			return nullptr;
		}

		inline EnumItemInfo* find_item(int value) const
		{
			for (auto& i : items)
			{
				if (i.value == value)
					return (EnumItemInfo*)&i;
			}
			return nullptr;
		}
	};

	struct FunctionInfo
	{
		UdtInfo* ui;
		std::string name;
		uint rva;
		int voff;
		bool is_static = false;
		TypeInfo* return_type;
		std::vector<TypeInfo*> parameters;
		std::string code;
		Metas metas;
		void* library;

		inline bool check(TypeInfo* ret, const std::vector<TypeInfo*> parms) const
		{
			if (return_type != ret || parameters.size() != parms.size())
				return false;
			for (auto i = 0; i < parameters.size(); i++)
			{
				if (parameters[i] != parms[i])
					return false;
			}
			return true;
		}

		inline void* get_address(void* obj = nullptr /* for virtual fucntion */) const
		{
			return voff == -1 ? (char*)library + rva : (obj ? *(void**)((*(char**)obj) + voff) : nullptr);
		}
	};

	struct VariableInfo
	{
		UdtInfo* ui;
		TypeInfo* type;
		std::string name;
		uint offset;
		uint array_size = 0;
		uint array_stride = 0;
		std::string default_value;
		Metas metas;
	};

	struct UdtInfo
	{
		std::string name;
		uint size;
		std::string base_class_name;
		std::vector<VariableInfo> variables;
		std::vector<FunctionInfo> functions;
		void* library;

		VariableInfo* find_variable(const std::string_view& name) const
		{
			for (auto& v : variables)
			{
				if (v.name == name)
					return (VariableInfo*)&v;
			}
			return nullptr;
		}

		FunctionInfo* find_function(const std::string_view& name) const
		{
			for (auto& f : functions)
			{
				if (f.name == name)
					return (FunctionInfo*)&f;
			}
			return nullptr;
		}
	};

	struct TypeInfoDataBase
	{
		FLAME_FOUNDATION_EXPORTS TypeInfoDataBase();

		std::unordered_map<uint, std::unique_ptr<TypeInfo>> typeinfos;

		std::unordered_map<std::string, EnumInfo> enums;
		std::unordered_map<std::string, FunctionInfo> functions;
		std::unordered_map<std::string, UdtInfo> udts;

		FLAME_FOUNDATION_EXPORTS bool load(std::ifstream& file, void* library = nullptr);
		FLAME_FOUNDATION_EXPORTS void load(const std::filesystem::path& filename);
		FLAME_FOUNDATION_EXPORTS void save(std::ofstream& file);
		FLAME_FOUNDATION_EXPORTS void save(const std::filesystem::path& filename);
	};

	inline void Metas::from_string(const std::string& str, TypeInfoDataBase& db)
	{
		for (auto& i : SUS::split(str, ';'))
		{
			auto sp = SUS::split(i, ':');
			auto& m = d.emplace_back();
			TypeInfo::unserialize_t(sp[0], &m.first);
			m.second.u = std::stoul(sp[1], 0, 16);
		}
	}

	inline std::string Metas::to_string(TypeInfoDataBase& db) const
	{
		std::string ret;
		for (auto& i : d)
			ret += TypeInfo::serialize_t(&i.first) + ":" + to_hex_string(i.second.u, false) + ";";
		return ret;
	}

	inline EnumInfo* find_enum(const std::string& name, TypeInfoDataBase& db = tidb)
	{
		auto it = db.enums.find(name);
		if (it != db.enums.end())
			return &it->second;
		if (&db != &tidb)
		{
			it = tidb.enums.find(name);
			if (it != tidb.enums.end())
				return &it->second;
		}
		return nullptr;
	}

	inline UdtInfo* find_udt(const std::string& name, TypeInfoDataBase& db = tidb)
	{
		auto it = db.udts.find(name);
		if (it != db.udts.end())
			return &it->second;
		if (&db != &tidb)
		{
			it = tidb.udts.find(name);
			if (it != tidb.udts.end())
				return &it->second;
		}
		return nullptr;
	}

	inline bool TypeInfo::is_basic_type(std::string_view name)
	{
		return tidb.typeinfos.find(get_hash(TagD, name)) != tidb.typeinfos.end();
	}

	struct TypeInfo_Enum : TypeInfo
	{
		EnumInfo* ei = nullptr;

		TypeInfo_Enum(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo(TagE, base_name, sizeof(int))
		{
			ei = find_enum(name, db);
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
			return ei->find_item(*(int*)p)->name;
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			*(int*)dst = ei->find_item(str)->value;
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
			auto v = *(int*)p;
			for (auto i = 0; i < ei->items.size(); i++)
			{
				if ((v & 1) == 1)
				{
					if (i > 0)
						ret += '|';
					ret += ei->find_item(1 << i)->name;
				}
				v >>= 1;
			}
			return ret;
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			auto v = 0;
			auto sp = SUS::split(str, '|');
			for (auto& t : sp)
				v |= ei->find_item(t)->value;
			*(int*)dst = v;
		}
	};

	struct TypeInfo_Data : TypeInfo
	{
		DataType data_type = DataVoid;
		bool is_signed = true;
		uint vec_size = 1;
		uint col_size = 1;

		TypeInfo_Data(std::string_view name, uint size) :
			TypeInfo(TagD, name, size)
		{
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
		TypeInfo_bool() :
			TypeInfo_Data("bool", sizeof(bool))
		{
			data_type = DataBoolean;
		}

		std::string serialize(const void* p) const override
		{
			return to_string(*(bool*)p);
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			if (str == "false")
				*(bool*)dst = false;
			else if (str == "true")
				*(bool*)dst = true;
			else
				*(bool*)dst = sto<int>(str) != 0;
		}
	};

	struct TypeInfo_char: TypeInfo_Data
	{
		TypeInfo_char() :
			TypeInfo_Data("char", sizeof(char))
		{
			data_type = DataChar;
		}

		std::string serialize(const void* p) const override
		{
			return to_string(*(char*)p);
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			*(char*)dst = sto<char>(str);
		}
	};

	struct TypeInfo_uchar: TypeInfo_Data
	{
		TypeInfo_uchar() :
			TypeInfo_Data("uchar", sizeof(uchar))
		{
			data_type = DataChar;
			is_signed = false;
		}

		std::string serialize(const void* p) const override
		{
			return to_string(*(uchar*)p);
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			*(uchar*)dst = sto<uchar>(str);
		}
	};

	struct TypeInfo_wchar: TypeInfo_Data
	{
		TypeInfo_wchar() :
			TypeInfo_Data("wchar_t", sizeof(wchar_t))
		{
			data_type = DataWideChar;
		}

		std::string serialize(const void* p) const override
		{
			return to_string(*(wchar_t*)p);
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			*(wchar_t*)dst = sto<wchar_t>(str);
		}
	};

	struct TypeInfo_short: TypeInfo_Data
	{
		TypeInfo_short() :
			TypeInfo_Data("short", sizeof(short))
		{
			data_type = DataInteger;
		}

		std::string serialize(const void* p) const override
		{
			return to_string(*(short*)p);
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			*(short*)dst = sto<short>(str);
		}
	};

	struct TypeInfo_ushort: TypeInfo_Data
	{
		TypeInfo_ushort() :
			TypeInfo_Data("ushort", sizeof(ushort))
		{
			data_type = DataInteger;
			is_signed = false;
		}

		std::string serialize(const void* p) const override
		{
			return to_string(*(ushort*)p);
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			*(ushort*)dst = sto<ushort>(str);
		}
	};

	struct TypeInfo_int: TypeInfo_Data
	{
		TypeInfo_int() :
			TypeInfo_Data("int", sizeof(int))
		{
			data_type = DataInteger;
		}

		std::string serialize(const void* p) const override
		{
			return to_string(*(int*)p);
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			*(int*)dst = sto<int>(str);
		}
	};

	struct TypeInfo_uint: TypeInfo_Data
	{
		TypeInfo_uint() :
			TypeInfo_Data("uint", sizeof(uint))
		{
			data_type = DataInteger;
			is_signed = false;
		}

		std::string serialize(const void* p) const override
		{
			return to_string(*(uint*)p);
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			*(uint*)dst = sto<uint>(str);
		}
	};

	struct TypeInfo_int64: TypeInfo_Data
	{
		TypeInfo_int64() :
			TypeInfo_Data("int64", sizeof(int64))
		{
			data_type = DataInteger;
		}

		std::string serialize(const void* p) const override
		{
			return to_string(*(int64*)p);
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			*(int64*)dst = sto<int64>(str);
		}
	};

	struct TypeInfo_uint64: TypeInfo_Data
	{
		TypeInfo_uint64() :
			TypeInfo_Data("uint64", sizeof(uint64))
		{
			data_type = DataInteger;
			is_signed = false;
		}

		std::string serialize(const void* p) const override
		{
			return to_string(*(uint64*)p);
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			*(uint64*)dst = sto<uint64>(str);
		}
	};

	struct TypeInfo_float: TypeInfo_Data
	{
		TypeInfo_float() :
			TypeInfo_Data("float", sizeof(float))
		{
			data_type = DataFloat;
		}

		std::string serialize(const void* p) const override
		{
			return to_string(*(float*)p);
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			*(float*)dst = sto<float>(str);
		}
	};

	struct TypeInfo_ivec2: TypeInfo_Data
	{
		TypeInfo_ivec2() :
			TypeInfo_Data("glm::ivec2", sizeof(ivec2))
		{
			data_type = DataInteger;
			vec_size = 2;
		}

		std::string serialize(const void* p) const override
		{
			return to_string(*(ivec2*)p);
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			*(ivec2*)dst = sto<2, int>(str);
		}
	};

	struct TypeInfo_ivec3: TypeInfo_Data
	{
		TypeInfo_ivec3() :
			TypeInfo_Data("glm::ivec3", sizeof(ivec3))
		{
			data_type = DataInteger;
			vec_size = 3;
		}

		std::string serialize(const void* p) const override
		{
			return to_string(*(ivec3*)p);
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			*(ivec3*)dst = sto<3, int>(str);
		}
	};

	struct TypeInfo_ivec4: TypeInfo_Data
	{
		TypeInfo_ivec4() :
			TypeInfo_Data("glm::ivec4", sizeof(ivec4))
		{
			data_type = DataInteger;
			vec_size = 4;
		}

		std::string serialize(const void* p) const override
		{
			return to_string(*(ivec4*)p);
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			*(ivec4*)dst = sto<4, int>(str);
		}
	};

	struct TypeInfo_uvec2: TypeInfo_Data
	{
		TypeInfo_uvec2() :
			TypeInfo_Data("glm::uvec2", sizeof(uvec2))
		{
			data_type = DataInteger;
			is_signed = false;
			vec_size = 2;
		}

		std::string serialize(const void* p) const override
		{
			return to_string(*(uvec2*)p);
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			*(uvec2*)dst = sto<2, uint>(str);
		}
	};

	struct TypeInfo_uvec3: TypeInfo_Data
	{
		TypeInfo_uvec3() :
			TypeInfo_Data("glm::uvec3", sizeof(uvec3))
		{
			data_type = DataInteger;
			is_signed = false;
			vec_size = 3;
		}

		std::string serialize(const void* p) const override
		{
			return to_string(*(uvec3*)p);
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			*(uvec3*)dst = sto<3, uint>(str);
		}
	};

	struct TypeInfo_uvec4: TypeInfo_Data
	{
		TypeInfo_uvec4() :
			TypeInfo_Data("glm::uvec4", sizeof(uvec4))
		{
			data_type = DataInteger;
			is_signed = false;
			vec_size = 4;
		}

		std::string serialize(const void* p) const override
		{
			return to_string(*(uvec4*)p);
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			*(uvec4*)dst = sto<4, uint>(str);
		}
	};

	struct TypeInfo_cvec2 : TypeInfo_Data
	{
		TypeInfo_cvec2() :
			TypeInfo_Data("glm::cvec2", sizeof(cvec2))
		{
			data_type = DataChar;
			is_signed = false;
			vec_size = 2;
		}

		std::string serialize(const void* p) const override
		{
			return to_string(*(cvec2*)p);
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			*(cvec2*)dst = sto<2, uchar>(str);
		}
	};

	struct TypeInfo_cvec3 : TypeInfo_Data
	{
		TypeInfo_cvec3() :
			TypeInfo_Data("glm::cvec3", sizeof(cvec3))
		{
			data_type = DataChar;
			is_signed = false;
			vec_size = 3;
		}

		std::string serialize(const void* p) const override
		{
			return to_string(*(cvec3*)p);
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			*(cvec3*)dst = sto<3, uchar>(str);
		}
	};

	struct TypeInfo_cvec4 : TypeInfo_Data
	{
		TypeInfo_cvec4() :
			TypeInfo_Data("glm::cvec4", sizeof(cvec4))
		{
			data_type = DataChar;
			is_signed = false;
			vec_size = 4;
		}

		std::string serialize(const void* p) const override
		{
			return to_string(*(cvec4*)p);
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			*(cvec4*)dst = sto<4, uchar>(str);
		}
	};

	struct TypeInfo_vec2: TypeInfo_Data
	{
		TypeInfo_vec2() :
			TypeInfo_Data("glm::vec2", sizeof(vec2))
		{
			data_type = DataFloat;
			vec_size = 2;
		}

		std::string serialize(const void* p) const override
		{
			return to_string(*(vec2*)p);
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			*(vec2*)dst = sto<2, float>(str);
		}
	};

	struct TypeInfo_vec3: TypeInfo_Data
	{
		TypeInfo_vec3() :
			TypeInfo_Data("glm::vec3", sizeof(vec3))
		{
			data_type = DataFloat;
			vec_size = 3;
		}

		std::string serialize(const void* p) const override
		{
			return to_string(*(vec3*)p);
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			*(vec3*)dst = sto<3, float>(str);
		}
	};

	struct TypeInfo_vec4: TypeInfo_Data
	{
		TypeInfo_vec4() :
			TypeInfo_Data("glm::vec4", sizeof(vec4))
		{
			data_type = DataFloat;
			vec_size = 4;
		}

		std::string serialize(const void* p) const override
		{
			return to_string(*(vec4*)p);
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			*(vec4*)dst = sto<4, float>(str);
		}
	};

	struct TypeInfo_mat2 : TypeInfo_Data
	{
		TypeInfo_mat2() :
			TypeInfo_Data("glm::mat2", sizeof(mat2))
		{
			data_type = DataFloat;
			vec_size = 2;
			col_size = 2;
		}
	};

	struct TypeInfo_mat3 : TypeInfo_Data
	{
		TypeInfo_mat3() :
			TypeInfo_Data("glm::mat3", sizeof(mat3))
		{
			data_type = DataFloat;
			vec_size = 3;
			col_size = 3;
		}
	};

	struct TypeInfo_mat4 : TypeInfo_Data
	{
		TypeInfo_mat4() :
			TypeInfo_Data("glm::mat4", sizeof(mat4))
		{
			data_type = DataFloat;
			vec_size = 4;
			col_size = 4;
		}
	};

	struct TypeInfo_quat : TypeInfo_Data
	{
		TypeInfo_quat() :
			TypeInfo_Data("glm::quat", sizeof(quat))
		{
		}

		std::string serialize(const void* p) const override
		{
			return to_string(*(vec4*)p);
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			*(vec4*)dst = sto<4, float>(str).yzwx();
		}
	};

	struct TypeInfo_string: TypeInfo_Data
	{
		TypeInfo_string() :
			TypeInfo_Data("std::string", sizeof(std::string))
		{
		}

		void* create() const override
		{
			return new std::string;
		}
		void destroy(void* p) const override
		{
			delete (std::string*)p;
		}
		void copy(void* dst, const void* src) const override
		{
			*(std::string*)dst = *(std::string*)src;
		}
		bool compare(const void* d1, const void* d2) const override
		{
			return *(std::string*)d1 == *(std::string*)d2;
		}
		std::string serialize(const void* p) const override
		{
			return *(std::string*)p;
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			*(std::string*)dst = str;
		}
	};

	struct TypeInfo_wstring: TypeInfo_Data
	{
		TypeInfo_wstring() :
			TypeInfo_Data("std::wstring", sizeof(std::string))
		{
		}

		void* create() const override
		{
			return new std::wstring;
		}
		void destroy(void* p) const override
		{
			delete (std::wstring*)p;
		}
		void copy(void* dst, const void* src) const override
		{
			*(std::wstring*)dst = *(std::wstring*)src;
		}
		bool compare(const void* d1, const void* d2) const override
		{
			return *(std::wstring*)d1 == *(std::wstring*)d2;
		}
		std::string serialize(const void* p) const override
		{
			return w2s(*(std::wstring*)p);
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			*(std::wstring*)dst = s2w(str);
		}
	};

	struct TypeInfo_path : TypeInfo_Data
	{
		TypeInfo_path() :
			TypeInfo_Data("std::filesystem::path", sizeof(std::filesystem::path))
		{
		}

		void* create() const override
		{
			return new std::filesystem::path;
		}
		void destroy(void* p) const override
		{
			delete (std::filesystem::path*)p;
		}
		void copy(void* dst, const void* src) const override
		{
			*(std::filesystem::path*)dst = *(std::filesystem::path*)src;
		}
		bool compare(const void* d1, const void* d2) const override
		{
			return *(std::filesystem::path*)d1 == *(std::filesystem::path*)d2;
		}
		std::string serialize(const void* p) const override
		{
			return (*(std::filesystem::path*)p).string();
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			*(std::filesystem::path*)dst = str;
		}
	};

	struct TypeInfo_Rect: TypeInfo_Data
	{
		TypeInfo_Rect() :
			TypeInfo_Data("flame::Rect", sizeof(Rect))
		{
			data_type = DataFloat;
			vec_size = 2;
			col_size = 2;
		}

		std::string serialize(const void* p) const override
		{
			return to_string(*(vec4*)p);
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			*(vec4*)dst = sto<4, float>(str);
		}
	};

	struct TypeInfo_AABB: TypeInfo_Data
	{
		TypeInfo_AABB() :
			TypeInfo_Data("flame::AABB", sizeof(AABB))
		{
			data_type = DataFloat;
			vec_size = 3;
			col_size = 2;
		}

		std::string serialize(const void* p) const override
		{
			return to_string(*(mat2x3*)p);
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			*(mat2x3*)dst = sto<2, 3, float>(str);
		}
	};

	struct TypeInfo_Plane: TypeInfo_Data
	{
		TypeInfo_Plane() :
			TypeInfo_Data("flame::Plane", sizeof(Plane))
		{
			data_type = DataFloat;
			vec_size = 4;
		}

		std::string serialize(const void* p) const override
		{
			return to_string(*(vec4*)p);
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			*(vec4*)dst = sto<4, float>(str);
		}
	};

	struct TypeInfo_Frustum: TypeInfo_Data
	{
		TypeInfo_Frustum() :
			TypeInfo_Data("flame::Frustum", sizeof(Plane))
		{
		}
	};

	struct TypeInfo_Udt : TypeInfo
	{
		UdtInfo* ui = nullptr;

		TypeInfo_Udt(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo(TagU, base_name, 0)
		{
			ui = find_udt(name, db);
			if (ui)
				size = ui->size;
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
	};

	struct TypeInfo_VectorOfEnum : TypeInfo
	{
		TypeInfo_Enum* ti = nullptr;

		TypeInfo_VectorOfEnum(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo(TagVE, base_name, sizeof(std::vector<int>))
		{
			ti = (TypeInfo_Enum*)get(TagE, name, db);
		}

		std::string serialize(const void* p) const override
		{
			std::string ret;
			auto& vec = *(std::vector<int>*)p;
			p = vec.data();
			for (auto i = 0; i < vec.size(); i++)
			{
				ret += ti->serialize(p) + "\n";
				p = (char*)p + ti->size;
			}
			return ret;
		}
	};
}
