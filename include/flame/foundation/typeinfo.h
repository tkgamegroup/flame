#pragma once

#include <flame/serialize.h>
#include <flame/foundation/foundation.h>

namespace flame
{
	enum TypeTag
	{
		TypeEnumSingle,
		TypeEnumMulti,
		TypeData,
		TypePointer,

		TypeTagCount
	};

	inline char type_tag(TypeTag tag)
	{
		static char names[] = {
			'S',
			'M',
			'D',
			'P'
		};
		return names[tag];
	}

	struct EnumInfo;
	struct VariableInfo;
	struct FunctionInfo;
	struct UdtInfo;
	struct TypeinfoDatabase;

	// type name archive:
	// ， no space
	// ， 'unsigned ' will be replaced to 'u'
	// ， '< ' will be replaced to '('
	// ， '> ' will be replaced to ')'
	// ， ', ' will be replaced to '+'

	inline std::string tn_c2a(const std::string& name) // type name code to archive
	{
		auto ret = name;
		for (auto& ch : ret)
		{
			if (ch == '<')
				ch = '(';
			else if (ch == '>')
				ch = ')';
			else if (ch == ',')
				ch = '+';
		}
		return ret;
	}

	inline std::string tn_a2c(const std::string& name) // type name archive to code
	{
		auto ret = name;
		for (auto& ch : ret)
		{
			if (ch == '(')
				ch = '<';
			else if (ch == ')')
				ch = '>';
			else if (ch == '+')
				ch = ',';
		}
		return ret;
	}

	struct TypeInfo
	{
		FLAME_FOUNDATION_EXPORTS TypeTag tag() const;
		FLAME_FOUNDATION_EXPORTS bool is_array() const;
		FLAME_FOUNDATION_EXPORTS const char* base_name() const;
		FLAME_FOUNDATION_EXPORTS const char* name() const; // tag[A]#base, order matters
		FLAME_FOUNDATION_EXPORTS uint base_hash() const;
		FLAME_FOUNDATION_EXPORTS uint hash() const;

		inline static std::string make_str(TypeTag tag, const std::string& base_name, bool is_array = false)
		{
			std::string ret;
			ret = type_tag(tag);
			if (is_array)
				ret += "A";
			ret += "#" + base_name;
			return ret;
		}

		inline static void break_str(const std::string& str, TypeTag& tag, std::string& base_name, bool& is_array)
		{
			auto pos_hash = str.find('#');
			{
				auto ch = str[0];
				for (auto i = 0; i < TypeTagCount; i++)
				{
					if (type_tag((TypeTag)i) == ch)
					{
						tag = (TypeTag)i;
						break;
					}
				}
			}
			is_array = false;
			if (pos_hash > 1 && str[1] == 'A')
				is_array = true;
			base_name = std::string(str.begin() + pos_hash + 1, str.end());
		}

		inline static uint get_hash(TypeTag tag, const std::string& base_name, bool is_array = false)
		{
			return FLAME_HASH(make_str(tag, base_name, is_array).c_str());
		}

		inline static uint get_hash(const std::string& str)
		{
			TypeTag tag;
			std::string base_name;
			bool is_array;
			break_str(str, tag, base_name, is_array);
			return TypeInfo::get_hash(tag, base_name, is_array);
		}

		FLAME_FOUNDATION_EXPORTS static const TypeInfo* get(TypeTag tag, const char* base_name, bool is_array = false);
		FLAME_FOUNDATION_EXPORTS static const TypeInfo* get(const char* str);

		inline std::string serialize(const void* src, int precision) const;
		inline void unserialize(const std::string& src, void* dst) const;
		inline void copy_from(const void* src, void* dst) const;
	};

	enum VariableFlags
	{
		VariableFlagInput = 1 << 0,
		VariableFlagOutput = 1 << 2,
		VariableFlagEnumMulti = 1 << 3
	};

	struct VariableInfo
	{
		FLAME_FOUNDATION_EXPORTS UdtInfo* udt() const;

		FLAME_FOUNDATION_EXPORTS const TypeInfo* type() const;
		FLAME_FOUNDATION_EXPORTS const char* name() const;
		FLAME_FOUNDATION_EXPORTS uint name_hash() const;
		FLAME_FOUNDATION_EXPORTS uint flags() const;
		FLAME_FOUNDATION_EXPORTS uint offset() const;
		FLAME_FOUNDATION_EXPORTS uint size() const;
		FLAME_FOUNDATION_EXPORTS const char* default_value() const;
	};

	struct EnumItem
	{
		FLAME_FOUNDATION_EXPORTS const char* name() const;
		FLAME_FOUNDATION_EXPORTS int value() const;
	};

	struct EnumInfo
	{
		FLAME_FOUNDATION_EXPORTS TypeinfoDatabase* db() const;

		FLAME_FOUNDATION_EXPORTS const char* name() const;

		FLAME_FOUNDATION_EXPORTS uint item_count() const;
		FLAME_FOUNDATION_EXPORTS EnumItem* item(int idx) const;
		FLAME_FOUNDATION_EXPORTS EnumItem* find_item(const char* name, int* out_idx = nullptr) const;
		FLAME_FOUNDATION_EXPORTS EnumItem* find_item(int value, int* out_idx = nullptr) const;
	};

	struct FunctionInfo
	{
		FLAME_FOUNDATION_EXPORTS TypeinfoDatabase* db() const;

		FLAME_FOUNDATION_EXPORTS const char* name() const;
		FLAME_FOUNDATION_EXPORTS void* rva() const;
		FLAME_FOUNDATION_EXPORTS const TypeInfo* return_type() const;

		FLAME_FOUNDATION_EXPORTS uint parameter_count() const;
		FLAME_FOUNDATION_EXPORTS const TypeInfo* parameter_type(uint idx) const;

	};

	struct UdtInfo
	{
		FLAME_FOUNDATION_EXPORTS TypeinfoDatabase* db() const;

		FLAME_FOUNDATION_EXPORTS const TypeInfo* type() const;
		FLAME_FOUNDATION_EXPORTS uint size() const;

		FLAME_FOUNDATION_EXPORTS uint variable_count() const;
		FLAME_FOUNDATION_EXPORTS VariableInfo* variable(uint idx) const;
		FLAME_FOUNDATION_EXPORTS VariableInfo* find_variable(const char* name, int* out_idx = nullptr) const;

		FLAME_FOUNDATION_EXPORTS uint function_count() const;
		FLAME_FOUNDATION_EXPORTS FunctionInfo* function(uint idx) const;
		FLAME_FOUNDATION_EXPORTS FunctionInfo* find_function(const char* name, int* out_idx = nullptr) const;

		inline void serialize(const void* src, int precision, nlohmann::json& dst) const
		{
			for (auto i = 0; i < variable_count(); i++)
			{
				auto v = variable(i);
				dst[v->name()] = v->type()->serialize((char*)src + v->offset(), precision);
			}
		}

		inline void unserialize(const nlohmann::json& src, const void* dst) const
		{
			for (auto i = 0; i < variable_count(); i++)
			{
				auto v = variable(i);
				v->type()->unserialize(src[v->name()].get<std::string>(), (char*)dst + v->offset());
			}
		}
	};

	struct TypeinfoDatabase
	{
		FLAME_FOUNDATION_EXPORTS void* module() const;
		FLAME_FOUNDATION_EXPORTS const wchar_t* module_name() const;

		FLAME_FOUNDATION_EXPORTS Array<EnumInfo*> get_enums();
		FLAME_FOUNDATION_EXPORTS EnumInfo* find_enum(uint name_hash);

		FLAME_FOUNDATION_EXPORTS Array<UdtInfo*> get_udts();
		FLAME_FOUNDATION_EXPORTS UdtInfo* find_udt(uint name_hash);

		FLAME_FOUNDATION_EXPORTS static TypeinfoDatabase* load(const wchar_t* module_filename, bool add_to_global, bool load_with_module);
		FLAME_FOUNDATION_EXPORTS static void destroy(TypeinfoDatabase* db);
	};

	FLAME_FOUNDATION_EXPORTS uint global_db_count();
	FLAME_FOUNDATION_EXPORTS TypeinfoDatabase* global_db(uint idx);
	FLAME_FOUNDATION_EXPORTS extern uint extra_global_db_count;
	FLAME_FOUNDATION_EXPORTS extern TypeinfoDatabase* const* extra_global_dbs;

	inline EnumInfo* find_enum(uint name_hash)
	{
		for (auto i = 0; i < global_db_count(); i++)
		{
			auto db = global_db(i);
			auto info = db->find_enum(name_hash);
			if (info)
				return info;
		}
		for (auto i = 0; i < extra_global_db_count; i++)
		{
			auto db = extra_global_dbs[i];
			auto info = db->find_enum(name_hash);
			if (info)
				return info;
		}
		return nullptr;
	}

	inline UdtInfo* find_udt(uint name_hash)
	{
		for (auto i = 0; i < global_db_count(); i++)
		{
			auto db = global_db(i);
			auto info = db->find_udt(name_hash);
			if (info)
				return info;
		}
		for (auto i = 0; i < extra_global_db_count; i++)
		{
			auto db = extra_global_dbs[i];
			auto info = db->find_udt(name_hash);
			if (info)
				return info;
		}
		return nullptr;
	}

	inline bool check_function(FunctionInfo* info, const char* return_type, const std::vector<const char*>& parameter_types)
	{
		TypeTag tag;
		std::string base_name;
		bool is_array;
		TypeInfo::break_str(return_type, tag, base_name, is_array);
		if (info->return_type()->hash() != TypeInfo::get_hash(tag, base_name, is_array) ||
			info->parameter_count() != parameter_types.size())
			return false;
		for (auto i = 0; i < parameter_types.size(); i++)
		{
			TypeInfo::break_str(parameter_types[i], tag, base_name, is_array);
			if (info->parameter_type(i)->hash() != TypeInfo::get_hash(tag, base_name, is_array))
				return false;
		}
		return true;
	}

	inline auto& basic_types()
	{
		static const char* types[] = {
			"bool",
			"int",
			"uint",
			"flame::Vec(1+int)",
			"flame::Vec(2+int)",
			"flame::Vec(3+int)",
			"flame::Vec(4+int)",
			"flame::Vec(1+uint)",
			"flame::Vec(2+uint)",
			"flame::Vec(3+uint)",
			"flame::Vec(4+uint)",
			"longlong",
			"ulonglong",
			"float",
			"flame::Vec(1+float)",
			"flame::Vec(2+float)",
			"flame::Vec(3+float)",
			"flame::Vec(4+float)",
			"uchar",
			"flame::Vec(1+uchar)",
			"flame::Vec(2+uchar)",
			"flame::Vec(3+uchar)",
			"flame::Vec(4+uchar)",
			"flame::StringA",
			"flame::StringW"
		};
		return types;
	}

	inline uint basic_type_size(uint type_hash)
	{
		switch (type_hash)
		{
		case FLAME_CHASH("bool"):
			return sizeof(bool);
		case FLAME_CHASH("int"):
		case FLAME_CHASH("uint"):
			return sizeof(int);
		case FLAME_CHASH("flame::Vec(1+int)"):
		case FLAME_CHASH("flame::Vec(1+uint)"):
			return sizeof(Vec1i);
		case FLAME_CHASH("flame::Vec(2+int)"):
		case FLAME_CHASH("flame::Vec(2+uint)"):
			return sizeof(Vec2i);
		case FLAME_CHASH("flame::Vec(3+int)"):
		case FLAME_CHASH("flame::Vec(3+uint)"):
			return sizeof(Vec3i);
		case FLAME_CHASH("flame::Vec(4+int)"):
		case FLAME_CHASH("flame::Vec(4+uint)"):
			return sizeof(Vec4i);
		case FLAME_CHASH("longlong"):
		case FLAME_CHASH("ulonglong"):
			return sizeof(longlong);
		case FLAME_CHASH("float"):
			return sizeof(float);
		case FLAME_CHASH("flame::Vec(1+float)"):
			return sizeof(Vec1f);
		case FLAME_CHASH("flame::Vec(2+float)"):
			return sizeof(Vec2f);
		case FLAME_CHASH("flame::Vec(3+float)"):
			return sizeof(Vec3f);
		case FLAME_CHASH("flame::Vec(4+float)"):
			return sizeof(Vec4f);
		case FLAME_CHASH("uchar"):
			return sizeof(uchar);
		case FLAME_CHASH("flame::Vec(1+uchar)"):
			return sizeof(Vec1c);
		case FLAME_CHASH("flame::Vec(2+uchar)"):
			return sizeof(Vec2c);
		case FLAME_CHASH("flame::Vec(3+uchar)"):
			return sizeof(Vec3c);
		case FLAME_CHASH("flame::Vec(4+uchar)"):
			return sizeof(Vec4c);
		case FLAME_CHASH("flame::StringA"):
			return sizeof(StringA);
		case FLAME_CHASH("flame::StringW"):
			return sizeof(StringW);
		}
		return 0;
	}

	inline void basic_type_copy(uint type_hash, const void* src, void* dst, uint size = 0)
	{
		switch (type_hash)
		{
		case FLAME_CHASH("flame::StringA"):
			*(StringA*)dst = *(StringA*)src;
			return;
		case FLAME_CHASH("flame::StringW"):
			*(StringW*)dst = *(StringW*)src;
			return;
		}

		memcpy(dst, src, size ? size : basic_type_size(type_hash));
	}

	inline void basic_type_dtor(uint type_hash, void* p)
	{
		switch (type_hash)
		{
		case FLAME_CHASH("flame::StringA"):
			((StringA*)p)->~String();
			return;
		case FLAME_CHASH("flame::StringW"):
			((StringW*)p)->~String();
			return;
		}
	}

	std::string TypeInfo::serialize(const void* src, int precision) const
	{
		switch (tag())
		{
		case TypeEnumSingle:
		{
			auto e = find_enum(base_hash());
			assert(e);
			return e->find_item(*(int*)src)->name();
		}
		case TypeEnumMulti:
		{
			std::string str;
			auto e = find_enum(base_hash());
			assert(e);
			auto v = *(int*)src;
			for (auto i = 0; i < e->item_count(); i++)
			{
				if ((v & 1) == 1)
				{
					if (!str.empty())
						str += ";";
					str += e->find_item(1 << i)->name();
				}
				v >>= 1;
			}
			return str;
		}
		case TypeData:
			switch (base_hash())
			{
			case FLAME_CHASH("bool"):
				return *(bool*)src ? "1" : "0";
			case FLAME_CHASH("int"):
				return std::to_string(*(int*)src);
			case FLAME_CHASH("flame::Vec(1+int)"):
				return to_string(*(Vec1i*)src);
			case FLAME_CHASH("flame::Vec(2+int)"):
				return to_string(*(Vec2i*)src);
			case FLAME_CHASH("flame::Vec(3+int)"):
				return to_string(*(Vec3i*)src);
			case FLAME_CHASH("flame::Vec(4+int)"):
				return to_string(*(Vec4i*)src);
			case FLAME_CHASH("uint"):
				return std::to_string(*(uint*)src);
			case FLAME_CHASH("flame::Vec(1+uint)"):
				return to_string(*(Vec1u*)src);
			case FLAME_CHASH("flame::Vec(2+uint)"):
				return to_string(*(Vec2u*)src);
			case FLAME_CHASH("flame::Vec(3+uint)"):
				return to_string(*(Vec3u*)src);
			case FLAME_CHASH("flame::Vec(4+uint)"):
				return to_string(*(Vec4u*)src);
			case FLAME_CHASH("ulonglong"):
				return std::to_string(*(ulonglong*)src);
			case FLAME_CHASH("float"):
				return to_string(*(float*)src, precision);
			case FLAME_CHASH("flame::Vec(1+float)"):
				return to_string(*(Vec1f*)src, precision);
			case FLAME_CHASH("flame::Vec(2+float)"):
				return to_string(*(Vec2f*)src, precision);
			case FLAME_CHASH("flame::Vec(3+float)"):
				return to_string(*(Vec3f*)src, precision);
			case FLAME_CHASH("flame::Vec(4+float)"):
				return to_string(*(Vec4f*)src, precision);
			case FLAME_CHASH("uchar"):
				return std::to_string(*(uchar*)src);
			case FLAME_CHASH("flame::Vec(1+uchar)"):
				return to_string(*(Vec1c*)src);
			case FLAME_CHASH("flame::Vec(2+uchar)"):
				return to_string(*(Vec2c*)src);
			case FLAME_CHASH("flame::Vec(3+uchar)"):
				return to_string(*(Vec3c*)src);
			case FLAME_CHASH("flame::Vec(4+uchar)"):
				return to_string(*(Vec4c*)src);
			case FLAME_CHASH("flame::StringA"):
				return ((StringA*)src)->str();
			case FLAME_CHASH("flame::StringW"):
				return w2s(((StringW*)src)->str());
			default:
				assert(0);
			}
		}
	}

	void TypeInfo::unserialize(const std::string& src, void* dst) const
	{
		switch (tag())
		{
		case TypeEnumSingle:
		{
			auto e = find_enum(base_hash());
			assert(e);
			e->find_item(src.c_str(), (int*)dst);
		}
		return;
		case TypeEnumMulti:
		{
			auto v = 0;
			auto e = find_enum(base_hash());
			assert(e);
			auto sp = SUS::split(src, ';');
			for (auto& t : sp)
				v |= e->find_item(t.c_str())->value();
			*(int*)dst = v;
		}
		return;
		case TypeData:
			switch (base_hash())
			{
			case FLAME_CHASH("bool"):
				*(bool*)dst = (src != "0");
				break;
			case FLAME_CHASH("int"):
				*(int*)dst = std::stoi(src);
				break;
			case FLAME_CHASH("flame::Vec(1+int)"):
				*(Vec1u*)dst = std::stoi(src.c_str());
				break;
			case FLAME_CHASH("flame::Vec(2+int)"):
				*(Vec2u*)dst = stoi2(src.c_str());
				break;
			case FLAME_CHASH("flame::Vec(3+int)"):
				*(Vec3u*)dst = stoi3(src.c_str());
				break;
			case FLAME_CHASH("flame::Vec(4+int)"):
				*(Vec4u*)dst = stoi4(src.c_str());
				break;
			case FLAME_CHASH("uint"):
				*(uint*)dst = std::stoul(src);
				break;
			case FLAME_CHASH("flame::Vec(1+uint)"):
				*(Vec1u*)dst = std::stoul(src.c_str());
				break;
			case FLAME_CHASH("flame::Vec(2+uint)"):
				*(Vec2u*)dst = stou2(src.c_str());
				break;
			case FLAME_CHASH("flame::Vec(3+uint)"):
				*(Vec3u*)dst = stou3(src.c_str());
				break;
			case FLAME_CHASH("flame::Vec(4+uint)"):
				*(Vec4u*)dst = stou4(src.c_str());
				break;
			case FLAME_CHASH("ulonglong"):
				*(ulonglong*)dst = std::stoull(src);
				break;
			case FLAME_CHASH("float"):
				*(float*)dst = std::stof(src.c_str());
				break;
			case FLAME_CHASH("flame::Vec(1+float)"):
				*(Vec1f*)dst = std::stof(src.c_str());
				break;
			case FLAME_CHASH("flame::Vec(2+float)"):
				*(Vec2f*)dst = stof2(src.c_str());
				break;
			case FLAME_CHASH("flame::Vec(3+float)"):
				*(Vec3f*)dst = stof3(src.c_str());
				break;
			case FLAME_CHASH("flame::Vec(4+float)"):
				*(Vec4f*)dst = stof4(src.c_str());
				break;
			case FLAME_CHASH("uchar"):
				*(uchar*)dst = std::stoul(src);
				break;
			case FLAME_CHASH("flame::Vec(1+uchar)"):
				*(Vec1c*)dst = std::stoul(src.c_str());
				break;
			case FLAME_CHASH("flame::Vec(2+uchar)"):
				*(Vec2c*)dst = stoc2(src.c_str());
				break;
			case FLAME_CHASH("flame::Vec(3+uchar)"):
				*(Vec3c*)dst = stoc3(src.c_str());
				break;
			case FLAME_CHASH("flame::Vec(4+uchar)"):
				*(Vec4c*)dst = stoc4(src.c_str());
				break;
			case FLAME_CHASH("flame::StringA"):
				*(StringA*)dst = src;
				break;
			case FLAME_CHASH("flame::StringW"):
				*(StringW*)dst = s2w(src);
				break;
			default:
				assert(0);
			}
			return;
		}
	}

	void TypeInfo::copy_from(const void* src, void* dst) const
	{
		if (tag() == TypeData)
			basic_type_copy(base_hash(), src, dst);
		else if (tag() == TypeEnumSingle || tag() == TypeEnumMulti)
			memcpy(dst, src, sizeof(int));
		else if (tag() == TypePointer)
			memcpy(dst, src, sizeof(void*));
	}
}
