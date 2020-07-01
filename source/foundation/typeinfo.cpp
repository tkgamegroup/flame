#include <flame/serialize.h>
#include "typeinfo_private.h"

#include <Windows.h>

namespace flame
{
	std::unordered_map<uint, std::unique_ptr<TypeInfoPrivate>> typeinfos;
	std::vector<TypeInfoDatabasePrivate*> global_typeinfo_databases;

	struct TypeInfoPrivate_Pod : TypeInfoPrivate
	{
		TypeInfoPrivate_Pod(TypeTag tag, const std::string& base_name, uint size) :
			TypeInfoPrivate(tag, base_name, size)
		{
		}

		void construct(void* p) const override {}
		void destruct(void* p) const override {}
		void copy(const void* src, void* dst) const override { memcpy(dst, src, _size); }
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
			const auto& str = _find_enum(_name_hash)->_find_item(*(int*)src)->_name;
			auto buf = callback((Capture&)capture, str.size());
			std::char_traits<char>::copy(buf, str.data(), str.size());
		}
		void unserialize(const char* src, void* dst) const override
		{
			*(int*)dst = _find_enum(_name_hash)->_find_item(src)->_value;
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
			auto e = _find_enum(_name_hash);
			std::string str;
			auto v = *(int*)src;
			for (auto i = 0; i < e->_items.size(); i++)
			{
				if ((v & 1) == 1)
				{
					if (i > 0)
						str += ";";
					str += e->_find_item(1 << i)->_name;
				}
				v >>= 1;
			}
			auto buf = callback((Capture&)capture, str.size());
			std::char_traits<char>::copy(buf, str.data(), str.size());
		}
		void unserialize(const char* src, void* dst) const override
		{
			auto e = _find_enum(_name_hash);
			auto v = 0;
			auto sp = SUS::split(src, ';');
			for (auto& t : sp)
				v |= e->_find_item(t)->_value;
			*(int*)dst = v;
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
			std::char_traits<char>::copy(buf, str.data(), str.size());
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
			std::char_traits<char>::copy(buf, str.data(), str.size());
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
			std::char_traits<char>::copy(buf, str.data(), str.size());
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
			std::char_traits<char>::copy(buf, str.data(), str.size());
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
			std::char_traits<char>::copy(buf, str.data(), str.size());
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
			std::char_traits<char>::copy(buf, str.data(), str.size());
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
			std::char_traits<char>::copy(buf, str.data(), str.size());
		}
		void unserialize(const char* src, void* dst) const override
		{
			*(float*)dst = sto<float>(src);
		}
	};

	struct TypeInfoPrivate_Vec1c : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Vec1c() :
			TypeInfoPrivate_Pod(TypeData, "Vec<1,uchar>", sizeof(Vec1c))
		{
		}

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(Vec1c*)src);
			auto buf = callback((Capture&)capture, str.size());
			std::char_traits<char>::copy(buf, str.data(), str.size());
		}
		void unserialize(const char* src, void* dst) const override
		{
			*(Vec1c*)dst = sto<Vec1c>(src);
		}
	};

	struct TypeInfoPrivate_Vec2c : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Vec2c() :
			TypeInfoPrivate_Pod(TypeData, "Vec<2,uchar>", sizeof(Vec2c))
		{
		}

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(Vec2c*)src);
			auto buf = callback((Capture&)capture, str.size());
			std::char_traits<char>::copy(buf, str.data(), str.size());
		}
		void unserialize(const char* src, void* dst) const override
		{
			*(Vec2c*)dst = sto<Vec2c>(src);
		}
	};

	struct TypeInfoPrivate_Vec3c : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Vec3c() :
			TypeInfoPrivate_Pod(TypeData, "Vec<3,uchar>", sizeof(Vec3c))
		{
		}

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(Vec3c*)src);
			auto buf = callback((Capture&)capture, str.size());
			std::char_traits<char>::copy(buf, str.data(), str.size());
		}
		void unserialize(const char* src, void* dst) const override
		{
			*(Vec3c*)dst = sto<Vec3c>(src);
		}
	};

	struct TypeInfoPrivate_Vec4c : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Vec4c() :
			TypeInfoPrivate_Pod(TypeData, "Vec<4,uchar>", sizeof(Vec4c))
		{
		}

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(Vec4c*)src);
			auto buf = callback((Capture&)capture, str.size());
			std::char_traits<char>::copy(buf, str.data(), str.size());
		}
		void unserialize(const char* src, void* dst) const override
		{
			*(Vec4c*)dst = sto<Vec4c>(src);
		}
	};

	struct TypeInfoPrivate_Vec1i : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Vec1i() :
			TypeInfoPrivate_Pod(TypeData, "Vec<1,int>", sizeof(Vec1i))
		{
		}

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(Vec1i*)src);
			auto buf = callback((Capture&)capture, str.size());
			std::char_traits<char>::copy(buf, str.data(), str.size());
		}
		void unserialize(const char* src, void* dst) const override
		{
			*(Vec1i*)dst = sto<Vec1i>(src);
		}
	};

	struct TypeInfoPrivate_Vec2i : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Vec2i() :
			TypeInfoPrivate_Pod(TypeData, "Vec<2,int>", sizeof(Vec2i))
		{
		}

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(Vec2i*)src);
			auto buf = callback((Capture&)capture, str.size());
			std::char_traits<char>::copy(buf, str.data(), str.size());
		}
		void unserialize(const char* src, void* dst) const override
		{
			*(Vec2i*)dst = sto<Vec2i>(src);
		}
	};

	struct TypeInfoPrivate_Vec3i : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Vec3i() :
			TypeInfoPrivate_Pod(TypeData, "Vec<3,int>", sizeof(Vec3i))
		{
		}

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(Vec3i*)src);
			auto buf = callback((Capture&)capture, str.size());
			std::char_traits<char>::copy(buf, str.data(), str.size());
		}
		void unserialize(const char* src, void* dst) const override
		{
			*(Vec3i*)dst = sto<Vec3i>(src);
		}
	};

	struct TypeInfoPrivate_Vec4i : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Vec4i() :
			TypeInfoPrivate_Pod(TypeData, "Vec<4,int>", sizeof(Vec4i))
		{
		}

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(Vec4i*)src);
			auto buf = callback((Capture&)capture, str.size());
			std::char_traits<char>::copy(buf, str.data(), str.size());
		}
		void unserialize(const char* src, void* dst) const override
		{
			*(Vec4i*)dst = sto<Vec4i>(src);
		}
	};

	struct TypeInfoPrivate_Vec1u : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Vec1u() :
			TypeInfoPrivate_Pod(TypeData, "Vec<1,uint>", sizeof(Vec1u))
		{
		}

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(Vec1u*)src);
			auto buf = callback((Capture&)capture, str.size());
			std::char_traits<char>::copy(buf, str.data(), str.size());
		}
		void unserialize(const char* src, void* dst) const override
		{
			*(Vec1u*)dst = sto<Vec1u>(src);
		}
	};

	struct TypeInfoPrivate_Vec2u : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Vec2u() :
			TypeInfoPrivate_Pod(TypeData, "Vec<2,uint>", sizeof(Vec2u))
		{
		}

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(Vec2u*)src);
			auto buf = callback((Capture&)capture, str.size());
			std::char_traits<char>::copy(buf, str.data(), str.size());
		}
		void unserialize(const char* src, void* dst) const override
		{
			*(Vec2u*)dst = sto<Vec2u>(src);
		}
	};

	struct TypeInfoPrivate_Vec3u : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Vec3u() :
			TypeInfoPrivate_Pod(TypeData, "Vec<3,uint>", sizeof(Vec3u))
		{
		}

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(Vec3u*)src);
			auto buf = callback((Capture&)capture, str.size());
			std::char_traits<char>::copy(buf, str.data(), str.size());
		}
		void unserialize(const char* src, void* dst) const override
		{
			*(Vec3u*)dst = sto<Vec3u>(src);
		}
	};

	struct TypeInfoPrivate_Vec4u : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Vec4u() :
			TypeInfoPrivate_Pod(TypeData, "Vec<4,uint>", sizeof(Vec4u))
		{
		}

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(Vec4u*)src);
			auto buf = callback((Capture&)capture, str.size());
			std::char_traits<char>::copy(buf, str.data(), str.size());
		}
		void unserialize(const char* src, void* dst) const override
		{
			*(Vec4u*)dst = sto<Vec4u>(src);
		}
	};

	struct TypeInfoPrivate_Vec1f : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Vec1f() :
			TypeInfoPrivate_Pod(TypeData, "Vec<1,float>", sizeof(Vec1f))
		{
		}

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(Vec1f*)src);
			auto buf = callback((Capture&)capture, str.size());
			std::char_traits<char>::copy(buf, str.data(), str.size());
		}
		void unserialize(const char* src, void* dst) const override
		{
			*(Vec1f*)dst = sto<Vec1f>(src);
		}
	};

	struct TypeInfoPrivate_Vec2f : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Vec2f() :
			TypeInfoPrivate_Pod(TypeData, "Vec<2,float>", sizeof(Vec2f))
		{
		}

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(Vec2f*)src);
			auto buf = callback((Capture&)capture, str.size());
			std::char_traits<char>::copy(buf, str.data(), str.size());
		}
		void unserialize(const char* src, void* dst) const override
		{
			*(Vec2f*)dst = sto<Vec2f>(src);
		}
	};

	struct TypeInfoPrivate_Vec3f : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Vec3f() :
			TypeInfoPrivate_Pod(TypeData, "Vec<3,float>", sizeof(Vec3f))
		{
		}

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(Vec3f*)src);
			auto buf = callback((Capture&)capture, str.size());
			std::char_traits<char>::copy(buf, str.data(), str.size());
		}
		void unserialize(const char* src, void* dst) const override
		{
			*(Vec3f*)dst = sto<Vec3f>(src);
		}
	};

	struct TypeInfoPrivate_Vec4f : TypeInfoPrivate_Pod
	{
		TypeInfoPrivate_Vec4f() :
			TypeInfoPrivate_Pod(TypeData, "Vec<4,float>", sizeof(Vec4f))
		{
		}

		void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const override
		{
			auto str = to_string(*(Vec4f*)src);
			auto buf = callback((Capture&)capture, str.size());
			std::char_traits<char>::copy(buf, str.data(), str.size());
		}
		void unserialize(const char* src, void* dst) const override
		{
			*(Vec4f*)dst = sto<Vec4f>(src);
		}
	};

	struct TypeInfoPrivate_StringA : TypeInfoPrivate
	{
		TypeInfoPrivate_StringA() :
			TypeInfoPrivate(TypeData, "StringA", sizeof(StringA))
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
			std::char_traits<char>::copy(buf, str.v, str.s);
		}
		void unserialize(const char* src, void* dst) const override
		{
			*(StringA*)dst = src;
		}
	};

	struct TypeInfoPrivate_StringW : TypeInfoPrivate
	{
		TypeInfoPrivate_StringW() :
			TypeInfoPrivate(TypeData, "StringW", sizeof(StringW))
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
			std::char_traits<char>::copy(buf, str.data(), str.size());
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

	static std::unordered_map<uint, TypeInfoPrivate*> basic_types;

	struct _InitializeBasicTypes
	{
		_InitializeBasicTypes()
		{
			{
				auto t = new TypeInfoPrivate_bool;
				auto nh = FLAME_HASH(t->_name.c_str());
				basic_types.emplace(nh, t);
				typeinfos.emplace(FLAME_TYPE_HASH(t->_tag, nh), t);
			}
			{
				auto t = new TypeInfoPrivate_uchar;
				auto nh = FLAME_HASH(t->_name.c_str());
				basic_types.emplace(nh, t);
				typeinfos.emplace(FLAME_TYPE_HASH(t->_tag, nh), t);
			}
			{
				auto t = new TypeInfoPrivate_int;
				auto nh = FLAME_HASH(t->_name.c_str());
				basic_types.emplace(nh, t);
				typeinfos.emplace(FLAME_TYPE_HASH(t->_tag, nh), t);
			}
			{
				auto t = new TypeInfoPrivate_uint;
				auto nh = FLAME_HASH(t->_name.c_str());
				basic_types.emplace(nh, t);
				typeinfos.emplace(FLAME_TYPE_HASH(t->_tag, nh), t);
			}
			{
				auto t = new TypeInfoPrivate_int64;
				auto nh = FLAME_HASH(t->_name.c_str());
				basic_types.emplace(nh, t);
				typeinfos.emplace(FLAME_TYPE_HASH(t->_tag, nh), t);
			}
			{
				auto t = new TypeInfoPrivate_uint64;
				auto nh = FLAME_HASH(t->_name.c_str());
				basic_types.emplace(nh, t);
				typeinfos.emplace(FLAME_TYPE_HASH(t->_tag, nh), t);
			}
			{
				auto t = new TypeInfoPrivate_float;
				auto nh = FLAME_HASH(t->_name.c_str());
				basic_types.emplace(nh, t);
				typeinfos.emplace(FLAME_TYPE_HASH(t->_tag, nh), t);
			}
			{
				auto t = new TypeInfoPrivate_Vec1c;
				auto nh = FLAME_HASH(t->_name.c_str());
				basic_types.emplace(nh, t);
				typeinfos.emplace(FLAME_TYPE_HASH(t->_tag, nh), t);
			}
			{
				auto t = new TypeInfoPrivate_Vec2c;
				auto nh = FLAME_HASH(t->_name.c_str());
				basic_types.emplace(nh, t);
				typeinfos.emplace(FLAME_TYPE_HASH(t->_tag, nh), t);
			}
			{
				auto t = new TypeInfoPrivate_Vec3c;
				auto nh = FLAME_HASH(t->_name.c_str());
				basic_types.emplace(nh, t);
				typeinfos.emplace(FLAME_TYPE_HASH(t->_tag, nh), t);
			}
			{
				auto t = new TypeInfoPrivate_Vec4c;
				auto nh = FLAME_HASH(t->_name.c_str());
				basic_types.emplace(nh, t);
				typeinfos.emplace(FLAME_TYPE_HASH(t->_tag, nh), t);
			}
			{
				auto t = new TypeInfoPrivate_Vec1i;
				auto nh = FLAME_HASH(t->_name.c_str());
				basic_types.emplace(nh, t);
				typeinfos.emplace(FLAME_TYPE_HASH(t->_tag, nh), t);
			}
			{
				auto t = new TypeInfoPrivate_Vec2i;
				auto nh = FLAME_HASH(t->_name.c_str());
				basic_types.emplace(nh, t);
				typeinfos.emplace(FLAME_TYPE_HASH(t->_tag, nh), t);
			}
			{
				auto t = new TypeInfoPrivate_Vec3i;
				auto nh = FLAME_HASH(t->_name.c_str());
				basic_types.emplace(nh, t);
				typeinfos.emplace(FLAME_TYPE_HASH(t->_tag, nh), t);
			}
			{
				auto t = new TypeInfoPrivate_Vec4i;
				auto nh = FLAME_HASH(t->_name.c_str());
				basic_types.emplace(nh, t);
				typeinfos.emplace(FLAME_TYPE_HASH(t->_tag, nh), t);
			}
			{
				auto t = new TypeInfoPrivate_Vec1u;
				auto nh = FLAME_HASH(t->_name.c_str());
				basic_types.emplace(nh, t);
				typeinfos.emplace(FLAME_TYPE_HASH(t->_tag, nh), t);
			}
			{
				auto t = new TypeInfoPrivate_Vec2u;
				auto nh = FLAME_HASH(t->_name.c_str());
				basic_types.emplace(nh, t);
				typeinfos.emplace(FLAME_TYPE_HASH(t->_tag, nh), t);
			}
			{
				auto t = new TypeInfoPrivate_Vec3u;
				auto nh = FLAME_HASH(t->_name.c_str());
				basic_types.emplace(nh, t);
				typeinfos.emplace(FLAME_TYPE_HASH(t->_tag, nh), t);
			}
			{
				auto t = new TypeInfoPrivate_Vec4u;
				auto nh = FLAME_HASH(t->_name.c_str());
				basic_types.emplace(nh, t);
				typeinfos.emplace(FLAME_TYPE_HASH(t->_tag, nh), t);
			}
			{
				auto t = new TypeInfoPrivate_Vec1f;
				auto nh = FLAME_HASH(t->_name.c_str());
				basic_types.emplace(nh, t);
				typeinfos.emplace(FLAME_TYPE_HASH(t->_tag, nh), t);
			}
			{
				auto t = new TypeInfoPrivate_Vec2f;
				auto nh = FLAME_HASH(t->_name.c_str());
				basic_types.emplace(nh, t);
				typeinfos.emplace(FLAME_TYPE_HASH(t->_tag, nh), t);
			}
			{
				auto t = new TypeInfoPrivate_Vec3f;
				auto nh = FLAME_HASH(t->_name.c_str());
				basic_types.emplace(nh, t);
				typeinfos.emplace(FLAME_TYPE_HASH(t->_tag, nh), t);
			}
			{
				auto t = new TypeInfoPrivate_Vec4f;
				auto nh = FLAME_HASH(t->_name.c_str());
				basic_types.emplace(nh, t);
				typeinfos.emplace(FLAME_TYPE_HASH(t->_tag, nh), t);
			}
		}
	};
	static _InitializeBasicTypes _initialize_basic_types;

	TypeInfoPrivate::TypeInfoPrivate(TypeTag tag, const std::string& name, uint size) :
		_tag(tag),
		_name(name),
		_size(size)
	{
		_name_hash = FLAME_HASH(_name.c_str());
	}

	TypeInfoPrivate* TypeInfoPrivate::_get(TypeTag tag, const std::string& name)
	{
		auto hash = FLAME_TYPE_HASH(tag, FLAME_HASH(name.c_str()));
		auto it = typeinfos.find(hash);
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
		typeinfos.emplace(hash, t);
		return t;
	}

	TypeInfoPrivate* TypeInfoPrivate::_get_basic_type(uint name_hash)
	{
		auto it = basic_types.find(name_hash);
		if (it != basic_types.end())
			return it->second;
		return nullptr;
	}

	VariableInfoPrivate::VariableInfoPrivate(UdtInfoPrivate* udt, uint index, TypeInfoPrivate* type, const std::string& name, uint flags, uint offset) :
		_udt(udt),
		_index(index),
		_type(type),
		_name(name),
		_flags(flags),
		_offset(offset),
		_default_value(nullptr)
	{
		_name_hash = FLAME_HASH(name.c_str());
		if (auto tags = std::array{ TypeEnumSingle, TypeEnumMulti, TypeData }; 
			std::any_of(tags.begin(), tags.end(), type->_tag) && type->_name_hash != FLAME_CHASH("flame::StringA") && type->_name_hash != FLAME_CHASH("flame::StringW"))
		{
			if (type->_tag == TypeData)
			{
				auto size = type->_size;
				_default_value = new char[size];
				memset(_default_value, 0, size);
			}
		}
	}

	VariableInfoPrivate::~VariableInfoPrivate()
	{
		delete[]_default_value;
	}

	EnumItemPrivate::EnumItemPrivate(EnumInfoPrivate* ei, uint index, const std::string& name, int value) :
		_ei(ei),
		_index(index),
		_name(name),
		_value(value)
	{
	}

	EnumInfoPrivate::EnumInfoPrivate(TypeInfoDatabasePrivate* db, const std::string& name) :
		_db(db),
		_name(name)
	{
	}

	EnumItemPrivate* EnumInfoPrivate::_find_item(const std::string& name) const
	{
		for (auto& i : _items)
		{
			if (i->_name == name)
				return i.get();
		}
		return nullptr;
	}
	EnumItemPrivate* EnumInfoPrivate::_find_item(int value) const
	{
		for (auto& i : _items)
		{
			if (i->_value == value)
				return i.get();
		}
		return nullptr;
	}

	FunctionInfoPrivate::FunctionInfoPrivate(TypeInfoDatabasePrivate* db, UdtInfoPrivate* udt, uint index, const std::string& name, void* rva, TypeInfoPrivate* type) :
		_db(db),
		_udt(udt),
		_index(index),
		_name(name),
		_rva(rva),
		_type(type)
	{
	}

	bool FunctionInfoPrivate::_check_v(uint type_hash, char* ap) const
	{
		if (type_hash != FLAME_TYPE_HASH(_type->_tag, _type->_name_hash))
			return false;
		auto c = 0;
		while (true)
		{
			auto t = va_arg(ap, uint);
			if (!t)
				break;
			auto p = _parameters[c];
			if (t != FLAME_TYPE_HASH(p->_tag, p->_name_hash))
			{
				c = -1;
				break;
			}
			c++;
		}
		return c == _parameters.size();
	}

	UdtInfoPrivate::UdtInfoPrivate(TypeInfoDatabasePrivate* db, const std::string& name, uint size, const std::string& base_name) :
		_db(db),
		_name(name),
		_size(size),
		_base_name(base_name)
	{
	}

	VariableInfoPrivate* UdtInfoPrivate::_find_variable(const std::string& name) const
	{
		for (auto& v : _variables)
		{
			if (v->_name == name)
				return v.get();
		}
		return nullptr;
	}
	FunctionInfoPrivate* UdtInfoPrivate::_find_function(const std::string& name) const
	{
		for (auto& f : _functions)
		{
			if (f->_name == name)
				return f.get();
		}
		return nullptr;
	}

	TypeInfoDatabasePrivate::TypeInfoDatabasePrivate(const std::wstring& library_name) :
		_library(nullptr),
		_library_name(library_name)
	{
	}

	TypeInfoDatabasePrivate::~TypeInfoDatabasePrivate()
	{
		if (_library)
			free_library(_library);
	}

	TypeInfoDatabase* TypeInfoDatabase::load(const wchar_t* library_filename)
	{
		std::filesystem::path library_path(library_filename);
		if (!library_path.is_absolute())
			library_path = get_app_path().str() / library_path;
		auto typeinfo_path = library_path;
		typeinfo_path.replace_extension(L".typeinfo");
		if (!std::filesystem::exists(typeinfo_path) || std::filesystem::last_write_time(typeinfo_path) < std::filesystem::last_write_time(library_path))
		{
			auto typeinfogen_path = std::filesystem::path(get_app_path().str()) / L"typeinfogen.exe";
			if (!std::filesystem::exists(typeinfogen_path))
			{
				printf("typeinfo out of date: %s, and cannot find typeinfogen\n", typeinfo_path.string().c_str());
				assert(0);
				return nullptr;
			}
			exec_and_redirect_to_std_output(nullptr, (wchar_t*)(typeinfogen_path.wstring() + L" " + library_path.wstring()).c_str());
		}

		pugi::xml_document file;
		pugi::xml_node file_root;
		if (!file.load_file(typeinfo_path.c_str()) || (file_root = file.first_child()).name() != std::string("typeinfo"))
		{
			printf("cannot find typeinfo: %s\n", typeinfo_path.string().c_str());
			assert(0);
			return nullptr;
		}

		auto db = new TypeInfoDatabasePrivate(library_path);
		global_typeinfo_databases.push_back(db);

		for (auto n_enum : file_root.child("enums"))
		{
			auto e = new EnumInfoPrivate(db, n_enum.attribute("name").value());
			db->_enums.emplace(FLAME_HASH(e->_name.c_str()), e);

			for (auto n_item : n_enum.child("items"))
				e->_items.emplace_back(new EnumItemPrivate(e, e->_items.size(), n_item.attribute("name").value(), n_item.attribute("value").as_int()));
		}

		for (auto n_udt : file_root.child("udts"))
		{
			auto u = new UdtInfoPrivate(db, n_udt.attribute("name").value(), n_udt.attribute("size").as_uint(), n_udt.attribute("base_name").value());
			db->_udts.emplace(FLAME_HASH(u->_name.c_str()), u);

			for (auto n_variable : n_udt.child("variables"))
			{
				auto type = TypeInfoPrivate::_get((TypeTag)n_variable.attribute("type_tag").as_int(), n_variable.attribute("type_name").value());
				auto v = new VariableInfoPrivate(u, u->_variables.size(), type, n_variable.attribute("name").value(),
					n_variable.attribute("flags").as_uint(), n_variable.attribute("offset").as_uint());
				u->_variables.emplace_back(v);

				if (v->_default_value)
					type->unserialize(n_variable.attribute("default_value").value(), v->_default_value);
			}

			for (auto n_function : n_udt.child("functions"))
			{
				auto f = new FunctionInfoPrivate(db, u, u->_functions.size(), n_function.attribute("name").value(), (void*)n_function.attribute("rva").as_uint(), TypeInfoPrivate::_get((TypeTag)n_function.attribute("type_tag").as_int(), n_function.attribute("type_name").value()));
				u->_functions.emplace_back(f);
				for (auto n_parameter : n_function.child("parameters"))
					f->_parameters.push_back(TypeInfoPrivate::_get((TypeTag)n_parameter.attribute("type_tag").as_int(), n_parameter.attribute("type_name").value()));
			}
		}

		db->_library = load_library(db->_library_name.c_str());
		global_typeinfo_databases.erase(global_typeinfo_databases.begin() + global_typeinfo_databases.size() - 1);

		auto typeinfo_code_path = library_path;
		typeinfo_code_path.replace_extension(L".typeinfo.code");
		std::ifstream typeinfo_code(typeinfo_code_path);
		if (typeinfo_code.good())
		{
			struct FunctionCode
			{
				std::string name;
				std::string code;
			};
			std::vector<FunctionCode> function_codes;

			while (!typeinfo_code.eof())
			{
				std::string line;
				std::getline(typeinfo_code, line);
				if (line.empty())
					continue;

				if (line.size() > 2 && line[0] == '#' && line[1] == '#')
				{
					FunctionCode fc;
					fc.name = line.substr(2);
					function_codes.push_back(fc);
				}
				else
					function_codes.back().code += line + "\n";
			}
			typeinfo_code.close();

			for (auto& u : db->_udts)
			{
				for (auto& f : u.second->_functions)
				{
					if (f->_name == "bp_update")
					{
						auto n = u.second->_name + "::" + f->_name;
						for (auto& fc : function_codes)
						{
							if (fc.name == n)
							{
								f->_code = fc.code;
								break;
							}
						}
					}
				}
			}
		}

		return db;
	}

	EnumInfoPrivate* _find_enum(uint hash)
	{
		for (auto db : global_typeinfo_databases)
		{
			auto it = db->_enums.find(hash);
			if (it != db->_enums.end())
				return it->second.get();
		}
		return nullptr;
	}
	UdtInfoPrivate* _find_udt(uint hash)
	{
		for (auto db : global_typeinfo_databases)
		{
			auto it = db->_udts.find(hash);
			if (it != db->_udts.end())
				return it->second.get();
		}
		return nullptr;
	}

	EnumInfo* find_enum(uint hash) { return _find_enum(hash); }
	UdtInfo* find_udt(uint hash) { return _find_udt(hash); }
}
