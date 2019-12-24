#pragma once

#include <flame/foundation/foundation.h>

namespace flame
{
	inline std::string to_string(int v)
	{
		char buf[20];
		sprintf(buf, "%d", v);
		return buf;
	}

	inline std::string to_string(uint v)
	{
		char buf[20];
		sprintf(buf, "%d", v);
		return buf;
	}

	inline std::string to_string(float v, int precision = 6)
	{
		char buf[20];
		sprintf(buf, "%.*f", precision, v);
		return buf;
	}

	inline std::string to_string(uchar v)
	{
		char buf[20];
		sprintf(buf, "%d", v);
		return buf;
	}

	template<uint N>
	inline std::string to_string(const Vec<N, uint>& v)
	{
		auto ret = to_string(v[0]);
		for (auto i = 1; i < N; i++)
			ret += ";" + to_string(v[i]);
		return ret;
	}

	template<uint N>
	inline std::string to_string(const Vec<N, int>& v)
	{
		auto ret = to_string(v[0]);
		for (auto i = 1; i < N; i++)
			ret += ";" + to_string(v[i]);
		return ret;
	}

	template<uint N>
	inline std::string to_string(const Vec<N, float>& v, int precision = 6)
	{
		auto ret = to_string(v[0], precision);
		for (auto i = 1; i < N; i++)
			ret += ";" + to_string(v[i], precision);
		return ret;
	}

	template<uint N>
	inline std::string to_string(const Vec<N, uchar>& v)
	{
		auto ret = to_string(v[0]);
		for (auto i = 1; i < N; i++)
			ret += ";" + to_string(v[i]);
		return ret;
	}

	inline std::wstring to_wstring(int v)
	{
		wchar_t buf[20];
		swprintf(buf, L"%d", v);
		return buf;
	}

	inline std::wstring to_wstring(uint v)
	{
		wchar_t buf[20];
		swprintf(buf, L"%d", v);
		return buf;
	}


	inline std::wstring to_wstring(float v, int precision = 6)
	{
		wchar_t buf[20];
		swprintf(buf, L"%.*f", precision, v);
		return buf;
	}

	inline std::wstring to_wstring(uchar v)
	{
		wchar_t buf[20];
		swprintf(buf, L"%d", v);
		return buf;
	}

	template<uint N>
	inline std::wstring to_wstring(const Vec<N, uint>& v)
	{
		auto ret = to_wstring(v[0]);
		for (auto i = 1; i < N; i++)
			ret += L";" + to_wstring(v[i]);
		return ret;
	}

	template<uint N>
	inline std::wstring to_wstring(const Vec<N, int>& v)
	{
		auto ret = to_wstring(v[0]);
		for (auto i = 1; i < N; i++)
			ret += L";" + to_wstring(v[i]);
		return ret;
	}

	template<uint N>
	inline std::wstring to_wstring(const Vec<N, float>& v, int precision = 6)
	{
		auto ret = to_wstring(v[0], precision);
		for (auto i = 1; i < N; i++)
			ret += L";" + to_wstring(v[i], precision);
		return ret;
	}

	template<uint N>
	inline std::wstring to_wstring(const Vec<N, uchar>& v)
	{
		auto ret = to_wstring(v[0]);
		for (auto i = 1; i < N; i++)
			ret += L";" + to_wstring(v[i]);
		return ret;
	}

	inline Vec2f stof2(const char* s)
	{
		Vec2f ret;
		sscanf(s, "%f;%f", &ret.x(), &ret.y());
		return ret;
	}

	inline Vec3f stof3(const char* s)
	{
		Vec3f ret;
		sscanf(s, "%f;%f;%f", &ret.x(), &ret.y(), &ret.z());
		return ret;
	}

	inline Vec4f stof4(const char* s)
	{
		Vec4f ret;
		sscanf(s, "%f;%f;%f;%f", &ret.x(), &ret.y(), &ret.z(), &ret.w());
		return ret;
	}

	inline Vec2u stou2(const char* s)
	{
		Vec2u ret;
		sscanf(s, "%u;%u", &ret.x(), &ret.y());
		return ret;
	}

	inline Vec3u stou3(const char* s)
	{
		Vec3u ret;
		sscanf(s, "%u;%u;%u", &ret.x(), &ret.y(), &ret.z());
		return ret;
	}

	inline Vec4u stou4(const char* s)
	{
		Vec4u ret;
		sscanf(s, "%u;%u;%u;%u", &ret.x(), &ret.y(), &ret.z(), &ret.w());
		return ret;
	}

	inline Vec2i stoi2(const char* s)
	{
		Vec2i ret;
		sscanf(s, "%d;%d", &ret.x(), &ret.y());
		return ret;
	}

	inline Vec3i stoi3(const char* s)
	{
		Vec3i ret;
		sscanf(s, "%d;%d;%d", &ret.x(), &ret.y(), &ret.z());
		return ret;
	}

	inline Vec4i stoi4(const char* s)
	{
		Vec4i ret;
		sscanf(s, "%d;%d;%d;%d", &ret.x(), &ret.y(), &ret.z(), &ret.w());
		return ret;
	}

	inline Vec2c stoc2(const char* s)
	{
		Vec2i ret;
		sscanf(s, "%d;%d", &ret.x(), &ret.y());
		return Vec2c(ret);
	}

	inline Vec3c stoc3(const char* s)
	{
		Vec3i ret;
		sscanf(s, "%d;%d;%d", &ret.x(), &ret.y(), &ret.z());
		return Vec3c(ret);
	}

	inline Vec4c stoc4(const char* s)
	{
		Vec4i ret;
		sscanf(s, "%d;%d;%d;%d", &ret.x(), &ret.y(), &ret.z(), &ret.w());
		return Vec4c(ret);
	}

	template<class T>
	T sto(const char* s); 

	template<>
	inline int sto<int>(const char* s)
	{
		return std::stoi(s);
	}

	template<>
	inline uint sto<uint>(const char* s)
	{
		return std::stoul(s);
	}

	template<>
	inline float sto<float>(const char* s)
	{
		return std::stof(s);
	}

	template<>
	inline uchar sto<uchar>(const char* s)
	{
		return std::stoul(s);
	}

	inline Vec2f stof2(const wchar_t* s)
	{
		Vec2f ret;
		swscanf(s, L"%f;%f", &ret.x(), &ret.y());
		return ret;
	}

	inline Vec3f stof3(const wchar_t* s)
	{
		Vec3f ret;
		swscanf(s, L"%f;%f;%f", &ret.x(), &ret.y(), &ret.z());
		return ret;
	}

	inline Vec4f stof4(const wchar_t* s)
	{
		Vec4f ret;
		swscanf(s, L"%f;%f;%f;%f", &ret.x(), &ret.y(), &ret.z(), &ret.w());
		return ret;
	}

	inline Vec2u stou2(const wchar_t* s)
	{
		Vec2u ret;
		swscanf(s, L"%u;%u", &ret.x(), &ret.y());
		return ret;
	}

	inline Vec3u stou3(const wchar_t* s)
	{
		Vec3u ret;
		swscanf(s, L"%u;%u;%u", &ret.x(), &ret.y(), &ret.z());
		return ret;
	}

	inline Vec4u stou4(const wchar_t* s)
	{
		Vec4u ret;
		swscanf(s, L"%u;%u;%u;%u", &ret.x(), &ret.y(), &ret.z(), &ret.w());
		return ret;
	}

	inline Vec2i stoi2(const wchar_t* s)
	{
		Vec2i ret;
		swscanf(s, L"%d;%d", &ret.x(), &ret.y());
		return ret;
	}

	inline Vec3i stoi3(const wchar_t* s)
	{
		Vec3i ret;
		swscanf(s, L"%d;%d;%d", &ret.x(), &ret.y(), &ret.z());
		return ret;
	}

	inline Vec4i stoi4(const wchar_t* s)
	{
		Vec4i ret;
		swscanf(s, L"%d;%d;%d;%d", &ret.x(), &ret.y(), &ret.z(), &ret.w());
		return ret;
	}

	inline Vec2c stoc2(const wchar_t* s)
	{
		Vec2i ret;
		swscanf(s, L"%d;%d", &ret.x(), &ret.y());
		return Vec2c(ret);
	}

	inline Vec3c stoc3(const wchar_t* s)
	{
		Vec3i ret;
		swscanf(s, L"%d;%d;%d", &ret.x(), &ret.y(), &ret.z());
		return Vec3c(ret);
	}

	inline Vec4c stoc4(const wchar_t* s)
	{
		Vec4i ret;
		swscanf(s, L"%d;%d;%d;%d", &ret.x(), &ret.y(), &ret.z(), &ret.w());
		return Vec4c(ret);
	}

	template<class T>
	T sto(const wchar_t* s);

	template<>
	inline int sto<int>(const wchar_t* s)
	{
		return std::stoi(s);
	}

	template<>
	inline uint sto<uint>(const wchar_t* s)
	{
		return std::stoul(s);
	}

	template<>
	inline float sto<float>(const wchar_t* s)
	{
		return std::stof(s);
	}

	template<>
	inline uchar sto<uchar>(const wchar_t* s)
	{
		return std::stoul(s);
	}

	struct SerializableAttribute
	{
		FLAME_FOUNDATION_EXPORTS const std::string& name() const;
		FLAME_FOUNDATION_EXPORTS const std::string& value() const;

		FLAME_FOUNDATION_EXPORTS void set_name(const std::string& name);
		FLAME_FOUNDATION_EXPORTS void set_value(const std::string& value);
	};

	struct SerializableNode
	{
		enum Type
		{
			Value,
			Object,
			Array,
			Cdata,
			Pcdata
		};

		FLAME_FOUNDATION_EXPORTS Type type() const;
		FLAME_FOUNDATION_EXPORTS void set_type(Type type);

		FLAME_FOUNDATION_EXPORTS const std::string& name() const;
		FLAME_FOUNDATION_EXPORTS const std::string& value() const;

		FLAME_FOUNDATION_EXPORTS void set_name(const std::string& name);
		FLAME_FOUNDATION_EXPORTS void set_value(const std::string& value);

		FLAME_FOUNDATION_EXPORTS SerializableAttribute* new_attr(const std::string& name, const std::string& value);
		FLAME_FOUNDATION_EXPORTS SerializableAttribute* insert_attr(int idx, const std::string& name, const std::string& value);
		FLAME_FOUNDATION_EXPORTS void remove_attr(int idx);
		FLAME_FOUNDATION_EXPORTS void remove_attr(SerializableAttribute* a);
		FLAME_FOUNDATION_EXPORTS void clear_attrs();
		FLAME_FOUNDATION_EXPORTS int attr_count() const;
		FLAME_FOUNDATION_EXPORTS SerializableAttribute* attr(int idx) const;
		FLAME_FOUNDATION_EXPORTS SerializableAttribute* find_attr(const std::string& name) const;

		FLAME_FOUNDATION_EXPORTS void add_node(SerializableNode* n);
		FLAME_FOUNDATION_EXPORTS SerializableNode* new_node(const std::string& name);
		FLAME_FOUNDATION_EXPORTS SerializableNode* insert_node(int idx, const std::string& name);
		FLAME_FOUNDATION_EXPORTS void remove_node(int idx);
		FLAME_FOUNDATION_EXPORTS void remove_node(SerializableNode* n);
		FLAME_FOUNDATION_EXPORTS void clear_nodes();
		FLAME_FOUNDATION_EXPORTS int node_count() const;
		FLAME_FOUNDATION_EXPORTS SerializableNode* node(int idx) const;
		FLAME_FOUNDATION_EXPORTS SerializableNode* find_node(const std::string& name) const;

		FLAME_FOUNDATION_EXPORTS static SerializableNode* create(const std::string& name);
		FLAME_FOUNDATION_EXPORTS static SerializableNode* create_from_xml_string(const std::string& str);
		FLAME_FOUNDATION_EXPORTS static SerializableNode* create_from_json_string(const std::string& str);
		FLAME_FOUNDATION_EXPORTS static SerializableNode* create_from_xml_file(const std::wstring& filename);
		FLAME_FOUNDATION_EXPORTS static SerializableNode* create_from_json_file(const std::wstring& filename);
		FLAME_FOUNDATION_EXPORTS static Mail<std::string> to_xml_string(SerializableNode* n);
		FLAME_FOUNDATION_EXPORTS static Mail<std::string> to_json_string(SerializableNode* n);
		FLAME_FOUNDATION_EXPORTS static void save_to_xml_file(SerializableNode* n, const std::wstring& filename);
		FLAME_FOUNDATION_EXPORTS static void save_to_json_file(SerializableNode* n, const std::wstring& filename);
		FLAME_FOUNDATION_EXPORTS static void destroy(SerializableNode* n);
	};

	enum TypeTag$
	{
		TypeEnumSingle,
		TypeEnumMulti,
		TypeData,
		TypePointer
	};

	inline char type_tag(TypeTag$ tag)
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

	typedef EnumInfo* EnumInfoPtr;
	typedef VariableInfo* VariableInfoPtr;
	typedef FunctionInfo* FunctionInfoPtr;
	typedef UdtInfo* UdtInfoPtr;
	typedef TypeinfoDatabase* TypeinfoDatabasePtr;

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

	inline uint vector_size(const void* p)
	{
		return ((std::vector<int>*)p)->size();
	}

	struct TypeInfo
	{
		TypeTag$ tag;
		bool is_attribute;
		bool is_vector;
		std::string base_name;
		std::string name; // tag[A][V]#base, order matters
		uint base_hash;
		uint hash;

		TypeInfo() :
			tag(TypeData),
			is_attribute(false),
			is_vector(false),
			base_hash(0),
			hash(0)
		{
		}

		TypeInfo(TypeTag$ tag, const std::string& base_name, bool is_attribute = false, bool is_vector = false) :
			tag(tag),
			base_name(base_name),
			base_hash(H(base_name.c_str())),
			is_attribute(is_attribute),
			is_vector(is_vector)
		{
			name = type_tag(tag);
			if (is_attribute)
				name += "A";
			if (is_vector)
				name += "V";
			name += "#" + base_name;
			hash = H(name.c_str());
		}

		bool is_serializable() const
		{
			return !is_vector && (tag == TypeEnumSingle || tag == TypeEnumMulti || tag == TypeData);
		}

		static TypeInfo from_str(const std::string& str)
		{
			auto sp = ssplit(str, '#');
			auto tag = -1;
			while (type_tag((TypeTag$)++tag) != sp[0][0]);
			auto is_attribute = false;
			auto is_vector = false;
			for (auto i = 1; i < sp[0].size(); i++)
			{
				if (sp[0][i] == 'A')
					is_attribute = true;
				else if (sp[0][i] == 'V')
					is_vector = true;
			}
			return TypeInfo((TypeTag$)tag, sp[1], is_attribute, is_vector);
		}

		inline std::string serialize(const std::vector<TypeinfoDatabase*>& dbs, const void* src, int precision) const;
		inline void serialize(const std::vector<TypeinfoDatabase*>& dbs, const void* src, int precision, SerializableNode* dst) const;
		inline void unserialize(const std::vector<TypeinfoDatabase*>& dbs, const std::string& src, void* dst, void* dst_module, TypeinfoDatabase* dst_db) const;
		inline void unserialize(const std::vector<TypeinfoDatabase*>& dbs, const SerializableNode* src, void* dst, void* dst_module, TypeinfoDatabase* dst_db) const;
		inline void copy_from(const void* src, uint size, void* dst, void* dst_module, TypeinfoDatabase* dst_db) const;
	};

	inline bool operator==(const TypeInfo& lhs, const TypeInfo& rhs)
	{
		return lhs.hash == rhs.hash;
	}

	struct VariableInfo
	{
		FLAME_FOUNDATION_EXPORTS const TypeInfo& type() const;
		FLAME_FOUNDATION_EXPORTS const std::string& name() const;
		FLAME_FOUNDATION_EXPORTS uint name_hash() const;
		FLAME_FOUNDATION_EXPORTS const std::string& decoration() const;
		FLAME_FOUNDATION_EXPORTS uint offset() const;
		FLAME_FOUNDATION_EXPORTS uint size() const;
		FLAME_FOUNDATION_EXPORTS const void* default_value() const;
	};

	struct EnumItem
	{
		FLAME_FOUNDATION_EXPORTS const std::string& name() const;
		FLAME_FOUNDATION_EXPORTS int value() const;
	};

	struct EnumInfo
	{
		FLAME_FOUNDATION_EXPORTS TypeinfoDatabase* db() const;

		FLAME_FOUNDATION_EXPORTS const std::string& name() const;

		FLAME_FOUNDATION_EXPORTS uint item_count() const;
		FLAME_FOUNDATION_EXPORTS EnumItem* item(int idx) const;
		FLAME_FOUNDATION_EXPORTS EnumItem* find_item(const std::string& name, int *out_idx = nullptr) const;
		FLAME_FOUNDATION_EXPORTS EnumItem* find_item(int value, int* out_idx = nullptr) const;
		FLAME_FOUNDATION_EXPORTS EnumItem* add_item(const std::string& name, int value);
	};

	struct FunctionInfo
	{
		FLAME_FOUNDATION_EXPORTS TypeinfoDatabase* db() const;

		FLAME_FOUNDATION_EXPORTS const std::string& name() const;
		FLAME_FOUNDATION_EXPORTS void* rva() const;
		FLAME_FOUNDATION_EXPORTS const TypeInfo& return_type() const;

		FLAME_FOUNDATION_EXPORTS uint parameter_count() const;
		FLAME_FOUNDATION_EXPORTS const TypeInfo& parameter_type(uint idx) const;
		FLAME_FOUNDATION_EXPORTS void add_parameter(const TypeInfo& type);

		FLAME_FOUNDATION_EXPORTS const std::string& code_pos() const; // source_file_name#line_begin:line_end

	};

	struct UdtInfo
	{
		FLAME_FOUNDATION_EXPORTS TypeinfoDatabase* db() const;

		FLAME_FOUNDATION_EXPORTS const TypeInfo& type() const;
		FLAME_FOUNDATION_EXPORTS uint size() const;

		FLAME_FOUNDATION_EXPORTS uint variable_count() const;
		FLAME_FOUNDATION_EXPORTS VariableInfo* variable(uint idx) const;
		FLAME_FOUNDATION_EXPORTS VariableInfo* find_variable(const std::string& name, int *out_idx = nullptr) const;
		FLAME_FOUNDATION_EXPORTS VariableInfo* add_variable(const TypeInfo& type, const std::string& name, const std::string& decoration, uint offset, uint size);

		FLAME_FOUNDATION_EXPORTS uint function_count() const;
		FLAME_FOUNDATION_EXPORTS FunctionInfo* function(uint idx) const;
		FLAME_FOUNDATION_EXPORTS FunctionInfo* find_function(const std::string& name, int *out_idx = nullptr) const;
		FLAME_FOUNDATION_EXPORTS FunctionInfo* add_function(const std::string& name, void* rva, const TypeInfo& return_type, const std::string& code_pos);
	};

	/*
		something end with '$[a]' means it is reflectable
		the 'a' is called decoration, and is optional
		if first char of member name is '_', then the '_' will be ignored in reflection

		such as:
			struct Apple$ // mark this will be collected by typeinfogen
			{
				float size$; // mark this member will be collected
				Vec3f color$i; // mark this member will be collected, and its decoration is 'i'
				float _1$i; // mark this member will be collected, and reflected name is '1'
			};

		the decoration can be one or more chars, and order doesn't matter

		currently, the following attributes are used by typeinfogen, others are free to use:
			'm' for enum variable, means it can hold combination of the enum
			'c' for function, means to collect the code of the function
			'f' for std::[w]string variable, means this is a filename
	*/

	struct TypeinfoDatabase
	{
		FLAME_FOUNDATION_EXPORTS const std::wstring& module_name() const;

		FLAME_FOUNDATION_EXPORTS Mail<std::vector<EnumInfo*>> get_enums();
		FLAME_FOUNDATION_EXPORTS EnumInfo* find_enum(uint name_hash);
		FLAME_FOUNDATION_EXPORTS EnumInfo* add_enum(const std::string& name);

		FLAME_FOUNDATION_EXPORTS Mail<std::vector<FunctionInfo*>> get_functions();
		FLAME_FOUNDATION_EXPORTS FunctionInfo* find_function(uint name_hash);
		FLAME_FOUNDATION_EXPORTS FunctionInfo* add_function(const std::string& name, void* rva, const TypeInfo& return_type, const std::string& code_pos);

		FLAME_FOUNDATION_EXPORTS Mail<std::vector<UdtInfo*>> get_udts();
		FLAME_FOUNDATION_EXPORTS UdtInfo* find_udt(uint name_hash);
		FLAME_FOUNDATION_EXPORTS UdtInfo* add_udt(const TypeInfo& type, uint size);

		FLAME_FOUNDATION_EXPORTS static TypeinfoDatabase* collect(const std::vector<TypeinfoDatabase*>& existed_dbs, const std::wstring& module_filename, const std::wstring& pdb_filename = L"");
		FLAME_FOUNDATION_EXPORTS static TypeinfoDatabase* load(const std::vector<TypeinfoDatabase*>& dbs, const std::wstring& typeinfo_filename);
		FLAME_FOUNDATION_EXPORTS static void save(const std::vector<TypeinfoDatabase*>& dbs, TypeinfoDatabase* db);
		FLAME_FOUNDATION_EXPORTS static void destroy(TypeinfoDatabase* db);
	};

	inline EnumInfo* find_enum(const std::vector<TypeinfoDatabase*>& dbs, uint name_hash)
	{
		for (auto db : dbs)
		{
			auto info = db->find_enum(name_hash);
			if (info)
				return info;
		}
		return nullptr;
	}

	inline UdtInfo* find_udt(const std::vector<TypeinfoDatabase*>& dbs, uint name_hash)
	{
		for (auto db : dbs)
		{
			auto info = db->find_udt(name_hash);
			if (info)
				return info;
		}
		return nullptr;
	}

	inline FunctionInfo* find_function(const std::vector<TypeinfoDatabase*>& dbs, uint name_hash)
	{
		for (auto db : dbs)
		{
			auto info = db->find_function(name_hash);
			if (info)
				return info;
		}
		return nullptr;
	}

	std::string TypeInfo::serialize(const std::vector<TypeinfoDatabase*>& dbs, const void* src, int precision) const
	{
		if (is_attribute)
			src = (char*)src + sizeof(AttributeBase);

		switch (tag)
		{
		case TypeEnumSingle:
		{
			auto e = find_enum(dbs, base_hash);
			assert(e);
			return e->find_item(*(int*)src)->name();
		}
		case TypeEnumMulti:
		{
			std::string str;
			auto e = find_enum(dbs, base_hash);
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
			switch (base_hash)
			{
			case cH("bool"):
				return *(bool*)src ? "1" : "0";
			case cH("int"):
				return std::to_string(*(int*)src);
			case cH("Vec(1+int)"):
				return to_string(*(Vec1i*)src);
			case cH("Vec(2+int)"):
				return to_string(*(Vec2i*)src);
			case cH("Vec(3+int)"):
				return to_string(*(Vec3i*)src);
			case cH("Vec(4+int)"):
				return to_string(*(Vec4i*)src);
			case cH("uint"):
				return std::to_string(*(uint*)src);
			case cH("Vec(1+uint)"):
				return to_string(*(Vec1u*)src);
			case cH("Vec(2+uint)"):
				return to_string(*(Vec2u*)src);
			case cH("Vec(3+uint)"):
				return to_string(*(Vec3u*)src);
			case cH("Vec(4+uint)"):
				return to_string(*(Vec4u*)src);
			case cH("ulonglong"):
				return std::to_string(*(ulonglong*)src);
			case cH("float"):
				return to_string(*(float*)src, precision);
			case cH("Vec(1+float)"):
				return to_string(*(Vec1f*)src, precision);
			case cH("Vec(2+float)"):
				return to_string(*(Vec2f*)src, precision);
			case cH("Vec(3+float)"):
				return to_string(*(Vec3f*)src, precision);
			case cH("Vec(4+float)"):
				return to_string(*(Vec4f*)src, precision);
			case cH("uchar"):
				return std::to_string(*(uchar*)src);
			case cH("Vec(1+uchar)"):
				return to_string(*(Vec1c*)src);
			case cH("Vec(2+uchar)"):
				return to_string(*(Vec2c*)src);
			case cH("Vec(3+uchar)"):
				return to_string(*(Vec3c*)src);
			case cH("Vec(4+uchar)"):
				return to_string(*(Vec4c*)src);
			case cH("std::string"):
				return *(std::string*)src;
			case cH("std::wstring"):
				return w2s(*(std::wstring*)src);
			default:
				assert(0);
			}
		}
	}

	void TypeInfo::serialize(const std::vector<TypeinfoDatabase*>& dbs, const void* src, int precision, SerializableNode* dst) const
	{
		if (is_vector)
		{

		}
		else
			dst->new_attr("v", serialize(dbs, src, precision));
	}

	void TypeInfo::unserialize(const std::vector<TypeinfoDatabase*>& dbs, const std::string& src, void* dst, void* dst_module, TypeinfoDatabase* dst_db) const
	{
		if (is_attribute)
			dst = (char*)dst + sizeof(AttributeBase);

		switch (tag)
		{
		case TypeEnumSingle:
		{
			auto e = find_enum(dbs, base_hash);
			assert(e);
			e->find_item(src, (int*)dst);
		}
			return;
		case TypeEnumMulti:
		{
			auto v = 0;
			auto e = find_enum(dbs, base_hash);
			assert(e);
			auto sp = ssplit(src, ';');
			for (auto& t : sp)
				v |= e->find_item(t)->value();
			*(int*)dst = v;
		}
			return;
		case TypeData:
			switch (base_hash)
			{
			case cH("bool"):
				*(bool*)dst = (src != "0");
				break;
			case cH("int"):
				*(int*)dst = std::stoi(src);
				break;
			case cH("Vec(1+int)"):
				*(Vec1u*)dst = std::stoi(src.c_str());
				break;
			case cH("Vec(2+int)"):
				*(Vec2u*)dst = stoi2(src.c_str());
				break;
			case cH("Vec(3+int)"):
				*(Vec3u*)dst = stoi3(src.c_str());
				break;
			case cH("Vec(4+int)"):
				*(Vec4u*)dst = stoi4(src.c_str());
				break;
			case cH("uint"):
				*(uint*)dst = std::stoul(src);
				break;
			case cH("Vec(1+uint)"):
				*(Vec1u*)dst = std::stoul(src.c_str());
				break;
			case cH("Vec(2+uint)"):
				*(Vec2u*)dst = stou2(src.c_str());
				break;
			case cH("Vec(3+uint)"):
				*(Vec3u*)dst = stou3(src.c_str());
				break;
			case cH("Vec(4+uint)"):
				*(Vec4u*)dst = stou4(src.c_str());
				break;
			case cH("ulonglong"):
				*(ulonglong*)dst = std::stoull(src);
				break;
			case cH("float"):
				*(float*)dst = std::stof(src.c_str());
				break;
			case cH("Vec(1+float)"):
				*(Vec1f*)dst = std::stof(src.c_str());
				break;
			case cH("Vec(2+float)"):
				*(Vec2f*)dst = stof2(src.c_str());
				break;
			case cH("Vec(3+float)"):
				*(Vec3f*)dst = stof3(src.c_str());
				break;
			case cH("Vec(4+float)"):
				*(Vec4f*)dst = stof4(src.c_str());
				break;
			case cH("uchar"):
				*(uchar*)dst = std::stoul(src);
				break;
			case cH("Vec(1+uchar)"):
				*(Vec1c*)dst = std::stoul(src.c_str());
				break;
			case cH("Vec(2+uchar)"):
				*(Vec2c*)dst = stoc2(src.c_str());
				break;
			case cH("Vec(3+uchar)"):
				*(Vec3c*)dst = stoc3(src.c_str());
				break;
			case cH("Vec(4+uchar)"):
				*(Vec4c*)dst = stoc4(src.c_str());
				break;
			case cH("std::string"):
				cmf(p2f<MF_vp_vp>((char*)dst_module + (uint)dst_db->find_udt(TypeInfo(TypeData, "std::string").hash)->find_function("operator=")->rva()), dst, (void*)&src);
				break;
			case cH("std::wstring"):
			{
				auto str = s2w(src);
				cmf(p2f<MF_vp_vp>((char*)dst_module + (uint)dst_db->find_udt(TypeInfo(TypeData, "std::wstring").hash)->find_function("operator=")->rva()), dst, (void*)&str);
			}
				break;
			default:
				assert(0);
			}
			return;
		}
	}

	inline void TypeInfo::unserialize(const std::vector<TypeinfoDatabase*>& dbs, const SerializableNode* src, void* dst, void* dst_module, TypeinfoDatabase* dst_db) const
	{
		if (is_vector)
		{
			if (is_attribute)
				dst = (char*)dst + sizeof(AttributeBase);

			auto size = std::stoi(src->find_attr("s")->value());

		}
		else
			unserialize(dbs, src->find_attr("v")->value(), dst, dst_module, dst_db);
	}

	void TypeInfo::copy_from(const void* src, uint size, void* dst, void* dst_module, TypeinfoDatabase* dst_db) const
	{
		if (is_attribute)
		{
			dst = (char*)dst + sizeof(AttributeBase);
			size -= sizeof(AttributeBase);
		}

		if (tag == TypeData)
		{
			switch (base_hash)
			{
			case cH("std::string"):
				cmf(p2f<MF_vp_vp>((char*)dst_module + (uint)dst_db->find_udt(TypeInfo(TypeData, "std::string").hash)->find_function("operator=")->rva()), dst, (void*)src);
				return;
			case cH("std::wstring"):
				cmf(p2f<MF_vp_vp>((char*)dst_module + (uint)dst_db->find_udt(TypeInfo(TypeData, "std::wstring").hash)->find_function("operator=")->rva()), dst, (void*)src);
				return;
			}
		}

		memcpy(dst, src, size);
	}
}

