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

#include <pugixml.hpp>
#include <nlohmann/json.hpp>

#include <Windows.h>
#include <dia2.h>
#include <atlbase.h>

namespace flame
{
	static const char* tag_names[] = {
		"enum_single",
		"enum_multi",
		"varible",
		"pointer"
	};

	const char* get_type_tag_name(TypeTag tag)
	{
		return tag_names[tag];
	}

	struct TypeInfoPrivate : TypeInfo
	{
		TypeTag tag;
		std::string name;
		uint name_hash;
	};

	TypeTag TypeInfo::tag() const
	{
		return ((TypeInfoPrivate*)this)->tag;
	}

	const char* TypeInfo::name() const
	{
		return ((TypeInfoPrivate*)this)->name.c_str();
	}

	uint TypeInfo::name_hash() const
	{
		return ((TypeInfoPrivate*)this)->name_hash;
	}

	struct EnumItemPrivate : EnumItem
	{
		std::string name;
		int value;
	};

	const char* EnumItem::name() const
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

	const char* EnumInfo::name() const
	{
		return ((EnumInfoPrivate*)this)->name.c_str();
	}

	int EnumInfo::item_count() const
	{
		return ((EnumInfoPrivate*)this)->items.size();
	}

	EnumItem* EnumInfo::item(int idx) const
	{
		return ((EnumInfoPrivate*)this)->items[idx].get();
	}

	EnumItem* EnumInfo::find_item(const char* name, int *out_idx) const
	{
		auto& items = ((EnumInfoPrivate*)this)->items;
		for (auto i = 0; i < items.size(); i++)
		{
			if (items[i]->name == name)
			{
				if (out_idx)
					* out_idx = i;
				return items[i].get();
			}
		}
		if (out_idx)
			* out_idx = -1;
		return nullptr;
	}

	EnumItem* EnumInfo::find_item(int value, int* out_idx) const
	{
		auto& items = ((EnumInfoPrivate*)this)->items;
		for (auto i = 0; i < items.size(); i++)
		{
			if (items[i]->value == value)
			{
				if (out_idx)
					* out_idx = i;
				return items[i].get();
			}
		}
		if (out_idx)
			* out_idx = -1;
		return nullptr;
	}

	String EnumInfo::serialize_value(bool single, int v) const
	{
		return ((EnumInfoPrivate*)this)->serialize_value(single, v);
	}

	static String to_string(uint type_hash, const void* src, int precision)
	{
		switch (type_hash)
		{
		case cH("bool"):
			return *(bool*)src ? "true" : "false";
		case cH("uint"):
			return to_string(*(uint*)src);
		case cH("int"): case cH("i"):
			return to_string(*(int*)src);
		case cH("Ivec2"): case cH("i2"):
			return to_string(*(Ivec2*)src);
		case cH("Ivec3"): case cH("i3"):
			return to_string(*(Ivec3*)src);
		case cH("Ivec4"): case cH("i4"):
			return to_string(*(Ivec4*)src);
		case cH("float"): case cH("f"):
			return to_string(*(float*)src, precision);
		case cH("Vec2"): case cH("f2"):
			return to_string(*(Vec2*)src, precision);
		case cH("Vec3"): case cH("f3"):
			return to_string(*(Vec3*)src, precision);
		case cH("Vec4"): case cH("f4"):
			return to_string(*(Vec4*)src, precision);
		case cH("uchar"): case cH("b"):
			return to_string(*(uchar*)src);
		case cH("Bvec2"): case cH("b2"):
			return to_string(*(Bvec2*)src);
		case cH("Bvec3"): case cH("b3"):
			return to_string(*(Bvec3*)src);
		case cH("Bvec4"): case cH("b4"):
			return to_string(*(Bvec4*)src);
		case cH("String"):
			return *(String*)src;
		case cH("StringW"):
			return w2s(((StringW*)src)->v);
		case cH("StringAndHash"):
			return *(StringAndHash*)src;
		}

		return "";
	}

	static void from_string(uint type_hash, const std::string& str, void* dst)
	{
		switch (type_hash)
		{
		case cH("bool"):
			*(bool*)dst = str == "true" ? true : false;
			break;
		case cH("uint"):
			*(uint*)dst = stou1(str.c_str());
			break;
		case cH("int"): case cH("i"):
			*(int*)dst = stoi1(str.c_str());
			break;
		case cH("Ivec2"): case cH("i2"):
			*(Ivec2*)dst = stoi2(str.c_str());
			break;
		case cH("Ivec3"): case cH("i3"):
			*(Ivec3*)dst = stoi3(str.c_str());
			break;
		case cH("Ivec4"): case cH("i4"):
			*(Ivec4*)dst = stoi4(str.c_str());
			break;
		case cH("float"): case cH("f"):
			*(float*)dst = stof1(str.c_str());
			break;
		case cH("Vec2"): case cH("f2"):
			*(Vec2*)dst = stof2(str.c_str());
			break;
		case cH("Vec3"): case cH("f3"):
			*(Vec3*)dst = stof3(str.c_str());
			break;
		case cH("Vec4"): case cH("f4"):
			*(Vec4*)dst = stof4(str.c_str());
			break;
		case cH("uchar"): case cH("b"):
			*(uchar*)dst = stob1(str.c_str());
			break;
		case cH("Bvec2"): case cH("b2"):
			*(Bvec2*)dst = stob2(str.c_str());
			break;
		case cH("Bvec3"): case cH("b3"):
			*(Bvec3*)dst = stob3(str.c_str());
			break;
		case cH("Bvec4"): case cH("b4"):
			*(Bvec4*)dst = stob4(str.c_str());
			break;
		case cH("String"):
			*(String*)dst = str;
			break;
		case cH("StringW"):
			*(StringW*)dst = s2w(str);
			break;
		case cH("StringAndHash"):
			*(StringAndHash*)dst = str;
			break;
		}
	}

	struct VariableInfoPrivate : VariableInfo
	{
		TypeInfoPrivate type;
		std::string name;
		std::string attribute;
		int offset, size, count;
		CommonData default_value;

		inline VariableInfoPrivate()
		{
			default_value.fmt[0] = 0;
			default_value.fmt[1] = 0;
			default_value.fmt[2] = 0;
			default_value.fmt[3] = 0;
		}
	};

	const TypeInfo* VariableInfo::type() const
	{
		return &(((VariableInfoPrivate*)this)->type);
	}

	const char* VariableInfo::name() const
	{
		return ((VariableInfoPrivate*)this)->name.c_str();
	}

	int VariableInfo::offset() const
	{
		return ((VariableInfoPrivate*)this)->offset;
	}

	int VariableInfo::size() const
	{
		return ((VariableInfoPrivate*)this)->size;
	}

	const char* VariableInfo::attribute() const
	{
		return ((VariableInfoPrivate*)this)->attribute.c_str();
	}

	const CommonData& VariableInfo::default_value() const
	{
		return ((VariableInfoPrivate*)this)->default_value;
	}

	void get(TypeTag tag, int size, const void* src, CommonData* dst)
	{
		switch (tag)
		{
		case TypeTagEnumSingle: case TypeTagEnumMulti:
			dst->i1() = *(int*)src;
			break;
		case TypeTagVariable:
			memcpy(&dst->v, src, size);
			break;
		case TypeTagPointer:
			dst->p() = *(void**)src;
			break;
		}
	}

	void set(TypeTag tag, int size, const CommonData* src, void* dst)
	{
		switch (tag)
		{
		case TypeTagEnumSingle: case TypeTagEnumMulti:
			*(int*)dst = src->v.i[0];
			break;
		case TypeTagVariable:
			memcpy(dst, &src->v, size);
			break;
		case TypeTagPointer:
			*(void**)dst = src->v.p;
			break;
		}
	}

	bool compare(TypeTag tag, int size, const void* src, const void* dst)
	{
		switch (tag)
		{
		case TypeTagEnumSingle: case TypeTagEnumMulti:
			return *(int*)src == *(int*)dst;
		case TypeTagVariable:
			return memcmp(src, dst, size) == 0;
		}

		return false;
	}

	String serialize_value(TypeTag tag, uint type_hash, const void* src, int precision)
	{
		switch (tag)
		{
		case TypeTagEnumSingle:
			return find_enum(type_hash)->serialize_value(true, *(int*)src);
		case TypeTagEnumMulti:
			break;
		case TypeTagVariable:
			return to_string(type_hash, src, precision);
		}

		return "";
	}

	void unserialize_value(TypeTag tag, uint type_hash, const std::string & str, void* dst)
	{
		switch (tag)
		{
		case TypeTagEnumSingle:
			find_enum(type_hash)->find_item(str.c_str(), (int*)dst);
			break;
		case TypeTagVariable:
			from_string(type_hash, str, dst);
			break;
		}
	}

	struct FunctionInfoPrivate : FunctionInfo
	{
		std::string name;
		void* rva;
		TypeInfoPrivate return_type;
		std::vector<TypeInfoPrivate> parameter_types;
		std::string code;
	};

	const char* FunctionInfo::name() const
	{
		return ((FunctionInfoPrivate*)this)->name.c_str();
	}

	void* FunctionInfo::rva() const
	{
		return ((FunctionInfoPrivate*)this)->rva;
	}

	const TypeInfo* FunctionInfo::return_type() const
	{
		return &(((FunctionInfoPrivate*)this)->return_type);
	}

	int FunctionInfo::parameter_count() const
	{
		return ((FunctionInfoPrivate*)this)->parameter_types.size();
	}

	const TypeInfo* FunctionInfo::parameter_type(int idx) const
	{
		return &(((FunctionInfoPrivate*)this)->parameter_types[idx]);
	}

	const char* FunctionInfo::code() const
	{
		return ((FunctionInfoPrivate*)this)->code.c_str();
	}

	struct UdtInfoPrivate : UdtInfo
	{
		std::string name;
		int size;
		std::wstring module_name;
		std::vector<std::unique_ptr<VariableInfoPrivate>> items;
		std::vector<std::unique_ptr<FunctionInfoPrivate>> functions;

		int item_find_pos;
		int func_find_pos;

		inline UdtInfoPrivate()
		{
			item_find_pos = 0;
			func_find_pos = 0;
		}

		inline VariableInfoPrivate* find_item(const char* name, int *out_idx)
		{
			if (items.empty())
			{
				if (out_idx)
					* out_idx = -1;
				return nullptr;
			}

			auto p = item_find_pos;
			while (true)
			{
				auto item = items[item_find_pos].get();
				if (item->name == name)
				{
					auto t = item_find_pos;
					item_find_pos++;
					if (item_find_pos >= items.size())
						item_find_pos = 0;
					if (out_idx)
						* out_idx = t;
					return item;
				}
				item_find_pos++;
				if (item_find_pos >= items.size())
					item_find_pos = 0;
				if (item_find_pos == p)
				{
					if (out_idx)
						* out_idx = -1;
					return nullptr;
				}
			}
			if (out_idx)
				* out_idx = -1;
			return nullptr;
		}

		inline FunctionInfoPrivate* find_func(const char* name, int* out_idx)
		{
			if (functions.empty())
			{
				if (out_idx)
					* out_idx = -1;
				return nullptr;
			}

			auto p = func_find_pos;
			while (true)
			{
				auto func = functions[func_find_pos].get();
				if (func->name == name)
				{
					auto t = func_find_pos;
					func_find_pos++;
					if (func_find_pos >= functions.size())
						func_find_pos = 0;
					if (out_idx)
						* out_idx = t;
					return func;
				}
				func_find_pos++;
				if (func_find_pos >= functions.size())
					func_find_pos = 0;
				if (func_find_pos == p)
				{
					if (out_idx)
						* out_idx = -1;
					return nullptr;
				}
			}
			if (out_idx)
				* out_idx = -1;
			return nullptr;
		}
	};

	const char* UdtInfo::name() const
	{
		return ((UdtInfoPrivate*)this)->name.c_str();
	}

	int UdtInfo::size() const
	{
		return ((UdtInfoPrivate*)this)->size;
	}

	const wchar_t* UdtInfo::module_name() const
	{
		return ((UdtInfoPrivate*)this)->module_name.c_str();
	}

	int UdtInfo::item_count() const
	{
		return ((UdtInfoPrivate*)this)->items.size();
	}

	VariableInfo* UdtInfo::item(int idx) const
	{
		return ((UdtInfoPrivate*)this)->items[idx].get();
	}

	VariableInfo* UdtInfo::find_item(const char* name, int *out_idx) const
	{
		return ((UdtInfoPrivate*)this)->find_item(name, out_idx);
	}

	int UdtInfo::function_count() const
	{
		return ((UdtInfoPrivate*)this)->functions.size();
	}

	FunctionInfo* UdtInfo::function(int idx) const
	{
		return ((UdtInfoPrivate*)this)->functions[idx].get();
	}

	FunctionInfo* UdtInfo::find_function(const char* name, int* out_idx) const
	{
		return ((UdtInfoPrivate*)this)->find_func(name, out_idx);
	}

	static std::map<unsigned int, std::unique_ptr<EnumInfoPrivate>> enums;
	static std::map<unsigned int, std::unique_ptr<UdtInfoPrivate>> udts;
	static std::map<unsigned int, std::unique_ptr<FunctionInfoPrivate>> functions;

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

	EnumInfo* find_enum(unsigned int name_hash)
	{
		auto it = enums.find(name_hash);
		return it == enums.end() ? nullptr : it->second.get();
	}

	Array<FunctionInfo*> get_functions()
	{
		Array<FunctionInfo*> ret;
		ret.resize(functions.size());
		auto i = 0;
		for (auto it = functions.begin(); it != functions.end(); it++)
		{
			ret[i] = (*it).second.get();
			i++;
		}
		return ret;
	}

	FunctionInfo* find_funcion(unsigned int name_hash)
	{
		auto it = functions.find(name_hash);
		return it == functions.end() ? nullptr : it->second.get();
	}

	Array<UdtInfo*> get_udts()
	{
		Array<UdtInfo*> ret;
		ret.resize(udts.size());
		auto i = 0;
		for (auto it = udts.begin(); it != udts.end(); it++)
		{
			ret[i] = (*it).second.get();
			i++;
		}
		return ret;
	}

	UdtInfo* find_udt(unsigned int name_hash)
	{
		auto it = udts.find(name_hash);
		return it == udts.end() ? nullptr : it->second.get();
	}

	struct SerializableAttributePrivate : SerializableAttribute
	{
		std::string name;
		std::string value;
	};

	const std::string& SerializableAttribute::name() const
	{
		return ((SerializableAttributePrivate*)this)->name;
	}

	const std::string& SerializableAttribute::value() const
	{
		return ((SerializableAttributePrivate*)this)->value;
	}

	void SerializableAttribute::set_name(const std::string & name)
	{
		((SerializableAttributePrivate*)this)->name = name;
	}

	void SerializableAttribute::set_value(const std::string & value)
	{
		((SerializableAttributePrivate*)this)->value = value;
	}

	static bool has_id(const std::vector<std::pair<void*, uint>> & obj_table, uint id)
	{
		for (auto& o : obj_table)
		{
			if (o.second == id)
				return true;
		}
		return false;
	}

	static uint generate_id(const std::vector<std::pair<void*, uint>> & obj_table)
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

	static SerializableNodePrivate* create_obj_node(std::vector<std::pair<void*, uint>> & table, void* obj)
	{
		auto n = SerializableNode::create("obj");
		auto id = generate_id(table);
		table.emplace_back(obj, id);
		n->new_attr("id", to_stdstring(id));
		return (SerializableNodePrivate*)n;
	}

	struct SerializableNodePrivate : SerializableNode
	{
		Type type;

		std::string name;
		std::string value;

		std::vector<std::unique_ptr<SerializableAttributePrivate>> attrs;
		std::vector<std::unique_ptr<SerializableNodePrivate>> nodes;
		SerializableNodePrivate* parent;

		int attr_find_pos;
		int node_find_pos;

		inline SerializableNodePrivate() :
			type(Object),
			parent(nullptr),
			attr_find_pos(0),
			node_find_pos(0)
		{
		}

		inline SerializableAttribute* new_attr(const std::string& name, const std::string& value)
		{
			return insert_attr(-1, name, value);
		}

		inline SerializableAttribute* insert_attr(int idx, const std::string& name, const std::string& value)
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

		inline void remove_attr(SerializableAttribute * a)
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

		inline SerializableAttribute* find_attr(const std::string & name)
		{
			if (attrs.empty())
				return nullptr;

			auto p = attr_find_pos;
			while (true)
			{
				auto attr = attrs[attr_find_pos].get();
				if (attr->name == name)
				{
					auto t = attr_find_pos;
					attr_find_pos++;
					if (attr_find_pos >= attrs.size())
						attr_find_pos = 0;
					return attr;
				}
				attr_find_pos++;
				if (attr_find_pos >= attrs.size())
					attr_find_pos = 0;
				if (attr_find_pos == p)
					return nullptr;
			}
			return nullptr;
		}

		inline void add_node(SerializableNodePrivate* n)
		{
			n->parent = this;
			nodes.emplace_back((SerializableNodePrivate*)n);
		}

		inline SerializableNode* new_node(const std::string & name)
		{
			return insert_node(-1, name);
		}

		inline SerializableNode* insert_node(int idx, const std::string & name)
		{
			if (idx == -1)
				idx = nodes.size();
			auto n = new SerializableNodePrivate;
			n->name = name;
			n->parent = this;
			nodes.emplace(nodes.begin() + idx, n);
			return n;
		}

		inline void remove_node(int idx)
		{
			nodes.erase(nodes.begin() + idx);
		}

		inline void remove_node(SerializableNode * n)
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

		inline SerializableNode* find_node(const std::string & name)
		{
			if (nodes.empty())
				return nullptr;

			auto p = node_find_pos;
			while (true)
			{
				auto node = nodes[node_find_pos].get();
				if (node->name == name)
				{
					auto t = node_find_pos;
					node_find_pos++;
					if (node_find_pos >= nodes.size())
						node_find_pos = 0;
					return node;
				}
				node_find_pos++;
				if (node_find_pos >= nodes.size())
					node_find_pos = 0;
				if (node_find_pos == p)
					return nullptr;
			}
			return nullptr;
		}

		void serialize(UdtInfo * u, void* src, int precision)
		{
			for (auto i = 0; i < u->item_count(); i++)
			{
				auto item = u->item(i);
				auto tag = item->type()->tag();
				auto hash = item->type()->name_hash();

				switch (tag)
				{
				case TypeTagVariable:
					switch (hash)
					{
					case cH("CommonData"):
					{
						auto d = (CommonData*)src;
						auto n_item = new_node("item");
						n_item->new_attr("name", item->name());
						n_item->new_attr("type", d->fmt);
						n_item->new_attr("value", to_string(H(d->fmt), &d->v, precision).v);
					}
						break;
					case cH("Function"):
						//const auto &f = arr[i_i];
						//auto r = find_registered_function(0, f.pf);

						//auto n_fn = n_item->new_node("function");
						//n_fn->new_attr("id", to_stdstring(r->id));

						//auto d = f.p.d + r->parameter_count;
						//for (auto i = 0; i < f.capture_count; i++)
						//{
						//	auto n_cpt = n_fn->new_node("capture");
						//	std::string ty_str, vl_str;
						//	serialize_commondata(obj_table, precision, ty_str, vl_str, (CommonData*)d);
						//	n_cpt->new_attr("type", ty_str);
						//	n_cpt->new_attr("value", vl_str);
						//	d++;
						//}
						break;
					default:
						if (!compare(tag, hash, src, &item->default_value()))
						{
							auto n_item = new_node("item");
							n_item->new_attr("name", item->name());
							n_item->new_attr("value", serialize_value(tag, hash, src, precision).v);
						}
					}
					break;
				}
			}
		}

		void unserialize(UdtInfo * u, void* dst)
		{
			for (auto& n_item : nodes)
			{
				if (n_item->name != "item")
					continue;

				auto item = u->find_item(n_item->find_attr("name")->value().c_str());
				auto tag = item->type()->tag();
				auto hash = item->type()->name_hash();

				switch (tag)
				{
				case TypeTagVariable:
					switch (hash)
					{
					case cH("CommonData"):
					{
						auto d = (CommonData*)dst;
						typefmt_assign(d->fmt, n_item->find_attr("type")->value().c_str());
						from_string(H(d->fmt), n_item->find_attr("value")->value(), &d->v);
					}
						break;
					case cH("Function"):
						//auto n_i = n_item->node(i_i);

						//if (n_i->name() == "function")
						//{
						//	auto cpt_cnt = n_i->node_count();
						//	auto id = stoi(n_i->find_attr("id")->value());
						//	std::vector<CommonData> capts;
						//	capts.resize(cpt_cnt);

						//	for (auto i_c = 0; i_c < cpt_cnt; i_c++)
						//	{
						//		auto n_c = n_i->node(i_c);
						//		if (n_c->name() == "capture")
						//			unserialize_commondata(obj_table, n_c->find_attr("type")->value(), n_c->find_attr("value")->value(), &capts[i_c]);
						//		else
						//			assert(0);
						//	}

						//	auto r = find_registered_function(id, nullptr);
						//	if (!r)
						//		assert(0);

						//	arr[i_i] = Function<>();
						//	arr[i_i].set(r->pf, r->parameter_count, capts);
						//}
						//else
						//	assert(0);
						break;
					default:
						unserialize_value(tag, hash, n_item->find_attr("value")->value(), dst);
					}
					break;
				}
			}
		}
	};

	const std::string& SerializableNode::name() const
	{
		return ((SerializableNodePrivate*)this)->name;
	}

	const std::string& SerializableNode::value() const
	{
		return ((SerializableNodePrivate*)this)->value;
	}

	SerializableNode::Type SerializableNode::type() const
	{
		return ((SerializableNodePrivate*)this)->type;
	}

	void SerializableNode::set_type(Type type)
	{
		((SerializableNodePrivate*)this)->type = type;
	}

	void SerializableNode::set_name(const std::string & name)
	{
		((SerializableNodePrivate*)this)->name = name;
	}

	void SerializableNode::set_value(const std::string & value)
	{
		((SerializableNodePrivate*)this)->value = value;
	}

	SerializableAttribute* SerializableNode::new_attr(const std::string & name, const std::string & value)
	{
		return ((SerializableNodePrivate*)this)->new_attr(name, value);
	}

	SerializableAttribute* SerializableNode::insert_attr(int idx, const std::string & name, const std::string & value)
	{
		return ((SerializableNodePrivate*)this)->insert_attr(idx, name, value);
	}

	void SerializableNode::remove_attr(int idx)
	{
		((SerializableNodePrivate*)this)->remove_attr(idx);
	}

	void SerializableNode::remove_attr(SerializableAttribute * a)
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

	SerializableAttribute* SerializableNode::attr(int idx) const
	{
		return ((SerializableNodePrivate*)this)->attrs[idx].get();
	}

	SerializableAttribute* SerializableNode::find_attr(const std::string & name)
	{
		return ((SerializableNodePrivate*)this)->find_attr(name);
	}

	void SerializableNode::add_node(SerializableNode * n)
	{
		((SerializableNodePrivate*)this)->add_node((SerializableNodePrivate*)n);
	}

	SerializableNode* SerializableNode::new_node(const std::string & name)
	{
		return ((SerializableNodePrivate*)this)->new_node(name);
	}

	SerializableNode* SerializableNode::insert_node(int idx, const std::string & name)
	{
		return ((SerializableNodePrivate*)this)->insert_node(idx, name);
	}

	void SerializableNode::remove_node(int idx)
	{
		((SerializableNodePrivate*)this)->remove_node(idx);
	}

	void SerializableNode::remove_node(SerializableNode * n)
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

	SerializableNode* SerializableNode::node(int idx) const
	{
		return ((SerializableNodePrivate*)this)->nodes[idx].get();
	}

	SerializableNode* SerializableNode::find_node(const std::string & name)
	{
		return ((SerializableNodePrivate*)this)->find_node(name);
	}

	void to_xml(pugi::xml_node dst, SerializableNodePrivate * src)
	{
		for (auto& sa : src->attrs)
			dst.append_attribute(sa->name.c_str()).set_value(sa->value.c_str());

		for (auto& sn : src->nodes)
		{
			auto n = sn->type == SerializableNode::Cdata ? dst.append_child(pugi::node_pcdata) : dst.append_child(sn->name.c_str());
			n.set_value(sn->value.c_str());
			to_xml(n, sn.get());
		}
	}

	String SerializableNode::to_string_xml() const
	{
		pugi::xml_document doc;
		auto rn = doc.append_child(name().c_str());

		to_xml(rn, (SerializableNodePrivate*)this);

		struct xml_string_writer : pugi::xml_writer
		{
			std::string result;

			virtual void write(const void* data, size_t size)
			{
				result.append(static_cast<const char*>(data), size);
			}
		};
		xml_string_writer writer;
		doc.print(writer);

		return writer.result;
	}

	static void to_json(nlohmann::json::reference dst, SerializableNodePrivate* src)
	{
		if (src->type != SerializableNode::Array)
		{
			if (src->type == SerializableNode::Value && !src->attrs.empty())
				dst = src->attrs[0]->value;
			else
			{
				for (auto& sa : src->attrs)
					dst[sa->name] = sa->value;
			}

			for (auto& sn : src->nodes)
				to_json(dst[sn->name], sn.get());
		}
		else
		{
			for (auto i = 0; i < src->nodes.size(); i++)
				to_json(dst[i], src->nodes[i].get());
		}
	}

	String SerializableNode::to_string_json() const
	{
		nlohmann::json doc;

		to_json(doc, (SerializableNodePrivate*)this);

 		return doc.dump();
	}

	void SerializableNode::save_xml(const std::wstring & filename) const
	{
		pugi::xml_document doc;
		auto rn = doc.append_child(name().c_str());

		to_xml(rn, (SerializableNodePrivate*)this);

		doc.save_file(filename.c_str());
	}

	static void save_file_string(std::ofstream & dst, const std::string & src)
	{
		int len = src.size();
		dst.write((char*)& len, sizeof(int));
		dst.write((char*)src.c_str(), len);
	}

	void SerializableNode::serialize(UdtInfo* u, void* src, int precision)
	{
		((SerializableNodePrivate*)this)->serialize(u, src, precision);
	}

	void SerializableNode::unserialize(UdtInfo * u, void* dst)
	{
		((SerializableNodePrivate*)this)->unserialize(u, dst);
	}

	SerializableNode* SerializableNode::create(const std::string & name)
	{
		auto n = new SerializableNodePrivate;
		n->name = name;

		return n;
	}

	static void from_xml(pugi::xml_node src, SerializableNode * dst)
	{
		for (auto& a : src.attributes())
			dst->new_attr(a.name(), a.value());

		dst->set_value(src.value());

		for (auto& n : src.children())
		{
			auto node = dst->new_node(n.name());
			if (n.type() == pugi::node_cdata)
				node->set_type(SerializableNode::Cdata);
			from_xml(n, node);
		}
	}

	SerializableNode* SerializableNode::create_from_xml_string(const std::string & str)
	{
		pugi::xml_document doc;
		auto result = doc.load_string(str.c_str());
		if (!result)
			return nullptr;

		auto n = new SerializableNodePrivate;

		auto rn = doc.first_child();
		n->name = rn.name();
		from_xml(rn, n);

		return n;
	}

	SerializableNode* SerializableNode::create_from_xml_file(const std::wstring & filename)
	{
		pugi::xml_document doc;
		auto result = doc.load_file(filename.c_str());
		if (!result)
			return nullptr;

		auto n = new SerializableNodePrivate;

		auto rn = doc.first_child();
		n->name = rn.name();
		from_xml(rn, n);

		return n;
	}

	static void from_json(nlohmann::json::reference src, SerializableNode* dst)
	{
		if (src.is_object())
		{
			dst->set_type(SerializableNode::Object);
			for (auto it = src.begin(); it != src.end(); ++it)
			{
				auto node = dst->new_node(it.key());
				from_json(it.value(), node);
			}
		}
		else if (src.is_array())
		{
			dst->set_type(SerializableNode::Array);
			for (auto& n : src)
			{
				auto node = dst->new_node("");
				from_json(n, node);
			}
		}
		else
		{
			dst->set_type(SerializableNode::Value);
			dst->set_value(src.dump());
		}
	}

	SerializableNode* SerializableNode::create_from_json_string(const std::string& str)
	{
		auto doc = nlohmann::json::parse(str);

		auto n = new SerializableNodePrivate;

		from_json(doc, n);

		return n;
	}

	SerializableNode* SerializableNode::create_from_json_file(const std::wstring& filename)
	{
		return nullptr;
	}

	void SerializableNode::destroy(SerializableNode * n)
	{
		delete (SerializableNodePrivate*)n;
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

	static std::string base_type_name(IDiaSymbol * s)
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

	static std::string prefix("flame::");
	static std::regex reg_str("^" + prefix + R"(BasicString<(char|wchar_t)>)");
	static std::regex reg_fun("^" + prefix + R"(Function<([\w:\<\>]+)\s*(\*)?>)");

	std::string format_name(const wchar_t* in, std::string * attribute = nullptr, bool* pass_prefix = nullptr, bool* pass_$ = nullptr)
	{
		if (pass_prefix)
			* pass_prefix = false;
		if (pass_$)
			* pass_$ = false;

		auto name = w2s(in);
		if (name.compare(0, prefix.size(), prefix) == 0)
		{
			name.erase(0, prefix.size());

			if (pass_prefix)
				* pass_prefix = true;
		}
		auto pos_$ = name.find('$');
		if (pos_$ != std::wstring::npos)
		{
			if (attribute)
				* attribute = std::string(name.c_str() + pos_$ + 1);
			name[pos_$] = 0;

			if (pass_$)
				* pass_$ = true;
		}
		return name;
	}

	TypeInfoPrivate symbol_to_typeinfo(IDiaSymbol * symbol, const std::string & attribute)
	{
		DWORD dw;
		wchar_t* pwname;

		TypeInfoPrivate info;

		symbol->get_symTag(&dw);
		switch (dw)
		{
		case SymTagEnum:
		{
			info.tag = attribute.find('m') != std::string::npos ? TypeTagEnumMulti : TypeTagEnumSingle;
			symbol->get_name(&pwname);
			auto type_name = format_name(pwname);
			info.name = type_name;
		}
		break;
		case SymTagBaseType:
		{
			info.tag = TypeTagVariable;
			info.name = base_type_name(symbol);
		}
		break;
		case SymTagPointerType:
		{
			info.tag = TypeTagPointer;
			IDiaSymbol* point_type;
			symbol->get_type(&point_type);
			point_type->get_symTag(&dw);
			switch (dw)
			{
			case SymTagBaseType:
				info.name = base_type_name(point_type);
				break;
			case SymTagPointerType:
				assert(0);
				break;
			case SymTagUDT:
				point_type->get_name(&pwname);
				auto type_name = format_name(pwname);
				info.name = type_name;
				break;
			}
			point_type->Release();
		}
		break;
		case SymTagUDT:
		{
			symbol->get_name(&pwname);
			auto type_name = format_name(pwname);
			std::smatch match;
			if (std::regex_search(type_name, match, reg_str))
			{
				info.tag = TypeTagVariable;
				if (match[1].str() == "char")
					type_name = "String";
				else
					type_name = "StringW";
			}
			else
				info.tag = TypeTagVariable;
			info.name = type_name;
		}
		break;
		case SymTagFunctionArgType:
		{
			IDiaSymbol* type;
			symbol->get_type(&type);
			info = symbol_to_typeinfo(type, "");
			type->Release();
		}
		break;
		}

		info.name_hash = H(info.name.c_str());

		return info;
	}

	void serialize_typeinfo(const TypeInfoPrivate & src, SerializableNode * dst)
	{
		dst->new_attr("tag", get_type_tag_name(src.tag));
		dst->new_attr("type", src.name);
	}

	TypeInfoPrivate unserialize_typeinfo(SerializableNode * src)
	{
		TypeInfoPrivate info;

		auto tag = src->find_attr("tag")->value();
		auto e_tag = 0;
		for (auto s : tag_names)
		{
			if (tag == s)
				break;
			e_tag++;
		}

		info.tag = (TypeTag)e_tag;
		info.name = src->find_attr("type")->value();
		info.name_hash = H(info.name.c_str());

		return info;
	}

	void symbol_to_function(IDiaSymbol * symbol, FunctionInfoPrivate * f, const std::string & attribute, CComPtr<IDiaSession> & session, std::map<DWORD, std::vector<std::string>> & source_files, const std::string & tab_str1, const std::string & tab_str2)
	{
		ULONG ul;
		ULONGLONG ull;
		DWORD dw;

		IDiaSymbol* function_type;
		symbol->get_type(&function_type);

		IDiaEnumSymbols* parameters;
		function_type->findChildren(SymTagFunctionArgType, NULL, nsNone, &parameters);

		IDiaSymbol* return_type;
		function_type->get_type(&return_type);

		symbol->get_relativeVirtualAddress(&dw);
		f->rva = (void*)dw;
		f->return_type = symbol_to_typeinfo(return_type, "");

		IDiaSymbol* parameter;
		while (SUCCEEDED(parameters->Next(1, &parameter, &ul)) && (ul == 1))
		{
			f->parameter_types.push_back(symbol_to_typeinfo(parameter, ""));

			parameter->Release();
		}

		if (f->rva && attribute.find('c') != std::string::npos)
		{
			symbol->get_length(&ull);
			IDiaEnumLineNumbers* lines;

			if (SUCCEEDED(session->findLinesByRVA(dw, (DWORD)ull, &lines)))
			{
				IDiaLineNumber* line;
				DWORD src_file_id = -1;
				DWORD line_num;

				int min_line = 1000000;
				int max_line = 0;

				while (SUCCEEDED(lines->Next(1, &line, &ul)) && (ul == 1))
				{
					if (src_file_id == -1)
					{
						line->get_sourceFileId(&src_file_id);

						if (source_files.find(src_file_id) == source_files.end())
						{
							BSTR filename;
							IDiaSourceFile* source_file;
							line->get_sourceFile(&source_file);
							source_file->get_fileName(&filename);
							source_file->Release();

							auto& vec = (source_files[src_file_id] = std::vector<std::string>());
							vec.push_back("\n");

							std::ifstream file(filename);
							if (file.good())
							{
								while (!file.eof())
								{
									std::string line;
									std::getline(file, line);

									vec.push_back(line + "\n");
								}
								file.close();
							}
						}
					}

					line->get_lineNumber(&line_num);
					if (min_line > line_num)
						min_line = line_num;
					if (max_line < line_num)
						max_line = line_num;

					line->Release();
				}

				lines->Release();

				if (max_line > min_line)
				{
					f->code = "\n";

					for (auto i = min_line; i <= max_line; i++)
						f->code += tab_str1 + source_files[src_file_id][i];
					f->code += tab_str2;
				}
			}
		}

		return_type->Release();
		parameters->Release();
		function_type->Release();
	}

	void serialize_function(FunctionInfoPrivate * src, SerializableNode * dst)
	{
		dst->new_attr("name", src->name);
		dst->new_attr("rva", to_string((uint)src->rva).v);
		auto n_return_type = dst->new_node("return_type");
		serialize_typeinfo(src->return_type, n_return_type);
		if (!src->parameter_types.empty())
		{
			auto n_parameters = dst->new_node("parameters");
			for (auto& p : src->parameter_types)
			{
				auto n_parameter = n_parameters->new_node("parameter");
				serialize_typeinfo(p, n_parameter);
			}
		}
		if (src->code.length() > 0)
			dst->new_node("code")->set_value(src->code);
	}

	void unserialize_function(SerializableNode * src, FunctionInfoPrivate * dst)
	{
		dst->name = src->find_attr("name")->value();
		dst->rva = (void*)stou1(src->find_attr("rva")->value().c_str());
		dst->return_type = unserialize_typeinfo(src->find_node("return_type"));
		auto n_parameters = src->find_node("parameters");
		if (n_parameters)
		{
			for (auto k = 0; k < n_parameters->node_count(); k++)
			{
				auto n_parameter = n_parameters->node(k);
				if (n_parameter->name() == "parameter")
					dst->parameter_types.push_back(unserialize_typeinfo(n_parameter));
			}
		}
		auto n_code = src->find_node("code");
		if (n_code)
			dst->code = n_code->value();
	}

	void typeinfo_collect(const std::wstring & filename)
	{
		CComPtr<IDiaDataSource> dia_source;
		if (FAILED(CoCreateInstance(CLSID_DiaSource, NULL, CLSCTX_INPROC_SERVER, __uuidof(IDiaDataSource), (void**)& dia_source)))
		{
			printf("dia not found\n");
			return;
		}
		if (FAILED(dia_source->loadDataFromPdb(ext_replace(filename, L".pdb").c_str())))
		{
			printf("pdb failed to open\n");
			return;
		}
		CComPtr<IDiaSession> session;
		if (FAILED(dia_source->openSession(&session)))
		{
			printf("session failed to open\n");
			return;
		}

		CComPtr<IDiaSymbol> global;
		if (FAILED(session->get_globalScope(&global)))
		{
			printf("failed to get global\n");
			return;
		}

		LONG l;
		ULONG ul;
		ULONGLONG ull;
		DWORD dw;
		wchar_t* pwname;

		std::map<DWORD, std::vector<std::string>> source_files;

		// enums
		IDiaEnumSymbols* _enums;
		global->findChildren(SymTagEnum, NULL, nsNone, &_enums);
		IDiaSymbol* _enum;
		while (SUCCEEDED(_enums->Next(1, &_enum, &ul)) && (ul == 1))
		{
			_enum->get_name(&pwname);
			bool pass_prefix, pass_$;
			auto name = format_name(pwname, nullptr, &pass_prefix, &pass_$);
			if (pass_prefix && pass_$ && name.find("unnamed") == std::string::npos)
			{
				auto hash = H(name.c_str());
				if (enums.find(hash) == enums.end())
				{
					auto e = new EnumInfoPrivate;
					e->name = name;

					IDiaEnumSymbols* items;
					_enum->findChildren(SymTagNull, NULL, nsNone, &items);
					IDiaSymbol* item;
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
			_enum->Release();
		}
		_enums->Release();

		// udts
		IDiaEnumSymbols* _udts;
		global->findChildren(SymTagUDT, NULL, nsNone, &_udts);
		IDiaSymbol* _udt;
		while (SUCCEEDED(_udts->Next(1, &_udt, &ul)) && (ul == 1))
		{
			_udt->get_name(&pwname);
			bool pass_prefix, pass_$;
			auto udt_name = format_name(pwname, nullptr, &pass_prefix, &pass_$);
			if (pass_prefix && pass_$)
			{
				auto udt_namehash = H(udt_name.c_str());
				if (udts.find(udt_namehash) == udts.end())
				{
					_udt->get_length(&ull);
					auto udt = new UdtInfoPrivate;
					udt->name = udt_name;
					udt->size = (int)ull;
					udt->module_name = std::filesystem::path(filename).filename().wstring();

					IDiaEnumSymbols* members;
					_udt->findChildren(SymTagData, NULL, nsNone, &members);
					IDiaSymbol* member;
					while (SUCCEEDED(members->Next(1, &member, &ul)) && (ul == 1))
					{
						member->get_name(&pwname);
						std::string attribute;
						auto name = format_name(pwname, &attribute, &pass_prefix, &pass_$);
						if (pass_$)
						{
							IDiaSymbol* type;
							member->get_type(&type);

							auto i = new VariableInfoPrivate;
							i->name = name;
							i->attribute = attribute;
							member->get_offset(&l);
							i->offset = l;
							type->get_length(&ull);
							i->size = (int)ull;
							memset(&i->default_value, 0, sizeof(CommonData));

							i->type = symbol_to_typeinfo(type, attribute);
							type->Release();

							udt->items.emplace_back(i);
						}
						member->Release();
					}
					members->Release();

					IDiaEnumSymbols* _functions;
					_udt->findChildren(SymTagFunction, NULL, nsNone, &_functions);
					IDiaSymbol* _function;
					while (SUCCEEDED(_functions->Next(1, &_function, &ul)) && (ul == 1))
					{
						_function->get_name(&pwname);
						std::string attribute;
						auto name = format_name(pwname, &attribute, &pass_prefix, &pass_$);
						if (pass_$)
						{
							if (name[0] != '~')
							{
								if (name == udt_name)
								{
									IDiaSymbol* function_type;
									_function->get_type(&function_type);

									IDiaEnumSymbols* parameters;
									function_type->findChildren(SymTagFunctionArgType, NULL, nsNone, &parameters);

									if (SUCCEEDED(parameters->get_Count(&l)) && l == 0)
									{
										// a ctor func is a func that its name equals its class's name and its count of parameters is 0
										// we get the ctor func and try to run it at a dummy memory to get the default value of the class

										_function->get_relativeVirtualAddress(&dw);

										auto new_obj = malloc(udt->size);
										auto library = load_module(filename.c_str());
										if (library)
										{
											struct Dummy { };
											typedef void (Dummy:: * F)();
											union
											{
												void* p;
												F f;
											}cvt;
											cvt.p = (char*)library + (uint)dw;
											(*((Dummy*)new_obj).*cvt.f)();

											for (auto& i : udt->items)
											{
												if (i->size <= sizeof(CommonData::v))
													memcpy(&i->default_value.v, (char*)new_obj + i->offset, i->size);
											}
											free_module(library);
										}
										free(new_obj);
									}

									parameters->Release();
									function_type->Release();
								}
								else
								{
									auto f = new FunctionInfoPrivate;
									f->name = name;
									symbol_to_function(_function, f, attribute, session, source_files, "\t\t\t\t", "\t\t\t\t\t");

									udt->functions.emplace_back(f);
								}
							}
						}
						_function->Release();
					}
					_functions->Release();

					udts.emplace(udt_namehash, udt);
				}
			}
			_udt->Release();
		}
		_udts->Release();

		// functions
		IDiaEnumSymbols* _functions;
		global->findChildren(SymTagFunction, NULL, nsNone, &_functions);
		IDiaSymbol* _function;
		while (SUCCEEDED(_functions->Next(1, &_function, &ul)) && (ul == 1))
		{
			_function->get_name(&pwname);
			bool pass_prefix, pass_$;
			std::string attribute;
			auto name = format_name(pwname, &attribute, &pass_prefix, &pass_$);
			if (pass_prefix && pass_$ && name.find("::") == std::string::npos)
			{
				auto f = new FunctionInfoPrivate;
				f->name = name;
				symbol_to_function(_function, f, attribute, session, source_files, "\t\t", "\t\t\t");

				functions.emplace(H(f->name.c_str()), f);
			}

			_function->Release();
		}
		_functions->Release();
	}

	void typeinfo_load(const std::wstring & filename)
	{
		auto file = SerializableNode::create_from_xml_file(filename);
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

		auto n_udts = file->find_node("udts");
		for (auto i = 0; i < n_udts->node_count(); i++)
		{
			auto n_udt = n_udts->node(i);
			if (n_udt->name() == "udt")
			{
				auto u = new UdtInfoPrivate;
				u->name = n_udt->find_attr("name")->value();
				u->size = std::stoi(n_udt->find_attr("size")->value());
				u->module_name = s2w(n_udt->find_attr("module_name")->value());

				for (auto j = 0; j < n_udt->node_count(); j++)
				{
					auto n_item = n_udt->node(j);
					if (n_item->name() == "item")
					{
						auto i = new VariableInfoPrivate;
						i->type = unserialize_typeinfo(n_item);
						i->name = n_item->find_attr("name")->value();
						i->attribute = n_item->find_attr("attribute")->value();
						i->offset = std::stoi(n_item->find_attr("offset")->value());
						i->size = std::stoi(n_item->find_attr("size")->value());
						memset(&i->default_value, 0, sizeof(CommonData));
						auto a_default_value = n_item->find_attr("default_value");
						if (a_default_value)
							unserialize_value(i->type.tag, i->type.name_hash, a_default_value->value(), &i->default_value.v);
						u->items.emplace_back(i);
					}
				}

				auto n_functions = n_udt->find_node("functions");
				if (n_functions)
				{
					for (auto j = 0; j < n_functions->node_count(); j++)
					{
						auto n_function = n_functions->node(j);
						if (n_function->name() == "function")
						{
							auto f = new FunctionInfoPrivate;
							unserialize_function(n_function, f);
							u->functions.emplace_back(f);
						}
					}
				}

				udts.emplace(H(u->name.c_str()), u);
			}
		}

		auto n_functions = file->find_node("functions");
		for (auto i = 0; i < n_functions->node_count(); i++)
		{
			auto n_function = n_functions->node(i);
			if (n_function->name() == "function")
			{
				auto f = new FunctionInfoPrivate;
				unserialize_function(n_function, f);
				functions.emplace(H(f->name.c_str()), f);
			}
		}

		SerializableNode::destroy(file);
	}

	void typeinfo_save(const std::wstring & filename)
	{
		auto file = SerializableNode::create("typeinfo");

		auto n_enums = file->new_node("enums");
		std::map<std::string, EnumInfoPrivate*> _enums;
		for (auto& e : enums)
			_enums.emplace(e.second->name, e.second.get());
		for (auto& e : _enums)
		{
			auto n_enum = n_enums->new_node("enum");
			n_enum->new_attr("name", e.second->name);

			for (auto& i : e.second->items)
			{
				auto n_item = n_enum->new_node("item");
				n_item->new_attr("name", i->name);
				n_item->new_attr("value", std::to_string(i->value));
			}
		}

		auto n_udts = file->new_node("udts");
		std::map<std::string, UdtInfoPrivate*> _udts;
		for (auto& u : udts)
			_udts.emplace(u.second->name, u.second.get());
		for (auto& u : _udts)
		{
			auto n_udt = n_udts->new_node("udt");
			n_udt->new_attr("name", u.second->name);
			n_udt->new_attr("size", std::to_string(u.second->size));
			n_udt->new_attr("module_name", w2s(u.second->module_name));

			for (auto& i : u.second->items)
			{
				auto n_item = n_udt->new_node("item");
				serialize_typeinfo(i->type, n_item);
				n_item->new_attr("name", i->name);
				n_item->new_attr("attribute", i->attribute);
				n_item->new_attr("offset", std::to_string(i->offset));
				n_item->new_attr("size", std::to_string(i->size));
				if (i->type.name_hash != cH("String") && i->type.name_hash != cH("StringAndHash"))
				{
					auto default_value_str = serialize_value(i->type.tag, i->type.name_hash, &i->default_value.v, 1);
					if (default_value_str.size > 0)
						n_item->new_attr("default_value", default_value_str.v);
				}
			}

			auto n_functions = n_udt->new_node("functions");
			for (auto& f : u.second->functions)
			{
				auto n_function = n_functions->new_node("function");
				serialize_function(f.get(), n_function);
			}
		}

		auto n_functions = file->new_node("functions");
		std::map<std::string, FunctionInfoPrivate*> _functions;
		for (auto& f : functions)
			_functions.emplace(f.second->name, f.second.get());
		for (auto& f : _functions)
		{
			auto n_function = n_functions->new_node("function");
			serialize_function(f.second, n_function);
		}

		file->save_xml(filename);
		SerializableNode::destroy(file);
	}

	void typeinfo_clear()
	{
		enums.clear();
		udts.clear();
		functions.clear();
	}
}
