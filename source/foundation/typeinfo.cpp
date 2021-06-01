#include "typeinfo_private.h"

#include <Windows.h>
#include <functional>
#include <pugixml.hpp>

namespace flame
{
	TypeInfoDataBasePrivate tidb;

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
		void serialize(const void* src, char* dst) const override {}
		void unserialize(void* dst, const char* src) const override {}
	};

	struct TypeInfoPrivate_EnumSingle : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_EnumSingle(const std::string& base_name) :
			TypeInfoPrivate_Pod(TypeEnumSingle, base_name, sizeof(int))
		{
		}

		void serialize(const void* src, char* dst) const override
		{
			const auto& s = find_enum(name)->find_item(*(int*)src)->name;
			strcpy(dst, s.data());
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

		void serialize(const void* src, char* dst) const override
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
			strcpy(dst, s.data());
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
			basic_type = VoidType;
		}
	};

	struct TypeInfoPrivate_bool : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_bool() :
			TypeInfoPrivate_Pod(TypeData, "bool", sizeof(bool))
		{
			basic_type = BooleanType;
		}

		void serialize(const void* src, char* dst) const override
		{
			auto s = to_string(*(bool*)src);
			strcpy(dst, s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			auto str = std::string(src);
			if (str == "false")
				*(bool*)dst = false;
			else if (str == "true")
				*(bool*)dst = true;
			else
				*(bool*)dst = sto<int>(str) != 0;
		}
	};

	struct TypeInfoPrivate_char : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_char() :
			TypeInfoPrivate_Pod(TypeData, "char", sizeof(char))
		{
			basic_type = CharType;
		}

		void serialize(const void* src, char* dst) const override
		{
			auto s = to_string(*(char*)src);
			strcpy(dst, s.data());
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

		void serialize(const void* src, char* dst) const override
		{
			auto s = to_string(*(uchar*)src);
			strcpy(dst, s.data());
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

		void serialize(const void* src, char* dst) const override
		{
			auto s = to_string(*(wchar_t*)src);
			strcpy(dst, s.data());
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

		void serialize(const void* src, char* dst) const override
		{
			auto s = to_string(*(int*)src);
			strcpy(dst, s.data());
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

		void serialize(const void* src, char* dst) const override
		{
			auto s = to_string(*(uint*)src);
			strcpy(dst, s.data());
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

		void serialize(const void* src, char* dst) const override
		{
			auto s = to_string(*(int64*)src);
			strcpy(dst, s.data());
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

		void serialize(const void* src, char* dst) const override
		{
			auto s = to_string(*(uint64*)src);
			strcpy(dst, s.data());
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

		void serialize(const void* src, char* dst) const override
		{
			auto s = to_string(*(float*)src);
			strcpy(dst, s.data());
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

		const char* get_code_name() const override { return "cvec2"; }

		void serialize(const void* src, char* dst) const override
		{
			auto s = to_string(*(cvec2*)src);
			strcpy(dst, s.data());
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

		const char* get_code_name() const override { return "cvec3"; }

		void serialize(const void* src, char* dst) const override
		{
			auto s = to_string(*(cvec3*)src);
			strcpy(dst, s.data());
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

		const char* get_code_name() const override { return "cvec4"; }

		void serialize(const void* src, char* dst) const override
		{
			auto s = to_string(*(cvec4*)src);
			strcpy(dst, s.data());
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

		const char* get_code_name() const override { return "ivec2"; }

		void serialize(const void* src, char* dst) const override
		{
			auto s = to_string(*(ivec2*)src);
			strcpy(dst, s.data());
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

		const char* get_code_name() const override { return "ivec3"; }

		void serialize(const void* src, char* dst) const override
		{
			auto s = to_string(*(ivec3*)src);
			strcpy(dst, s.data());
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

		const char* get_code_name() const override { return "ivec4"; }

		void serialize(const void* src, char* dst) const override
		{
			auto s = to_string(*(ivec4*)src);
			strcpy(dst, s.data());
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

		const char* get_code_name() const override { return "uvec2"; }

		void serialize(const void* src, char* dst) const override
		{
			auto s = to_string(*(uvec2*)src);
			strcpy(dst, s.data());
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

		const char* get_code_name() const override { return "uvec3"; }

		void serialize(const void* src, char* dst) const override
		{
			auto s = to_string(*(uvec3*)src);
			strcpy(dst, s.data());
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

		const char* get_code_name() const override { return "uvec4"; }

		void serialize(const void* src, char* dst) const override
		{
			auto s = to_string(*(uvec4*)src);
			strcpy(dst, s.data());
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

		const char* get_code_name() const override { return "vec2"; }

		void serialize(const void* src, char* dst) const override
		{
			auto s = to_string(*(vec2*)src);
			strcpy(dst, s.data());
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

		const char* get_code_name() const override { return "vec3"; }

		void serialize(const void* src, char* dst) const override
		{
			auto s = to_string(*(vec3*)src);
			strcpy(dst, s.data());
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

		const char* get_code_name() const override { return "vec4"; }

		void serialize(const void* src, char* dst) const override
		{
			auto s = to_string(*(vec4*)src);
			strcpy(dst, s.data());
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

	struct TypeInfoPrivate_AABB : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_AABB() :
			TypeInfoPrivate_Pod(TypeData, "flame::AABB", sizeof(AABB))
		{
			ret_by_reg = false;
		}
	};

	struct TypeInfoPrivate_Plane : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Plane() :
			TypeInfoPrivate_Pod(TypeData, "flame::Plane", sizeof(Plane))
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

		const char* get_code_name() const override { return "mat2"; }
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

		const char* get_code_name() const override { return "mat3"; }
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

		const char* get_code_name() const override { return "mat4"; }
	};

	struct TypeInfoPrivate_quat : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_quat() :
			TypeInfoPrivate_Pod(TypeData, "glm::qua<float,0>", sizeof(quat))
		{
		}

		const char* get_code_name() const override { return "quat"; }

		void serialize(const void* src, char* dst) const override
		{
			auto s = to_string(*(quat*)src);
			strcpy(dst, s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(quat*)dst = sto<quat>(src);
		}
	};

	struct TypeInfoPrivate_Pointer : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Pointer(const std::string& base_name) :
			TypeInfoPrivate_Pod(TypePointer, base_name, sizeof(void*))
		{
			pointed_type = TypeInfoPrivate::get(TypeData, name);
			if (pointed_type)
			{
				basic_type = pointed_type->basic_type;
				vec_size = pointed_type->vec_size;
				col_size = pointed_type->col_size;
			}
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
		void serialize(const void* src, char* dst) const override
		{
			if (pointed_type)
				pointed_type->serialize(*(void**)src, dst);
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
				*(char**)p = (char*)malloc(sizeof(char) * 256);
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
			auto str = *(char**)src;
			if (str)
				strcpy(*(char**)dst, str);
		}
		bool compare(void* a, const void* b) const override
		{
			return std::string(*(char**)a) == std::string(*(char**)b);
		}
		void serialize(const void* src, char* dst) const override
		{
			auto& p = *(char**)src;
			strcpy(dst, p);
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
				*(wchar_t**)p = (wchar_t*)malloc(sizeof(wchar_t) * 256);
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
			auto str = *(wchar_t**)src;
			if (str)
				wcscpy(*(wchar_t**)dst, *(wchar_t**)src);
		}
		bool compare(void* a, const void* b) const override
		{
			return std::wstring(*(wchar_t**)a) == std::wstring(*(wchar_t**)b);
		}
		void serialize(const void* src, char* dst) const override 
		{
			auto& p = *(wchar_t**)src;
			auto s = w2s(p);
			strcpy(dst, s.c_str());
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
				tidb.typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
				void_type = t;
			}
			{
				auto t = new TypeInfoPrivate_bool;
				tidb.typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_char;
				tidb.typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_uchar;
				tidb.typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_wchar;
				tidb.typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_int;
				tidb.typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_uint;
				tidb.typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_int64;
				tidb.typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_uint64;
				tidb.typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_float;
				tidb.typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_cvec2;
				tidb.typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_cvec3;
				tidb.typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_cvec4;
				tidb.typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_ivec2;
				tidb.typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_ivec3;
				tidb.typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_ivec4;
				tidb.typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_uvec2;
				tidb.typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_uvec3;
				tidb.typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_uvec4;
				tidb.typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_vec2;
				tidb.typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_vec3;
				tidb.typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_vec4;
				tidb.typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_Rect;
				tidb.typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_mat2;
				tidb.typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_mat3;
				tidb.typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_mat4;
				tidb.typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_quat;
				tidb.typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_charp;
				tidb.typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}
			{
				auto t = new TypeInfoPrivate_wcharp;
				tidb.typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
			}

			wchar_t app_name_buf[260];
			get_app_path(app_name_buf, true);
			auto app_name = std::wstring(app_name_buf);
			if (!app_name.ends_with(L"typeinfogen.exe"))
			{
				get_module_dependencies(app_name.c_str(), [](Capture& c, const wchar_t* filename) {
					auto path = std::filesystem::path(filename);
					auto ti_path = path;
					ti_path.replace_extension(".typeinfo");
					if (std::filesystem::exists(ti_path))
						load_typeinfo(path);
				}, Capture());
			}
		}
	};
	static _Initializer _initializer;

	TypeInfoPrivate::TypeInfoPrivate(TypeTag tag, const std::string& name, uint size) :
		tag(tag),
		name(name),
		size(size)
	{
		switch (tag)
		{
		case TypeEnumSingle:
			full_name = "es__" + name;
			break;
		case TypeEnumMulti:
			full_name = "em__" + name;
			break;
		case TypeData:
			full_name = "d__" + name;
			break;
		case TypePointer:
			full_name = "p__" + name;
			break;
		}
		SUS::replace_all(full_name, "::", "__");
		ret_by_reg = size <= sizeof(void*);
	}

	TypeInfoPrivate* TypeInfoPrivate::get(TypeTag tag, const std::string& name, TypeInfoDataBasePrivate* db)
	{
		if (tag == TypeData && name.empty())
			return void_type;

		if (!db)
			db = &tidb;

		auto key = TypeInfoKey(tag, name);
		if (db != &tidb)
		{
			auto it = db->typeinfos.find(key);
			if (it != db->typeinfos.end())
				return it->second.get();
		}
		{
			auto it = tidb.typeinfos.find(key);
			if (it != tidb.typeinfos.end())
				return it->second.get();
		}

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
		case TypeData:
		{
			auto udt = find_udt(name, db);
			if (udt)
				t = new TypeInfoPrivate_Pod(TypeData, name, udt->size);
		}
			break;
		}

		if (t)
			db->typeinfos.emplace(key, t);
		return t;
	}

	TypeInfo* TypeInfo::get(TypeTag tag, const char* name, TypeInfoDataBase* db)
	{
		return TypeInfoPrivate::get(tag, std::string(name), (TypeInfoDataBasePrivate*)db);
	}

	VariableInfoPrivate::VariableInfoPrivate(UdtInfoPrivate* udt, uint index, TypeInfoPrivate* type, const std::string& name, uint offset, 
		uint array_size, uint array_stride, void* _default_value, const std::string& _metas) :
		udt(udt),
		index(index),
		type(type),
		name(name),
		offset(offset),
		array_size(array_size),
		array_stride(array_stride)
	{
		if (_default_value)
		{
			default_value = type->create();
			type->copy(default_value, _default_value);
		}
		metas.from_string(_metas);
	}

	VariableInfoPrivate::~VariableInfoPrivate()
	{
		if (default_value)
			type->destroy(default_value);
	}

	bool VariableInfoPrivate::get_meta(TypeMeta m, LightCommonValue* v) const
	{
		return metas.get_meta(m, v);
	}

	EnumItemInfoPrivate::EnumItemInfoPrivate(EnumInfoPrivate* ei, uint index, const std::string& name, int value) :
		ei(ei),
		index(index),
		name(name),
		value(value)
	{
	}

	EnumInfoPrivate::EnumInfoPrivate(const std::string& name) :
		name(name)
	{
	}

	EnumItemInfoPtr EnumInfoPrivate::find_item(const std::string& name) const
	{
		for (auto& i : items)
		{
			if (i->name == name)
				return i.get();
		}
		return nullptr;
	}

	EnumItemInfoPtr EnumInfoPrivate::find_item(int value) const
	{
		for (auto& i : items)
		{
			if (i->value == value)
				return i.get();
		}
		return nullptr;
	}

	EnumItemInfoPtr EnumInfoPrivate::add_item(const std::string& name, int value, int idx)
	{
		if (idx == -1)
			idx = items.size();
		auto ret = new EnumItemInfoPrivate(this, idx, name, value);
		items.emplace(items.begin() + idx, ret);
		return ret;
	}

	void EnumInfoPrivate::remove_item(EnumItemInfoPtr item)
	{
		fassert(item->ei == this);
		for (auto it = items.begin(); it != items.end(); it++)
		{
			if (it->get() == item)
			{
				items.erase(it);
				return;
			}
		}
	}

	FunctionInfoPrivate::FunctionInfoPrivate(UdtInfoPrivate* udt, void* library, uint index, const std::string& name, uint rva, uint voff, 
		TypeInfoPrivate* type, const std::string& _metas) :
		udt(udt),
		library(library),
		index(index),
		name(name),
		rva(rva),
		voff(voff),
		type(type)
	{
		metas.from_string(_metas);
	}

	bool FunctionInfoPrivate::get_meta(TypeMeta m, LightCommonValue* v) const
	{
		return metas.get_meta(m, v);
	}

	void FunctionInfoPrivate::add_parameter(TypeInfoPtr ti, int idx)
	{
		if (idx == -1)
			idx = parameters.size();
		parameters.emplace(parameters.begin() + idx, ti);
	}

	void FunctionInfoPrivate::remove_parameter(uint idx)
	{
		if (idx >= parameters.size())
			return;
		parameters.erase(parameters.begin() + idx);
	}

	bool FunctionInfoPrivate::check(TypeInfoPtr ret, uint parms_count, TypeInfoPtr const* parms) const
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

	void* FunctionInfoPrivate::get_address(void* obj) const
	{
		auto address = rva ? (char*)library + rva : (obj ? *(void**)((*(char**)obj) + voff) : nullptr);
		fassert(address);
		return address;
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

	UdtInfoPrivate::UdtInfoPrivate(void* library, const std::string& name, uint size, const std::string& base_name) :
		library(library),
		name(name),
		size(size),
		base_name(base_name)
	{
	}

	VariableInfoPtr UdtInfoPrivate::find_variable(const std::string& name) const
	{
		for (auto& v : variables)
		{
			if (v->name == name)
				return v.get();
		}
		return nullptr;
	}

	VariableInfoPtr UdtInfoPrivate::add_variable(TypeInfoPtr ti, const std::string& name, uint offset, uint array_size, uint array_stride, 
		void* default_value, const std::string& metas, int idx)
	{
		if (idx == -1)
			idx = variables.size();
		auto ret = new VariableInfoPrivate(this, idx, ti, name, offset, array_size, array_stride, default_value, metas);
		variables.emplace(variables.begin() + idx, ret);
		return ret;
	}

	void UdtInfoPrivate::remove_variable(VariableInfoPtr vi)
	{
		fassert(vi->udt == this);
		for (auto it = variables.begin(); it != variables.end(); it++)
		{
			if (it->get() == vi)
			{
				variables.erase(it);
				return;
			}
		}
	}

	FunctionInfoPtr UdtInfoPrivate::find_function(const std::string& name) const
	{
		for (auto& f : functions)
		{
			if (f->name == name)
				return f.get();
		}
		return nullptr;
	}

	FunctionInfoPtr UdtInfoPrivate::add_function(const std::string& name, uint rva, uint voff, TypeInfoPtr ti, const std::string& metas, int idx)
	{
		if (idx == -1)
			idx = functions.size();
		auto ret = new FunctionInfoPrivate(this, library, idx, name, rva, voff, ti, metas);
		functions.emplace(functions.begin() + idx, ret);
		return ret;
	}

	void UdtInfoPrivate::remove_function(FunctionInfoPtr fi)
	{
		fassert(fi->udt == this);
		for (auto it = functions.begin(); it != functions.end(); it++)
		{
			if (it->get() == fi)
			{
				functions.erase(it);
				return;
			}
		}
	}

	void TypeInfoDataBasePrivate::sort_udts()
	{
		if (!udts_sorted)
			udts_sorted = true;
		for (auto& u : udts)
			u.second->ranking = -1;
		std::function<void(UdtInfoPrivate* u)> get_ranking;
		get_ranking = [&](UdtInfoPrivate* u) {
			if (u->ranking == -1)
			{
				auto ranking = 0;
				for (auto& v : u->variables)
				{
					if (v->type->tag == TypePointer || v->type->tag == TypeData)
					{
						auto t = find_udt(v->type->name, this);
						if (t && t != u)
						{
							get_ranking(t);
							ranking = max(ranking, t->ranking + 1);
						}
					}
				}
				u->ranking = ranking;
			}
		};
		for (auto& u : udts)
			get_ranking(u.second.get());
	}

	TypeInfoDataBase* TypeInfoDataBase::create()
	{
		return new TypeInfoDataBasePrivate;
	}

	std::vector<TypeInfoPrivate*> get_types(TypeInfoDataBasePrivate* db)
	{
		if (!db)
			db = &tidb;
		std::vector<TypeInfoPrivate*> ret(db->typeinfos.size());
		auto idx = 0;
		for (auto& i : db->typeinfos)
			ret[idx++] = i.second.get();
		std::sort(ret.begin(), ret.end(), [](const auto& a, const auto& b) {
			return a->name < b->name;
		});
		return ret;
	}

	void get_types(TypeInfo** dst, uint* len, TypeInfoDataBase* _db)
	{
		auto db = (TypeInfoDataBasePrivate*)_db;
		if (!db)
			db = &tidb;
		if (len)
			*len = db->typeinfos.size();
		if (dst)
		{
			auto vec = get_types(db);
			memcpy(dst, vec.data(), sizeof(void*) * vec.size());
		}
	}

	EnumInfoPrivate* find_enum(const std::string& name, TypeInfoDataBasePrivate* db)
	{
		if (db && db != &tidb)
		{
			for (auto& e : db->enums)
			{
				if (e.second->name == name)
					return e.second.get();
			}
		}
		for (auto& e : tidb.enums)
		{
			if (e.second->name == name)
				return e.second.get();
		}
		return nullptr;
	}

	EnumInfo* find_enum(const char* name, TypeInfoDataBase* db) { return find_enum(std::string(name), (TypeInfoDataBasePrivate*)db); }

	EnumInfoPrivate* add_enum(const std::string& name, TypeInfoDataBasePrivate* db)
	{
		if (!db)
			db = &tidb;
		auto ret = new EnumInfoPrivate(name);
		db->enums.emplace(ret->name, ret);
		return ret;
	}

	EnumInfo* add_enum(const char* name, TypeInfoDataBase* db)
	{
		return add_enum(std::string(name), (TypeInfoDataBasePrivate*)db);
	}

	std::vector<EnumInfoPrivate*> get_enums(TypeInfoDataBasePrivate* db)
	{
		if (!db)
			db = &tidb;
		std::vector<EnumInfoPrivate*> ret(db->enums.size());
		auto idx = 0;
		for (auto& i : db->enums)
			ret[idx++] = i.second.get();
		std::sort(ret.begin(), ret.end(), [](const auto& a, const auto& b) {
			return a->name < b->name;
		});
		return ret;
	}

	void get_enums(EnumInfo** dst, uint* len, TypeInfoDataBase* _db)
	{
		auto db = (TypeInfoDataBasePrivate*)_db;
		if (!db)
			db = &tidb;
		if (len)
			*len = db->enums.size();
		if (dst)
		{
			auto vec = get_enums(db);
			memcpy(dst, vec.data(), sizeof(void*) * vec.size());
		}
	}

	UdtInfoPrivate* find_udt(const std::string& name, TypeInfoDataBasePrivate* db)
	{
		if (db && db != &tidb)
		{
			for (auto& u : db->udts)
			{
				if (u.second->name == name)
					return u.second.get();
			}
		}
		for (auto& u : tidb.udts)
		{
			if (u.second->name == name)
				return u.second.get();
		}
		return nullptr;
	}

	UdtInfo* find_udt(const char* name, TypeInfoDataBase* db) { return find_udt(std::string(name), (TypeInfoDataBasePrivate*)db); }

	UdtInfoPrivate* add_udt(const std::string& name, uint size, const std::string& base_name, void* library, TypeInfoDataBasePrivate* db)
	{
		if (!db)
			db = &tidb;
		db->udts_sorted = false;
		auto ret = new UdtInfoPrivate(library, name, size, base_name);
		db->udts.emplace(ret->name, ret);
		return ret;
	}

	UdtInfo* add_udt(const char* name, uint size, const char* base_name, TypeInfoDataBase* db)
	{
		return add_udt(name, size, base_name, nullptr, (TypeInfoDataBasePrivate*)db);
	}

	std::vector<UdtInfoPrivate*> get_udts(TypeInfoDataBasePrivate* db)
	{
		if (!db)
			db = &tidb;
		db->sort_udts();
		std::vector<UdtInfoPrivate*> ret(db->udts.size());
		auto idx = 0;
		for (auto& i : db->udts)
			ret[idx++] = i.second.get();
		std::sort(ret.begin(), ret.end(), [](const auto& a, const auto& b) {
			return a->ranking < b->ranking;
		});
		return ret;
	}

	void get_udts(UdtInfo** dst, uint* len, TypeInfoDataBase* _db)
	{
		auto db = (TypeInfoDataBasePrivate*)_db;
		if (!db)
			db = &tidb;
		if (len)
			*len = db->udts.size();
		if (dst)
		{
			auto vec = get_udts(db);
			memcpy(dst, vec.data(), sizeof(void*)* vec.size());
		}
	}

	void load_typeinfo(const std::filesystem::path& filename, TypeInfoDataBasePrivate* db)
	{
		if (!db)
			db = &tidb;

		std::filesystem::path path(filename);
		if (!path.is_absolute())
		{
			wchar_t app_path[260];
			get_app_path(app_path);
			path = app_path / path;
		}

		void* library = nullptr;
		if (path.extension() != L".typeinfo")
		{
			library = LoadLibraryW(path.c_str());
			path.replace_extension(L".typeinfo");
		}

		pugi::xml_document file;
		pugi::xml_node file_root;
		if (!file.load_file(path.c_str()) || (file_root = file.first_child()).name() != std::string("typeinfo"))
		{
			printf("cannot find typeinfo or wrong format: %s\n", path.string().c_str());
			fassert(0);
		}

		auto read_ti = [&](pugi::xml_node n) {
			TypeTag tag;
			TypeInfoPrivate::get(TypeEnumSingle, "flame::TypeTag")->unserialize(&tag, n.attribute("type_tag").value());
			return TypeInfoPrivate::get(tag, n.attribute("type_name").value(), db);
		};

		for (auto n_enum : file_root.child("enums"))
		{
			auto e = add_enum(std::string(n_enum.attribute("name").value()), db);

			for (auto n_item : n_enum.child("items"))
				e->items.emplace_back(new EnumItemInfoPrivate(e, e->items.size(), n_item.attribute("name").value(), n_item.attribute("value").as_int()));
		}
		for (auto n_udt : file_root.child("udts"))
		{
			auto u = add_udt(n_udt.attribute("name").value(), n_udt.attribute("size").as_uint(), n_udt.attribute("base_name").value(), library, db);

			for (auto n_variable : n_udt.child("variables"))
			{
				auto type = read_ti(n_variable);
				void* default_value = nullptr;
				if (auto n = n_variable.attribute("default_value"); n)
				{
					default_value = type->create();
					type->unserialize(default_value, n.value());
				}
				auto v = new VariableInfoPrivate(u, u->variables.size(), type, n_variable.attribute("name").value(),
					n_variable.attribute("offset").as_uint(), n_variable.attribute("array_size").as_uint(), n_variable.attribute("array_stride").as_uint(),
					default_value, n_variable.attribute("metas").value());
				u->variables.emplace_back(v);
				if (default_value)
					type->destroy(default_value);
			}
			for (auto n_function : n_udt.child("functions"))
			{
				auto f = new FunctionInfoPrivate(u, library, u->functions.size(), n_function.attribute("name").value(),
					n_function.attribute("rva").as_uint(), n_function.attribute("voff").as_uint(), read_ti(n_function), n_function.attribute("metas").value());
				u->functions.emplace_back(f);
				for (auto n_parameter : n_function.child("parameters"))
					f->parameters.push_back(read_ti(n_parameter));
			}
		}
	}

	void save_typeinfo(const std::filesystem::path& filename, TypeInfoDataBasePrivate* db)
	{
		if (!db)
			db = &tidb;

		pugi::xml_document file;
		auto file_root = file.append_child("typeinfo");

		auto e_tag = TypeInfo::get(TypeEnumSingle, "flame::TypeTag");
		auto write_ti = [&](TypeInfoPrivate* ti, pugi::xml_node n) {
			n.append_attribute("type_tag").set_value(e_tag->serialize(&ti->tag).c_str());
			n.append_attribute("type_name").set_value(ti->name.c_str());
		};

		if (!db->enums.empty())
		{
			auto enums = get_enums(db);
			auto n_enums = file_root.append_child("enums");
			for (auto& ei : enums)
			{
				auto n_enum = n_enums.append_child("enum");
				n_enum.append_attribute("name").set_value(ei->name.c_str());
				auto n_items = n_enum.append_child("items");
				for (auto& i : ei->items)
				{
					auto n_item = n_items.append_child("item");
					n_item.append_attribute("name").set_value(i->name.c_str());
					n_item.append_attribute("value").set_value(i->value);
				}
			}
		}
		if (!db->udts.empty())
		{
			auto udts = get_udts(db);
			auto n_udts = file_root.append_child("udts");
			for (auto& ui : udts)
			{
				auto n_udt = n_udts.append_child("udt");
				n_udt.append_attribute("name").set_value(ui->name.c_str());
				n_udt.append_attribute("size").set_value(ui->size);
				n_udt.append_attribute("base_name").set_value(ui->base_name.c_str());
				if (!ui->variables.empty())
				{
					auto n_variables = n_udt.prepend_child("variables");
					for (auto& vi : ui->variables)
					{
						auto n_variable = n_variables.append_child("variable");
						write_ti(vi->type, n_variable);
						n_variable.append_attribute("name").set_value(vi->name.c_str());
						n_variable.append_attribute("offset").set_value(vi->offset);
						n_variable.append_attribute("array_size").set_value(vi->array_size);
						n_variable.append_attribute("array_stride").set_value(vi->array_stride);
						n_variable.append_attribute("default_value").set_value(vi->default_value ? vi->type->serialize(vi->default_value).c_str() : "");
						n_variable.append_attribute("metas").set_value(vi->metas.to_string().c_str());
					}
				}
				if (!ui->functions.empty())
				{
					auto n_functions = n_udt.append_child("functions");
					for (auto& fi : ui->functions)
					{
						auto n_function = n_functions.append_child("function");
						n_function.append_attribute("name").set_value(fi->name.c_str());
						n_function.append_attribute("rva").set_value(fi->rva);
						n_function.append_attribute("voff").set_value(fi->voff);
						n_function.append_attribute("metas").set_value(fi->metas.to_string().c_str());
						write_ti(fi->type, n_function);
						if (!fi->parameters.empty())
						{
							auto n_parameters = n_function.append_child("parameters");
							for (auto p : fi->parameters)
								write_ti(p, n_parameters.append_child("parameter"));
						}
					}
				}
			}
		}

		file.save_file(filename.c_str());
	}

	void load_typeinfo(const wchar_t* filename, TypeInfoDataBase* db)
	{
		load_typeinfo(std::filesystem::path(filename), (TypeInfoDataBasePrivate*)db);
	}

	void save_typeinfo(const wchar_t* filename, TypeInfoDataBase* db)
	{
		save_typeinfo(std::filesystem::path(filename), (TypeInfoDataBasePrivate*)db);
	}
}
