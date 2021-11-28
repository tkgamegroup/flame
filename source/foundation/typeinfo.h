#pragma once

#include "../serialize.h"
#include "foundation.h"

namespace flame
{
	enum TypeTag
	{
		TypeEnumSingle,
		TypeEnumMulti,
		TypeData,
		TypePointer,
		TypeFunction,

		TypeTagCount
	};

	enum BasicType
	{
		VoidType,
		BooleanType,
		IntegerType,
		FloatingType,
		CharType,
		WideCharType,
		ElseType
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

		BasicType basic_type = ElseType;
		bool is_signed = true;
		uint vec_size = 1;
		uint col_size = 1;
		TypeInfo* pointed_type = nullptr;

		inline static std::string format_name(const std::string& str)
		{
			auto ret = str;
			SUS::replace_all(ret, "enum ", "");
			SUS::replace_all(ret, "struct ", "");
			SUS::replace_all(ret, "class ", "");
			SUS::replace_all(ret, "unsigned ", "u");
			SUS::replace_all(ret, "__int64 ", "int64");
			SUS::replace_all(ret, "Private", "");
			SUS::remove_ch(ret, ' ');
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

		virtual void* create(bool create_pointing = true) const { return malloc(size); }
		virtual void destroy(void* p, bool destroy_pointing = true) const { free(p); }
		virtual void copy(void* dst, const void* src) const { memcpy(dst, src, size); }
		virtual bool compare(const void* d1, const void* d2) const { return memcmp(d1, d2, size) == 0; }
		virtual std::string serialize(const void* p) const { return ""; }
		virtual void unserialize(const std::string& str, void* p) const {}

		FLAME_FOUNDATION_EXPORTS static TypeInfo* get(TypeTag tag, const std::string& name, TypeInfoDataBase& db = tidb);

		template<class T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
		static TypeInfo* get()
		{
			auto get_type = [](const std::string& name) {
				return get(name.ends_with("Flags") ? TypeEnumMulti : TypeEnumSingle, name, tidb);
			};
			static auto ret = get_type(format_name(typeid(T).name()));
			return ret;
		}

		template<class T, std::enable_if_t<!std::is_enum_v<T>, int> = 0>
		static TypeInfo* get()
		{
			static auto ret = get(TypeData, format_name(typeid(T).name()), tidb);
			return ret;
		}

		template <class T>
		static std::string serialize_t(T* v)
		{
			return get<T>()->serialize(v);
		}

		template <class T>
		static void unserialize_t(const std::string& str, T* v)
		{
			return get<T>()->unserialize(str, v);
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

	struct VariableInfo
	{
		UdtInfo* udt;
		uint index;
		TypeInfo* type;
		std::string name;
		uint offset;
		uint array_size;
		uint array_stride;
		std::string default_value;
		Metas metas;
	};

	struct EnumItemInfo
	{
		EnumInfo* ei;
		uint index;
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
		UdtInfo* udt;
		uint index;
		std::string name;
		uint rva;
		int voff;
		bool is_static;
		TypeInfo* type;
		std::string code;
		std::vector<TypeInfo*> parameters;
		Metas metas;
		void* library;

		inline bool check(TypeInfo* ret, const std::vector<TypeInfo*> parms) const
		{
			if (type != ret || parameters.size() != parms.size())
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

	struct UdtInfo
	{
		std::string name;
		uint size;
		// base class name
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
}
