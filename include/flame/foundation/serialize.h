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

namespace flame
{
	inline String to_string(float v, int precision = 6)
	{
		char buf[20];
		sprintf(buf, "%.*f", precision, v);
		return buf;
	}

	inline String to_string(const Vec2 &v, int precision = 6)
	{
		char buf[40];
		sprintf(buf, "%.*f;%.*f", precision, v.x, precision, v.y);
		return buf;
	}

	inline String to_string(const Vec3 &v, int precision = 6)
	{
		char buf[60];
		sprintf(buf, "%.*f;%.*f;%.*f", precision, v.x, precision, v.y, precision, v.z);
		return buf;
	}

	inline String to_string(const Vec4 &v, int precision = 6)
	{
		char buf[80];
		sprintf(buf, "%.*f;%.*f;%.*f;%.*f", precision, v.x, precision, v.y, precision, v.z, precision, v.w);
		return buf;
	}

	inline String to_string(bool v)
	{
		return v ? "true" : "false";
	}

	inline String to_string(int v)
	{
		char buf[20];
		sprintf(buf, "%d", v);
		return buf;
	}

	inline String to_string(uint v)
	{
		char buf[20];
		sprintf(buf, "%u", v);
		return buf;
	}

	inline String to_string(const Ivec2 &v)
	{
		char buf[40];
		sprintf(buf, "%d;%d", v.x, v.y);
		return buf;
	}

	inline String to_string(const Ivec3 &v)
	{
		char buf[60];
		sprintf(buf, "%d;%d;%d", v.x, v.y, v.z);
		return buf;
	}

	inline String to_string(const Ivec4 &v)
	{
		char buf[80];
		sprintf(buf, "%d;%d;%d;%d", v.x, v.y, v.z, v.w);
		return buf;
	}

	inline String to_string(const Bvec2 &v)
	{
		char buf[40];
		sprintf(buf, "%d;%d", v.x, v.y);
		return buf;
	}

	inline String to_string(const Bvec3 &v)
	{
		char buf[60];
		sprintf(buf, "%d;%d;%d", v.x, v.y, v.z);
		return buf;
	}

	inline String to_string(const Bvec4 &v)
	{
		char buf[80];
		sprintf(buf, "%d;%d;%d;%d", v.x, v.y, v.z, v.w);
		return buf;
	}

	inline std::wstring to_wstring(float v, int precision = 6)
	{
		wchar_t buf[20];
		swprintf(buf, L"%.*f", precision, v);
		return buf;
	}

	inline StringW to_wstring(const Vec2 &v, int precision = 6)
	{
		wchar_t buf[40];
		swprintf(buf, L"%.*f;%.*f", precision, v.x, precision, v.y);
		return buf;
	}

	inline StringW to_wstring(const Vec3 &v, int precision = 6)
	{
		wchar_t buf[60];
		swprintf(buf, L"%.*f;%.*f;%.*f", precision, v.x, precision, v.y, precision, v.z);
		return buf;
	}

	inline StringW to_wstring(const Vec4 &v, int precision = 6)
	{
		wchar_t buf[80];
		swprintf(buf, L"%.*f;%.*f;%.*f;%.*f", precision, v.x, precision, v.y, precision, v.z, precision, v.w);
		return buf;
	}

	inline StringW to_wstring(bool v)
	{
		return v ? L"true" : L"false";
	}

	inline StringW to_wstring(int v)
	{
		wchar_t buf[20];
		swprintf(buf, L"%d", v);
		return buf;
	}

	inline StringW to_wstring(uint v)
	{
		wchar_t buf[20];
		swprintf(buf, L"%u", v);
		return buf;
	}

	inline StringW to_wstring(const Ivec2 &v)
	{
		wchar_t buf[40];
		swprintf(buf, L"%d;%d", v.x, v.y);
		return buf;
	}

	inline StringW to_wstring(const Ivec3 &v)
	{
		wchar_t buf[60];
		swprintf(buf, L"%d;%d;%d", v.x, v.y, v.z);
		return buf;
	}

	inline StringW to_wstring(const Ivec4 &v)
	{
		wchar_t buf[80];
		swprintf(buf, L"%d;%d;%d;%d", v.x, v.y, v.z, v.w);
		return buf;
	}

	inline StringW to_wstring(const Bvec2 &v)
	{
		wchar_t buf[40];
		swprintf(buf, L"%d;%d", v.x, v.y);
		return buf;
	}

	inline StringW to_wstring(const Bvec3 &v)
	{
		wchar_t buf[60];
		swprintf(buf, L"%d;%d;%d", v.x, v.y, v.z);
		return buf;
	}

	inline StringW to_wstring(const Bvec4 &v)
	{
		wchar_t buf[80];
		swprintf(buf, L"%d;%d;%d;%d", v.x, v.y, v.z, v.w);
		return buf;
	}

	inline std::string to_stdstring(float v, int precision = 6)
	{
		char buf[20];
		sprintf(buf, "%.*f", precision, v);
		return buf;
	}

	inline std::string to_stdstring(const Vec2 &v, int precision = 6)
	{
		char buf[40];
		sprintf(buf, "%.*f;%.*f", precision, v.x, precision, v.y);
		return buf;
	}

	inline std::string to_stdstring(const Vec3 &v, int precision = 6)
	{
		char buf[60];
		sprintf(buf, "%.*f;%.*f;%.*f", precision, v.x, precision, v.y, precision, v.z);
		return buf;
	}

	inline std::string to_stdstring(const Vec4 &v, int precision = 6)
	{
		char buf[80];
		sprintf(buf, "%.*f;%.*f;%.*f;%.*f", precision, v.x, precision, v.y, precision, v.z, precision, v.w);
		return buf;
	}

	inline std::string to_stdstring(bool v)
	{
		return v ? "true" : "false";
	}

	inline std::string to_stdstring(int v)
	{
		char buf[20];
		sprintf(buf, "%d", v);
		return buf;
	}

	inline std::string to_stdstring(uint v)
	{
		char buf[20];
		sprintf(buf, "%u", v);
		return buf;
	}

	inline std::string to_stdstring(const Ivec2 &v)
	{
		char buf[40];
		sprintf(buf, "%d;%d", v.x, v.y);
		return buf;
	}

	inline std::string to_stdstring(const Ivec3 &v)
	{
		char buf[60];
		sprintf(buf, "%d;%d;%d", v.x, v.y, v.z);
		return buf;
	}

	inline std::string to_stdstring(const Ivec4 &v)
	{
		char buf[80];
		sprintf(buf, "%d;%d;%d;%d", v.x, v.y, v.z, v.w);
		return buf;
	}

	inline std::string to_stdstring(const Bvec2 &v)
	{
		char buf[40];
		sprintf(buf, "%d;%d", v.x, v.y);
		return buf;
	}

	inline std::string to_stdstring(const Bvec3 &v)
	{
		char buf[60];
		sprintf(buf, "%d;%d;%d", v.x, v.y, v.z);
		return buf;
	}

	inline std::string to_stdstring(const Bvec4 &v)
	{
		char buf[80];
		sprintf(buf, "%d;%d;%d;%d", v.x, v.y, v.z, v.w);
		return buf;
	}

	inline std::wstring to_stdwstring(float v, int precision = 6)
	{
		wchar_t buf[20];
		swprintf(buf, L"%.*f", precision, v);
		return buf;
	}

	inline std::wstring to_stdwstring(const Vec2 &v, int precision = 6)
	{
		wchar_t buf[40];
		swprintf(buf, L"%.*f;%.*f", precision, v.x, precision, v.y);
		return buf;
	}

	inline std::wstring to_stdwstring(const Vec3 &v, int precision = 6)
	{
		wchar_t buf[60];
		swprintf(buf, L"%.*f;%.*f;%.*f", precision, v.x, precision, v.y, precision, v.z);
		return buf;
	}

	inline std::wstring to_stdwstring(const Vec4 &v, int precision = 6)
	{
		wchar_t buf[80];
		swprintf(buf, L"%.*f;%.*f;%.*f;%.*f", precision, v.x, precision, v.y, precision, v.z, precision, v.w);
		return buf;
	}

	inline std::wstring to_stdwstring(bool v)
	{
		return v ? L"true" : L"false";
	}

	inline std::wstring to_stdwstring(int v)
	{
		wchar_t buf[20];
		swprintf(buf, L"%d", v);
		return buf;
	}

	inline std::wstring to_stdwstring(uint v)
	{
		wchar_t buf[20];
		swprintf(buf, L"%u", v);
		return buf;
	}

	inline std::wstring to_stdwstring(const Ivec2 &v)
	{
		wchar_t buf[40];
		swprintf(buf, L"%d;%d", v.x, v.y);
		return buf;
	}

	inline std::wstring to_stdwstring(const Ivec3 &v)
	{
		wchar_t buf[60];
		swprintf(buf, L"%d;%d;%d", v.x, v.y, v.z);
		return buf;
	}

	inline std::wstring to_stdwstring(const Ivec4 &v)
	{
		wchar_t buf[80];
		swprintf(buf, L"%d;%d;%d;%d", v.x, v.y, v.z, v.w);
		return buf;
	}

	inline std::wstring to_stdwstring(const Bvec2 &v)
	{
		wchar_t buf[40];
		swprintf(buf, L"%d;%d", v.x, v.y);
		return buf;
	}

	inline std::wstring to_stdwstring(const Bvec3 &v)
	{
		wchar_t buf[60];
		swprintf(buf, L"%d;%d;%d", v.x, v.y, v.z);
		return buf;
	}

	inline std::wstring to_stdwstring(const Bvec4 &v)
	{
		wchar_t buf[80];
		swprintf(buf, L"%d;%d;%d;%d", v.x, v.y, v.z, v.w);
		return buf;
	}

	inline float stof1(const char *s)
	{
		float ret;
		sscanf(s, "%f", &ret);
		return ret;
	}

	inline Vec2 stof2(const char *s)
	{
		Vec2 ret;
		sscanf(s, "%f;%f", &ret.x, &ret.y);
		return ret;
	}

	inline Vec3 stof3(const char *s)
	{
		Vec3 ret;
		sscanf(s, "%f;%f;%f", &ret.x, &ret.y, &ret.z);
		return ret;
	}

	inline Vec4 stof4(const char *s)
	{
		Vec4 ret;
		sscanf(s, "%f;%f;%f;%f", &ret.x, &ret.y, &ret.z, &ret.w);
		return ret;
	}

	inline int stou1(const char *s)
	{
		int ret;
		sscanf(s, "%u", &ret);
		return ret;
	}

	inline int stoi1(const char *s)
	{
		int ret;
		sscanf(s, "%d", &ret);
		return ret;
	}

	inline Ivec2 stoi2(const char *s)
	{
		Ivec2 ret;
		sscanf(s, "%d;%d", &ret.x, &ret.y);
		return ret;
	}

	inline Ivec3 stoi3(const char *s)
	{
		Ivec3 ret;
		sscanf(s, "%d;%d;%d", &ret.x, &ret.y, &ret.z);
		return ret;
	}

	inline Ivec4 stoi4(const char *s)
	{
		Ivec4 ret;
		sscanf(s, "%d;%d;%d;%d", &ret.x, &ret.y, &ret.z, &ret.w);
		return ret;
	}

	inline uchar stob1(const char *s)
	{
		int ret;
		sscanf(s, "%d", &ret);
		return (uchar)ret;
	}

	inline Bvec2 stob2(const char *s)
	{
		Ivec2 ret;
		sscanf(s, "%d;%d;", &ret.x, &ret.y);
		return Bvec2(ret.x, ret.y);
	}

	inline Bvec3 stob3(const char *s)
	{
		Ivec3 ret;
		sscanf(s, "%d;%d;%d", &ret.x, &ret.y, &ret.z);
		return Bvec3(ret.x, ret.y, ret.z);
	}

	inline Bvec4 stob4(const char *s)
	{
		Ivec4 ret;
		sscanf(s, "%d;%d;%d;%d", &ret.x, &ret.y, &ret.z, &ret.w);
		return Bvec4(ret.x, ret.y, ret.z, ret.w);
	}

	inline float stof1(const wchar_t *s)
	{
		float ret;
		swscanf(s, L"%f", &ret);
		return ret;
	}

	inline Vec2 stof2(const wchar_t *s)
	{
		Vec2 ret;
		swscanf(s, L"%f;%f", &ret.x, &ret.y);
		return ret;
	}

	inline Vec3 stof3(const wchar_t *s)
	{
		Vec3 ret;
		swscanf(s, L"%f;%f;%f", &ret.x, &ret.y, &ret.z);
		return ret;
	}

	inline Vec4 stof4(const wchar_t *s)
	{
		Vec4 ret;
		swscanf(s, L"%f;%f;%f;%f", &ret.x, &ret.y, &ret.z, &ret.w);
		return ret;
	}

	inline uint stou1(const wchar_t *s)
	{
		int ret;
		swscanf(s, L"%u", &ret);
		return ret;
	}

	inline int stoi1(const wchar_t *s)
	{
		int ret;
		swscanf(s, L"%d", &ret);
		return ret;
	}

	inline Ivec2 stoi2(const wchar_t *s)
	{
		Ivec2 ret;
		swscanf(s, L"%d;%d", &ret.x, &ret.y);
		return ret;
	}

	inline Ivec3 stoi3(const wchar_t *s)
	{
		Ivec3 ret;
		swscanf(s, L"%d;%d;%d", &ret.x, &ret.y, &ret.z);
		return ret;
	}

	inline Ivec4 stoi4(const wchar_t *s)
	{
		Ivec4 ret;
		swscanf(s, L"%d;%d;%d;%d", &ret.x, &ret.y, &ret.z, &ret.w);
		return ret;
	}

	inline uchar stob1(const wchar_t *s)
	{
		int ret;
		swscanf(s, L"%d", &ret);
		return (uchar)ret;
	}

	inline Bvec2 stob2(const wchar_t *s)
	{
		Ivec2 ret;
		swscanf(s, L"%d;%d;", &ret.x, &ret.y);
		return Bvec2(ret.x, ret.y);
	}

	inline Bvec3 stob3(const wchar_t *s)
	{
		Ivec3 ret;
		swscanf(s, L"%d;%d;%d", &ret.x, &ret.y, &ret.z);
		return Bvec3(ret.x, ret.y, ret.z);
	}

	inline Bvec4 stob4(const wchar_t *s)
	{
		Ivec4 ret;
		swscanf(s, L"%d;%d;%d;%d", &ret.x, &ret.y, &ret.z, &ret.w);
		return Bvec4(ret.x, ret.y, ret.z, ret.w);
	}

	struct EnumItem
	{
		FLAME_FOUNDATION_EXPORTS const char *name() const;
		FLAME_FOUNDATION_EXPORTS int value() const;
	};

	struct EnumInfo
	{
		FLAME_FOUNDATION_EXPORTS const char *name() const;

		FLAME_FOUNDATION_EXPORTS int item_count() const;
		FLAME_FOUNDATION_EXPORTS EnumItem *item(int idx) const;
		FLAME_FOUNDATION_EXPORTS int find_item(const char *name) const;
		FLAME_FOUNDATION_EXPORTS int find_item(int value) const;

		FLAME_FOUNDATION_EXPORTS String serialize_value(bool single, int v) const;
	};

	enum VariableTag
	{
		VariableTagEnumSingle,
		VariableTagEnumMulti,
		VariableTagVariable,
		VariableTagPointer,
		VariableTagArrayOfVariable,
		VariableTagArrayOfPointer

		// 'Array' means Array<>, which is a special UDT, NOT 'array' in C/C++ language (e.g. int abc[100])
	};

	FLAME_FOUNDATION_EXPORTS const char *get_variable_tag_name(VariableTag tag);

	struct UdtInfo;

	struct VariableInfo
	{
		FLAME_FOUNDATION_EXPORTS VariableTag tag() const;
		FLAME_FOUNDATION_EXPORTS const char *type_name() const;
		FLAME_FOUNDATION_EXPORTS uint type_hash() const;
		FLAME_FOUNDATION_EXPORTS UdtInfo* type() const; // return nullptr or this is an UDT
		FLAME_FOUNDATION_EXPORTS const char *name() const;
		FLAME_FOUNDATION_EXPORTS const char *attribute() const;
		FLAME_FOUNDATION_EXPORTS int offset() const;
		FLAME_FOUNDATION_EXPORTS int size() const;
		FLAME_FOUNDATION_EXPORTS int count() const; // for array count in C/C++ language (e.g. int abc[100] is 100)
		FLAME_FOUNDATION_EXPORTS const CommonData &default_value() const;

		FLAME_FOUNDATION_EXPORTS void get(const void *src, bool is_obj, int item_index, CommonData *dst) const;
		FLAME_FOUNDATION_EXPORTS void set(const CommonData *src, void *dst, bool is_obj, int item_index) const;
		FLAME_FOUNDATION_EXPORTS void array_resize(int size, void *dst, bool is_obj) const;
		FLAME_FOUNDATION_EXPORTS bool compare(void *src, void *dst) const;
		FLAME_FOUNDATION_EXPORTS bool compare_to_default(void *src, bool is_obj) const;
		FLAME_FOUNDATION_EXPORTS String serialize_value(const void *src, bool is_obj, int item_index, int precision = 6) const;
		FLAME_FOUNDATION_EXPORTS void unserialize_value(const std::string &str, void *dst, bool is_obj, int item_index) const;
		// when this is an array, item_index is ignored
		// when item_index is -1, means the data is one item of the array
		// else, means data is an array, and use item_index to index the item
	};

	typedef VariableInfo* VariableInfoPtr;

	struct UdtInfo
	{
		FLAME_FOUNDATION_EXPORTS const char *name() const;

		FLAME_FOUNDATION_EXPORTS int size() const;

		FLAME_FOUNDATION_EXPORTS int item_count() const;
		FLAME_FOUNDATION_EXPORTS VariableInfo*item(int idx) const;
		FLAME_FOUNDATION_EXPORTS int find_item_i(const char *name) const;

		FLAME_FOUNDATION_EXPORTS const wchar_t* module_name() const;
		FLAME_FOUNDATION_EXPORTS const void* update_function_rva() const;
		FLAME_FOUNDATION_EXPORTS const char* update_function_code() const;

		FLAME_FOUNDATION_EXPORTS void construct(void *dst) const;
		FLAME_FOUNDATION_EXPORTS void destruct(void *dst) const;
	};

	typedef UdtInfo* UdtInfoPtr;

	struct FunctionInfo
	{
		FLAME_FOUNDATION_EXPORTS const char* name() const;

		FLAME_FOUNDATION_EXPORTS void* rva() const;
		FLAME_FOUNDATION_EXPORTS int parameter_count() const;
	};

	typedef FunctionInfo* FunctionInfoPtr;

	FLAME_FOUNDATION_EXPORTS Array<EnumInfo*> get_enums();
	FLAME_FOUNDATION_EXPORTS EnumInfo *find_enum(uint name_hash);

	FLAME_FOUNDATION_EXPORTS Array<UdtInfo*> get_udts();
	FLAME_FOUNDATION_EXPORTS UdtInfo*find_udt(uint name_hash);

	FLAME_FOUNDATION_EXPORTS Array<FunctionInfo*> get_functions();
	FLAME_FOUNDATION_EXPORTS FunctionInfo* find_function(uint name_hash);

	struct SerializableAttribute
	{
		FLAME_FOUNDATION_EXPORTS const std::string &name() const;
		FLAME_FOUNDATION_EXPORTS const std::string &value() const;

		FLAME_FOUNDATION_EXPORTS void set_name(const std::string &name);
		FLAME_FOUNDATION_EXPORTS void set_value(const std::string &value);
	};

	struct SerializableNode
	{
		FLAME_FOUNDATION_EXPORTS const std::string &name() const;
		FLAME_FOUNDATION_EXPORTS const std::string &value() const;
		FLAME_FOUNDATION_EXPORTS bool is_xml_CDATA() const;

		FLAME_FOUNDATION_EXPORTS void set_name(const std::string &name);
		FLAME_FOUNDATION_EXPORTS void set_value(const std::string &value);
		FLAME_FOUNDATION_EXPORTS void set_xml_CDATA(bool v);

		FLAME_FOUNDATION_EXPORTS SerializableAttribute *new_attr(const std::string &name, const std::string &value);
		FLAME_FOUNDATION_EXPORTS SerializableAttribute *insert_attr(int idx, const std::string &name, const std::string &value);
		FLAME_FOUNDATION_EXPORTS void remove_attr(int idx);
		FLAME_FOUNDATION_EXPORTS void remove_attr(SerializableAttribute *a);
		FLAME_FOUNDATION_EXPORTS void clear_attrs();
		FLAME_FOUNDATION_EXPORTS int attr_count() const;
		FLAME_FOUNDATION_EXPORTS SerializableAttribute *attr(int idx) const;
		FLAME_FOUNDATION_EXPORTS SerializableAttribute *find_attr(const std::string &name);

		FLAME_FOUNDATION_EXPORTS void add_node(SerializableNode *n);
		FLAME_FOUNDATION_EXPORTS SerializableNode *new_node(const std::string &name);
		FLAME_FOUNDATION_EXPORTS SerializableNode *insert_node(int idx, const std::string &name);
		FLAME_FOUNDATION_EXPORTS void remove_node(int idx);
		FLAME_FOUNDATION_EXPORTS void remove_node(SerializableNode *n);
		FLAME_FOUNDATION_EXPORTS void clear_nodes();
		FLAME_FOUNDATION_EXPORTS int node_count() const;
		FLAME_FOUNDATION_EXPORTS SerializableNode *node(int idx) const;
		FLAME_FOUNDATION_EXPORTS SerializableNode *find_node(const std::string &name);

		FLAME_FOUNDATION_EXPORTS void save_xml(const std::wstring &filename) const;
		FLAME_FOUNDATION_EXPORTS void save_bin(const std::wstring &filename) const;

		FLAME_FOUNDATION_EXPORTS void *unserialize(UdtInfo*u, Function<voidptr(void* c, UdtInfoPtr udt, voidptr parent, uint att_hash)>& obj_generator);

		FLAME_FOUNDATION_EXPORTS static SerializableNode *create(const std::string &name);
		FLAME_FOUNDATION_EXPORTS static SerializableNode *create_from_xml(const std::wstring &filename);
		FLAME_FOUNDATION_EXPORTS static SerializableNode *create_from_bin(const std::wstring &filename);
		FLAME_FOUNDATION_EXPORTS static SerializableNode *serialize(UdtInfo*u, void *src, int precision = 6);
		FLAME_FOUNDATION_EXPORTS static void destroy(SerializableNode *n);
	};

	// something with $ means it is reflected

	FLAME_FOUNDATION_EXPORTS int typeinfo_collect_init();
	FLAME_FOUNDATION_EXPORTS void typeinfo_collect(const std::vector<std::wstring> &filenames);
	FLAME_FOUNDATION_EXPORTS void typeinfo_load(const std::wstring &filename);
	FLAME_FOUNDATION_EXPORTS void typeinfo_save(const std::wstring &filename);
	FLAME_FOUNDATION_EXPORTS void typeinfo_clear();
}

