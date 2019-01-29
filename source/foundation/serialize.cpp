// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <flame/foundation/serialize.h>

#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>
#include <rapidxml_print.hpp>

#include <Windows.h>
#include <dia2.h>
#include <atlbase.h>

namespace flame
{
	struct EnumItemPrivate : EnumItem
	{
		std::string name;
		int value;
	};

	const char *EnumItem::name() const
	{
		return ((EnumItemPrivate*)this)->name.c_str();
	}

	int EnumItem::value() const
	{
		return ((EnumItemPrivate*)this)->value;
	}

	struct EnumInfoPrivate : EnumInfo
	{
		std::string name;
		std::vector<std::unique_ptr<EnumItemPrivate>> items;

		inline std::string serialize_value(bool single, int v) const
		{
			if (single)
				return item(v)->name();

			return "";
		}
	};

	const char *EnumInfo::name() const
	{
		return ((EnumInfoPrivate*)this)->name.c_str();
	}

	int EnumInfo::item_count() const
	{
		return ((EnumInfoPrivate*)this)->items.size();
	}

	EnumItem *EnumInfo::item(int idx) const
	{
		return ((EnumInfoPrivate*)this)->items[idx].get();
	}

	int EnumInfo::find_item(const char *name) const
	{
		auto &items = ((EnumInfoPrivate*)this)->items;
		for (auto i = 0; i < items.size(); i++)
		{
			if (items[i]->name == name)
				return i;
		}
		return -1;
	}

	int EnumInfo::find_item(int value) const
	{
		auto &items = ((EnumInfoPrivate*)this)->items;
		for (auto i = 0; i < items.size(); i++)
		{
			if (items[i]->value == value)
				return i;
		}
		return -1;
	}

	String EnumInfo::serialize_value(bool single, int v) const
	{
		return ((EnumInfoPrivate*)this)->serialize_value(single, v);
	}

	struct VaribleInfoPrivate : VaribleInfo
	{
		VariableTag tag;
		std::string type_name;
		uint type_hash;
		std::string name;
		std::string attribute;
		int offset;
		int size;
		CommonData default_value;

		inline VaribleInfoPrivate()
		{
			default_value.fmt[0] = 0;
			default_value.fmt[1] = 0;
			default_value.fmt[2] = 0;
			default_value.fmt[3] = 0;
		}

		inline std::string serialize_default_value(int precision = 6) const
		{
			switch (tag)
			{
			case VariableTagEnumSingle:
			{
				auto e = find_enum(type_hash);
				return e->serialize_value(true, default_value.v.i[0]).v;
			}
				break;
			case VariableTagEnumMulti:
				break;
			case VariableTagVariable:
				switch (type_hash)
				{
				case cH("bool"):
					return default_value.v.i[0] ? "true" : "false";
				case cH("uint"):
					return to_stdstring((uint)default_value.v.i[0]);
				case cH("int"):
					return to_stdstring(default_value.v.i[0]);
				case cH("Ivec2"):
					return to_stdstring((Ivec2)default_value.v.i);
				case cH("Ivec3"):
					return to_stdstring((Ivec3)default_value.v.i);
				case cH("Ivec4"):
					return to_stdstring(default_value.v.i);
				case cH("float"):
					return to_stdstring(default_value.v.f[0], precision);
				case cH("Vec2"):
					return to_stdstring((Vec2)default_value.v.f, precision);
				case cH("Vec3"):
					return to_stdstring((Vec3)default_value.v.f, precision);
				case cH("Vec4"):
					return to_stdstring(default_value.v.f, precision);
				case cH("uchar"):
					return to_stdstring(default_value.v.b[0]);
				case cH("Bvec2"):
					return to_stdstring((Bvec2)default_value.v.b);
				case cH("Bvec3"):
					return to_stdstring((Bvec3)default_value.v.b);
				case cH("Bvec4"):
					return to_stdstring(default_value.v.b);
				}
				break;
			}

			return "";
		}

		inline void unserialize_default_value(const std::string &str)
		{
			switch (tag)
			{
			case VariableTagEnumSingle:
			{
				auto e = find_enum(type_hash);
				default_value.i1() = e->find_item(str.c_str());
			}
				break;
			case VariableTagVariable:
				switch (type_hash)
				{
				case cH("bool"):
					default_value.i1() = str == "true" ? true : false;
					break;
				case cH("uint"):
					default_value.i1() = stou1(str.c_str());
					break;
				case cH("int"):
					default_value.i1() = stoi1(str.c_str());
					break;
				case cH("Ivec2"):
					default_value.i2() = stoi2(str.c_str());
					break;
				case cH("Ivec3"):
					default_value.i3() = stoi3(str.c_str());
					break;
				case cH("Ivec4"):
					default_value.i4() = stoi4(str.c_str());
					break;
				case cH("float"):
					default_value.f1() = stof1(str.c_str());
					break;
				case cH("Vec2"):
					default_value.f2() = stof2(str.c_str());
					break;
				case cH("Vec3"):
					default_value.f3() = stof3(str.c_str());
					break;
				case cH("Vec4"):
					default_value.f4() = stof4(str.c_str());
					break;
				case cH("uchar"):
					default_value.b1() = stob1(str.c_str());
					break;
				case cH("Bvec2"):
					default_value.b2() = stob2(str.c_str());
					break;
				case cH("Bvec3"):
					default_value.b3() = stob3(str.c_str());
					break;
				case cH("Bvec4"):
					default_value.b4() = stob4(str.c_str());
					break;
				}
				break;
			}
		}

		inline bool compare(void *src, void *dst) const
		{
			src = (char*)src + offset;
			dst = (char*)dst + offset;

			switch (tag)
			{
			case VariableTagEnumSingle: case VariableTagEnumMulti:
				return *(int*)src == *(int*)dst;
			case VariableTagVariable:
				switch (type_hash)
				{
				case cH("bool"):
					return *(bool*)src == *(bool*)dst;
				case cH("uint"):
					return *(uint*)src == *(uint*)dst;
				case cH("int"):
					return *(int*)src == *(int*)dst;
				case cH("Ivec2"):
					return *(Ivec2*)src == *(Ivec2*)dst;
				case cH("Ivec3"):
					return *(Ivec3*)src == *(Ivec3*)dst;
				case cH("Ivec4"):
					return *(Ivec4*)src == *(Ivec4*)dst;
				case cH("float"):
					return *(float*)src == *(float*)dst;
				case cH("Vec2"):
					return *(Vec2*)src == *(Vec2*)dst;
				case cH("Vec3"):
					return *(Vec3*)src == *(Vec3*)dst;
				case cH("Vec4"):
					return *(Vec4*)src == *(Vec4*)dst;
				case cH("uchar"):
					return *(uchar*)src == *(uchar*)dst;
				case cH("Bvec2"):
					return *(Bvec2*)src == *(Bvec2*)dst;
				case cH("Bvec3"):
					return *(Bvec3*)src == *(Bvec3*)dst;
				case cH("Bvec4"):
					return *(Bvec4*)src == *(Bvec4*)dst;
				case cH("String"):
					return *(String*)src == *(String*)dst;
				case cH("StringAndHash"):
					return *(StringAndHash*)src == *(StringAndHash*)dst;
				}
				break;
			}

			return false;
		}

		inline bool compare_to_default(void *src, bool is_obj) const
		{
			if (is_obj)
				src = (char*)src + offset;

			switch (tag)
			{
			case VariableTagEnumSingle: case VariableTagEnumMulti:
				return *(int*)src == default_value.v.i[0];
			case VariableTagVariable:
				switch (type_hash)
				{
				case cH("bool"):
					return *(bool*)src == default_value.v.i[0];
				case cH("uint"):
					return *(uint*)src == (uint)default_value.v.i[0];
				case cH("int"):
					return *(int*)src == default_value.v.i[0];
				case cH("Ivec2"):
					return *(Ivec2*)src == (Ivec2)default_value.v.i;
				case cH("Ivec3"):
					return *(Ivec3*)src == (Ivec3)default_value.v.i;
				case cH("Ivec4"):
					return *(Ivec4*)src == default_value.v.i;
				case cH("float"):
					return *(float*)src == default_value.v.f[0];
				case cH("Vec2"):
					return *(Vec2*)src == (Vec2)default_value.v.f;
				case cH("Vec3"):
					return *(Vec3*)src == (Vec3)default_value.v.f;
				case cH("Vec4"):
					return *(Vec4*)src == default_value.v.f;
				case cH("uchar"):
					return *(uchar*)src == default_value.v.b[0];
				case cH("Bvec2"):
					return *(Bvec2*)src == (Bvec2)default_value.v.b;
				case cH("Bvec3"):
					return *(Bvec3*)src == (Bvec3)default_value.v.b;
				case cH("Bvec4"):
					return *(Bvec4*)src == default_value.v.b;
				}
				break;
			}

			return false;
		}

		inline String serialize_value(void *src, bool is_obj, int precision) const
		{
			if (is_obj)
				src = (char*)src + offset;

			switch (tag)
			{
			case VariableTagEnumSingle:
			{
				auto e = find_enum(type_hash);
				return e->serialize_value(true, *(int*)src);
			}
				break;
			case VariableTagEnumMulti:
				break;
			case VariableTagVariable:
				switch (type_hash)
				{
				case cH("bool"):
					return *(bool*)src ? "true" : "false";
				case cH("uint"):
					return to_string(*(uint*)src);
				case cH("int"):
					return to_string(*(int*)src);
				case cH("Ivec2"):
					return to_string(*(Ivec2*)src);
				case cH("Ivec3"):
					return to_string(*(Ivec3*)src);
				case cH("Ivec4"):
					return to_string(*(Ivec4*)src);
				case cH("float"):
					return to_string(*(float*)src, precision);
				case cH("Vec2"):
					return to_string(*(Vec2*)src, precision);
				case cH("Vec3"):
					return to_string(*(Vec3*)src, precision);
				case cH("Vec4"):
					return to_string(*(Vec4*)src, precision);
				case cH("uchar"):
					return to_string(*(uchar*)src);
				case cH("Bvec2"):
					return to_string(*(Bvec2*)src);
				case cH("Bvec3"):
					return to_string(*(Bvec3*)src);
				case cH("Bvec4"):
					return to_string(*(Bvec4*)src);
				case cH("String"):
					return *(String*)src;
				case cH("StringAndHash"):
					return *(StringAndHash*)src;
				}
				break;
			}

			return "";
		}

		inline void unserialize_value(const std::string &str, void *dst, bool is_obj) const
		{
			if (is_obj)
				dst = (char*)dst + offset;

			switch (tag)
			{
			case VariableTagEnumSingle:
			{
				auto e = find_enum(type_hash);
				*(int*)dst = e->find_item(str.c_str());
			}
				break;
			case VariableTagVariable:
				switch (type_hash)
				{
				case cH("bool"):
					*(bool*)dst = str == "true" ? true : false;
					break;
				case cH("uint"):
					*(uint*)dst = stou1(str.c_str());
					break;
				case cH("int"):
					*(int*)dst = stoi1(str.c_str());
					break;
				case cH("Ivec2"):
					*(Ivec2*)dst = stoi2(str.c_str());
					break;
				case cH("Ivec3"):
					*(Ivec3*)dst = stoi3(str.c_str());
					break;
				case cH("Ivec4"):
					*(Ivec4*)dst = stoi4(str.c_str());
					break;
				case cH("float"):
					*(float*)dst = stof1(str.c_str());
					break;
				case cH("Vec2"):
					*(Vec2*)dst = stof2(str.c_str());
					break;
				case cH("Vec3"):
					*(Vec3*)dst = stof3(str.c_str());
					break;
				case cH("Vec4"):
					*(Vec4*)dst = stof4(str.c_str());
					break;
				case cH("uchar"):
					*(uchar*)dst = stob1(str.c_str());
					break;
				case cH("Bvec2"):
					*(Bvec2*)dst = stob2(str.c_str());
					break;
				case cH("Bvec3"):
					*(Bvec3*)dst = stob3(str.c_str());
					break;
				case cH("Bvec4"):
					*(Bvec4*)dst = stob4(str.c_str());
					break;
				}
				break;
			}
		}
	};

	static const char *tag_names[] = {
		"enum_single",
		"enum_multi",
		"varible",
		"pointer",
		"array_of_varible",
		"array_of_pointer"
	};

	const char *get_variable_tag_name(VariableTag tag)
	{
		return tag_names[tag];
	}

	VariableTag VaribleInfo::tag() const
	{
		return ((VaribleInfoPrivate*)this)->tag;
	}

	const char *VaribleInfo::type_name() const
	{
		return ((VaribleInfoPrivate*)this)->type_name.c_str();
	}

	uint VaribleInfo::type_hash() const
	{
		return ((VaribleInfoPrivate*)this)->type_hash;
	}

	const char *VaribleInfo::name() const
	{
		return ((VaribleInfoPrivate*)this)->name.c_str();
	}

	int VaribleInfo::offset() const
	{
		return ((VaribleInfoPrivate*)this)->offset;
	}

	int VaribleInfo::size() const
	{
		return ((VaribleInfoPrivate*)this)->size;
	}

	const char *VaribleInfo::attribute() const
	{
		return ((VaribleInfoPrivate*)this)->attribute.c_str();
	}

	const CommonData &VaribleInfo::default_value() const
	{
		return ((VaribleInfoPrivate*)this)->default_value;
	}

	bool VaribleInfo::compare(void *src, void *dst) const
	{
		return ((VaribleInfoPrivate*)this)->compare(src, dst);
	}

	bool VaribleInfo::compare_to_default(void *src, bool is_obj) const
	{
		return ((VaribleInfoPrivate*)this)->compare_to_default(src, is_obj);
	}

	String VaribleInfo::serialize_value(void *src, bool is_obj, int precision) const
	{
		return ((VaribleInfoPrivate*)this)->serialize_value(src, is_obj, precision);
	}

	void VaribleInfo::unserialize_value(const std::string &str, void *dst, bool is_obj) const
	{
		return ((VaribleInfoPrivate*)this)->unserialize_value(str, dst, is_obj);
	}

	struct UDTPrivate : UDT
	{
		std::string name;
		int size;
		std::vector<std::unique_ptr<VaribleInfoPrivate>> items;
		void* update_function_rva;
		std::wstring update_function_module_name;

		int find_pos;

		inline UDTPrivate()
		{
			update_function_rva = nullptr;

			find_pos = 0;
		}

		inline int find_item_i(const char *name)
		{
			if (items.empty())
				return -1;

			auto p = find_pos;
			while (true)
			{
				if (items[find_pos]->name == name)
				{
					auto t = find_pos;
					find_pos++;
					if (find_pos >= items.size())
						find_pos = 0;
					return t;
				}
				find_pos++;
				if (find_pos == p)
					return -1;
				if (find_pos >= items.size())
					find_pos = 0;
			}
			return -1;
		}
	};

	const char *UDT::name() const
	{
		return ((UDTPrivate*)this)->name.c_str();
	}

	int UDT::size() const
	{
		return ((UDTPrivate*)this)->size;
	}

	int UDT::item_count() const
	{
		return ((UDTPrivate*)this)->items.size();
	}

	VaribleInfo *UDT::item(int idx) const
	{
		return ((UDTPrivate*)this)->items[idx].get();
	}

	int UDT::find_item_i(const char *name) const
	{
		return ((UDTPrivate*)this)->find_item_i(name);
	}

	const void* UDT::update_function_rva() const
	{
		return ((UDTPrivate*)this)->update_function_rva;
	}

	const wchar_t* UDT::update_function_module_name() const
	{
		return ((UDTPrivate*)this)->update_function_module_name.c_str();
	}

	static std::map<unsigned int, std::unique_ptr<EnumInfoPrivate>> enums;
	static std::map<unsigned int, std::unique_ptr<UDTPrivate>> udts;

	Array<EnumInfo*> get_enums()
	{
		Array<EnumInfo*> ret;
		ret.resize(enums.size());
		auto i = 0;
		for (auto it = enums.begin(); it != enums.end(); it++)
		{
			ret[i] = (*it).second.get();
			i++;
		}
		return ret;
	}

	EnumInfo *find_enum(unsigned int name_hash)
	{
		auto it = enums.find(name_hash);
		return it == enums.end() ? nullptr : it->second.get();
	}

	Array<UDT*> get_udts()
	{
		Array<UDT*> ret;
		ret.resize(udts.size());
		auto i = 0;
		for (auto it = udts.begin(); it != udts.end(); it++)
		{
			ret[i] = (*it).second.get();
			i++;
		}
		return ret;
	}

	UDT *find_udt(unsigned int name_hash)
	{
		auto it = udts.find(name_hash);
		return it == udts.end() ? nullptr : it->second.get();
	}

	struct SerializableAttributePrivate : SerializableAttribute
	{
		std::string name;
		std::string value;
	};

	const std::string &SerializableAttribute::name() const
	{
		return ((SerializableAttributePrivate*)this)->name;
	}

	const std::string &SerializableAttribute::value() const
	{
		return ((SerializableAttributePrivate*)this)->value;
	}

	void SerializableAttribute::set_name(const std::string &name)
	{
		((SerializableAttributePrivate*)this)->name = name;
	}

	void SerializableAttribute::set_value(const std::string &value)
	{
		((SerializableAttributePrivate*)this)->value = value;
	}

	static int find_obj_pos(const std::vector<std::pair<void*, uint>> &table, void *obj)
	{
		for (auto i = 0; i < table.size(); i++)
		{
			if (table[i].first == obj)
				return table[i].second;
		}
		assert(0);
		return -1;
	}

	static void serialize_commondata(const std::vector<std::pair<void*, uint>> &table, int precision, std::string &ty_str, std::string &vl_str, CommonData *d)
	{
		if (typefmt_compare(d->fmt, "f"))
		{
			ty_str = "f";
			vl_str = to_stdstring(d->f1(), precision);
		}
		else if (typefmt_compare(d->fmt, "f2"))
		{
			ty_str = "f2";
			vl_str = to_stdstring(d->f2(), precision);
		}
		else if (typefmt_compare(d->fmt, "f3"))
		{
			ty_str = "f3";
			vl_str = to_stdstring(d->f3(), precision);
		}
		else if (typefmt_compare(d->fmt, "f4"))
		{
			ty_str = "f4";
			vl_str = to_stdstring(d->f4(), precision);
		}
		else if (typefmt_compare(d->fmt, "i"))
		{
			ty_str = "i";
			vl_str = to_stdstring(d->i1());
		}
		else if (typefmt_compare(d->fmt, "i2"))
		{
			ty_str = "i2";
			vl_str = to_stdstring(d->i2());
		}
		else if (typefmt_compare(d->fmt, "i3"))
		{
			ty_str = "i3";
			vl_str = to_stdstring(d->i3());
		}
		else if (typefmt_compare(d->fmt, "i4"))
		{
			ty_str = "i4";
			vl_str = to_stdstring(d->i4());
		}
		else if (typefmt_compare(d->fmt, "b"))
		{
			ty_str = "b";
			vl_str = to_stdstring((int)d->b1());
		}
		else if (typefmt_compare(d->fmt, "b2"))
		{
			ty_str = "b2";
			vl_str = to_stdstring(d->b2());
		}
		else if (typefmt_compare(d->fmt, "b3"))
		{
			ty_str = "b3";
			vl_str = to_stdstring(d->b3());
		}
		else if (typefmt_compare(d->fmt, "b4"))
		{
			ty_str = "b4";
			vl_str = to_stdstring(d->b4());
		}
		else if (typefmt_compare(d->fmt, "p"))
		{
			ty_str = "p";
			vl_str = to_stdstring(find_obj_pos(table, d->p()));
		}
		else
			assert(0);
	}

	static void *find_obj(const std::vector<std::pair<void*, uint>> &table, uint id)
	{
		for (auto i = 0; i < table.size(); i++)
		{
			if (table[i].second == id)
				return table[i].first;
		}
		assert(0);
		return nullptr;
	}

	static void unserialize_commondata(const std::vector<std::pair<void*, uint>> &table, const std::string &ty_str, const std::string &vl_str, CommonData *d)
	{
		if (ty_str == "f")
		{
			typefmt_assign(d->fmt, "f");
			d->f1() = stof1(vl_str.c_str());
		}
		else if (ty_str == "f2")
		{
			typefmt_assign(d->fmt, "f2");
			d->f2() = stof2(vl_str.c_str());
		}
		else if (ty_str == "f3")
		{
			typefmt_assign(d->fmt, "f3");
			d->f3() = stof3(vl_str.c_str());
		}
		else if (ty_str == "f4")
		{
			typefmt_assign(d->fmt, "f4");
			d->f4() = stof4(vl_str.c_str());
		}
		else if (ty_str == "i")
		{
			typefmt_assign(d->fmt, "i");
			d->i1() = stoi1(vl_str.c_str());
		}
		else if (ty_str == "i2")
		{
			typefmt_assign(d->fmt, "i2");
			d->i2() = stoi2(vl_str.c_str());
		}
		else if (ty_str == "i3")
		{
			typefmt_assign(d->fmt, "i3");
			d->i3() = stoi3(vl_str.c_str());
		}
		else if (ty_str == "i4")
		{
			typefmt_assign(d->fmt, "i4");
			d->i4() = stoi4(vl_str.c_str());
		}
		else if (ty_str == "b")
		{
			typefmt_assign(d->fmt, "b");
			d->b1() = stob1(vl_str.c_str());
		}
		else if (ty_str == "b2")
		{
			typefmt_assign(d->fmt, "b2");
			d->b2() = stob2(vl_str.c_str());
		}
		else if (ty_str == "b3")
		{
			typefmt_assign(d->fmt, "b3");
			d->b3() = stob3(vl_str.c_str());
		}
		else if (ty_str == "b4")
		{
			typefmt_assign(d->fmt, "b4");
			d->b4() = stob4(vl_str.c_str());
		}
		else if (ty_str == "p")
		{
			typefmt_assign(d->fmt, "p");
			d->p() = find_obj(table, stoi1(vl_str.c_str()));
		}
		else
			assert(0);
	}

	static bool has_id(const std::vector<std::pair<void*, uint>> &obj_table, uint id)
	{
		for (auto &o : obj_table)
		{
			if (o.second == id)
				return true;
		}
		return false;
	}

	static uint generate_id(const std::vector<std::pair<void*, uint>> &obj_table)
	{
		while (true)
		{
			auto id = ::rand() % 1000000;
			if (!has_id(obj_table, id))
				return id;
		}
		return 0;
	}

	struct SerializableNodePrivate;

	static SerializableNodePrivate *create_obj_node(std::vector<std::pair<void*, uint>> &table, void *obj)
	{
		auto n = SerializableNode::create("obj");
		auto id = generate_id(table);
		table.emplace_back(obj, id);
		n->new_attr("id", to_stdstring(id));
		return (SerializableNodePrivate*)n;
	}

	static void *create_obj(UDT *u, Function<SerializableNode::ObjGeneratorParm> &obj_generator, void *parent, uint att_hash)
	{
		obj_generator.p.udt() = u;
		obj_generator.p.udt() = u;
		obj_generator.p.parent() = parent;
		obj_generator.p.att_hash() = att_hash;
		obj_generator.exec();
		auto obj = obj_generator.p.out_obj();
		assert(obj);
		return obj;
	}

	struct SerializableNodePrivate : SerializableNode
	{
		std::string name;
		std::string value;

		std::vector<std::unique_ptr<SerializableAttributePrivate>> attrs;
		std::vector<std::unique_ptr<SerializableNodePrivate>> nodes;

		int attr_find_pos;
		int node_find_pos;

		bool xml_CDATA;

		inline SerializableNodePrivate()
		{
			attr_find_pos = 0;
			node_find_pos = 0;
			xml_CDATA = false;
		}

		inline SerializableAttribute *new_attr(const std::string &name, const std::string &value)
		{
			return insert_attr(-1, name, value);
		}

		inline SerializableAttribute *insert_attr(int idx, const std::string &name, const std::string &value)
		{
			if (idx == -1)
				idx = attrs.size();
			auto a = new SerializableAttributePrivate;
			a->name = name;
			a->value = value;
			attrs.emplace(attrs.begin() + idx, a);
			return a;
		}

		inline void remove_attr(int idx)
		{
			attrs.erase(attrs.begin() + idx);
		}

		inline void remove_attr(SerializableAttribute *a)
		{
			for (auto it = attrs.begin(); it != attrs.end(); it++)
			{
				if (it->get() == a)
				{
					attrs.erase(it);
					return;
				}
			}
		}

		inline void clear_attrs()
		{
			attrs.clear();
		}

		inline SerializableAttribute *find_attr(const std::string &name)
		{
			if (attrs.empty())
				return nullptr;

			auto p = attr_find_pos;
			while (true)
			{
				if (attrs[attr_find_pos]->name == name)
				{
					auto t = attr_find_pos;
					attr_find_pos++;
					if (attr_find_pos >= attrs.size())
						attr_find_pos = 0;
					return attrs[t].get();
				}
				attr_find_pos++;
				if (attr_find_pos >= attrs.size())
					attr_find_pos = 0;
				if (attr_find_pos == p)
					return nullptr;
			}
			return nullptr;
		}

		inline void add_node(SerializableNode *n)
		{
			nodes.emplace_back((SerializableNodePrivate*)n);
		}

		inline SerializableNode *new_node(const std::string &name)
		{
			return insert_node(-1, name);
		}

		inline SerializableNode *insert_node(int idx, const std::string &name)
		{
			if (idx == -1)
				idx = nodes.size();
			auto n = new SerializableNodePrivate;
			n->name = name;
			nodes.emplace(nodes.begin() + idx, n);
			return n;
		}

		inline void remove_node(int idx)
		{
			nodes.erase(nodes.begin() + idx);
		}

		inline void remove_node(SerializableNode *n)
		{
			for (auto it = nodes.begin(); it != nodes.end(); it++)
			{
				if (it->get() == n)
				{
					nodes.erase(it);
					return;
				}
			}
		}

		inline void clear_nodes()
		{
			nodes.clear();
		}

		inline SerializableNode *find_node(const std::string &name)
		{
			if (nodes.empty())
				return nullptr;

			auto p = node_find_pos;
			while (true)
			{
				if (nodes[node_find_pos]->name == name)
				{
					auto t = node_find_pos;
					node_find_pos++;
					if (node_find_pos >= nodes.size())
						node_find_pos = 0;
					return nodes[t].get();
				}
				node_find_pos++;
				if (node_find_pos == p)
					return nullptr;
				if (node_find_pos >= nodes.size())
					node_find_pos = 0;
			}
			return nullptr;
		}

		void serialize_RE(UDT *u, std::vector<std::pair<void*, uint>> &obj_table, void *src, int precision)
		{
			for (auto i = 0; i < u->item_count(); i++)
			{
				auto item = u->item(i);

				switch (item->tag())
				{
				case VariableTagArrayOfVariable:
				{
					auto cnt = ((Array<int>*)((char*)src + item->offset()))->size;

					if (cnt == 0)
						break;

					auto n_item = new_node("attribute");
					n_item->new_attr("name", item->name());

					if (item->type_hash() == cH("CommonData"))
					{
						auto &arr = *(Array<CommonData>*)((char*)src + item->offset());

						for (auto i_i = 0; i_i < cnt; i_i++)
						{
							auto n_it = n_item->new_node("item");
							std::string ty_str, vl_str;
							serialize_commondata(obj_table, precision, ty_str, vl_str, &arr[i_i]);
							n_it->new_attr("type", ty_str);
							n_it->new_attr("value", vl_str);
						}
					}
					else if (item->type_hash() == cH("String"))
					{
						auto &arr = *(Array<String>*)((char*)src + item->offset());

						for (auto i_i = 0; i_i < cnt; i_i++)
						{
							auto n_it = n_item->new_node("item");
							n_it->new_attr("value", arr[i_i].v);
						}
					}
					else if (item->type_hash() == cH("StringW"))
					{
						auto &arr = *(Array<StringW>*)((char*)src + item->offset());

						for (auto i_i = 0; i_i < cnt; i_i++)
						{
							auto n_it = n_item->new_node("item");
							n_it->new_attr("value", w2s(arr[i_i].v));
						}
					}
					else if (item->type_hash() == cH("StringAndHash"))
					{
						auto &arr = *(Array<StringAndHash>*)((char*)src + item->offset());

						for (auto i_i = 0; i_i < cnt; i_i++)
						{
							auto n_it = n_item->new_node("item");
							n_it->new_attr("value", arr[i_i].v);
						}
					}
					else if (item->type_hash() == cH("Function"))
					{
						auto &arr = *(Array<Function<>>*)((char*)src + item->offset());

						for (auto i_i = 0; i_i < arr.size; i_i++)
						{
							const auto &f = arr[i_i];
							auto r = find_registered_function(0, f.pf);

							auto n_fn = n_item->new_node("function");
							n_fn->new_attr("id", to_stdstring(r->id));

							auto d = f.p.d + r->parameter_count;
							for (auto i = 0; i < f.capture_count; i++)
							{
								auto n_cpt = n_fn->new_node("capture");
								std::string ty_str, vl_str;
								serialize_commondata(obj_table, precision, ty_str, vl_str, (CommonData*)d);
								n_cpt->new_attr("type", ty_str);
								n_cpt->new_attr("value", vl_str);
								d++;
							}
						}
					}
				}
					break;
				case VariableTagArrayOfPointer:
				{
					const auto &arr = *(Array<void*>*)((char*)src + item->offset());

					if (arr.size == 0)
						break;

					auto n_item = new_node("attribute");
					n_item->new_attr("name", item->name());

					auto u_sub = find_udt(item->type_hash());
					if (u_sub)
					{
						for (auto i_i = 0; i_i < arr.size; i_i++)
						{
							auto obj_sub = arr[i_i];

							auto n_sub = create_obj_node(obj_table, obj_sub);
							n_sub->serialize_RE(u_sub, obj_table, obj_sub, precision);
							n_item->add_node(n_sub);
						}
					}
				}
					break;
				default:
					if (!item->compare_to_default(src, true))
					{
						auto n_item = new_node("attribute");
						n_item->new_attr("name", item->name());

						n_item->new_attr("value", item->serialize_value(src, true, precision).v);
					}
				}
			}
		}

		void unserialize_RE(UDT *u, std::vector<std::pair<void*, uint>> &obj_table, void *obj, Function<SerializableNode::ObjGeneratorParm> &obj_generator)
		{
			for (auto i = 0; i < node_count(); i++)
			{
				auto n_item = node(i);

				auto item = u->item(u->find_item_i(n_item->find_attr("name")->value().c_str()));

				switch (item->tag())
				{
				case VariableTagArrayOfVariable:
				{
					if (item->type_hash() == cH("CommonData"))
					{
						auto &arr = *(Array<CommonData>*)((char*)obj + item->offset());
						auto cnt = n_item->node_count();
						arr.resize(cnt);

						for (auto i_i = 0; i_i < cnt; i_i++)
						{
							auto n_i = n_item->node(i_i);
							if (n_i->name() == "item")
								unserialize_commondata(obj_table, n_i->find_attr("type")->value(), n_i->find_attr("value")->value(), &arr[i_i]);
							else
								assert(0);
						}
					}
					else if (item->type_hash() == cH("String"))
					{
						auto &arr = *(Array<String>*)((char*)obj + item->offset());
						auto cnt = n_item->node_count();
						arr.resize(cnt);

						for (auto i_i = 0; i_i < cnt; i_i++)
						{
							auto n_i = n_item->node(i_i);
							if (n_i->name() == "item")
								arr[i_i] = n_i->find_attr("value")->value();
							else
								assert(0);
						}
					}
					else if (item->type_hash() == cH("StringW"))
					{
						auto &arr = *(Array<StringW>*)((char*)obj + item->offset());
						auto cnt = n_item->node_count();
						arr.resize(cnt);

						for (auto i_i = 0; i_i < cnt; i_i++)
						{
							auto n_i = n_item->node(i_i);
							if (n_i->name() == "item")
								arr[i_i] = s2w(n_i->find_attr("value")->value());
							else
								assert(0);
						}
					}
					else if (item->type_hash() == cH("StringAndHash"))
					{
						auto &arr = *(Array<StringAndHash>*)((char*)obj + item->offset());
						auto cnt = n_item->node_count();
						arr.resize(cnt);

						for (auto i_i = 0; i_i < cnt; i_i++)
						{
							auto n_i = n_item->node(i_i);
							if (n_i->name() == "item")
								arr[i_i] = n_i->find_attr("value")->value();
							else
								assert(0);
						}
					}
					else if (item->type_hash() == cH("Function"))
					{
						auto &arr = *(Array<Function<>>*)((char*)obj + item->offset());
						auto cnt = n_item->node_count();
						arr.resize(cnt);

						for (auto i_i = 0; i_i < cnt; i_i++)
						{
							auto n_i = n_item->node(i_i);

							if (n_i->name() == "function")
							{
								auto cpt_cnt = n_i->node_count();
								auto id = stoi(n_i->find_attr("id")->value());
								std::vector<CommonData> capts;
								capts.resize(cpt_cnt);

								for (auto i_c = 0; i_c < cpt_cnt; i_c++)
								{
									auto n_c = n_i->node(i_c);
									if (n_c->name() == "capture")
										unserialize_commondata(obj_table, n_c->find_attr("type")->value(), n_c->find_attr("value")->value(), &capts[i_c]);
									else
										assert(0);
								}

								auto r = find_registered_function(id, nullptr);
								if (!r)
									assert(0);

								arr[i_i] = Function<>();
								arr[i_i].set(r->pf, r->parameter_count, capts);
							}
							else
								assert(0);
						}
					}
				}
					break;
				case VariableTagArrayOfPointer:
				{
					auto &arr = *(Array<void*>*)((char*)obj + item->offset());
					auto cnt = n_item->node_count();
					arr.resize(cnt);

					auto u_sub = find_udt(item->type_hash());
					if (u_sub)
					{
						auto name_hash = H(item->name());
						for (auto i_i = 0; i_i < cnt; i_i++)
						{
							auto n_i = n_item->node(i_i);
							assert(n_i->name() == "obj");

							auto obj_sub = create_obj(u_sub, obj_generator, obj, name_hash);
							if (obj_sub)
							{
								obj_table.emplace_back(obj_sub, stoi1(n_i->find_attr("id")->value().c_str()));
								((SerializableNodePrivate*)n_i)->unserialize_RE(u_sub, obj_table, obj_sub, obj_generator);
							}

							arr[i_i] = obj_sub;
						}
					}
				}
					break;
				default:
					item->unserialize_value(n_item->find_attr("value")->value(), obj, true);
				}
			}
		}
	};

	const std::string &SerializableNode::name() const
	{
		return ((SerializableNodePrivate*)this)->name;
	}

	const std::string &SerializableNode::value() const
	{
		return ((SerializableNodePrivate*)this)->value;
	}

	bool SerializableNode::is_xml_CDATA() const
	{
		return ((SerializableNodePrivate*)this)->xml_CDATA;
	}

	void SerializableNode::set_name(const std::string &name)
	{
		((SerializableNodePrivate*)this)->name = name;
	}

	void SerializableNode::set_value(const std::string &value)
	{
		((SerializableNodePrivate*)this)->value = value;
	}

	void SerializableNode::set_xml_CDATA(bool v)
	{
		((SerializableNodePrivate*)this)->xml_CDATA = v;
	}

	SerializableAttribute *SerializableNode::new_attr(const std::string &name, const std::string &value)
	{
		return ((SerializableNodePrivate*)this)->new_attr(name, value);
	}

	SerializableAttribute *SerializableNode::insert_attr(int idx, const std::string &name, const std::string &value)
	{
		return ((SerializableNodePrivate*)this)->insert_attr(idx, name, value);
	}

	void SerializableNode::remove_attr(int idx)
	{
		((SerializableNodePrivate*)this)->remove_attr(idx);
	}

	void SerializableNode::remove_attr(SerializableAttribute *a)
	{
		((SerializableNodePrivate*)this)->remove_attr(a);
	}

	void SerializableNode::clear_attrs()
	{
		((SerializableNodePrivate*)this)->clear_attrs();
	}

	int SerializableNode::attr_count() const
	{
		return ((SerializableNodePrivate*)this)->attrs.size();
	}

	SerializableAttribute *SerializableNode::attr(int idx) const
	{
		return ((SerializableNodePrivate*)this)->attrs[idx].get();
	}

	SerializableAttribute *SerializableNode::find_attr(const std::string &name)
	{
		return ((SerializableNodePrivate*)this)->find_attr(name);
	}

	void SerializableNode::add_node(SerializableNode *n)
	{
		((SerializableNodePrivate*)this)->add_node(n);
	}

	SerializableNode *SerializableNode::new_node(const std::string &name)
	{
		return ((SerializableNodePrivate*)this)->new_node(name);
	}

	SerializableNode *SerializableNode::insert_node(int idx, const std::string &name)
	{
		return ((SerializableNodePrivate*)this)->insert_node(idx, name);
	}

	void SerializableNode::remove_node(int idx)
	{
		((SerializableNodePrivate*)this)->remove_node(idx);
	}

	void SerializableNode::remove_node(SerializableNode *n)
	{
		((SerializableNodePrivate*)this)->remove_node(n);
	}

	void SerializableNode::clear_nodes()
	{
		((SerializableNodePrivate*)this)->clear_nodes();
	}

	int SerializableNode::node_count() const
	{
		return ((SerializableNodePrivate*)this)->nodes.size();
	}

	SerializableNode *SerializableNode::node(int idx) const
	{
		return ((SerializableNodePrivate*)this)->nodes[idx].get();
	}

	SerializableNode *SerializableNode::find_node(const std::string &name)
	{
		return ((SerializableNodePrivate*)this)->find_node(name);
	}

	void xml_save(rapidxml::xml_document<> &doc, rapidxml::xml_node<> *dst, SerializableNodePrivate *src)
	{
		for (auto &sa : src->attrs)
			dst->append_attribute(doc.allocate_attribute(sa->name.c_str(), sa->value.c_str()));

		for (auto &sn : src->nodes)
		{
			auto n = doc.allocate_node(sn->xml_CDATA ? rapidxml::node_cdata : rapidxml::node_element, sn->name.c_str(), sn->value.c_str());
			dst->append_node(n);
			xml_save(doc, n, sn.get());
		}
	}

	void SerializableNode::save_xml(const std::wstring &filename) const
	{
		rapidxml::xml_document<> xml_doc;
		auto rn = xml_doc.allocate_node(rapidxml::node_element, name().c_str());
		xml_doc.append_node(rn);

		xml_save(xml_doc, rn, (SerializableNodePrivate*)this);

		std::string str;
		rapidxml::print(std::back_inserter(str), xml_doc);

		std::ofstream file(filename);
		file.write(str.data(), str.size());
	}

	static void save_file_string(std::ofstream &dst, const std::string &src)
	{
		int len = src.size();
		dst.write((char*)&len, sizeof(int));
		dst.write((char*)src.c_str(), len);
	}

	void bin_save(std::ofstream &dst, SerializableNodePrivate *src)
	{
		int att_cnt = src->attrs.size();
		dst.write((char*)&att_cnt, sizeof(int));
		for (auto &a : src->attrs)
		{
			save_file_string(dst, a->name);
			save_file_string(dst, a->value);
		}

		save_file_string(dst, src->value);

		int node_cnt = src->nodes.size();
		dst.write((char*)&node_cnt, sizeof(int));
		for (auto &n : src->nodes)
		{
			save_file_string(dst, n->name);
			bin_save(dst, n.get());
		}
	}

	void SerializableNode::save_bin(const std::wstring &filename) const
	{
		std::ofstream file(filename);

		save_file_string(file, name());

		bin_save(file, (SerializableNodePrivate*)this);
	}

	void *SerializableNode::unserialize(UDT *u, Function<ObjGeneratorParm> &obj_generator)
	{
		assert(name() == "obj");

		auto obj = create_obj(u, obj_generator, nullptr, 0);

		std::vector<std::pair<void*, uint>> obj_table;
		obj_table.emplace_back(obj, stoi1(find_attr("id")->value().c_str()));

		((SerializableNodePrivate*)this)->unserialize_RE(u, obj_table, obj, obj_generator);

		return obj;
	}

	SerializableNode *SerializableNode::create(const std::string &name)
	{
		auto n = new SerializableNodePrivate;
		n->name = name;

		return n;
	}

	void xml_load(rapidxml::xml_node<> *src, SerializableNode *dst)
	{
		for (auto a = src->first_attribute(); a; a = a->next_attribute())
			dst->new_attr(a->name(), a->value());

		dst->set_value(src->value());

		for (auto n = src->first_node(); n; n = n->next_sibling())
		{
			auto node = dst->new_node(n->name());
			if (n->type() == rapidxml::node_cdata)
				node->set_xml_CDATA(true);
			xml_load(n, node);
		}
	}

	static std::string load_file_string(std::ifstream &src)
	{
		int len;
		src.read((char*)&len, sizeof(int));
		std::string ret;
		ret.resize(len);
		src.read((char*)ret.c_str(), len);
		return ret;
	}

	void bin_load(std::ifstream &src, SerializableNode *dst)
	{
		int att_cnt;
		src.read((char*)&att_cnt, sizeof(int));
		for (auto i = 0; i < att_cnt; i++)
		{
			auto name = load_file_string(src);
			auto value = load_file_string(src);
			dst->new_attr(name, value);
		}

		dst->set_value(load_file_string(src));

		int node_cnt;
		src.read((char*)&node_cnt, sizeof(int));
		for (auto i = 0; i < node_cnt; i++)
		{
			auto name = load_file_string(src);
			auto node = dst->new_node(name);
			bin_load(src, node);
		}
	}

	SerializableNode *SerializableNode::create_from_xml(const std::wstring &filename)
	{
		auto content = get_file_content(filename);
		if (!content.first)
			return nullptr;

		auto n = new SerializableNodePrivate;

		rapidxml::xml_document<> xml_doc;
		xml_doc.parse<0>(content.first.get());

		auto rn = xml_doc.first_node();
		n->name = rn->name();
		xml_load(rn, n);

		return n;
	}

	SerializableNode *SerializableNode::create_from_bin(const std::wstring &filename)
	{
		std::ifstream file(filename);
		if (!file.good())
			return nullptr;

		auto n = new SerializableNodePrivate;

		n->name = load_file_string(file);

		bin_load(file, n);

		return n;
	}

	void SerializableNode::destroy(SerializableNode *n)
	{
		delete (SerializableNodePrivate*)n;
	}

	SerializableNode *SerializableNode::serialize(UDT *u, void *src, int precision)
	{
		srand(time(0));

		std::vector<std::pair<void*, uint>> obj_table;

		auto n = create_obj_node(obj_table, src);
		n->serialize_RE(u, obj_table, src, precision);
		return n;
	}

	int typeinfo_collect_init()
	{
		return FAILED(CoInitialize(NULL));
	}

	static const char* name_base_type[] = {
		"<NoType>",                         // btNoType = 0,
		"void",                             // btVoid = 1,
		"char",                             // btChar = 2,
		"wchar_t",                          // btWChar = 3,
		"signed char",
		"unsigned char",
		"int",                              // btInt = 6,
		"unsigned int",                     // btUInt = 7,
		"float",                            // btFloat = 8,
		"<BCD>",                            // btBCD = 9,
		"bool",                              // btBool = 10,
		"short",
		"unsigned short",
		"long",                             // btLong = 13,
		"unsigned long",                    // btULong = 14,
		"__int8",
		"__int16",
		"__int32",
		"__int64",
		"__int128",
		"unsigned __int8",
		"unsigned __int16",
		"unsigned __int32",
		"unsigned __int64",
		"unsigned __int128",
		"<currency>",                       // btCurrency = 25,
		"<date>",                           // btDate = 26,
		"VARIANT",                          // btVariant = 27,
		"<complex>",                        // btComplex = 28,
		"<bit>",                            // btBit = 29,
		"BSTR",                             // btBSTR = 30,
		"HRESULT",                          // btHresult = 31
		"char16_t",                         // btChar16 = 32
		"char32_t"                          // btChar32 = 33
	};

	static std::string get_base_type_name(IDiaSymbol *s)
	{
		DWORD baseType;
		s->get_baseType(&baseType);
		ULONGLONG len;
		s->get_length(&len);
		std::string name;
		switch (baseType)
		{
		case btUInt:
			name = "u";
		case btInt:
			switch (len)
			{
			case 1:
				name += "char";
				return name;
			case 2:
				name += "short";
				return name;
			case 4:
				name += "int";
				return name;
			case 8:
				name += "longlong";
				return name;
			}
			break;
		case btFloat:
			switch (len)
			{
			case 4:
				return "float";
			case 8:
				return "double";
			}
			break;
		}
		return name_base_type[baseType];
	}

	void typeinfo_collect(const std::wstring &pdb_dir, const std::wstring &pdb_prefix)
	{
		auto pdb_prefix_len = wcslen(pdb_prefix.c_str());
		std::string prefix("flame::");
		std::wstring wprefix(s2w(prefix));

		for (std::filesystem::directory_iterator end, it(pdb_dir); it != end; it++)
		{
			if (!std::filesystem::is_directory(it->status()) && it->path().extension() == L".pdb")
			{
				auto fn = it->path().filename().wstring();

				if (fn.compare(0, pdb_prefix_len, pdb_prefix) == 0)
				{
					CComPtr<IDiaDataSource> dia_source;
					if (FAILED(CoCreateInstance(CLSID_DiaSource, NULL, CLSCTX_INPROC_SERVER, __uuidof(IDiaDataSource), (void**)&dia_source)))
					{
						printf("dia not found\n");
						continue;
					}
					if (FAILED(dia_source->loadDataFromPdb(fn.c_str())))
					{
						printf("pdb failed to open\n");
						continue;
					}
					CComPtr<IDiaSession> session;
					if (FAILED(dia_source->openSession(&session)))
					{
						printf("session failed to open\n");
						continue;
					}
					CComPtr<IDiaSymbol> global;
					if (FAILED(session->get_globalScope(&global)))
					{
						printf("failed to get global\n");
						continue;
					}

					auto dll_path = ext_replace(fn, L".dll");

					LONG l;
					ULONG ul;
					ULONGLONG ull;
					IDiaEnumSymbols *symbols;
					IDiaSymbol *symbol;
					DWORD dw;
					wchar_t *pwname;
					std::regex reg_str("^" + prefix + R"(BasicString<(char|wchar_t)>)");
					std::regex reg_arr("^" + prefix + R"(Array<([\w:\<\>]+)\s*(\*)?>)");
					std::regex reg_fun("^" + prefix + R"(Function<([\w:\<\>]+)\s*(\*)?>)");

					global->findChildren(SymTagEnum, NULL, nsNone, &symbols);
					while (SUCCEEDED(symbols->Next(1, &symbol, &ul)) && (ul == 1))
					{
						symbol->get_name(&pwname);
						std::wstring wname(pwname);
						if (wname.compare(0, wprefix.size(), wprefix) == 0)
						{
							auto name = w2s(wname.c_str() + wprefix.size());
							if (name.find("unnamed", 0) != std::string::npos)
								continue;
							auto hash = H(name.c_str());
							if (enums.find(hash) == enums.end())
							{
								auto e = new EnumInfoPrivate;
								e->name = name;

								IDiaEnumSymbols *items;
								symbol->findChildren(SymTagNull, NULL, nsNone, &items);
								IDiaSymbol *item;
								while (SUCCEEDED(items->Next(1, &item, &ul)) && (ul == 1))
								{
									VARIANT v;
									ZeroMemory(&v, sizeof(v));
									item->get_name(&pwname);
									item->get_value(&v);

									auto i = new EnumItemPrivate;
									i->name = w2s(pwname);
									i->value = v.lVal;
									e->items.emplace_back(i);

									item->Release();
								}
								items->Release();

								enums.emplace(hash, e);
							}
						}
						symbol->Release();
					}
					symbols->Release();

					global->findChildren(SymTagUDT, NULL, nsNone, &symbols);
					while (SUCCEEDED(symbols->Next(1, &symbol, &ul)) && (ul == 1))
					{
						symbol->get_name(&pwname);
						std::wstring wname(pwname);
						if (wname.compare(0, wprefix.size(), wprefix) == 0)
						{
							auto udt_name = w2s(wname.c_str() + wprefix.size());

							IDiaEnumSymbols *bases;
							symbol->findChildren(SymTagBaseClass, NULL, nsNone, &bases);
							if (SUCCEEDED(bases->get_Count(&l)) && l == 1)
							{
								IDiaSymbol *base;
								bases->Item(0, &base);
								base->get_name(&pwname);
								base->Release();
								std::wstring wname(pwname);
								if (wname == wprefix + L"R")
								{
									auto udt_namehash = H(udt_name.c_str());

									if (udts.find(udt_namehash) == udts.end())
									{
										symbol->get_length(&ull);
										auto udt = new UDTPrivate;
										udt->name = udt_name;
										udt->size = (int)ull;

										IDiaEnumSymbols *members;
										symbol->findChildren(SymTagData, NULL, nsNone, &members);
										IDiaSymbol *member;
										while (SUCCEEDED(members->Next(1, &member, &ul)) && (ul == 1))
										{
											member->get_name(&pwname);
											std::wstring wname(pwname);
											auto pos_$ = wname.find(L'$');
											if (pos_$ != std::wstring::npos)
											{
												auto attribute = w2s(wname.c_str() + pos_$ + 1);
												wname[pos_$] = 0;

												IDiaSymbol *type;
												member->get_type(&type);

												member->get_offset(&l);
												type->get_length(&ull);

												auto i = new VaribleInfoPrivate;
												i->name = w2s(wname);
												i->attribute = attribute;
												i->offset = l;
												i->size = (int)ull;
												memset(&i->default_value.fmt, 0, sizeof(CommonData::fmt));
												memset(&i->default_value.v, 0, sizeof(CommonData::v));

												type->get_symTag(&dw);
												switch (dw)
												{
												case SymTagEnum:
												{
													i->tag = attribute.find('m') != std::string::npos ? VariableTagEnumMulti : VariableTagEnumSingle;
													type->get_name(&pwname);
													auto type_name = w2s(pwname);
													if (type_name.compare(0, prefix.size(), prefix) == 0)
														type_name = type_name.c_str() + prefix.size();
													i->type_name = type_name;
												}
													break;
												case SymTagBaseType:
												{
													i->tag = VariableTagVariable;
													i->type_name = get_base_type_name(type);
												}
													break;
												case SymTagPointerType:
												{
													i->tag = VariableTagPointer;
													IDiaSymbol *point_type;
													type->get_type(&point_type);
													point_type->get_symTag(&dw);
													switch (dw)
													{
													case SymTagBaseType:
														i->type_name = get_base_type_name(point_type);
														break;
													case SymTagPointerType:
														assert(0);
														break;
													case SymTagUDT:
														point_type->get_name(&pwname);
														auto type_name = w2s(pwname);
														if (type_name.compare(0, prefix.size(), prefix) == 0)
															type_name = type_name.c_str() + prefix.size();
														i->type_name = type_name;
														break;
													}
													point_type->Release();
												}
													break;
												case SymTagUDT:
												{
													type->get_name(&pwname);
													auto type_name = w2s(pwname);
													std::smatch match;
													if (std::regex_search(type_name, match, reg_str))
													{
														i->tag = VariableTagVariable;
														if (match[1].str() == "char")
															type_name = "String";
														else
															type_name = "StringW";
													}
													else if (std::regex_search(type_name, match, reg_arr))
													{
														if (match[2].matched)
															i->tag = VariableTagArrayOfPointer;
														else
															i->tag = VariableTagArrayOfVariable;
														type_name = match[1].str().c_str();
														if (std::regex_search(type_name, match, reg_str))
														{
															if (match[1].str() == "char")
																type_name = "String";
															else
																type_name = "StringW";
														}
													}
													else
														i->tag = VariableTagVariable;
													if (type_name.compare(0, prefix.size(), prefix) == 0)
														type_name = type_name.c_str() + prefix.size();
													i->type_name = type_name;
												}
													break;
												}
												type->Release();

												i->type_hash = H(i->type_name.c_str());
												udt->items.emplace_back(i);
											}
											member->Release();
										}
										members->Release();

										std::wstring udt_name_no_ns; // no namespace
										{
											auto sp = string_split(udt_name, ':');
											udt_name_no_ns = s2w(sp.back());
										}

										IDiaEnumSymbols *functions;
										symbol->findChildren(SymTagFunction, NULL, nsNone, &functions);
										IDiaSymbol *function;
										while (SUCCEEDED(functions->Next(1, &function, &ul)) && (ul == 1))
										{
											function->get_name(&pwname);
											std::wstring wname(pwname);
											IDiaSymbol* function_type;
											function->get_type(&function_type);
											IDiaSymbol* return_type;
											function_type->get_type(&return_type);
											IDiaEnumSymbols *parameters;
											function->findChildren(SymTagFunctionArgType, NULL, nsNone, &parameters);
											if (SUCCEEDED(parameters->get_Count(&l)))
											{
												auto parameters_count = l;
												if (wname == udt_name_no_ns && parameters_count == 0)
												{
													// a ctor func is a func that its name equals its class's name and its count of parameters is 0
													// we get the ctor func and try to run it at a dummy memory to get the default value of the class

													function->get_relativeVirtualAddress(&dw);
													if (dw)
													{
														auto new_obj = malloc(udt->size);
														run_module_function(dll_path.c_str(), (void*)dw, new_obj);
														for (auto &i : udt->items)
														{
															if (i->size <= sizeof(CommonData::v))
																memcpy(&i->default_value.v, (char*)new_obj + i->offset, i->size);
														}
														free(new_obj);
													}
												}
												else if (wname == L"update")
												{
													DWORD baseType;
													return_type->get_baseType(&baseType);
													if (baseType == btVoid)
													{
														function->get_relativeVirtualAddress(&dw);
														if (dw)
														{
															udt->update_function_rva = (void*)dw;
															udt->update_function_module_name = dll_path;
														}
													}
												}
											}
											function_type->Release();
											return_type->Release();
											parameters->Release();

											function->Release();
										}
										functions->Release();

										udts.emplace(udt_namehash, udt);
									}
								}
							}
							bases->Release();
						}
						symbol->Release();
					}
					symbols->Release();
				}
			}
		}
	}

	void typeinfo_load(const std::wstring &filename)
	{
		auto file = SerializableNode::create_from_xml(filename);
		if (!file)
			return;

		auto n_enums = file->find_node("enums");
		for (auto i = 0; i < n_enums->node_count(); i++)
		{
			auto n_enum = n_enums->node(i);
			if (n_enum->name() == "enum")
			{
				auto e = new EnumInfoPrivate;
				e->name = n_enum->find_attr("name")->value();

				for (auto j = 0; j < n_enum->node_count(); j++)
				{
					auto n_item = n_enum->node(j);
					if (n_item->name() == "item")
					{
						auto i = new EnumItemPrivate;
						i->name = n_item->find_attr("name")->value();
						i->value = std::stoi(n_item->find_attr("value")->value());
						e->items.emplace_back(i);
					}
				}

				enums.emplace(H(e->name.c_str()), e);
			}
		}

		auto n_serializables = file->find_node("udts");
		for (auto i = 0; i < n_serializables->node_count(); i++)
		{
			auto n_udt = n_serializables->node(i);
			if (n_udt->name() == "udt")
			{
				auto u = new UDTPrivate;
				u->name = n_udt->find_attr("name")->value();
				u->size = std::stoi(n_udt->find_attr("size")->value());

				for (auto j = 0; j < n_udt->node_count(); j++)
				{
					auto n_item = n_udt->node(j);
					if (n_item->name() == "item")
					{
						auto tag = n_item->find_attr("tag")->value();
						auto e_tag = 0;
						for (auto s : tag_names)
						{
							if (tag == s)
								break;
							e_tag++;
						}

						auto i = new VaribleInfoPrivate;
						i->tag = (VariableTag)e_tag;
						i->type_name = n_item->find_attr("type")->value();
						i->type_hash = H(i->type_name.c_str());
						i->name = n_item->find_attr("name")->value();
						i->attribute = n_item->find_attr("attribute")->value();
						i->offset = std::stoi(n_item->find_attr("offset")->value());
						i->size = std::stoi(n_item->find_attr("size")->value());
						auto a_default_value = n_item->find_attr("default_value");
						if (a_default_value)
							i->unserialize_default_value(a_default_value->value());
						u->items.emplace_back(i);
					}
				}

				udts.emplace(H(u->name.c_str()), u);
			}
		}

		SerializableNode::destroy(file);
	}

	void typeinfo_save(const std::wstring &filename)
	{
		auto file = SerializableNode::create("typeinfo");

		auto n_enums = file->new_node("enums");
		for (auto &e : enums)
		{
			auto n_enum = n_enums->new_node("enum");
			n_enum->new_attr("name", e.second->name);

			for (auto &i : e.second->items)
			{
				auto n_item = n_enum->new_node("item");
				n_item->new_attr("name", i->name);
				n_item->new_attr("value", std::to_string(i->value));
			}
		}

		auto n_serializables = file->new_node("udts");
		for (auto &u : udts)
		{
			auto n_udt = n_serializables->new_node("udt");
			n_udt->new_attr("name", u.second->name);
			n_udt->new_attr("size", std::to_string(u.second->size));

			for (auto &i : u.second->items)
			{
				auto n_item = n_udt->new_node("item");
				n_item->new_attr("tag", get_variable_tag_name(i->tag));
				n_item->new_attr("type", i->type_name);
				n_item->new_attr("name", i->name);
				n_item->new_attr("attribute", i->attribute);
				n_item->new_attr("offset", std::to_string(i->offset));
				n_item->new_attr("size", std::to_string(i->size));
				auto default_value_str = i->serialize_default_value(1);
				if (default_value_str.size() > 0)
					n_item->new_attr("default_value", default_value_str);
			}

			if (u.second->update_function_rva)
			{
				auto n_update_function = n_udt->new_node("update_function");
				n_update_function->new_attr("module", w2s(u.second->update_function_module_name));
				n_update_function->new_attr("rva", to_string((uint)u.second->update_function_rva).v);
			}
		}

		file->save_xml(filename);
		SerializableNode::destroy(file);
	}

	void typeinfo_clear()
	{
		enums.clear();
		udts.clear();
	}
}
