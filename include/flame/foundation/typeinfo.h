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

	struct TypeInfo
	{
		virtual TypeTag get_tag() const = 0;
		virtual bool get_is_array() const = 0;
		virtual const char* get_base_name() const = 0; // no space, 'unsigned ' will be replace to 'u'
		virtual uint get_base_hash() const = 0;
		virtual const char* get_name() const = 0; // tag[A]#base
		virtual uint get_hash() const = 0;

		inline bool is_pod() const
		{
			if (get_is_array())
				return false;
			auto tag = get_tag();
			if (tag == TypePointer)
				return false;
			auto base_hash = get_base_hash();
			if (tag == TypeData && (base_hash == FLAME_CHASH("flame::StringA") || base_hash == FLAME_CHASH("flame::StringW")))
				return false;
			return true;
		}

		inline std::string get_cpp_name() const
		{
			auto ret = std::string(get_base_name());
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
			if (get_is_array())
				ret = "Array<" + ret + ">";
			if (get_tag() == TypePointer)
				ret += "*";
			return ret;
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

		FLAME_FOUNDATION_EXPORTS static TypeInfo* get(TypeTag tag, const char* base_name, bool is_array = false);
		inline static TypeInfo* TypeInfo::get(const std::string& str)
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
		virtual UdtInfo* get_udt() const = 0;
		virtual uint get_index() const = 0;
		virtual TypeInfo* get_type() const = 0;
		virtual const char* get_name() const = 0;
		virtual uint get_name_hash() const = 0;
		virtual uint get_flags() const = 0;
		virtual uint get_offset() const = 0;
		virtual uint get_size() const = 0;
		virtual const void* get_default_value() const = 0;

	};

	struct EnumItem
	{
		virtual EnumInfo* get_enum() const = 0;
		virtual uint get_index() const = 0;
		virtual const char* get_name() const = 0;
		virtual int get_value() const = 0;
	};

	struct EnumInfo
	{
		virtual TypeInfoDatabase* get_database() const = 0;
		virtual const char* get_name() const = 0;
		virtual uint get_items_count() const = 0;
		virtual EnumItem* get_item(uint idx) const = 0;
		virtual EnumItem* find_item(const char* name) const = 0;
		virtual EnumItem* find_item(int value) const = 0;
	};

	struct FunctionInfo
	{
		virtual TypeInfoDatabase* get_database() const = 0;
		virtual UdtInfo* get_udt() const = 0;
		virtual uint get_index() const = 0;
		virtual const char* get_name() const = 0;
		virtual const void* get_rva() const = 0;
		virtual TypeInfo* get_type() const = 0;
		virtual uint get_parameters_count() const = 0;
		virtual TypeInfo* get_parameter(uint idx) const = 0;
		virtual const char* get_code() const = 0;
	};

	struct UdtInfo
	{
		virtual TypeInfoDatabase* get_database() const = 0;
		virtual const char* get_name() const = 0;
		virtual uint get_size() const = 0;
		virtual const char* get_base_name() const = 0; // base class name

		virtual uint get_variables_count() const = 0;
		virtual VariableInfo* get_variable(uint idx) const = 0;
		virtual VariableInfo* find_variable(const char* name) const = 0;
		virtual uint get_functions_count() const = 0;
		virtual FunctionInfo* get_function(uint idx) const = 0;
		virtual FunctionInfo* find_function(const char* name) const = 0;

		inline void serialize(const void* src, nlohmann::json& dst) const
		{
			auto count = get_variables_count();
			for (auto i = 0; i < count; i++)
			{
				auto v = get_variable(i);
				dst[v->get_name()] = v->get_type()->serialize((char*)src + v->get_offset());
			}
		}

		inline void unserialize(const nlohmann::json& src, const void* dst) const
		{
			auto count = get_variables_count();
			for (auto i = 0; i < count; i++)
			{
				auto v = get_variable(i);
				v->get_type()->unserialize(src[v->get_name()].get<std::string>(), (char*)dst + v->get_offset());
			}
		}
	};

	struct TypeInfoDatabase
	{
		virtual void release() = 0;

		virtual const void* get_library() const = 0;
		virtual const wchar_t* get_library_name() const = 0;

		virtual EnumInfo* get_enum(uint hash) const = 0;
		virtual UdtInfo* get_udt(uint hash) const = 0;

		FLAME_FOUNDATION_EXPORTS static TypeInfoDatabase* load(const wchar_t* library_filename, bool add_to_global, bool load_with_library);
	};

	FLAME_FOUNDATION_EXPORTS void push_global_typeinfo_database(TypeInfoDatabase* db);
	FLAME_FOUNDATION_EXPORTS void pop_global_typeinfo_database();

	FLAME_FOUNDATION_EXPORTS EnumInfo* find_enum(uint hash);
	FLAME_FOUNDATION_EXPORTS UdtInfo* find_udt(uint hash);

	inline bool check_function(FunctionInfo* info, const char* type, const std::vector<const char*>& parameters)
	{
		if (info->get_type()->get_hash() != FLAME_HASH(type) ||
			info->get_parameters_count() != parameters.size())
			return false;
		for (auto i = 0; i < parameters.size(); i++)
		{
			if (info->get_parameter(i)->get_hash() != FLAME_HASH(parameters[i]))
				return false;
		}
		return true;
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
		case FLAME_CHASH("flame::Vec<1,int>"):
		case FLAME_CHASH("flame::Vec<1,uint>"):
			return sizeof(Vec1i);
		case FLAME_CHASH("flame::Vec<2,int>"):
		case FLAME_CHASH("flame::Vec<2,uint>"):
			return sizeof(Vec2i);
		case FLAME_CHASH("flame::Vec<3,int>"):
		case FLAME_CHASH("flame::Vec<3,uint>"):
			return sizeof(Vec3i);
		case FLAME_CHASH("flame::Vec<4,int>"):
		case FLAME_CHASH("flame::Vec<4,uint>"):
			return sizeof(Vec4i);
		case FLAME_CHASH("int64"):
		case FLAME_CHASH("uint64"):
			return sizeof(int64);
		case FLAME_CHASH("float"):
			return sizeof(float);
		case FLAME_CHASH("flame::Vec<1,float>"):
			return sizeof(Vec1f);
		case FLAME_CHASH("flame::Vec<2,float>"):
			return sizeof(Vec2f);
		case FLAME_CHASH("flame::Vec<3,float>"):
			return sizeof(Vec3f);
		case FLAME_CHASH("flame::Vec<4,float>"):
			return sizeof(Vec4f);
		case FLAME_CHASH("uchar"):
			return sizeof(uchar);
		case FLAME_CHASH("flame::Vec<1,uchar>"):
			return sizeof(Vec1c);
		case FLAME_CHASH("flame::Vec<2,uchar>"):
			return sizeof(Vec2c);
		case FLAME_CHASH("flame::Vec<3,uchar>"):
			return sizeof(Vec3c);
		case FLAME_CHASH("flame::Vec<4,uchar>"):
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
		auto base_hash = get_base_hash();
		switch (get_tag())
		{
		case TypeEnumSingle:
		{
			auto e = find_enum(base_hash);
			assert(e);
			auto i = e->find_item(*(int*)src);
			return i ? i->get_name() : "";
		}
		case TypeEnumMulti:
		{
			std::string str;
			auto e = find_enum(base_hash);
			assert(e);
			auto v = *(int*)src;
			auto count = e->get_items_count();
			for (auto i = 0; i < count; i++)
			{
				if ((v & 1) == 1)
				{
					if (!str.empty())
						str += ";";
					str += e->find_item(1 << i)->get_name();
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
			case FLAME_CHASH("flame::Vec<1,int>"):
				return to_string(*(Vec1i*)src);
			case FLAME_CHASH("flame::Vec<2,int>"):
				return to_string(*(Vec2i*)src);
			case FLAME_CHASH("flame::Vec<3,int>"):
				return to_string(*(Vec3i*)src);
			case FLAME_CHASH("flame::Vec<4,int>"):
				return to_string(*(Vec4i*)src);
			case FLAME_CHASH("uint"):
				return std::to_string(*(uint*)src);
			case FLAME_CHASH("flame::Vec<1,uint>"):
				return to_string(*(Vec1u*)src);
			case FLAME_CHASH("flame::Vec<2,uint>"):
				return to_string(*(Vec2u*)src);
			case FLAME_CHASH("flame::Vec<3,uint>"):
				return to_string(*(Vec3u*)src);
			case FLAME_CHASH("flame::Vec<4,uint>"):
				return to_string(*(Vec4u*)src);
			case FLAME_CHASH("uint64"):
				return std::to_string(*(uint64*)src);
			case FLAME_CHASH("float"):
				return to_string(*(float*)src);
			case FLAME_CHASH("flame::Vec<1,float>"):
				return to_string(*(Vec1f*)src);
			case FLAME_CHASH("flame::Vec<2,float>"):
				return to_string(*(Vec2f*)src);
			case FLAME_CHASH("flame::Vec<3,float>"):
				return to_string(*(Vec3f*)src);
			case FLAME_CHASH("flame::Vec<4,float>"):
				return to_string(*(Vec4f*)src);
			case FLAME_CHASH("uchar"):
				return std::to_string(*(uchar*)src);
			case FLAME_CHASH("flame::Vec<1,uchar>"):
				return to_string(*(Vec1c*)src);
			case FLAME_CHASH("flame::Vec<2,uchar>"):
				return to_string(*(Vec2c*)src);
			case FLAME_CHASH("flame::Vec<3,uchar>"):
				return to_string(*(Vec3c*)src);
			case FLAME_CHASH("flame::Vec<4,uchar>"):
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
		auto base_hash = get_base_hash();
		switch (get_tag())
		{
		case TypeEnumSingle:
		{
			auto e = find_enum(base_hash);
			assert(e);
			*(int*)dst = e->find_item(src.c_str())->get_value();
		}
			return;
		case TypeEnumMulti:
		{
			auto v = 0;
			auto e = find_enum(base_hash);
			assert(e);
			auto sp = SUS::split(src, ';');
			for (auto& t : sp)
				v |= e->find_item(t.c_str())->get_value();
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
			case FLAME_CHASH("flame::Vec<1,int>"):
				*(Vec1u*)dst = std::stoi(src.c_str());
				break;
			case FLAME_CHASH("flame::Vec<2,int>"):
				*(Vec2u*)dst = stoi2(src.c_str());
				break;
			case FLAME_CHASH("flame::Vec<3,int>"):
				*(Vec3u*)dst = stoi3(src.c_str());
				break;
			case FLAME_CHASH("flame::Vec<4,int>"):
				*(Vec4u*)dst = stoi4(src.c_str());
				break;
			case FLAME_CHASH("uint"):
				*(uint*)dst = std::stoul(src);
				break;
			case FLAME_CHASH("flame::Vec<1,uint>"):
				*(Vec1u*)dst = std::stoul(src.c_str());
				break;
			case FLAME_CHASH("flame::Vec<2,uint>"):
				*(Vec2u*)dst = stou2(src.c_str());
				break;
			case FLAME_CHASH("flame::Vec<3,uint>"):
				*(Vec3u*)dst = stou3(src.c_str());
				break;
			case FLAME_CHASH("flame::Vec<4,uint>"):
				*(Vec4u*)dst = stou4(src.c_str());
				break;
			case FLAME_CHASH("uint64"):
				*(uint64*)dst = std::stoull(src);
				break;
			case FLAME_CHASH("float"):
				*(float*)dst = std::stof(src.c_str());
				break;
			case FLAME_CHASH("flame::Vec<1,float>"):
				*(Vec1f*)dst = std::stof(src.c_str());
				break;
			case FLAME_CHASH("flame::Vec<2,float>"):
				*(Vec2f*)dst = stof2(src.c_str());
				break;
			case FLAME_CHASH("flame::Vec<3,float>"):
				*(Vec3f*)dst = stof3(src.c_str());
				break;
			case FLAME_CHASH("flame::Vec<4,float>"):
				*(Vec4f*)dst = stof4(src.c_str());
				break;
			case FLAME_CHASH("uchar"):
				*(uchar*)dst = std::stoul(src);
				break;
			case FLAME_CHASH("flame::Vec<1,uchar>"):
				*(Vec1c*)dst = std::stoul(src.c_str());
				break;
			case FLAME_CHASH("flame::Vec<2,uchar>"):
				*(Vec2c*)dst = stoc2(src.c_str());
				break;
			case FLAME_CHASH("flame::Vec<3,uchar>"):
				*(Vec3c*)dst = stoc3(src.c_str());
				break;
			case FLAME_CHASH("flame::Vec<4,uchar>"):
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
		auto tag = get_tag();
		if (tag == TypeData)
			basic_type_copy(get_base_hash(), src, dst, size);
		else if (tag == TypeEnumSingle || tag == TypeEnumMulti)
			memcpy(dst, src, sizeof(int));
		else if (tag == TypePointer)
			memcpy(dst, src, sizeof(void*));
	}
}
