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

	struct TypeInfo;
	struct EnumInfo;
	struct VariableInfo;
	struct FunctionInfo;
	struct UdtInfo;
	struct TypeInfoDatabase;

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

	FLAME_FOUNDATION_EXPORTS extern HashMap<256, TypeInfo> typeinfos;

	struct TypeInfo
	{
		TypeTag tag;
		bool is_array;
		StringA base_name;
		uint base_hash;
		StringA name;  // tag[A]#base, order matters
		uint hash;

		TypeInfo(TypeTag tag, const std::string& base_name, bool is_array) :
			tag(tag),
			base_name(base_name),
			is_array(is_array)
		{
			base_hash = FLAME_HASH(base_name.c_str());
			name = make_str(tag, base_name, is_array);
			hash = FLAME_HASH(name.v);
		}

		static std::string make_str(TypeTag tag, const std::string& base_name, bool is_array = false)
		{
			std::string ret;
			ret = type_tag(tag);
			if (is_array)
				ret += "A";
			ret += "#" + base_name;
			return ret;
		}

		static void break_str(const std::string& str, TypeTag& tag, std::string& base_name, bool& is_array)
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

		static uint get_hash(TypeTag tag, const std::string& base_name, bool is_array = false)
		{
			return FLAME_HASH(make_str(tag, base_name, is_array).c_str());
		}

		static uint get_hash(const std::string& str)
		{
			TypeTag tag;
			std::string base_name;
			bool is_array;
			break_str(str, tag, base_name, is_array);
			return TypeInfo::get_hash(tag, base_name, is_array);
		}

		bool is_pod() const
		{
			if (is_array)
				return false;
			if (tag == TypePointer)
				return false;
			if (tag == TypeData && (base_hash == FLAME_CHASH("flame::StringA") || base_hash == FLAME_CHASH("flame::StringW")))
				return false;
			return true;
		}

		std::string get_cpp_name() const
		{
			std::string ret = tn_a2c(base_name.str());
			static FLAME_SAL(str_flame, "flame::");
			if (ret.compare(0, str_flame.l, str_flame.s) == 0)
				ret.erase(ret.begin(), ret.begin() + str_flame.l);
			std::regex reg_vec(R"(Vec<([0-9]+),(\w+)>)");
			std::smatch res;
			if (std::regex_search(ret, res, reg_vec))
			{
				auto t = res[2].str();
				ret = "Vec" + res[1].str() + (t == "uchar" ? 'c' : t[0]);
			}
			if (is_array)
				ret = "Array<" + ret + ">";
			if (tag == TypePointer)
				ret += "*";
			return ret;
		}

		static TypeInfo* get(TypeTag tag, const char* base_name, bool is_array = false)
		{
			auto hash = FLAME_HASH(make_str(tag, base_name, is_array).c_str());
			auto i = typeinfos.find(hash);
			if (i)
				return i;
			i = f_new<TypeInfo>(tag, base_name, is_array);
			typeinfos.add(hash, i);
			return i;
		}

		static TypeInfo* TypeInfo::get(const std::string& str)
		{
			TypeTag tag;
			std::string base_name;
			bool is_array;
			break_str(str, tag, base_name, is_array);
			return TypeInfo::get(tag, base_name.c_str(), is_array);
		}

		inline std::string serialize(const void* src) const;
		inline void unserialize(const std::string& src, void* dst) const;
		inline void copy_from(const void* src, void* dst, uint size = 0) const;
	};

	enum VariableFlags
	{
		VariableFlagInput = 1 << 0,
		VariableFlagOutput = 1 << 2,
		VariableFlagEnumMulti = 1 << 3
	};

	struct VariableInfo
	{
		UdtInfo* udt;
		TypeInfo* type;
		StringA name;
		uint name_hash;
		uint flags;
		uint offset;
		uint size;
		void* default_value;

		VariableInfo(UdtInfo* udt, TypeInfo* type, const std::string& name, uint flags, uint offset, uint size) :
			udt(udt),
			type(type),
			name(name),
			flags(flags),
			offset(offset),
			size(size),
			default_value(nullptr)
		{
			name_hash = FLAME_HASH(name.c_str());
		}

		~VariableInfo()
		{
			f_free(default_value);
		}
	};

	struct EnumItem
	{
		StringA name;
		int value;

		EnumItem(const std::string& name, int value) :
			name(name),
			value(value)
		{
		}
	};

	struct EnumInfo
	{
		TypeInfoDatabase* db;
		StringA name;
		Array<EnumItem*> items;

		EnumInfo(TypeInfoDatabase* db, const std::string& name) :
			db(db),
			name(name)
		{
		}

		~EnumInfo()
		{
			for (auto i : items)
				f_delete(i);
		}

		EnumItem* add_item(const std::string& name, int value)
		{
			auto i = f_new<EnumItem>(name, value);
			items.push_back(i);
			return i;
		}

		EnumItem* find_item(const std::string& name, int* out_idx = nullptr) const
		{
			for (auto i : items)
			{
				if (i->name == name)
				{
					if (out_idx)
						*out_idx = i->value;
					return i;
				}
			}
			if (out_idx)
				*out_idx = -1;
			return nullptr;
		}

		EnumItem* find_item(int value, int* out_idx = nullptr) const
		{
			for (auto i = 0; i < items.s; i++)
			{
				auto item = items[i];
				if (item->value == value)
				{
					if (out_idx)
						*out_idx = i;
					return item;
				}
			}
			if (out_idx)
				*out_idx = -1;
			return nullptr;
		}
	};

	struct FunctionInfo
	{
		TypeInfoDatabase* db;
		UdtInfo* udt;
		StringA name;
		void* rva;
		TypeInfo* type;
		Array<TypeInfo*> parameters;
		StringA code;

		FunctionInfo(TypeInfoDatabase* db, UdtInfo* udt, const std::string& name, void* rva, TypeInfo* type) :
			db(db),
			udt(udt),
			name(name),
			rva(rva),
			type(type)
		{
		}

		void add_parameter(TypeInfo* type)
		{
			parameters.push_back(type);
		}
	};

	struct UdtInfo
	{
		TypeInfoDatabase* db;
		StringA name;
		uint size;
		StringA base_name;
		Array<VariableInfo*> variables;
		Array<FunctionInfo*> functions;

		UdtInfo(TypeInfoDatabase* db, const std::string& name, uint size, const std::string& base_name) :
			db(db),
			name(name),
			size(size),
			base_name(base_name)
		{
		}

		~UdtInfo()
		{
			for (auto v : variables)
				f_delete(v);
			for (auto f : functions)
				f_delete(f);
		}

		VariableInfo* add_variable(TypeInfo* type, const std::string& name, uint flags, uint offset, uint size)
		{
			auto v = f_new<VariableInfo>(this, type, name, flags, offset, size);
			if (type->is_pod())
			{
				v->default_value = new char[size];
				memset(v->default_value, 0, size);
			}
			else
				v->default_value = nullptr;
			variables.push_back(v);
			return v;
		}

		FunctionInfo* add_function(const std::string& name, void* rva, TypeInfo* type)
		{
			auto f = f_new<FunctionInfo>(db, this, name, rva, type);
			functions.push_back(f);
			return f;
		}

		VariableInfo* find_variable(const std::string& name, int* out_idx = nullptr) const
		{
			for (auto i = 0; i < variables.s; i++)
			{
				auto v = variables[i];
				if (v->name == name)
				{
					if (out_idx)
						*out_idx = i;
					return v;
				}
			}
			if (out_idx)
				*out_idx = -1;
			return nullptr;
		}

		FunctionInfo* find_function(const std::string& name, int* out_idx = nullptr) const
		{
			for (auto i = 0; i < functions.s; i++)
			{
				auto f = functions[i];
				if (f->name == name)
				{
					if (out_idx)
						*out_idx = i;
					return f;
				}
			}
			if (out_idx)
				*out_idx = -1;
			return nullptr;
		}

		inline void serialize(const void* src, nlohmann::json& dst) const
		{
			for (auto v : variables)
				dst[v->name.str()] = v->type->serialize((char*)src + v->offset);
		}

		inline void unserialize(const nlohmann::json& src, const void* dst) const
		{
			for (auto v : variables)
				v->type->unserialize(src[v->name.str()].get<std::string>(), (char*)dst + v->offset);
		}
	};

	struct TypeInfoDatabase
	{
		void* library;
		StringW library_name;
		HashMap<256, EnumInfo> enums;
		HashMap<256, UdtInfo> udts;

		TypeInfoDatabase(const std::wstring& library_name) :
			library_name(library_name)
		{
			library = nullptr;
		}

		~TypeInfoDatabase()
		{
			if (library)
				free_library(library);
			for (auto e : enums.get_all())
				f_delete(e);
			for (auto u : udts.get_all())
				f_delete(u);
		}

		EnumInfo* add_enum(const std::string name)
		{
			auto e = f_new<EnumInfo>(this, name);
			enums.add(FLAME_HASH(name.c_str()), e);
			return e;
		}

		UdtInfo* add_udt(const std::string name, uint size, const std::string& base_name)
		{
			auto u = f_new<UdtInfo>(this, name, size, base_name);
			udts.add(FLAME_HASH(name.c_str()), u);
			return u;
		}

		FLAME_FOUNDATION_EXPORTS static TypeInfoDatabase* load(const wchar_t* library_filename, bool add_to_global, bool load_with_library);
		FLAME_FOUNDATION_EXPORTS static void destroy(TypeInfoDatabase* db);
	};

	FLAME_FOUNDATION_EXPORTS extern Array<TypeInfoDatabase*> global_typeinfo_databases;

	inline EnumInfo* find_enum(uint hash)
	{
		for (auto db : global_typeinfo_databases)
		{
			auto info = db->enums.find(hash);
			if (info)
				return info;
		}
		return nullptr;
	}

	inline UdtInfo* find_udt(uint hash)
	{
		for (auto db : global_typeinfo_databases)
		{
			auto info = db->udts.find(hash);
			if (info)
				return info;
		}
		return nullptr;
	}

	inline bool check_function(FunctionInfo* info, const char* type, const std::vector<const char*>& parameters)
	{
		if (info->type->hash != FLAME_HASH(type) ||
			info->parameters.s != parameters.size())
			return false;
		for (auto i = 0; i < parameters.size(); i++)
		{
			if (info->parameters[i]->hash != FLAME_HASH(parameters[i]))
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

	std::string TypeInfo::serialize(const void* src) const
	{
		switch (tag)
		{
		case TypeEnumSingle:
		{
			auto e = find_enum(base_hash);
			assert(e);
			auto i = e->find_item(*(int*)src);
			return i ? i->name.str() : "";
		}
		case TypeEnumMulti:
		{
			std::string str;
			auto e = find_enum(base_hash);
			assert(e);
			auto v = *(int*)src;
			for (auto i = 0; i < e->items.s; i++)
			{
				if ((v & 1) == 1)
				{
					if (!str.empty())
						str += ";";
					str += e->find_item(1 << i)->name.str();
				}
				v >>= 1;
			}
			return str;
		}
		case TypeData:
			switch (base_hash)
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
				return to_string(*(float*)src);
			case FLAME_CHASH("flame::Vec(1+float)"):
				return to_string(*(Vec1f*)src);
			case FLAME_CHASH("flame::Vec(2+float)"):
				return to_string(*(Vec2f*)src);
			case FLAME_CHASH("flame::Vec(3+float)"):
				return to_string(*(Vec3f*)src);
			case FLAME_CHASH("flame::Vec(4+float)"):
				return to_string(*(Vec4f*)src);
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
			case FLAME_CHASH("ListenerHub"):
				return "";
			default:
				assert(0);
			}
		}
		return "";
	}

	void TypeInfo::unserialize(const std::string& src, void* dst) const
	{
		switch (tag)
		{
		case TypeEnumSingle:
		{
			auto e = find_enum(base_hash);
			assert(e);
			e->find_item(src.c_str(), (int*)dst);
		}
			return;
		case TypeEnumMulti:
		{
			auto v = 0;
			auto e = find_enum(base_hash);
			assert(e);
			auto sp = SUS::split(src, ';');
			for (auto& t : sp)
				v |= e->find_item(t.c_str())->value;
			*(int*)dst = v;
		}
			return;
		case TypeData:
			switch (base_hash)
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
			case FLAME_CHASH("ListenerHub"):
				break;
			default:
				assert(0);
			}
			return;
		}
	}

	void TypeInfo::copy_from(const void* src, void* dst, uint size) const
	{
		if (tag == TypeData)
			basic_type_copy(base_hash, src, dst, size);
		else if (tag == TypeEnumSingle || tag == TypeEnumMulti)
			memcpy(dst, src, sizeof(int));
		else if (tag == TypePointer)
			memcpy(dst, src, sizeof(void*));
	}
}
