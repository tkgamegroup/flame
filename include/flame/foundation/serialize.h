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

	inline std::vector<std::pair<std::string, Vec4u>> load_texture_pack(const std::wstring& filename)
	{
		std::vector<std::pair<std::string, Vec4u>> ret;
		std::ifstream file(filename);

		while (!file.eof())
		{
			std::string line;
			std::getline(file, line);
			if (line.empty())
				break;
			std::stringstream ss(line);
			std::pair<std::string, Vec4u> v;
			ss >> v.first;
			std::string t;
			ss >> t;
			v.second = stou4(t.c_str());
			ret.push_back(v);
		}

		file.close();
		return ret;
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
		FLAME_FOUNDATION_EXPORTS SerializableAttribute* find_attr(const std::string& name);

		FLAME_FOUNDATION_EXPORTS void add_node(SerializableNode* n);
		FLAME_FOUNDATION_EXPORTS SerializableNode* new_node(const std::string& name);
		FLAME_FOUNDATION_EXPORTS SerializableNode* insert_node(int idx, const std::string& name);
		FLAME_FOUNDATION_EXPORTS void remove_node(int idx);
		FLAME_FOUNDATION_EXPORTS void remove_node(SerializableNode* n);
		FLAME_FOUNDATION_EXPORTS void clear_nodes();
		FLAME_FOUNDATION_EXPORTS int node_count() const;
		FLAME_FOUNDATION_EXPORTS SerializableNode* node(int idx) const;
		FLAME_FOUNDATION_EXPORTS SerializableNode* find_node(const std::string& name);

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
		TypeTagEnumSingle,
		TypeTagEnumMulti,
		TypeTagVariable,
		TypeTagPointer,
		TypeTagAttributeES, // AttributeE<T>, single
		TypeTagAttributeEM, // AttributeE<T>, multi
		TypeTagAttributeV,  // AttributeV<T>
		TypeTagAttributeP   // AttributeP<T>
	};

	FLAME_FOUNDATION_EXPORTS const char* get_name(TypeTag$ tag);

	struct TypeInfo;
	struct EnumInfo;
	struct VariableInfo;
	struct FunctionInfo;
	struct UdtInfo;
	struct TypeinfoDatabase;

	typedef TypeInfo* TypeInfoPtr;
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

	struct TypeInfo
	{
		bool equal(TypeTag$ t, uint h) const
		{
			return t == tag() && h == hash();
		}

		FLAME_FOUNDATION_EXPORTS TypeTag$ tag() const;
		FLAME_FOUNDATION_EXPORTS const std::string& name() const; // type name archive
		FLAME_FOUNDATION_EXPORTS uint hash() const;
	};

	struct VariableInfo
	{
		FLAME_FOUNDATION_EXPORTS const TypeInfo* type() const;
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
		FLAME_FOUNDATION_EXPORTS const TypeInfo* return_type() const;

		FLAME_FOUNDATION_EXPORTS uint parameter_count() const;
		FLAME_FOUNDATION_EXPORTS const TypeInfo* parameter_type(uint idx) const;
		FLAME_FOUNDATION_EXPORTS void add_parameter(TypeTag$ tag, const std::string& type_name);

		FLAME_FOUNDATION_EXPORTS const std::string& code_pos() const; // source_file_name#line_begin:line_end

	};

	struct UdtInfo
	{
		FLAME_FOUNDATION_EXPORTS TypeinfoDatabase* db() const;

		FLAME_FOUNDATION_EXPORTS const std::string& name() const;
		FLAME_FOUNDATION_EXPORTS uint size() const; // if 0, then this is a template

		FLAME_FOUNDATION_EXPORTS uint variable_count() const;
		FLAME_FOUNDATION_EXPORTS VariableInfo* variable(uint idx) const;
		FLAME_FOUNDATION_EXPORTS VariableInfo* find_variable(const std::string& name, int *out_idx = nullptr) const;
		FLAME_FOUNDATION_EXPORTS VariableInfo* add_variable(TypeTag$ tag, const std::string& type_name, const std::string& name, const std::string& decoration, uint offset, uint size);

		FLAME_FOUNDATION_EXPORTS uint function_count() const;
		FLAME_FOUNDATION_EXPORTS FunctionInfo* function(uint idx) const;
		FLAME_FOUNDATION_EXPORTS FunctionInfo* find_function(const std::string& name, int *out_idx = nullptr) const;
		FLAME_FOUNDATION_EXPORTS FunctionInfo* add_function(const std::string& name, void* rva, TypeTag$ return_type_tag, const std::string& return_type_name, const std::string& code_pos);
	};

	/*
		something end with '$[a]' means it is reflectable
		the 'a' is called decoration, and is optional

		such as:
			struct Apple$ // mark this will be collected by typeinfogen
			{
				float size$; // mark this member will be collected
				Vec3f color$i; // mark this member will be collected, and its decoration is 'i'
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
		FLAME_FOUNDATION_EXPORTS FunctionInfo* add_function(const std::string& name, void* rva, TypeTag$ return_type_tag, const std::string& return_type_name, const std::string& code_pos);

		FLAME_FOUNDATION_EXPORTS Mail<std::vector<UdtInfo*>> get_udts();
		FLAME_FOUNDATION_EXPORTS UdtInfo* find_udt(uint name_hash);
		FLAME_FOUNDATION_EXPORTS UdtInfo* add_udt(const std::string& name, uint size);

		FLAME_FOUNDATION_EXPORTS static TypeinfoDatabase* collect(const std::vector<TypeinfoDatabase*>& existed_dbs, const std::wstring& dll_filename, const std::wstring& pdb_filename = L"");
		FLAME_FOUNDATION_EXPORTS static TypeinfoDatabase* load(const std::vector<TypeinfoDatabase*>& existed_dbs, const std::wstring& typeinfo_filename);
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

	FLAME_FOUNDATION_EXPORTS Mail<std::string> serialize_value(const std::vector<TypeinfoDatabase*>& dbs, TypeTag$ tag, uint hash, const void* src, int precision = 6);
	FLAME_FOUNDATION_EXPORTS void unserialize_value(const std::vector<TypeinfoDatabase*>& dbs, TypeTag$ tag, uint hash, const std::string& src, void* dst);
}

