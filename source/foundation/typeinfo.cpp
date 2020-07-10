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

		void construct(void* p) const override {}
		void destruct(void* p) const override {}
		void copy(const void* src, void* dst) const override { memcpy(dst, src, size); }
		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override {}
		void unserialize(const char* src, void* dst) const override {}
	};

	struct TypeInfoPrivate_EnumSingle : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_EnumSingle(const std::string& base_name) :
			TypeInfoPrivate_Pod(TypeEnumSingle, base_name, sizeof(int))
		{
		}

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			const auto& str = find_enum(name)->find_item(*(int*)src)->name;
			auto buf = callback((Capture&)capture, str.size());
			strncpy(buf, str.data(), str.size());
			buf[str.size()] = 0;
		}
		void unserialize(const char* src, void* dst) const override
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

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto e = find_enum(name);
			std::string str;
			auto v = *(int*)src;
			for (auto i = 0; i < e->items.size(); i++)
			{
				if ((v & 1) == 1)
				{
					if (i > 0)
						str += ";";
					str += e->find_item(1 << i)->name;
				}
				v >>= 1;
			}
			auto buf = callback((Capture&)capture, str.size());
			strncpy(buf, str.data(), str.size());
			buf[str.size()] = 0;
		}
		void unserialize(const char* src, void* dst) const override
		{
			auto e = find_enum(name);
			auto v = 0;
			auto sp = SUS::split(src, ';');
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

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(bool*)src);
			auto buf = callback((Capture&)capture, str.size());
			strncpy(buf, str.data(), str.size());
			buf[str.size()] = 0;
		}
		void unserialize(const char* src, void* dst) const override
		{
			*(bool*)dst = std::stoi(src) != 0;
		}
	};

	struct TypeInfoPrivate_uchar : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_uchar() :
			TypeInfoPrivate_Pod(TypeData, "uchar", sizeof(uchar))
		{
		}

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(uchar*)src);
			auto buf = callback((Capture&)capture, str.size());
			strncpy(buf, str.data(), str.size());
			buf[str.size()] = 0;
		}
		void unserialize(const char* src, void* dst) const override
		{
			*(uchar*)dst = sto<uchar>(src);
		}
	};

	struct TypeInfoPrivate_int : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_int() :
			TypeInfoPrivate_Pod(TypeData, "int", sizeof(int))
		{
		}

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(int*)src);
			auto buf = callback((Capture&)capture, str.size());
			strncpy(buf, str.data(), str.size());
			buf[str.size()] = 0;
		}
		void unserialize(const char* src, void* dst) const override
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

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(uint*)src);
			auto buf = callback((Capture&)capture, str.size());
			strncpy(buf, str.data(), str.size());
			buf[str.size()] = 0;
		}
		void unserialize(const char* src, void* dst) const override
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

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(int64*)src);
			auto buf = callback((Capture&)capture, str.size());
			strncpy(buf, str.data(), str.size());
			buf[str.size()] = 0;
		}
		void unserialize(const char* src, void* dst) const override
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

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(uint64*)src);
			auto buf = callback((Capture&)capture, str.size());
			strncpy(buf, str.data(), str.size());
			buf[str.size()] = 0;
		}
		void unserialize(const char* src, void* dst) const override
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

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(float*)src);
			auto buf = callback((Capture&)capture, str.size());
			strncpy(buf, str.data(), str.size());
			buf[str.size()] = 0;
		}
		void unserialize(const char* src, void* dst) const override
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

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(Vec1c*)src);
			auto buf = callback((Capture&)capture, str.size());
			strncpy(buf, str.data(), str.size());
			buf[str.size()] = 0;
		}
		void unserialize(const char* src, void* dst) const override
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

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(Vec2c*)src);
			auto buf = callback((Capture&)capture, str.size());
			strncpy(buf, str.data(), str.size());
			buf[str.size()] = 0;
		}
		void unserialize(const char* src, void* dst) const override
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

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(Vec3c*)src);
			auto buf = callback((Capture&)capture, str.size());
			strncpy(buf, str.data(), str.size());
			buf[str.size()] = 0;
		}
		void unserialize(const char* src, void* dst) const override
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

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(Vec4c*)src);
			auto buf = callback((Capture&)capture, str.size());
			strncpy(buf, str.data(), str.size());
			buf[str.size()] = 0;
		}
		void unserialize(const char* src, void* dst) const override
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

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(Vec1i*)src);
			auto buf = callback((Capture&)capture, str.size());
			strncpy(buf, str.data(), str.size());
			buf[str.size()] = 0;
		}
		void unserialize(const char* src, void* dst) const override
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

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(Vec2i*)src);
			auto buf = callback((Capture&)capture, str.size());
			strncpy(buf, str.data(), str.size());
			buf[str.size()] = 0;
		}
		void unserialize(const char* src, void* dst) const override
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

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(Vec3i*)src);
			auto buf = callback((Capture&)capture, str.size());
			strncpy(buf, str.data(), str.size());
			buf[str.size()] = 0;
		}
		void unserialize(const char* src, void* dst) const override
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

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(Vec4i*)src);
			auto buf = callback((Capture&)capture, str.size());
			strncpy(buf, str.data(), str.size());
			buf[str.size()] = 0;
		}
		void unserialize(const char* src, void* dst) const override
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

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(Vec1u*)src);
			auto buf = callback((Capture&)capture, str.size());
			strncpy(buf, str.data(), str.size());
			buf[str.size()] = 0;
		}
		void unserialize(const char* src, void* dst) const override
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

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(Vec2u*)src);
			auto buf = callback((Capture&)capture, str.size());
			strncpy(buf, str.data(), str.size());
			buf[str.size()] = 0;
		}
		void unserialize(const char* src, void* dst) const override
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

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(Vec3u*)src);
			auto buf = callback((Capture&)capture, str.size());
			strncpy(buf, str.data(), str.size());
			buf[str.size()] = 0;
		}
		void unserialize(const char* src, void* dst) const override
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

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(Vec4u*)src);
			auto buf = callback((Capture&)capture, str.size());
			strncpy(buf, str.data(), str.size());
			buf[str.size()] = 0;
		}
		void unserialize(const char* src, void* dst) const override
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

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(Vec1f*)src);
			auto buf = callback((Capture&)capture, str.size());
			strncpy(buf, str.data(), str.size());
			buf[str.size()] = 0;
		}
		void unserialize(const char* src, void* dst) const override
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

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(Vec2f*)src);
			auto buf = callback((Capture&)capture, str.size());
			strncpy(buf, str.data(), str.size());
			buf[str.size()] = 0;
		}
		void unserialize(const char* src, void* dst) const override
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

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(Vec3f*)src);
			auto buf = callback((Capture&)capture, str.size());
			strncpy(buf, str.data(), str.size());
			buf[str.size()] = 0;
		}
		void unserialize(const char* src, void* dst) const override
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

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(Vec4f*)src);
			auto buf = callback((Capture&)capture, str.size());
			strncpy(buf, str.data(), str.size());
			buf[str.size()] = 0;
		}
		void unserialize(const char* src, void* dst) const override
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

		void construct(void* p) const override
		{
		}
		void destruct(void* p) const override
		{
			((StringA*)p)->~String();
		}
		void copy(const void* src, void* dst) const override
		{
			*(StringA*)dst = *(StringA*)src;
		}
		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			const auto& str = *(StringA*)src;
			auto buf = callback((Capture&)capture, str.s);
			strncpy(buf, str.v, str.s);
			buf[str.s] = 0;
		}
		void unserialize(const char* src, void* dst) const override
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

		void construct(void* p) const override
		{
		}
		void destruct(void* p) const override
		{
			((StringW*)p)->~String();
		}
		void copy(const void* src, void* dst) const override
		{
			*(StringW*)dst = *(StringW*)src;
		}
		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = w2s(((StringW*)src)->str());
			auto buf = callback((Capture&)capture, str.size());
			strncpy(buf, str.data(), str.size());
			buf[str.size()] = 0;
		}
		void unserialize(const char* src, void* dst) const override
		{
			*(StringW*)dst = s2w(src);
		}
	};

	struct TypeInfoPrivate_Pointer : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Pointer(const std::string& base_name) :
			TypeInfoPrivate_Pod(TypePointer, base_name, sizeof(void*))
		{
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
				auto t = new TypeInfoPrivate_uchar;
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
		assert(t);
		typeinfos.emplace(key, t);
		return t;
	}

	VariableInfoPrivate::VariableInfoPrivate(UdtInfoPrivate* udt, uint index, TypeInfoPrivate* type, const std::string& name, uint flags, uint offset) :
		udt(udt),
		index(index),
		type(type),
		name(name),
		flags(flags),
		offset(offset),
		default_value(nullptr)
	{
	}

	VariableInfoPrivate::~VariableInfoPrivate()
	{
		delete[]default_value;
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

	EnumItem* EnumInfoBridge::find_item(const char* name) const
	{
		return ((EnumInfoPrivate*)this)->find_item(name);
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

	EnumItem* EnumInfoBridge::find_item(int value) const
	{
		return ((EnumInfoPrivate*)this)->find_item(value);
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

	void FunctionInfoPrivate::call(void* obj, void* ret, ...) const
	{
		auto ap = (char*)var_end(&ret);
		if (type->tag == TypeData && type->name == "void")
		{
			switch (parameters.size())
			{
			case 1:
			{
				auto p1 = parameters[0];
				switch (p1->tag)
				{
				case TypeData:
					if (p1->name == "float")
					{
						auto t = va_arg(ap, float*);
						void* pf = nullptr;
						if (rva)
							;
						else
							pf = *(void**)((*(char**)obj) + voff);
						cmf(p2f<void(__Dummy__::*)(float)>(pf), obj, *t);
						return;
					}
					break;
				case TypePointer:
					if (p1->name == "char")
					{
						auto t = va_arg(ap, char*);
						void* pf = nullptr;
						if (rva)
							;
						else
							pf = *(void**)((*(char**)obj) + voff);
						cmf(p2f<void(__Dummy__::*)(char*)>(pf), obj, t);
						return;
					}
					else if (p1->name == "wchar_t")
					{
						auto t = va_arg(ap, wchar_t*);
						void* pf = nullptr;
						if (rva)
							;
						else
							pf = *(void**)((*(char**)obj) + voff);
						cmf(p2f<void(__Dummy__::*)(wchar_t*)>(pf), obj, t);
						return;
					}
					else if (p1->name == "flame::Vec<3,uchar>")
					{
						auto t = va_arg(ap, Vec3c*);
						void* pf = nullptr;
						if (rva)
							;
						else
							pf = *(void**)((*(char**)obj) + voff);
						cmf(p2f<void(__Dummy__::*)(Vec3c*)>(pf), obj, t);
						return;
					}
					break;
				}

			}
				break;
			}
		}
		else if (type->tag == TypePointer)
		{
			switch (parameters.size())
			{
			case 0:
				if (!obj)
				{
					*(void**)ret = cf(p2f<void* (*)()>(library->address + rva));
					return;
				}
				break;
			}
		}

		assert(0);
	}

	UdtInfoPrivate::UdtInfoPrivate(LibraryPrivate* library, const std::string& name, uint size, const std::string& base_name) :
		library(library),
		name(name),
		size(size),
		base_name(base_name)
	{
	}

	VariableInfo* UdtInfoBridge::find_variable(const char* name) const
	{ 
		return ((UdtInfoPrivate*)this)->find_variable(name); 
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

	FunctionInfo* UdtInfoBridge::find_function(const char* name) const
	{ 
		return ((UdtInfoPrivate*)this)->find_function(name);
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
					auto v = new VariableInfoPrivate(u, u->variables.size(), type, n_variable.attribute("name").value(),
						n_variable.attribute("flags").as_uint(), n_variable.attribute("offset").as_uint());
					u->variables.emplace_back(v);
					auto dv = n_variable.attribute("default_value");
					if (dv)
					{
						v->default_value = new char[type->size];
						type->unserialize(n_variable.attribute("default_value").value(), v->default_value);
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

			auto code_path = library_path;
			code_path.replace_extension(L".code");
			std::ifstream code_file(code_path);
			if (code_file.good())
			{
				FunctionInfoPrivate* curr_func = nullptr;

				while (!code_file.eof())
				{
					std::string line;
					std::getline(code_file, line);
					if (line.empty())
						continue;

					if (line.size() > 2 && line[0] == '#' && line[1] == '#')
					{
						auto name = line.substr(2);
						for (auto& u : udts)
						{
							for (auto& f : u.second->functions)
							{
								if (name == u.second->name + "::" + f->name)
									curr_func = f.get();
							}
						}
					}
					else
						curr_func->code += line + "\n";
				}
				code_file.close();
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
