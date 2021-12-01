#pragma once

#include "foundation.h"

namespace flame
{
	enum TypeTag
	{
		TagEnumSingle,
		TagEnumMulti,
		TagData,
		TagPointer,
		TagFunction,
		TagUdt,
		TagVector,

		TagCount
	};

	enum BasicType
	{
		VoidType,
		BooleanType,
		IntegerType,
		FloatType,
		CharType,
		WideCharType
	};

	enum TypeMeta
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

		inline static std::string format_name(const std::string& str)
		{
			auto ret = str;
			SUS::replace_all(ret, "enum ", "");
			SUS::replace_all(ret, "struct ", "");
			SUS::replace_all(ret, "class ", "");
			SUS::replace_all(ret, "unsigned ", "u");
			SUS::replace_all(ret, "__int64 ", "int64");
			SUS::replace_all(ret, "Private", "");
			SUS::remove_char(ret, ' ');
			return ret;
		}

		TypeInfo(TypeTag tag, std::string_view _name, uint size) :
			tag(tag),
			name(_name),
			size(size)
		{
			hash = ch(name.c_str());
			hash ^= std::hash<int>()(tag);
		}

		virtual ~TypeInfo() {}

		virtual void* create() const { return malloc(size); }
		virtual void destroy(void* p) const { free(p); }
		virtual void copy(void* dst, const void* src) const { memcpy(dst, src, size); }
		virtual bool compare(const void* d1, const void* d2) const { return memcmp(d1, d2, size) == 0; }
		virtual std::string serialize(const void* p) const { return ""; }
		virtual void unserialize(const std::string& str, void* p) const {}

		FLAME_FOUNDATION_EXPORTS static TypeInfo* get(TypeTag tag, const std::string& name, TypeInfoDataBase& db = tidb);

		template<enum_type T>
		static TypeInfo* get(TypeInfoDataBase& db = tidb)
		{
			auto get_type = [&](const std::string& name) {
				return get(name.ends_with("Flags") ? TagEnumMulti : TagEnumSingle, name, db);
			};
			static auto ret = get_type(format_name(typeid(T).name()));
			return ret;
		}

		template<not_enum_type T>
		static TypeInfo* get(TypeInfoDataBase& db = tidb)
		{
			static auto ret = get(TagData, format_name(typeid(T).name()), db);
			return ret;
		}

		template <class T>
		static std::string serialize_t(T* v, TypeInfoDataBase& db = tidb)
		{
			return get<T>(db)->serialize(v);
		}

		template <class T>
		static void unserialize_t(const std::string& str, T* v, TypeInfoDataBase& db = tidb)
		{
			return get<T>(db)->unserialize(str, v);
		}
	};

	struct Metas
	{
		std::vector<std::pair<TypeMeta, LightCommonValue>> d;

		void from_string(const std::string& str, TypeInfoDataBase& db = tidb);
		std::string to_string(TypeInfoDataBase& db = tidb) const;

		inline bool get(TypeMeta m, LightCommonValue* v) const
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

		FLAME_FOUNDATION_EXPORTS void load_typeinfo(const std::filesystem::path& filename);
		FLAME_FOUNDATION_EXPORTS void save_typeinfo(const std::filesystem::path& filename);
	};

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

	struct TypeInfo_Data : TypeInfo
	{
		BasicType basic_type = VoidType;
		bool is_signed = true;
		uint vec_size = 1;
		uint col_size = 1;

		TypeInfo_Data(std::string_view name, uint size) :
			TypeInfo(TagData, name, size)
		{
		}
	};

	struct TypeInfo_void : TypeInfo_Data
	{
		TypeInfo_void() :
			TypeInfo_Data("void", 0)
		{
			basic_type = VoidType;
		}
	};

	struct TypeInfo_bool : TypeInfo_Data
	{
		TypeInfo_bool() :
			TypeInfo_Data("bool", sizeof(bool))
		{
			basic_type = BooleanType;
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
			basic_type = CharType;
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
			basic_type = CharType;
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
			basic_type = WideCharType;
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
			basic_type = IntegerType;
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
			basic_type = IntegerType;
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
			basic_type = IntegerType;
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
			basic_type = IntegerType;
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
			basic_type = IntegerType;
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
			basic_type = IntegerType;
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
			basic_type = FloatType;
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
			basic_type = IntegerType;
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
			basic_type = IntegerType;
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
			basic_type = IntegerType;
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
			basic_type = IntegerType;
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
			basic_type = IntegerType;
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
			basic_type = IntegerType;
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
			basic_type = CharType;
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
			basic_type = CharType;
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
			basic_type = CharType;
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
			basic_type = FloatType;
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
			basic_type = FloatType;
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
			basic_type = FloatType;
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
			basic_type = FloatType;
			vec_size = 2;
			col_size = 2;
		}
	};

	struct TypeInfo_mat3 : TypeInfo_Data
	{
		TypeInfo_mat3() :
			TypeInfo_Data("glm::mat3", sizeof(mat3))
		{
			basic_type = FloatType;
			vec_size = 3;
			col_size = 3;
		}
	};

	struct TypeInfo_mat4 : TypeInfo_Data
	{
		TypeInfo_mat4() :
			TypeInfo_Data("glm::mat4", sizeof(mat4))
		{
			basic_type = FloatType;
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

	struct TypeInfo_Rect: TypeInfo_Data
	{
		TypeInfo_Rect() :
			TypeInfo_Data("flame::Rect", sizeof(Rect))
		{
			basic_type = FloatType;
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
			basic_type = FloatType;
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
			basic_type = FloatType;
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

	struct TypeInfo_EnumSingle : TypeInfo
	{
		EnumInfo* ei = nullptr;

		TypeInfo_EnumSingle(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo(TagEnumSingle, base_name, sizeof(int))
		{
			ei = find_enum(name, db);
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

	struct TypeInfo_EnumMulti : TypeInfo
	{
		EnumInfo* ei = nullptr;

		TypeInfo_EnumMulti(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo(TagEnumMulti, base_name, sizeof(int))
		{
			ei = find_enum(name, db);
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

	struct TypeInfo_Pointer : TypeInfo
	{
		TypeInfo* ti = nullptr;

		TypeInfo_Pointer(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo(TagPointer, base_name, sizeof(void*))
		{
			ti = TypeInfo::get(TagData, name, db);
		}
	};

	struct TypeInfo_Udt : TypeInfo
	{
		UdtInfo* ui = nullptr;

		TypeInfo_Udt(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo(TagUdt, base_name, 0)
		{
			ui = find_udt(name, db);
			if (ui)
				size = ui->size;
		}
	};

	struct TypeInfo_Vector : TypeInfo
	{
		TypeInfo* ti = nullptr;

		TypeInfo_Vector(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo(TagVector, base_name, sizeof(std::vector<int>))
		{
			if (find_enum(name, db))
				ti = TypeInfo::get(name.ends_with("Flags") ? TagEnumMulti : TagEnumSingle, name, db);
			else
				ti = TypeInfo::get(TagData, name, db);
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
