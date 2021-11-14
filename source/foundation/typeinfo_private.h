#pragma once

#include "typeinfo.h"

namespace flame
{
	struct TypeInfo_EnumSingle : TypeInfo
	{
		EnumInfo* ei = nullptr;

		TypeInfo_EnumSingle(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo(TypeEnumSingle, base_name, sizeof(int))
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
			TypeInfo(TypeEnumMulti, base_name, sizeof(int))
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

	struct TypeInfo_void : TypeInfo
	{
		TypeInfo_void() :
			TypeInfo(TypeData, "void", 0)
		{
			basic_type = VoidType;
		}
	};

	struct TypeInfo_bool : TypeInfo
	{
		TypeInfo_bool() :
			TypeInfo(TypeData, "bool", sizeof(bool))
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

	struct TypeInfo_char : TypeInfo
	{
		TypeInfo_char() :
			TypeInfo(TypeData, "char", sizeof(char))
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

	struct TypeInfo_uchar : TypeInfo
	{
		TypeInfo_uchar() :
			TypeInfo(TypeData, "uchar", sizeof(uchar))
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

	struct TypeInfo_wchar : TypeInfo
	{
		TypeInfo_wchar() :
			TypeInfo(TypeData, "wchar_t", sizeof(wchar_t))
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

	struct TypeInfo_short : TypeInfo
	{
		TypeInfo_short() :
			TypeInfo(TypeData, "short", sizeof(short))
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

	struct TypeInfo_ushort : TypeInfo
	{
		TypeInfo_ushort() :
			TypeInfo(TypeData, "ushort", sizeof(ushort))
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

	struct TypeInfo_int : TypeInfo
	{
		TypeInfo_int() :
			TypeInfo(TypeData, "int", sizeof(int))
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

	struct TypeInfo_uint : TypeInfo
	{
		TypeInfo_uint() :
			TypeInfo(TypeData, "uint", sizeof(uint))
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

	struct TypeInfo_int64 : TypeInfo
	{
		TypeInfo_int64() :
			TypeInfo(TypeData, "int64", sizeof(int64))
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

	struct TypeInfo_uint64 : TypeInfo
	{
		TypeInfo_uint64() :
			TypeInfo(TypeData, "uint64", sizeof(uint64))
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

	struct TypeInfo_float : TypeInfo
	{
		TypeInfo_float() :
			TypeInfo(TypeData, "float", sizeof(float))
		{
			basic_type = FloatingType;
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

	struct TypeInfo_cvec2 : TypeInfo
	{
		TypeInfo_cvec2() :
			TypeInfo(TypeData, "glm::vec<2,uchar,0>", sizeof(cvec2))
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

	struct TypeInfo_cvec3 : TypeInfo
	{
		TypeInfo_cvec3() :
			TypeInfo(TypeData, "glm::vec<3,uchar,0>", sizeof(cvec3))
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

	struct TypeInfo_cvec4 : TypeInfo
	{
		TypeInfo_cvec4() :
			TypeInfo(TypeData, "glm::vec<4,uchar,0>", sizeof(cvec4))
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

	struct TypeInfo_ivec2 : TypeInfo
	{
		TypeInfo_ivec2() :
			TypeInfo(TypeData, "glm::vec<2,int,0>", sizeof(ivec2))
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

	struct TypeInfo_ivec3 : TypeInfo
	{
		TypeInfo_ivec3() :
			TypeInfo(TypeData, "glm::vec<3,int,0>", sizeof(ivec3))
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

	struct TypeInfo_ivec4 : TypeInfo
	{
		TypeInfo_ivec4() :
			TypeInfo(TypeData, "glm::vec<4,int,0>", sizeof(ivec4))
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

	struct TypeInfo_uvec2 : TypeInfo
	{
		TypeInfo_uvec2() :
			TypeInfo(TypeData, "glm::vec<2,uint,0>", sizeof(uvec2))
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

	struct TypeInfo_uvec3 : TypeInfo
	{
		TypeInfo_uvec3() :
			TypeInfo(TypeData, "glm::vec<3,uint,0>", sizeof(uvec3))
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

	struct TypeInfo_uvec4 : TypeInfo
	{
		TypeInfo_uvec4() :
			TypeInfo(TypeData, "glm::vec<4,uint,0>", sizeof(uvec4))
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

	struct TypeInfo_vec2 : TypeInfo
	{
		TypeInfo_vec2() :
			TypeInfo(TypeData, "glm::vec<2,float,0>", sizeof(vec2))
		{
			basic_type = FloatingType;
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

	struct TypeInfo_vec3 : TypeInfo
	{
		TypeInfo_vec3() :
			TypeInfo(TypeData, "glm::vec<3,float,0>", sizeof(vec3))
		{
			basic_type = FloatingType;
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

	struct TypeInfo_vec4 : TypeInfo
	{
		TypeInfo_vec4() :
			TypeInfo(TypeData, "glm::vec<4,float,0>", sizeof(vec4))
		{
			basic_type = FloatingType;
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
	
	struct TypeInfo_string : TypeInfo
	{
		TypeInfo_string() :
			TypeInfo(TypeData, "std::basic_string<char,std::char_traits<char>,std::allocator<char>>", sizeof(std::string))
		{
		}

		void* create(bool create_pointing) const override 
		{ 
			return new std::string; 
		}
		void destroy(void* p, bool destroy_pointing) const override
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

	struct TypeInfo_wstring : TypeInfo
	{
		TypeInfo_wstring() :
			TypeInfo(TypeData, "std::basic_string<wchar_t,std::char_traits<wchar_t>,std::allocator<wchar_t>>", sizeof(std::string))
		{
		}

		void* create(bool create_pointing) const override
		{
			return new std::wstring;
		}
		void destroy(void* p, bool destroy_pointing) const override
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

	struct TypeInfo_Rect : TypeInfo
	{
		TypeInfo_Rect() :
			TypeInfo(TypeData, "flame::Rect", sizeof(Rect))
		{
			basic_type = FloatingType;
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

	struct TypeInfo_AABB : TypeInfo
	{
		TypeInfo_AABB() :
			TypeInfo(TypeData, "flame::AABB", sizeof(AABB))
		{
			basic_type = FloatingType;
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

	struct TypeInfo_Plane : TypeInfo
	{
		TypeInfo_Plane() :
			TypeInfo(TypeData, "flame::Plane", sizeof(Plane))
		{
			basic_type = FloatingType;
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

	struct TypeInfo_Frustum : TypeInfo
	{
		TypeInfo_Frustum() :
			TypeInfo(TypeData, "flame::Frustum", sizeof(Plane))
		{
		}
	};

	struct TypeInfo_mat2 : TypeInfo
	{
		TypeInfo_mat2() :
			TypeInfo(TypeData, "glm::mat<2,2,float,0>", sizeof(mat2))
		{
			basic_type = FloatingType;
			vec_size = 2;
			col_size = 2;
		}
	};

	struct TypeInfo_mat3 : TypeInfo
	{
		TypeInfo_mat3() :
			TypeInfo(TypeData, "glm::mat<3,3,float,0>", sizeof(mat3))
		{
			basic_type = FloatingType;
			vec_size = 3;
			col_size = 3;
		}
	};

	struct TypeInfo_mat4 : TypeInfo
	{
		TypeInfo_mat4() :
			TypeInfo(TypeData, "glm::mat<4,4,float,0>", sizeof(mat4))
		{
			basic_type = FloatingType;
			vec_size = 4;
			col_size = 4;
		}
	};

	struct TypeInfo_quat : TypeInfo
	{
		TypeInfo_quat() :
			TypeInfo(TypeData, "glm::qua<float,0>", sizeof(quat))
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

	struct TypeInfo_Pointer : TypeInfo
	{
		TypeInfo_Pointer(std::string_view base_name, TypeInfoDataBase& db) :
			TypeInfo(TypePointer, base_name, sizeof(void*))
		{
			pointed_type = TypeInfo::get(TypeData, name, db);
			if (pointed_type)
			{
				basic_type = pointed_type->basic_type;
				vec_size = pointed_type->vec_size;
				col_size = pointed_type->col_size;
			}
		}

		void* create(bool create_pointing) const override
		{
			auto p = malloc(sizeof(void*));
			if (create_pointing && pointed_type)
				*(void**)p = pointed_type->create();
			return p;
		}
		void destroy(void* p, bool destroy_pointing) const override
		{
			if (destroy_pointing && pointed_type)
				pointed_type->destroy(*(void**)p);
			free(p);
		}
		std::string serialize(const void* p) const override
		{
			auto pp = *(void**)p;
			if (pointed_type && pp)
				return pointed_type->serialize(pp);
			return "";
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			auto pp = *(void**)dst;
			if (pointed_type && pp)
				pointed_type->unserialize(str, pp);
		}
	};

	struct TypeInfo_charp : TypeInfo_Pointer
	{
		TypeInfo_charp(TypeInfoDataBase& db) :
			TypeInfo_Pointer("char", db)
		{
		}

		void* create(bool create_pointing) const override
		{
			auto p = malloc(sizeof(void*));
			if (create_pointing)
			{
				auto str = (char*)malloc(sizeof(char) * 256);
				*(char**)p = str;
				str[0] = 0;
			}
			return p;
		}
		void destroy(void* p, bool destroy_pointing) const override
		{
			if (destroy_pointing)
				free(*(char**)p);
			free(p);
		}
		void copy(void* dst, const void* src) const override
		{
			auto str = *(char**)src;
			strcpy(*(char**)dst, str ? str : "");
		}
		bool compare(const void* a, const void* b) const override
		{
			return std::string(*(char**)a) == std::string(*(char**)b);
		}
		std::string serialize(const void* p) const override
		{
			auto str = *(char**)p;
			return str ? str : "";
		}
		void unserialize(const std::string& str, void* dst) const override
		{
			auto& p = *(char**)dst;
			free(p);
			p = (char*)malloc(str.size() + 1);
			strcpy(p, str.c_str());
		}
	};

	struct TypeInfo_wcharp : TypeInfo_Pointer
	{
		TypeInfo_wcharp(TypeInfoDataBase& db) :
			TypeInfo_Pointer("wchar_t", db)
		{
		}

		void* create(bool create_pointing) const override
		{
			auto p = malloc(sizeof(void*));
			if (create_pointing)
			{
				auto str = (wchar_t*)malloc(sizeof(wchar_t) * 256);
				*(wchar_t**)p = str;
				str[0] = 0;
			}
			return p;
		}
		void destroy(void* p, bool destroy_pointing) const override
		{
			if (destroy_pointing)
				free(*(wchar_t**)p);
			free(p);
		}
		void copy(void* dst, const void* src) const override
		{
			auto str = *(wchar_t**)src;
			wcscpy(*(wchar_t**)dst, str ? str : L"");
		}
		bool compare(const void* a, const void* b) const override
		{
			return std::wstring(*(wchar_t**)a) == std::wstring(*(wchar_t**)b);
		}
		std::string serialize(const void* p) const override
		{
			auto str = *(wchar_t**)p;
			return str ? w2s(str) : "";
		}
		void unserialize(const std::string& _str, void* dst) const override
		{
			auto str = s2w(_str);
			auto& p = *(wchar_t**)dst;
			free(p);
			p = (wchar_t*)malloc(sizeof(wchar_t) * (str.size() + 1));
			wcscpy(p, str.c_str());
		}
	};

}
