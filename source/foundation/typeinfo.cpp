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

		void* create(void* p) const override { return p ? p : f_malloc(size); }
		void destroy(void* p, bool free_memory) const override { if (free_memory) f_free(p); }
		void copy(void* dst, const void* src) const override { memcpy(dst, src, size); }
		void serialize(void* str, const void* src) const override {}
		void unserialize(void* dst, const char* src) const override {}
	};

	struct TypeInfoPrivate_EnumSingle : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_EnumSingle(const std::string& base_name) :
			TypeInfoPrivate_Pod(TypeEnumSingle, base_name, sizeof(int))
		{
		}

		void serialize(void* str, const void* src) const override
		{
			const auto& s = find_enum(name)->find_item(*(int*)src)->name;
			strcpy(f_stralloc(str, s.size()), s.data());
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

		void serialize(void* str, const void* src) const override
		{
			auto e = find_enum(name);
			std::string s;
			auto v = *(int*)src;
			for (auto i = 0; i < e->items.size(); i++)
			{
				if ((v & 1) == 1)
				{
					if (i > 0)
						s += ',';
					s += e->find_item(1 << i)->name;
				}
				v >>= 1;
			}
			strcpy(f_stralloc(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			auto e = find_enum(name);
			auto v = 0;
			auto sp = SUS::split(src, ',');
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
		}

		void serialize(void* str, const void* src) const override
		{
			auto s = to_string(*(bool*)src);
			strcpy(f_stralloc(str, s.size()), s.data());
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
		}

		void serialize(void* str, const void* src) const override
		{
			auto s = to_string(*(char*)src);
			strcpy(f_stralloc(str, s.size()), s.data());
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
		}

		void serialize(void* str, const void* src) const override
		{
			auto s = to_string(*(uchar*)src);
			strcpy(f_stralloc(str, s.size()), s.data());
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
		}

		void serialize(void* str, const void* src) const override
		{
			auto s = to_string(*(wchar_t*)src);
			strcpy(f_stralloc(str, s.size()), s.data());
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
		}

		void serialize(void* str, const void* src) const override
		{
			auto s = to_string(*(int*)src);
			strcpy(f_stralloc(str, s.size()), s.data());
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
		}

		void serialize(void* str, const void* src) const override
		{
			auto s = to_string(*(uint*)src);
			strcpy(f_stralloc(str, s.size()), s.data());
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
		}

		void serialize(void* str, const void* src) const override
		{
			auto s = to_string(*(int64*)src);
			strcpy(f_stralloc(str, s.size()), s.data());
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
		}

		void serialize(void* str, const void* src) const override
		{
			auto s = to_string(*(uint64*)src);
			strcpy(f_stralloc(str, s.size()), s.data());
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
		}

		void serialize(void* str, const void* src) const override
		{
			auto s = to_string(*(float*)src);
			strcpy(f_stralloc(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(float*)dst = sto<float>(src);
		}
	};

	struct TypeInfoPrivate_Vec1c : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Vec1c() :
			TypeInfoPrivate_Pod(TypeData, "flame::Vec<1,uchar>", sizeof(Vec1c))
		{
		}

		void serialize(void* str, const void* src) const override
		{
			auto s = to_string(*(Vec1c*)src);
			strcpy(f_stralloc(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(Vec1c*)dst = sto<Vec1c>(src);
		}
	};

	struct TypeInfoPrivate_Vec2c : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Vec2c() :
			TypeInfoPrivate_Pod(TypeData, "flame::Vec<2,uchar>", sizeof(Vec2c))
		{
		}

		void serialize(void* str, const void* src) const override
		{
			auto s = to_string(*(Vec2c*)src);
			strcpy(f_stralloc(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(Vec2c*)dst = sto<Vec2c>(src);
		}
	};

	struct TypeInfoPrivate_Vec3c : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Vec3c() :
			TypeInfoPrivate_Pod(TypeData, "flame::Vec<3,uchar>", sizeof(Vec3c))
		{
		}

		void serialize(void* str, const void* src) const override
		{
			auto s = to_string(*(Vec3c*)src);
			strcpy(f_stralloc(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(Vec3c*)dst = sto<Vec3c>(src);
		}
	};

	struct TypeInfoPrivate_Vec4c : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Vec4c() :
			TypeInfoPrivate_Pod(TypeData, "flame::Vec<4,uchar>", sizeof(Vec4c))
		{
		}

		void serialize(void* str, const void* src) const override
		{
			auto s = to_string(*(Vec4c*)src);
			strcpy(f_stralloc(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(Vec4c*)dst = sto<Vec4c>(src);
		}
	};

	struct TypeInfoPrivate_Vec1i : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Vec1i() :
			TypeInfoPrivate_Pod(TypeData, "flame::Vec<1,int>", sizeof(Vec1i))
		{
		}

		void serialize(void* str, const void* src) const override
		{
			auto s = to_string(*(Vec1i*)src);
			strcpy(f_stralloc(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(Vec1i*)dst = sto<Vec1i>(src);
		}
	};

	struct TypeInfoPrivate_Vec2i : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Vec2i() :
			TypeInfoPrivate_Pod(TypeData, "flame::Vec<2,int>", sizeof(Vec2i))
		{
		}

		void serialize(void* str, const void* src) const override
		{
			auto s = to_string(*(Vec2i*)src);
			strcpy(f_stralloc(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(Vec2i*)dst = sto<Vec2i>(src);
		}
	};

	struct TypeInfoPrivate_Vec3i : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Vec3i() :
			TypeInfoPrivate_Pod(TypeData, "flame::Vec<3,int>", sizeof(Vec3i))
		{
		}

		void serialize(void* str, const void* src) const override
		{
			auto s = to_string(*(Vec3i*)src);
			strcpy(f_stralloc(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(Vec3i*)dst = sto<Vec3i>(src);
		}
	};

	struct TypeInfoPrivate_Vec4i : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Vec4i() :
			TypeInfoPrivate_Pod(TypeData, "flame::Vec<4,int>", sizeof(Vec4i))
		{
		}

		void serialize(void* str, const void* src) const override
		{
			auto s = to_string(*(Vec4i*)src);
			strcpy(f_stralloc(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(Vec4i*)dst = sto<Vec4i>(src);
		}
	};

	struct TypeInfoPrivate_Vec1u : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Vec1u() :
			TypeInfoPrivate_Pod(TypeData, "flame::Vec<1,uint>", sizeof(Vec1u))
		{
		}

		void serialize(void* str, const void* src) const override
		{
			auto s = to_string(*(Vec1u*)src);
			strcpy(f_stralloc(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(Vec1u*)dst = sto<Vec1u>(src);
		}
	};

	struct TypeInfoPrivate_Vec2u : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Vec2u() :
			TypeInfoPrivate_Pod(TypeData, "flame::Vec<2,uint>", sizeof(Vec2u))
		{
		}

		void serialize(void* str, const void* src) const override
		{
			auto s = to_string(*(Vec2u*)src);
			strcpy(f_stralloc(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(Vec2u*)dst = sto<Vec2u>(src);
		}
	};

	struct TypeInfoPrivate_Vec3u : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Vec3u() :
			TypeInfoPrivate_Pod(TypeData, "flame::Vec<3,uint>", sizeof(Vec3u))
		{
		}

		void serialize(void* str, const void* src) const override
		{
			auto s = to_string(*(Vec3u*)src);
			strcpy(f_stralloc(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(Vec3u*)dst = sto<Vec3u>(src);
		}
	};

	struct TypeInfoPrivate_Vec4u : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Vec4u() :
			TypeInfoPrivate_Pod(TypeData, "flame::Vec<4,uint>", sizeof(Vec4u))
		{
		}

		void serialize(void* str, const void* src) const override
		{
			auto s = to_string(*(Vec4u*)src);
			strcpy(f_stralloc(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(Vec4u*)dst = sto<Vec4u>(src);
		}
	};

	struct TypeInfoPrivate_Vec1f : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Vec1f() :
			TypeInfoPrivate_Pod(TypeData, "flame::Vec<1,float>", sizeof(Vec1f))
		{
		}

		void serialize(void* str, const void* src) const override
		{
			auto s = to_string(*(Vec1f*)src);
			strcpy(f_stralloc(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(Vec1f*)dst = sto<Vec1f>(src);
		}
	};

	struct TypeInfoPrivate_Vec2f : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Vec2f() :
			TypeInfoPrivate_Pod(TypeData, "flame::Vec<2,float>", sizeof(Vec2f))
		{
		}

		void serialize(void* str, const void* src) const override
		{
			auto s = to_string(*(Vec2f*)src);
			strcpy(f_stralloc(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(Vec2f*)dst = sto<Vec2f>(src);
		}
	};

	struct TypeInfoPrivate_Vec3f : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Vec3f() :
			TypeInfoPrivate_Pod(TypeData, "flame::Vec<3,float>", sizeof(Vec3f))
		{
		}

		void serialize(void* str, const void* src) const override
		{
			auto s = to_string(*(Vec3f*)src);
			strcpy(f_stralloc(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(Vec3f*)dst = sto<Vec3f>(src);
		}
	};

	struct TypeInfoPrivate_Vec4f : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Vec4f() :
			TypeInfoPrivate_Pod(TypeData, "flame::Vec<4,float>", sizeof(Vec4f))
		{
		}

		void serialize(void* str, const void* src) const override
		{
			auto s = to_string(*(Vec4f*)src);
			strcpy(f_stralloc(str, s.size()), s.data());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(Vec4f*)dst = sto<Vec4f>(src);
		}
	};

	struct TypeInfoPrivate_StringA : TypeInfoPrivate
	{
		TypeInfoPrivate_StringA() :
			TypeInfoPrivate(TypeData, "flame::StringA", sizeof(StringA))
		{
		}

		void* create(void* p) const override
		{
			if (p)
			{
				new (p) StringA;
				return p;
			}
			return f_new<StringA>();
		}
		void destroy(void* p, bool free_memory) const override
		{
			if (free_memory)
				f_delete((StringA*)p);
			else
				((StringA*)p)->~String();
		}
		void copy(void* dst, const void* src) const override
		{
			*(StringA*)dst = *(StringA*)src;
		}
		void serialize(void* str, const void* src) const override
		{
			const auto& s = *(StringA*)src;
			auto dst = f_stralloc(str, s.s);
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

		void* create(void* p) const override
		{
			if (p)
			{
				new (p) StringW;
				return p;
			}
			return f_new<StringW>();
		}
		void destroy(void* p, bool free_memory) const override
		{
			if (free_memory)
				f_delete((StringW*)p);
			else
				((StringW*)p)->~String();
		}
		void copy(void* dst, const void* src) const override
		{
			*(StringW*)dst = *(StringW*)src;
		}
		void serialize(void* str, const void* src) const override
		{
			const auto s = w2s((*(StringW*)src).str());
			strcpy(f_stralloc(str, s.size()), s.c_str());
		}
		void unserialize(void* dst, const char* src) const override
		{
			*(StringW*)dst = s2w(src);
		}
	};

	struct TypeInfoPrivate_Pointer : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate* base;

		TypeInfoPrivate_Pointer(const std::string& base_name) :
			TypeInfoPrivate_Pod(TypePointer, base_name, sizeof(void*))
		{
			base = TypeInfoPrivate::get(TypeData, name);
		}

		void* create(void* p) const override 
		{ 
			if (!p)
				p = new void*;
			*(void**)p = base->create();
			return p; 
		}
		void destroy(void* p, bool free_memory) const override 
		{
			if (p)
				base->destroy(*(void**)p);
			if (free_memory)
				f_free(p); 
		}
		void serialize(void* str, const void* src) const override
		{
			base->serialize(str, *(void**)src);
		}
		void unserialize(void* dst, const char* src) const override
		{
			base->unserialize(*(void**)dst, src);
		}
	};

	struct TypeInfoPrivate_charp : TypeInfoPrivate_Pointer
	{
		TypeInfoPrivate_charp() :
			TypeInfoPrivate_Pointer("char")
		{
		}

		void* create(void* p) const override
		{
			if (!p)
				p = new char*;
			*(char**)p = new char[1];
			(*(char**)p)[0] = 0;
			return p;
		}
		void destroy(void* p, bool free_memory) const override
		{
			if (p)
				f_free(*(char**)p);
			if (free_memory)
				f_free(p);
		}
		void copy(void* dst, const void* src) const override
		{
			strcpy((char*)dst, (char*)src);
		}
		void serialize(void* str, const void* src) const override
		{
			strcpy(f_stralloc(str, strlen((char*)src)), (char*)src);
		}
		void unserialize(void* dst, const char* src) const override
		{
			auto& p = *(char**)dst;
			f_free(p);
			p = new char[strlen(src) + 1];
			strcpy(p, src);
		}
	};

	struct TypeInfoPrivate_wcharp : TypeInfoPrivate_Pointer
	{
		TypeInfoPrivate_wcharp() :
			TypeInfoPrivate_Pointer("wchar_t")
		{
		}

		void* create(void* p) const override
		{
			if (!p)
				p = new wchar_t*;
			*(wchar_t**)p = new wchar_t[1];
			(*(wchar_t**)p)[0] = 0;
			return p;
		}
		void destroy(void* p, bool free_memory) const override
		{
			if (p)
				f_free(*(wchar_t**)p);
			if (free_memory)
				f_free(p);
		}
		void copy(void* dst, const void* src) const override
		{
			wcscpy((wchar_t*)dst, (wchar_t*)src);
		}
		void serialize(void* str, const void* src) const override 
		{
			const auto s = w2s((wchar_t*)src);
			strcpy(f_stralloc(str, s.size()), s.c_str());
		}
		void unserialize(void* dst, const char* src) const override
		{
			auto str = s2w(src);
			auto& p = *(wchar_t**)dst;
			f_free(p);
			p = new wchar_t[str.size() + 1];
			wcscpy(p, str.c_str());
		}
	};

	static std::vector<TypeInfoPrivate*> basic_types;

	struct _Initializer
	{
		_Initializer()
		{
			{
				auto t = new TypeInfoPrivate_void;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
				basic_types.push_back(t);
			}
			{
				auto t = new TypeInfoPrivate_bool;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
				basic_types.push_back(t);
			}
			{
				auto t = new TypeInfoPrivate_char;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
				basic_types.push_back(t);
			}
			{
				auto t = new TypeInfoPrivate_uchar;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
				basic_types.push_back(t);
			}
			{
				auto t = new TypeInfoPrivate_wchar;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
				basic_types.push_back(t);
			}
			{
				auto t = new TypeInfoPrivate_int;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
				basic_types.push_back(t);
			}
			{
				auto t = new TypeInfoPrivate_uint;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
				basic_types.push_back(t);
			}
			{
				auto t = new TypeInfoPrivate_int64;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
				basic_types.push_back(t);
			}
			{
				auto t = new TypeInfoPrivate_uint64;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
				basic_types.push_back(t);
			}
			{
				auto t = new TypeInfoPrivate_float;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
				basic_types.push_back(t);
			}
			{
				auto t = new TypeInfoPrivate_Vec1c;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
				basic_types.push_back(t);
			}
			{
				auto t = new TypeInfoPrivate_Vec2c;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
				basic_types.push_back(t);
			}
			{
				auto t = new TypeInfoPrivate_Vec3c;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
				basic_types.push_back(t);
			}
			{
				auto t = new TypeInfoPrivate_Vec4c;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
				basic_types.push_back(t);
			}
			{
				auto t = new TypeInfoPrivate_Vec1i;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
				basic_types.push_back(t);
			}
			{
				auto t = new TypeInfoPrivate_Vec2i;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
				basic_types.push_back(t);
			}
			{
				auto t = new TypeInfoPrivate_Vec3i;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
				basic_types.push_back(t);
			}
			{
				auto t = new TypeInfoPrivate_Vec4i;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
				basic_types.push_back(t);
			}
			{
				auto t = new TypeInfoPrivate_Vec1u;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
				basic_types.push_back(t);
			}
			{
				auto t = new TypeInfoPrivate_Vec2u;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
				basic_types.push_back(t);
			}
			{
				auto t = new TypeInfoPrivate_Vec3u;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
				basic_types.push_back(t);
			}
			{
				auto t = new TypeInfoPrivate_Vec4u;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
				basic_types.push_back(t);
			}
			{
				auto t = new TypeInfoPrivate_Vec1f;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
				basic_types.push_back(t);
			}
			{
				auto t = new TypeInfoPrivate_Vec2f;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
				basic_types.push_back(t);
			}
			{
				auto t = new TypeInfoPrivate_Vec3f;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
				basic_types.push_back(t);
			}
			{
				auto t = new TypeInfoPrivate_Vec4f;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
				basic_types.push_back(t);
			}
			{
				auto t = new TypeInfoPrivate_StringA;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
				basic_types.push_back(t);
			}
			{
				auto t = new TypeInfoPrivate_StringW;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
				basic_types.push_back(t);
			}
			{
				auto t = new TypeInfoPrivate_charp;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
				basic_types.push_back(t);
			}
			{
				auto t = new TypeInfoPrivate_wcharp;
				typeinfos.emplace(TypeInfoKey(t->tag, t->name), t);
				basic_types.push_back(t);
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
	}

	TypeInfo* TypeInfo::get(TypeTag tag, const char* name)
	{
		return TypeInfoPrivate::get(tag, name);
	}

	TypeInfoPrivate* TypeInfoPrivate::get(TypeTag tag, const std::string& name)
	{
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

	void ReflectMetaPrivate::get_token(void* str, uint idx) const
	{
		const auto& s = tokens[idx];
		strcpy(f_stralloc(str, s.size()), s.data());
	}

	bool ReflectMetaPrivate::has_token(const std::string& str) const
	{
		for (auto& t : tokens)
		{
			if (t == str)
				return true;
		}
		return false;
	}

	VariableInfoPrivate::VariableInfoPrivate(UdtInfoPrivate* udt, uint index, TypeInfoPrivate* type, const std::string& name, uint flags, const std::string& _meta) :
		udt(udt),
		index(index),
		type(type),
		name(name),
		offset(offset),
		default_value(nullptr)
	{
		meta.tokens = SUS::split(_meta);
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

	bool FunctionInfoPrivate::check(void* _type, ...) const
	{
		if (type != _type)
			return false;
		auto ap = (char*)var_end(&_type);
		auto c = 0;
		while (true)
		{
			auto t = va_arg(ap, TypeInfoPrivate*);
			if (!t)
				break;
			auto p = parameters[c];
			if (t != p)
			{
				c = -1;
				break;
			}
			c++;
		}
		return c == parameters.size();
	}

	extern "C" void __call(void* f, void* ret, void* list1, void* list2);

	void FunctionInfoPrivate::call(void* obj, void* ret, void** parms) const
	{
		if (parameters.size() > 4 || type->size > sizeof(void*))
		{
			assert(0);
			return;
		}

		auto idx = 0;
		std::vector<void*> list1(4);
		std::vector<float> list2(4);
		if (obj)
			list1[idx++] = obj;

		auto _p = parms;
		for (auto p : parameters)
		{
			switch (p->tag)
			{
			case TypeEnumSingle:
			case TypeEnumMulti:
				list1[idx++] = (void*)*((int*)*_p++);
				break;
			case TypeData:
				if (p->name == "float")
					list2[idx++] = *((float*)*_p++);
				else if (p->size == 1)
					list1[idx++] = (void*)*((char*)*_p++);
				else if (p->size == 4)
					list1[idx++] = (void*)*((int*)*_p++);
				else if (p->size == 8)
					list1[idx++] = *((void**)*_p++);
				break;
			case TypePointer:
				list1[idx] = *_p++;
				break;
			}
		}

		void* r = nullptr;
		__call(rva ? library->address + rva : *(void**)((*(char**)obj) + voff), &r, list1.data(), list2.data());
		if (ret)
			memcpy(ret, &r, type->size);
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
				assert(0);
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

	EnumInfo* find_enum(const char* name) { return find_enum(std::string(name)); }
	UdtInfo* find_udt(const char* name) { return find_udt(std::string(name)); }
}
