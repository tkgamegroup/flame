//MIT License
//
//Copyright (c) 2018 wjs
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#pragma once

#include <flame/foundation/foundation.h>

template<class T>
struct StringTraits
{
	typedef char char_type;

	static const char* choose(const char* narrow, const wchar_t* wide)
	{
		return narrow;
	}

	static char choose(char narrow, wchar_t wide)
	{
		return narrow;
	}

	static void copy(char* dst, const char* src)
	{
		strcpy(dst, src);
	}

	static int sprintf(char* buf, const char* fmt, ...)
	{
		int res;
		va_list args;
		va_start(args, fmt);
		res = vsprintf(buf, fmt, args);
		va_end(args);
		return res;
	}
};

template<>
struct StringTraits<wchar_t>
{
	typedef wchar_t char_type;

	static const wchar_t* choose(const char* narrow, const wchar_t* wide)
	{
		return wide;
	}

	static wchar_t choose(char narrow, wchar_t wide)
	{
		return wide;
	}

	static void copy(wchar_t* dst, const wchar_t* src)
	{
		wcscpy(dst, src);
	}

	static int sprintf(wchar_t* buf, const wchar_t* fmt, ...)
	{
		int res;
		va_list args;
		va_start(args, fmt);
		res = vswprintf(buf, fmt, args);
		va_end(args);
		return res;
	}
};

#define LITERAL(T, x) StringTraits<T>::choose(x, L##x)

namespace flame
{
	template<class CH, class T>
	inline void to_tstring(std::basic_string<CH>& out, T v, int precision = std::is_floating_point<T>::value ? 6 : 0)
	{
		std::char_traits<char>::copy
		CH buf[20];
		if (std::is_same<T, float>::value)
			StringTraits<CH>::sprintf(buf, LITERAL(CH, "%.*f"), precision, v);
		else if (std::is_same<T, uint>::value)
		{
			if (precision == 0)
				StringTraits<CH>::sprintf(buf, LITERAL(CH, "%u"), v);
			else
				StringTraits<CH>::sprintf(buf, LITERAL(CH, "%0*u"), precision, v);
		}
		else if (std::is_same<T, int>::value)
		{
			if (precision == 0)
				StringTraits<CH>::sprintf(buf, LITERAL(CH, "%d"), v);
			else
				StringTraits<CH>::sprintf(buf, LITERAL(CH, "%0*d"), precision, v);
		}
		else if (std::is_same<T, bool>::value)
			StringTraits<CH>::copy(buf, v ? LITERAL(CH, "true") : LITERAL(CH, "false"), v);
		else
			assert(0);
		out = buf;
	}

	template<class T>
	inline std::string to_string(T v, int precision = std::is_floating_point<T>::value ? 6 : 0)
	{
		std::string ret;
		to_tstring(ret, v, precision);
		return ret;
	}

	template<class T>
	inline std::wstring to_wstring(T v, int precision = std::is_floating_point<T>::value ? 6 : 0)
	{
		std::wstring ret;
		to_tstring(ret, v, precision);
		return ret;
	}

	template<uint N, class T>
	inline std::string to_string(const Vec<N, T>& v, int precision = std::is_floating_point<T>::value ? 6 : 0)
	{
		auto ret = to_string(v[0], precision);
		for (auto i = 1; i < N; i++)
			ret += ";" + to_string(v[i], precision);
		return ret;
	}

	template<uint N, class T>
	inline std::string to_wstring(const Vec<N, T>& v, int precision = std::is_floating_point<T>::value ? 6 : 0)
	{
		auto ret = to_wstring(v[0], precision);
		for (auto i = 1; i < N; i++)
			ret += L";" + to_wstring(v[i], precision);
		return ret;
	}

	inline float stof(const char* s)
	{
		float ret;
		sscanf(s, "%f", &ret);
		return ret;
	}

	inline uint stou(const char* s)
	{
		uint ret;
		sscanf(s, "%u", &ret);
		return ret;
	}

	inline int stoi(const char* s)
	{
		int ret;
		sscanf(s, "%d", &ret);
		return ret;
	}

	inline uchar stoc(const char* s)
	{
		uchar ret;
		sscanf(s, "%d", &ret);
		return ret;
	}

	inline bool stob(const char* s)
	{
		if (strcmp(s, "true") == 0)
			return true;
		else if (strcmp(s, "false") == 0)
			return false;
		else
			assert(0);
	}

	template<class T>
	inline void stot(const char* s, T& out)
	{
		if (std::is_same<T, float>::value)
			out = stof(s);
		else if (std::is_same<T, uint>::value)
		{
			if (precision == 0)
				StringTraits<T>::sprintf(buf, LITERAL(CH, "%u"), v);
			else
				StringTraits<T>::sprintf(buf, LITERAL(CH, "%0*u"), precision, v);
		}
		else if (std::is_same<T, int>::value)
		{
			if (precision == 0)
				StringTraits<T>::sprintf(buf, LITERAL(CH, "%d"), v);
			else
				StringTraits<T>::sprintf(buf, LITERAL(CH, "%0*d"), precision, v);
		}
		else if (std::is_same<T, bool>::value)
			StringTraits<T>::copy(buf, v ? LITERAL(CH, "true") : LITERAL(CH, "false"), v);
		else
			assert(0);
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

	inline Ivec2 stoi2(const char* s)
	{
		Ivec2 ret;
		sscanf(s, "%d;%d", &ret.x, &ret.y);
		return ret;
	}

	inline Ivec3 stoi3(const char* s)
	{
		Ivec3 ret;
		sscanf(s, "%d;%d;%d", &ret.x, &ret.y, &ret.z);
		return ret;
	}

	inline Ivec4 stoi4(const char* s)
	{
		Ivec4 ret;
		sscanf(s, "%d;%d;%d;%d", &ret.x, &ret.y, &ret.z, &ret.w);
		return ret;
	}

	inline Bvec2 stob2(const char* s)
	{
		Ivec2 ret;
		sscanf(s, "%d;%d;", &ret.x, &ret.y);
		return Bvec2(ret.x, ret.y);
	}

	inline Bvec3 stob3(const char* s)
	{
		Ivec3 ret;
		sscanf(s, "%d;%d;%d", &ret.x, &ret.y, &ret.z);
		return Bvec3(ret.x, ret.y, ret.z);
	}

	inline Bvec4 stob4(const char* s)
	{
		Ivec4 ret;
		sscanf(s, "%d;%d;%d;%d", &ret.x, &ret.y, &ret.z, &ret.w);
		return Bvec4(ret.x, ret.y, ret.z, ret.w);
	}

	inline float stof1(const wchar_t* s)
	{
		float ret;
		swscanf(s, L"%f", &ret);
		return ret;
	}

	inline Vec2 stof2(const wchar_t* s)
	{
		Vec2 ret;
		swscanf(s, L"%f;%f", &ret.x, &ret.y);
		return ret;
	}

	inline Vec3 stof3(const wchar_t* s)
	{
		Vec3 ret;
		swscanf(s, L"%f;%f;%f", &ret.x, &ret.y, &ret.z);
		return ret;
	}

	inline Vec4 stof4(const wchar_t* s)
	{
		Vec4 ret;
		swscanf(s, L"%f;%f;%f;%f", &ret.x, &ret.y, &ret.z, &ret.w);
		return ret;
	}

	inline uint stou1(const wchar_t* s)
	{
		int ret;
		swscanf(s, L"%u", &ret);
		return ret;
	}

	inline int stoi1(const wchar_t* s)
	{
		int ret;
		swscanf(s, L"%d", &ret);
		return ret;
	}

	inline Ivec2 stoi2(const wchar_t* s)
	{
		Ivec2 ret;
		swscanf(s, L"%d;%d", &ret.x, &ret.y);
		return ret;
	}

	inline Ivec3 stoi3(const wchar_t* s)
	{
		Ivec3 ret;
		swscanf(s, L"%d;%d;%d", &ret.x, &ret.y, &ret.z);
		return ret;
	}

	inline Ivec4 stoi4(const wchar_t* s)
	{
		Ivec4 ret;
		swscanf(s, L"%d;%d;%d;%d", &ret.x, &ret.y, &ret.z, &ret.w);
		return ret;
	}

	inline uchar stob1(const wchar_t* s)
	{
		int ret;
		swscanf(s, L"%d", &ret);
		return (uchar)ret;
	}

	inline Bvec2 stob2(const wchar_t* s)
	{
		Ivec2 ret;
		swscanf(s, L"%d;%d;", &ret.x, &ret.y);
		return Bvec2(ret.x, ret.y);
	}

	inline Bvec3 stob3(const wchar_t* s)
	{
		Ivec3 ret;
		swscanf(s, L"%d;%d;%d", &ret.x, &ret.y, &ret.z);
		return Bvec3(ret.x, ret.y, ret.z);
	}

	inline Bvec4 stob4(const wchar_t* s)
	{
		Ivec4 ret;
		swscanf(s, L"%d;%d;%d;%d", &ret.x, &ret.y, &ret.z, &ret.w);
		return Bvec4(ret.x, ret.y, ret.z, ret.w);
	}

	enum TypeTag$
	{
		TypeTagEnumSingle,
		TypeTagEnumMulti,
		TypeTagVariable,
		TypeTagPointer,
		TypeTagArray,
		TypeTagLengthAndArray
	};

	FLAME_FOUNDATION_EXPORTS const char* get_type_tag_name(TypeTag$ tag);

	struct TypeInfo;
	struct EnumInfo;
	struct VariableInfo;
	struct UdtInfo;
	struct FunctionInfo;

	typedef TypeInfo* TypeInfoPtr;
	typedef EnumInfo* EnumInfoPtr;
	typedef VariableInfo* VariableInfoPtr;
	typedef FunctionInfo* FunctionInfoPtr;
	typedef UdtInfo* UdtInfoPtr;

	struct TypeInfo
	{
		FLAME_FOUNDATION_EXPORTS TypeTag$ tag() const;
		FLAME_FOUNDATION_EXPORTS const char* name() const;
		FLAME_FOUNDATION_EXPORTS uint name_hash() const;

		static bool equal(const TypeInfo* lhs, const TypeInfo* rhs)
		{
			return lhs->tag() == rhs->tag() && lhs->name_hash() == rhs->name_hash();
		}
	};

	struct EnumItem
	{
		FLAME_FOUNDATION_EXPORTS const char* name() const;
		FLAME_FOUNDATION_EXPORTS int value() const;
	};

	struct EnumInfo
	{
		FLAME_FOUNDATION_EXPORTS int level() const;
		FLAME_FOUNDATION_EXPORTS const wchar_t* module_name() const;

		FLAME_FOUNDATION_EXPORTS const char* name() const;

		FLAME_FOUNDATION_EXPORTS int item_count() const;
		FLAME_FOUNDATION_EXPORTS EnumItem* item(int idx) const;
		FLAME_FOUNDATION_EXPORTS EnumItem* find_item(const char* name, int *out_idx = nullptr) const;
		FLAME_FOUNDATION_EXPORTS EnumItem* find_item(int value, int* out_idx = nullptr) const;

		FLAME_FOUNDATION_EXPORTS String serialize_value(bool single, int v) const;
	};

	struct VariableInfo
	{
		FLAME_FOUNDATION_EXPORTS const TypeInfo* type() const;
		FLAME_FOUNDATION_EXPORTS const char* name() const;
		FLAME_FOUNDATION_EXPORTS const char* attribute() const;
		FLAME_FOUNDATION_EXPORTS int offset() const;
		FLAME_FOUNDATION_EXPORTS int size() const;
		FLAME_FOUNDATION_EXPORTS int count() const; // for native array count
		FLAME_FOUNDATION_EXPORTS const CommonData& default_value() const;
	};

	FLAME_FOUNDATION_EXPORTS void set(void* dst, TypeTag$ tag, int size, const void* src);
	FLAME_FOUNDATION_EXPORTS bool compare(TypeTag$ tag, int size, const void* a, const void* b);
	FLAME_FOUNDATION_EXPORTS String serialize_value(TypeTag$ tag, uint type_hash, const void* src, int precision = 6);
	FLAME_FOUNDATION_EXPORTS void unserialize_value(TypeTag$ tag, uint type_hash, const std::string& str, void* dst);

	struct FunctionInfo
	{
		FLAME_FOUNDATION_EXPORTS int level() const;
		FLAME_FOUNDATION_EXPORTS const wchar_t* module_name() const;

		FLAME_FOUNDATION_EXPORTS const char* name() const;

		FLAME_FOUNDATION_EXPORTS void* rva() const;
		FLAME_FOUNDATION_EXPORTS const TypeInfo* return_type() const;
		FLAME_FOUNDATION_EXPORTS int parameter_count() const;
		FLAME_FOUNDATION_EXPORTS const TypeInfo* parameter_type(int idx) const;
		FLAME_FOUNDATION_EXPORTS const char* code() const;

	};

	struct UdtInfo
	{
		FLAME_FOUNDATION_EXPORTS int level() const;
		FLAME_FOUNDATION_EXPORTS const wchar_t* module_name() const;

		FLAME_FOUNDATION_EXPORTS const char* name() const;

		FLAME_FOUNDATION_EXPORTS int size() const;

		FLAME_FOUNDATION_EXPORTS int item_count() const;
		FLAME_FOUNDATION_EXPORTS VariableInfo* item(int idx) const;
		FLAME_FOUNDATION_EXPORTS VariableInfo* find_item(const char* name, int *out_idx = nullptr) const;

		FLAME_FOUNDATION_EXPORTS int function_count() const;
		FLAME_FOUNDATION_EXPORTS FunctionInfo* function(int idx) const;
		FLAME_FOUNDATION_EXPORTS FunctionInfo* find_function(const char* name, int *out_idx = nullptr) const;
	};

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

		FLAME_FOUNDATION_EXPORTS String to_string_xml() const;
		FLAME_FOUNDATION_EXPORTS String to_string_json() const;
		FLAME_FOUNDATION_EXPORTS void save_xml(const std::wstring& filename) const;
		FLAME_FOUNDATION_EXPORTS void save_json(const std::wstring& filename) const;

		FLAME_FOUNDATION_EXPORTS void serialize(UdtInfo* u, void* src, int precision = 6);
		FLAME_FOUNDATION_EXPORTS void unserialize(UdtInfo* u, void* dst);

		FLAME_FOUNDATION_EXPORTS static SerializableNode* create(const std::string& name);
		FLAME_FOUNDATION_EXPORTS static SerializableNode* create_from_xml_string(const std::string& str);
		FLAME_FOUNDATION_EXPORTS static SerializableNode* create_from_xml_file(const std::wstring& filename);
		FLAME_FOUNDATION_EXPORTS static SerializableNode* create_from_json_string(const std::string& str);
		FLAME_FOUNDATION_EXPORTS static SerializableNode* create_from_json_file(const std::wstring& filename);
		FLAME_FOUNDATION_EXPORTS static void destroy(SerializableNode* n);

		/*
			for from json action, no attrs will be used, all represented by nodes
		*/
	};

	/*
		something end with '$[a]' means it is reflectable
		the 'a' is called attribute, and it is optional

		such as:
			struct Apple$ // mark this will be collected by typeinfogen
			{
				float size$; // mark this member will be collected
				Vec3 color$i; // mark this member will be collected, and its attribute is 'i'
			};

		the attribute can be one or more chars, and order doesn't matter

		currently, the following attributes are used by typeinfogen, others are free to use:
			'm' for enum variable, means it can hold combination of the enum
			'c' for function, means to collect the code of the function
	*/

	FLAME_FOUNDATION_EXPORTS Array<EnumInfo*> get_enums();
	FLAME_FOUNDATION_EXPORTS EnumInfo* find_enum(uint name_hash);

	FLAME_FOUNDATION_EXPORTS Array<UdtInfo*> get_udts();
	FLAME_FOUNDATION_EXPORTS UdtInfo* find_udt(uint name_hash);

	FLAME_FOUNDATION_EXPORTS Array<FunctionInfo*> get_functions();
	FLAME_FOUNDATION_EXPORTS FunctionInfo* find_function(uint name_hash);

	FLAME_FOUNDATION_EXPORTS int typeinfo_free_level();
	FLAME_FOUNDATION_EXPORTS void typeinfo_collect(const std::wstring& filename, int level = 0);
	FLAME_FOUNDATION_EXPORTS void typeinfo_load(const std::wstring& filename, int level = 0);
	FLAME_FOUNDATION_EXPORTS void typeinfo_save(const std::wstring& filename, int level = -1);
	FLAME_FOUNDATION_EXPORTS void typeinfo_clear(int level = -1);

	// level is to separate different sources, such as typeinfos that come from different files, level of -1 means all
}

