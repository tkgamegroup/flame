#include <flame/serialize.h>
#include "typeinfo_private.h"

#include <Windows.h>

namespace flame
{
	struct TypeInfoKey
	{
		int t;
		std::string n;

		TypeInfoKey(int t, const std::string& n) :
			t(t),
			n(n)
		{
		}

		bool operator==(const TypeInfoKey& rhs) const
		{
			return t == rhs.t && n == rhs.n;
		}
	};

	struct Hasher_TypeInfoKey
	{
		std::size_t operator()(const TypeInfoKey& k) const
		{
			return std::hash<std::string>()(k.n) ^ std::hash<int>()(k.t);
		}
	};

	static std::unordered_map<TypeInfoKey, std::unique_ptr<TypeInfoPrivate>, Hasher_TypeInfoKey> typeinfos;

	static std::unordered_map<std::string, std::unique_ptr<EnumInfoPrivate>> enums;
	static std::unordered_map<std::string, std::unique_ptr<FunctionInfoPrivate>> functions;
	static std::unordered_map<std::string, std::unique_ptr<UdtInfoPrivate>> udts;
	static std::vector<std::unique_ptr<LibraryPrivate>> libraries;

	struct TypeInfoPrivate_Pod : TypeInfoPrivate
	{
		TypeInfoPrivate_Pod(TypeTag tag, const std::string& base_name, uint size) :
			TypeInfoPrivate(tag, base_name, size)
		{
		}

		void* create(bool) const override { return malloc(size); }
		void destroy(void* p, bool) const override { free(p); }
		void copy(void* dst, const void* src) const override { memcpy(dst, src, size); }
		bool compare(void* a, const void* b) const override { return memcmp(a, b, size) == 0; }
		void serialize(const void* src, void* str, char* (*str_allocator)(void* str, uint size)) const override {}
		void unserialize(void* dst, const char* src) const override {}
	};

	struct TypeInfoPrivate_EnumSingle : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_EnumSingle(const std::string& base_name) :
			TypeInfoPrivate_Pod(TypeEnumSingle, base_name, sizeof(int))
		{
		}

		void serialize(const void* src, void* str, char* (*str_allocator)(void* str, uint size)) const override
		{
			const auto& s = find_enum(name)->find_item(*(int*)src)->name;
			strcpy(str_allocator(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(int*)dst = find_enum(name)->find_item(src)->value;
		}
	};

	struct TypeInfoPrivate_EnumMulti : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_EnumMulti(const std::string& base_name) :
			TypeInfoPrivate_Pod(TypeEnumMulti, base_name, sizeof(int))
		{
		}

		void serialize(const void* src, void* str, char* (*str_allocator)(void* str, uint size)) const override
		{
			auto e = find_enum(name);
			std::string s;
			auto v = *(int*)src;
			for (auto i = 0; i < e->items.size(); i++)
			{
				if ((v & 1) == 1)
				{
					if (i > 0)
						s += '|';
					s += e->find_item(1 << i)->name;
				}
				v >>= 1;
			}
			strcpy(str_allocator(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			auto e = find_enum(name);
			auto v = 0;
			auto sp = SUS::split(src, '|');
			for (auto& t : sp)
				v |= e->find_item(t)->value;
			*(int*)dst = v;
		}
	};

	struct TypeInfoPrivate_void : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_void() :
			TypeInfoPrivate_Pod(TypeData, "void", 0)
		{
		}
	};

	struct TypeInfoPrivate_bool : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_bool() :
			TypeInfoPrivate_Pod(TypeData, "bool", sizeof(bool))
		{
			basic_type = BooleanType;
		}

		void serialize(const void* src, void* str, char* (*str_allocator)(void* str, uint size)) const override
		{
			auto s = to_string(*(bool*)src);
			strcpy(str_allocator(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			auto str = std::string(src);
			if (str == "false")
				*(bool*)dst = false;
			else if (str == "true")
				*(bool*)dst = true;
			else
				*(bool*)dst = std::stoi(str) != 0;
		}
	};

	struct TypeInfoPrivate_char : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_char() :
			TypeInfoPrivate_Pod(TypeData, "char", sizeof(char))
		{
			basic_type = CharType;
		}

		void serialize(const void* src, void* str, char* (*str_allocator)(void* str, uint size)) const override
		{
			auto s = to_string(*(char*)src);
			strcpy(str_allocator(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(char*)dst = sto<char>(src);
		}
	};

	struct TypeInfoPrivate_uchar : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_uchar() :
			TypeInfoPrivate_Pod(TypeData, "uchar", sizeof(uchar))
		{
			basic_type = CharType;
			is_signed = false;
		}

		void serialize(const void* src, void* str, char* (*str_allocator)(void* str, uint size)) const override
		{
			auto s = to_string(*(uchar*)src);
			strcpy(str_allocator(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(uchar*)dst = sto<uchar>(src);
		}
	};

	struct TypeInfoPrivate_wchar : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_wchar() :
			TypeInfoPrivate_Pod(TypeData, "wchar_t", sizeof(wchar_t))
		{
			basic_type = WideCharType;
		}

		void serialize(const void* src, void* str, char* (*str_allocator)(void* str, uint size)) const override
		{
			auto s = to_string(*(wchar_t*)src);
			strcpy(str_allocator(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(wchar_t*)dst = sto<wchar_t>(src);
		}
	};

	struct TypeInfoPrivate_int : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_int() :
			TypeInfoPrivate_Pod(TypeData, "int", sizeof(int))
		{
			basic_type = IntegerType;
		}

		void serialize(const void* src, void* str, char* (*str_allocator)(void* str, uint size)) const override
		{
			auto s = to_string(*(int*)src);
			strcpy(str_allocator(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(int*)dst = sto<int>(src);
		}
	};

	struct TypeInfoPrivate_uint : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_uint() :
			TypeInfoPrivate_Pod(TypeData, "uint", sizeof(uint))
		{
			basic_type = IntegerType;
			is_signed = false;
		}

		void serialize(const void* src, void* str, char* (*str_allocator)(void* str, uint size)) const override
		{
			auto s = to_string(*(uint*)src);
			strcpy(str_allocator(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(uint*)dst = sto<uint>(src);
		}
	};

	struct TypeInfoPrivate_int64 : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_int64() :
			TypeInfoPrivate_Pod(TypeData, "int64", sizeof(int64))
		{
			basic_type = IntegerType;
		}

		void serialize(const void* src, void* str, char* (*str_allocator)(void* str, uint size)) const override
		{
			auto s = to_string(*(int64*)src);
			strcpy(str_allocator(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(int64*)dst = sto<int64>(src);
		}
	};

	struct TypeInfoPrivate_uint64 : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_uint64() :
			TypeInfoPrivate_Pod(TypeData, "uint64", sizeof(uint64))
		{
			basic_type = IntegerType;
			is_signed = false;
		}

		void serialize(const void* src, void* str, char* (*str_allocator)(void* str, uint size)) const override
		{
			auto s = to_string(*(uint64*)src);
			strcpy(str_allocator(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(uint64*)dst = sto<uint64>(src);
		}
	};

	struct TypeInfoPrivate_float : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_float() :
			TypeInfoPrivate_Pod(TypeData, "float", sizeof(float))
		{
			basic_type = FloatingType;
		}

		void serialize(const void* src, void* str, char* (*str_allocator)(void* str, uint size)) const override
		{
			auto s = to_string(*(float*)src);
			strcpy(str_allocator(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(float*)dst = sto<float>(src);
		}
	};

	struct TypeInfoPrivate_cvec2 : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_cvec2() :
			TypeInfoPrivate_Pod(TypeData, "glm::vec<2,uchar,0>", sizeof(cvec2))
		{
			basic_type = CharType;
			is_signed = false;
			vec_size = 2;
		}

		void serialize(const void* src, void* str, char* (*str_allocator)(void* str, uint size)) const override
		{
			auto s = to_string(*(cvec2*)src);
			strcpy(str_allocator(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(cvec2*)dst = sto<cvec2>(src);
		}
	};

	struct TypeInfoPrivate_cvec3 : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_cvec3() :
			TypeInfoPrivate_Pod(TypeData, "glm::vec<3,uchar,0>", sizeof(cvec3))
		{
			basic_type = CharType;
			is_signed = false;
			vec_size = 3;
			ret_by_reg = false;
		}

		void serialize(const void* src, void* str, char* (*str_allocator)(void* str, uint size)) const override
		{
			auto s = to_string(*(cvec3*)src);
			strcpy(str_allocator(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(cvec3*)dst = sto<cvec3>(src);
		}
	};

	struct TypeInfoPrivate_cvec4 : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_cvec4() :
			TypeInfoPrivate_Pod(TypeData, "glm::vec<4,uchar,0>", sizeof(cvec4))
		{
			basic_type = CharType;
			is_signed = false;
			vec_size = 4;
			ret_by_reg = false;
		}

		void serialize(const void* src, void* str, char* (*str_allocator)(void* str, uint size)) const override
		{
			auto s = to_string(*(cvec4*)src);
			strcpy(str_allocator(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(cvec4*)dst = sto<cvec4>(src);
		}
	};

	struct TypeInfoPrivate_ivec2 : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_ivec2() :
			TypeInfoPrivate_Pod(TypeData, "glm::vec<2,int,0>", sizeof(ivec2))
		{
			basic_type = IntegerType;
			vec_size = 2;
			ret_by_reg = false;
		}

		void serialize(const void* src, void* str, char* (*str_allocator)(void* str, uint size)) const override
		{
			auto s = to_string(*(ivec2*)src);
			strcpy(str_allocator(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(ivec2*)dst = sto<ivec2>(src);
		}
	};

	struct TypeInfoPrivate_ivec3 : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_ivec3() :
			TypeInfoPrivate_Pod(TypeData, "glm::vec<3,int,0>", sizeof(ivec3))
		{
			basic_type = IntegerType;
			vec_size = 3;
			ret_by_reg = false;
		}

		void serialize(const void* src, void* str, char* (*str_allocator)(void* str, uint size)) const override
		{
			auto s = to_string(*(ivec3*)src);
			strcpy(str_allocator(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(ivec3*)dst = sto<ivec3>(src);
		}
	};

	struct TypeInfoPrivate_ivec4 : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_ivec4() :
			TypeInfoPrivate_Pod(TypeData, "glm::vec<4,int,0>", sizeof(ivec4))
		{
			basic_type = IntegerType;
			vec_size = 4;
			ret_by_reg = false;
		}

		void serialize(const void* src, void* str, char* (*str_allocator)(void* str, uint size)) const override
		{
			auto s = to_string(*(ivec4*)src);
			strcpy(str_allocator(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(ivec4*)dst = sto<ivec4>(src);
		}
	};

	struct TypeInfoPrivate_uvec2 : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_uvec2() :
			TypeInfoPrivate_Pod(TypeData, "glm::vec<2,uint,0>", sizeof(uvec2))
		{
			basic_type = IntegerType;
			is_signed = false;
			vec_size = 2;
			ret_by_reg = false;
		}

		void serialize(const void* src, void* str, char* (*str_allocator)(void* str, uint size)) const override
		{
			auto s = to_string(*(uvec2*)src);
			strcpy(str_allocator(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(uvec2*)dst = sto<uvec2>(src);
		}
	};

	struct TypeInfoPrivate_uvec3 : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_uvec3() :
			TypeInfoPrivate_Pod(TypeData, "glm::vec<3,uint,0>", sizeof(uvec3))
		{
			basic_type = IntegerType;
			is_signed = false;
			vec_size = 3;
			ret_by_reg = false;
		}

		void serialize(const void* src, void* str, char* (*str_allocator)(void* str, uint size)) const override
		{
			auto s = to_string(*(uvec3*)src);
			strcpy(str_allocator(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(uvec3*)dst = sto<uvec3>(src);
		}
	};

	struct TypeInfoPrivate_uvec4 : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_uvec4() :
			TypeInfoPrivate_Pod(TypeData, "glm::vec<4,uint,0>", sizeof(uvec4))
		{
			basic_type = IntegerType;
			is_signed = false;
			vec_size = 4;
			ret_by_reg = false;
		}

		void serialize(const void* src, void* str, char* (*str_allocator)(void* str, uint size)) const override
		{
			auto s = to_string(*(uvec4*)src);
			strcpy(str_allocator(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(uvec4*)dst = sto<uvec4>(src);
		}
	};

	struct TypeInfoPrivate_vec2 : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_vec2() :
			TypeInfoPrivate_Pod(TypeData, "glm::vec<2,float,0>", sizeof(vec2))
		{
			basic_type = FloatingType;
			vec_size = 2;
			ret_by_reg = false;
		}

		void serialize(const void* src, void* str, char* (*str_allocator)(void* str, uint size)) const override
		{
			auto s = to_string(*(vec2*)src);
			strcpy(str_allocator(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(vec2*)dst = sto<vec2>(src);
		}
	};

	struct TypeInfoPrivate_vec3 : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_vec3() :
			TypeInfoPrivate_Pod(TypeData, "glm::vec<3,float,0>", sizeof(vec3))
		{
			basic_type = FloatingType;
			vec_size = 3;
			ret_by_reg = false;
		}

		void serialize(const void* src, void* str, char* (*str_allocator)(void* str, uint size)) const override
		{
			auto s = to_string(*(vec3*)src);
			strcpy(str_allocator(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(vec3*)dst = sto<vec3>(src);
		}
	};

	struct TypeInfoPrivate_vec4 : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_vec4() :
			TypeInfoPrivate_Pod(TypeData, "glm::vec<4,float,0>", sizeof(vec4))
		{
			basic_type = FloatingType;
			vec_size = 4;
			ret_by_reg = false;
		}

		void serialize(const void* src, void* str, char* (*str_allocator)(void* str, uint size)) const override
		{
			auto s = to_string(*(vec4*)src);
			strcpy(str_allocator(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(vec4*)dst = sto<vec4>(src);
		}
	};

	struct TypeInfoPrivate_Rect : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Rect() :
			TypeInfoPrivate_Pod(TypeData, "flame::Rect", sizeof(Rect))
		{
			ret_by_reg = false;
		}
	};

	struct TypeInfoPrivate_mat2 : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_mat2() :
			TypeInfoPrivate_Pod(TypeData, "glm::mat<2,2,float,0>", sizeof(mat2))
		{
			basic_type = FloatingType;
			vec_size = 2;
			col_size = 2;
			ret_by_reg = false;
		}
	};

	struct TypeInfoPrivate_mat3 : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_mat3() :
			TypeInfoPrivate_Pod(TypeData, "glm::mat<3,3,float,0>", sizeof(mat3))
		{
			basic_type = FloatingType;
			vec_size = 3;
			col_size = 3;
			ret_by_reg = false;
		}
	};

	struct TypeInfoPrivate_mat4 : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_mat4() :
			TypeInfoPrivate_Pod(TypeData, "glm::mat<4,4,float,0>", sizeof(mat4))
		{
			basic_type = FloatingType;
			vec_size = 4;
			col_size = 4;
			ret_by_reg = false;
		}
	};

	struct TypeInfoPrivate_quat : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_quat() :
			TypeInfoPrivate_Pod(TypeData, "glm::qua<float,0>", sizeof(quat))
		{
		}

		void serialize(const void* src, void* str, char* (*str_allocator)(void* str, uint size)) const override
		{
			auto s = to_string(*(quat*)src);
			strcpy(str_allocator(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(quat*)dst = sto<quat>(src);
		}
	};

	struct TypeInfoPrivate_StringA : TypeInfoPrivate
	{
		TypeInfoPrivate_StringA() :
			TypeInfoPrivate(TypeData, "flame::StringA", sizeof(StringA))
		{
		}

		void* create(bool) const override
		{
			return f_new<StringA>();
		}
		void destroy(void* p, bool) const override
		{
			f_delete((StringA*)p);
		}
		void copy(void* dst, const void* src) const override
		{
			*(StringA*)dst = *(StringA*)src;
		}
		bool compare(void* dst, const void* src) const override
		{
			return (*(StringA*)dst).str() == (*(StringA*)src).str();
		}
		void serialize(const void* src, void* str, char* (*str_allocator)(void* str, uint size)) const override
		{
			const auto& s = *(StringA*)src;
			auto dst = str_allocator(str, s.s);
			if (s.v)
				strcpy(dst, s.v);
			else
				dst[0] = 0;
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(StringA*)dst = src;
		}
	};

	struct TypeInfoPrivate_StringW : TypeInfoPrivate
	{
		TypeInfoPrivate_StringW() :
			TypeInfoPrivate(TypeData, "flame::StringW", sizeof(StringW))
		{
		}

		void* create(bool) const override
		{
			return f_new<StringW>();
		}
		void destroy(void* p, bool) const override
		{
			f_delete((StringW*)p);
		}
		void copy(void* dst, const void* src) const override
		{
			*(StringW*)dst = *(StringW*)src;
		}
		bool compare(void* a, const void* b) const override
		{
			return (*(StringW*)a).str() == (*(StringW*)b).str();
		}
		void serialize(const void* src, void* str, char* (*str_allocator)(void* str, uint size)) const override
		{
			const auto s = w2s((*(StringW*)src).str());
			strcpy(str_allocator(str, s.size()), s.c_str());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(StringW*)dst = s2w(src);
		}
	};

	struct TypeInfoPrivate_Pointer : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Pointer(const std::string& base_name) :
			TypeInfoPrivate_Pod(TypePointer, base_name, sizeof(void*))
		{
			pointed_type = TypeInfoPrivate::get(TypeData, name);
		}

		void* create(bool create_pointing) const override 
		{ 
			auto p = new void*;
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
		void serialize(const void* src, void* str, char* (*str_allocator)(void* str, uint size)) const override
		{
			if (pointed_type)
				pointed_type->serialize(*(void**)src, str, str_allocator);
		}
		void unserialize(void* dst, const char* src) const override
		{
			if (pointed_type)
				pointed_type->unserialize(*(void**)dst, src);
		}
	};

	struct TypeInfoPrivate_charp : TypeInfoPrivate_Pointer
	{
		TypeInfoPrivate_charp() :
			TypeInfoPrivate_Pointer("char")
		{
		}

		void* create(bool create_pointing) const override
		{
			auto p = malloc(sizeof(void*));
			if (create_pointing)
			{
				*(char**)p = (char*)malloc(sizeof(char));
				(*(char**)p)[0] = 0;
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
			strcpy(*(char**)dst, *(char**)src);
		}
		bool compare(void* a, const void* b) const override
		{
			return std::string(*(char**)a) == std::string(*(char**)b);
		}
		void serialize(const void* src, void* str, char* (*str_allocator)(void* str, uint size)) const override
		{
			auto& p = *(char**)src;
			strcpy(str_allocator(str, strlen(p)), p);
		}
		void unserialize(void* dst, const char* src) const override
		{
			auto& p = *(char**)dst;
			free(p);
			p = (char*)malloc(strlen(src) + 1);
			strcpy(p, src);
		}
	};

	struct TypeInfoPrivate_wcharp : TypeInfoPrivate_Pointer
	{
		TypeInfoPrivate_wcharp() :
			TypeInfoPrivate_Pointer("wchar_t")
		{
		}

		void* create(bool create_pointing) const override
		{
			auto p = malloc(sizeof(void*));
			if (create_pointing)
			{
				*(wchar_t**)p = (wchar_t*)malloc(sizeof(wchar_t));
				(*(wchar_t**)p)[0] = 0;
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
			wcscpy(*(wchar_t**)dst, *(wchar_t**)src);
		}
		bool compare(void* a, const void* b) const override
		{
			return std::wstring(*(wchar_t**)a) == std::wstring(*(wchar_t**)b);
		}
		void serialize(const void* src, void* str, char* (*str_allocator)(void* str, uint size)) const override 
		{
			auto& p = *(wchar_t**)src;
			const auto s = w2s(p);
			strcpy(str_allocator(str, s.size()), s.c_str());
		}
		void unserialize(void* dst, const char* src) const override
		{
			auto str = s2w(src);
			auto& p = *(wchar_t**)dst;
			free(p);
			p = new wchar_t[str.size() + 1];
			wcscpy(p, str.c_str());
		}
	};

	static TypeInfoPrivate* void_type = nullptr;

	struct _Initializer
	{
		_Initializer()
		{
			{
				auto t = new TypeInfoPrivate_void;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
				void_type = t;
			}
			{
				auto t = new TypeInfoPrivate_bool;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_char;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_uchar;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_wchar;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_int;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_uint;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_int64;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_uint64;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_float;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_cvec2;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_cvec3;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_cvec4;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_ivec2;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_ivec3;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_ivec4;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_uvec2;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_uvec3;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_uvec4;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_vec2;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_vec3;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_vec4;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_Rect;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_mat2;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_mat3;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_mat4;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_quat;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_StringA;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_StringW;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_charp;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_wcharp;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}

			wchar_t app_name[260];
			GetModuleFileNameW(nullptr, app_name, size(app_name));
			get_library_dependencies(app_name, [](Capture& c, const wchar_t* filename) {
				auto path = std::filesystem::path(filename);
				path.replace_extension(".typeinfo");
				if (std::filesystem::exists(path))
					Library::load(filename);
			}, Capture());
		}
	};
	static _Initializer _initializer;

	TypeInfoPrivate::TypeInfoPrivate(TypeTag tag, const std::string& name, uint size) :
		tag(tag),
		name(name),
		size(size)
	{
		ret_by_reg = size <= sizeof(void*);
	}

	TypeInfo* TypeInfo::get(TypeTag tag, const char* name)
	{
		return TypeInfoPrivate::get(tag, name);
	}

	TypeInfoPrivate* TypeInfoPrivate::get(TypeTag tag, const std::string& name)
	{
		if (tag == TypeData && name.empty())
			return void_type;

		auto key = TypeInfoKey(tag, name);
		auto it = typeinfos.find(key);
		if (it != typeinfos.end())
			return it->second.get();
		TypeInfoPrivate* t = nullptr;
		switch (tag)
		{
		case TypeEnumSingle:
			t = new TypeInfoPrivate_EnumSingle(name);
			break;
		case TypeEnumMulti:
			t = new TypeInfoPrivate_EnumMulti(name);
			break;
		case TypePointer:
			t = new TypeInfoPrivate_Pointer(name);
			break;
		}
		if (!t)
			return t;
		typeinfos.emplace(key, t);
		return t;
	}

	void ReflectMetaPrivate::get_token(char** pname, char** pvalue, uint idx) const
	{
		const auto& s = tokens[idx];
		*pname = (char*)s.first.data();
		*pvalue = (char*)s.second.data();
	}

	bool ReflectMetaPrivate::get_token(const std::string& str, char** pvalue) const
	{
		for (auto& t : tokens)
		{
			if (t.first == str)
			{
				if (pvalue)
					*pvalue = (char*)t.second.data();
				return true;
			}
		}
		return false;
	}

	VariableInfoPrivate::VariableInfoPrivate(UdtInfoPrivate* udt, uint index, TypeInfoPrivate* type, const std::string& name, uint offset, const std::string& _meta) :
		udt(udt),
		index(index),
		type(type),
		name(name),
		offset(offset),
		default_value(nullptr)
	{
		auto sp1 = SUS::split(_meta);
		for (auto& t : sp1)
		{
			auto sp2 = SUS::split(t, '=');
			meta.tokens.emplace_back(sp2[0], sp2.size() > 1 ? sp2[1] : "");
		}
	}

	VariableInfoPrivate::~VariableInfoPrivate()
	{
		type->destroy(default_value);
	}

	EnumItemPrivate::EnumItemPrivate(EnumInfoPrivate* ei, uint index, const std::string& name, int value) :
		ei(ei),
		index(index),
		name(name),
		value(value)
	{
	}

	EnumInfoPrivate::EnumInfoPrivate(LibraryPrivate* library, const std::string& name) :
		library(library),
		name(name)
	{
	}

	EnumItemPrivate* EnumInfoPrivate::find_item(const std::string& name) const
	{
		for (auto& i : items)
		{
			if (i->name == name)
				return i.get();
		}
		return nullptr;
	}

	EnumItemPrivate* EnumInfoPrivate::find_item(int value) const
	{
		for (auto& i : items)
		{
			if (i->value == value)
				return i.get();
		}
		return nullptr;
	}

	FunctionInfoPrivate::FunctionInfoPrivate(LibraryPrivate* library, UdtInfoPrivate* udt, uint index, const std::string& name, uint rva, uint voff, TypeInfoPrivate* type) :
		library(library),
		udt(udt),
		index(index),
		name(name),
		rva(rva),
		voff(voff),
		type(type)
	{
	}

	void* FunctionInfoPrivate::get_address(void* obj) const
	{
		auto address = rva ? library->address + rva : (obj ? *(void**)((*(char**)obj) + voff) : nullptr);
		fassert(address);
		return address;
	}

	bool FunctionInfoPrivate::check(TypeInfo* ret, uint parms_count, TypeInfo* const* parms) const
	{
		if (type != ret)
			return false;
		if (parameters.size() != parms_count)
			return false;
		for (auto i = 0; i < parms_count; i++)
		{
			if (parameters[i] != parms[i])
				return false;
		}
		return true;
	}

	extern "C" void __call(void* f, void* list1, void* list2, void* dummy);

	void FunctionInfoPrivate::call(void* obj, void* ret, void* ps) const
	{
		auto idx = 0;
		void* list1[6];
		float list2[4];
		if (obj)
			list1[idx++] = obj;
		if (!type->ret_by_reg)
		{
			list1[idx++] = ret;
			ret = nullptr;
		}

		auto p = (char*)ps;
		for (auto parm : parameters)
		{
			switch (parm->tag)
			{
			case TypeEnumSingle:
			case TypeEnumMulti:
				list1[idx++] = (void*)*(int*)p;
				p += sizeof(int);
				break;
			case TypeData:
				if (parm->basic_type == FloatingType)
				{
					list2[idx++] = *(float*)p;
					p += sizeof(float);
				}
				else if (parm->size == 1)
				{
					list1[idx++] = (void*)*(char*)p;
					p += 1;
				}
				else if (parm->size == 4)
				{
					list1[idx++] = (void*)*(int*)p;
					p += 4;
				}
				else if (parm->size == 8)
				{
					list1[idx++] = *(void**)p;
					p += 8;
				}
				break;
			case TypePointer:
				list1[idx++] = *(void**)p;
				p += 8;
				break;
			}
		}

		void* staging_rax;
		float staging_xmm0;
		list1[4] = &staging_rax;
		list1[5] = &staging_xmm0;
		__call(get_address(obj), list1, list2, nullptr);
		if (ret)
		{
			if (type->basic_type == FloatingType)
				memcpy(ret, &staging_xmm0, type->size);
			else
				memcpy(ret, &staging_rax, type->size);
		}
	}

	UdtInfoPrivate::UdtInfoPrivate(LibraryPrivate* library, const std::string& name, uint size, const std::string& base_name) :
		library(library),
		name(name),
		size(size),
		base_name(base_name)
	{
	}

	VariableInfoPrivate* UdtInfoPrivate::find_variable(const std::string& name) const
	{
		for (auto& v : variables)
		{
			if (v->name == name)
				return v.get();
		}
		return nullptr;
	}

	FunctionInfoPrivate* UdtInfoPrivate::find_function(const std::string& name) const
	{
		for (auto& f : functions)
		{
			if (f->name == name)
				return f.get();
		}
		return nullptr;
	}

	LibraryPrivate::LibraryPrivate(const std::wstring& filename, bool require_typeinfo) :
		filename(filename)
	{
		address = (char*)LoadLibraryW(filename.c_str());

		if (require_typeinfo)
		{
			std::filesystem::path library_path(filename);
			if (!library_path.is_absolute())
			{
				wchar_t app_path[260];
				get_app_path(app_path);
				library_path = app_path / library_path;
			}
			auto typeinfo_path = library_path;
			typeinfo_path.replace_extension(L".typeinfo");

			pugi::xml_document file;
			pugi::xml_node file_root;
			if (!file.load_file(typeinfo_path.c_str()) || (file_root = file.first_child()).name() != std::string("typeinfo"))
			{
				printf("cannot find typeinfo: %s\n", typeinfo_path.string().c_str());
				fassert(0);
			}

			for (auto n_enum : file_root.child("enums"))
			{
				auto e = new EnumInfoPrivate(this, n_enum.attribute("name").value());
				enums.emplace(e->name, e);

				for (auto n_item : n_enum.child("items"))
					e->items.emplace_back(new EnumItemPrivate(e, e->items.size(), n_item.attribute("name").value(), n_item.attribute("value").as_int()));
			}

			for (auto n_udt : file_root.child("udts"))
			{
				auto u = new UdtInfoPrivate(this, n_udt.attribute("name").value(), n_udt.attribute("size").as_uint(), n_udt.attribute("base_name").value());
				udts.emplace(u->name, u);

				for (auto n_variable : n_udt.child("variables"))
				{
					auto type = TypeInfoPrivate::get((TypeTag)n_variable.attribute("type_tag").as_int(), n_variable.attribute("type_name").value());
					auto v = new VariableInfoPrivate(u, u->variables.size(), type, n_variable.attribute("name").value(), n_variable.attribute("offset").as_uint(),
						n_variable.attribute("meta").value());
					u->variables.emplace_back(v);
					auto dv = n_variable.attribute("default_value");
					if (dv)
					{
						v->default_value = type->create();
						type->unserialize(v->default_value, dv.value());
					}
				}

				for (auto n_function : n_udt.child("functions"))
				{
					auto f = new FunctionInfoPrivate(this, u, u->functions.size(), n_function.attribute("name").value(), 
						n_function.attribute("rva").as_uint(), n_function.attribute("voff").as_uint(),
						TypeInfoPrivate::get((TypeTag)n_function.attribute("type_tag").as_int(), n_function.attribute("type_name").value()));
					u->functions.emplace_back(f);
					for (auto n_parameter : n_function.child("parameters"))
						f->parameters.push_back(TypeInfoPrivate::get((TypeTag)n_parameter.attribute("type_tag").as_int(), n_parameter.attribute("type_name").value()));
				}
			}

			has_typeinfo = true;
		}
	}

	LibraryPrivate::~LibraryPrivate()
	{
		if (address)
			FreeLibrary((HMODULE)address);

		if (has_typeinfo)
		{
			for (auto it = enums.begin(); it != enums.end();)
			{
				if (it->second->library == this)
					it = enums.erase(it);
				else
					it++;
			}
			for (auto it = udts.begin(); it != udts.end();)
			{
				if (it->second->library == this)
					it = udts.erase(it);
				else
					it++;
			}
		}
	}

	void LibraryPrivate::release()
	{
		ref_count--;
		if (ref_count == 0)
		{
			for (auto it = libraries.begin(); it != libraries.end(); it++)
			{
				if (it->get() == this)
				{
					libraries.erase(it);
					break;
				}
			}
		}
	}

	void* LibraryPrivate::_get_exported_function(const char* name)
	{
		return GetProcAddress((HMODULE)address, name);
	}

	Library* Library::load(const wchar_t* filename, bool require_typeinfo)
	{
		for (auto& l : libraries)
		{
			if (l->filename == filename)
				return l.get();
		}
		auto library = new LibraryPrivate(filename, require_typeinfo);
		libraries.emplace_back(library);
		return library;
	}

	EnumInfoPrivate* find_enum(const std::string& name)
	{
		for (auto& e : enums)
		{
			if (e.second->name == name)
				return e.second.get();
		}
		return nullptr;
	}
	UdtInfoPrivate* find_udt(const std::string& name)
	{
		for (auto& u : udts)
		{
			if (u.second->name == name)
				return u.second.get();
		}
		return nullptr;
	}

	EnumInfo* add_enum(const char* name, uint items_count, char** item_names, int* item_values)
	{
		auto e = new EnumInfoPrivate(nullptr, name);
		enums.emplace(e->name, e);

		for (auto i = 0; i < items_count; i++)
			e->items.emplace_back(new EnumItemPrivate(e, i, item_names[i], item_values[i]));

		return e;
	}

	EnumInfo* find_enum(const char* name) { return find_enum(std::string(name)); }
	UdtInfo* find_udt(const char* name) { return find_udt(std::string(name)); }

	void traverse_enums(void (*callback)(Capture& c, EnumInfo* ei), const Capture& capture)
	{
		for (auto& e : enums)
			callback((Capture&)capture, e.second.get());
		free(capture._data);
	}

	void traverse_udts(void (*callback)(Capture& c, UdtInfo* ui), const Capture& capture)
	{
		for (auto& u : udts)
			callback((Capture&)capture, u.second.get());
		free(capture._data);
	}
}
