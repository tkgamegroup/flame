// MIT License
// 
// Copyright (c) 2019 wjs
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
		"variable",
		"pointer"
	};

	const char* get_type_tag_name(TypeTag$ tag)
	{
		return tag_names[tag];
	}

	struct TypeInfoPrivate : TypeInfo
	{
		TypeTag$ tag;
		std::string name;
		uint hash;
	};

	TypeTag$ TypeInfo::tag() const
	{
		return ((TypeInfoPrivate*)this)->tag;
	}

	const char* TypeInfo::name() const
	{
		return ((TypeInfoPrivate*)this)->name.c_str();
	}

	uint TypeInfo::hash() const
	{
		return ((TypeInfoPrivate*)this)->hash;
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
		std::wstring module_name;

		std::string name;
		std::vector<std::unique_ptr<EnumItemPrivate>> items;
	};

	const wchar_t* EnumInfo::module_name() const
	{
		return ((EnumInfoPrivate*)this)->module_name.c_str();
	}

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

	EnumItem* EnumInfo::find_item(const char* name, int* out_idx) const
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

	struct VariableInfoPrivate : VariableInfo
	{
		TypeInfoPrivate type;
		std::string name;
		std::string attribute;
		int offset, size;
		void* default_value;

		~VariableInfoPrivate()
		{
			delete default_value;
		}

		void _init_default_value()
		{
			default_value = nullptr;
			if (type.tag != TypeTagEnumSingle && type.tag != TypeTagEnumMulti && type.tag != TypeTagVariable)
				return;
			const std::string ignore_types[] = {
				"LNA",
				"Array",
				"String",
				"Function"
			};
			if (type.tag == TypeTagVariable)
			{
				for (auto& t : ignore_types)
				{
					if (type.name.compare(0, t.size(), t) == 0)
						return;
				}
			}
			default_value = new char[size];
			memset(default_value, 0, size);
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

	const void* VariableInfo::default_value() const
	{
		return ((VariableInfoPrivate*)this)->default_value;
	}

	void set(void* dst, TypeTag$ tag, int size, const void* src)
	{
		switch (tag)
		{
		case TypeTagEnumSingle: case TypeTagEnumMulti:
			*(int*)dst = *(int*)src;
			break;
		case TypeTagVariable:
			memcpy(dst, src, size);
			break;
		case TypeTagPointer:
			*(void**)dst = *(void**)src;
			break;
		}
	}

	bool compare(TypeTag$ tag, int size, const void* a, const void* b)
	{
		switch (tag)
		{
		case TypeTagEnumSingle: case TypeTagEnumMulti:
			return *(int*)a == *(int*)b;
		case TypeTagVariable:
			return memcmp(a, b, size) == 0;
		}

		return false;
	}

	String serialize_value(TypeTag$ tag, uint hash, const char* name, const void* src, int precision)
	{
		switch (tag)
		{
		case TypeTagEnumSingle:
			return find_enum(hash, name)->find_item(*(int*)src)->name();
		case TypeTagEnumMulti:
		{
			std::string ret;
			auto e = (EnumInfoPrivate*)find_enum(hash, name);
			auto v = *(int*)src;
			for (auto i = 0; i < e->items.size(); i++)
			{
				if ((v & 1) == 1)
				{
					if (!ret.empty())
						ret += ";";
					ret += e->find_item(1 << i)->name();
				}
				v >>= 1;
			}
			return ret;
		}
			break;
		case TypeTagVariable:
			switch (hash)
			{
			case cH("bool"):
				return *(bool*)src ? "1" : "0";
			case cH("uint"):
				return to_string(*(uint*)src);
			case cH("int"):
				return to_string(*(int*)src);
				//case cH("Ivec2"): case cH("i2"):
				//	return to_string(*(Ivec2*)src);
				//case cH("Ivec3"): case cH("i3"):
				//	return to_string(*(Ivec3*)src);
				//case cH("Ivec4"): case cH("i4"):
				//	return to_string(*(Ivec4*)src);
				//case cH("float"): case cH("f"):
				//	return to_string(*(float*)src, precision);
				//case cH("Vec2f"): case cH("f2"):
				//	return to_string(*(Vec2f*)src, precision);
				//case cH("Vec3f"): case cH("f3"):
				//	return to_string(*(Vec3f*)src, precision);
				//case cH("Vec4f"): case cH("f4"):
				//	return to_string(*(Vec4f*)src, precision);
				//case cH("uchar"): case cH("b"):
				//	return to_string(*(uchar*)src);
				//case cH("Vec2c"): case cH("b2"):
				//	return to_string(*(Vec2c*)src);
				//case cH("Vec3c"): case cH("b3"):
				//	return to_string(*(Vec3c*)src);
			case cH("Vec<4,uchar>"):
				return to_string(*(Vec4c*)src);
			case cH("String"):
				return ((String*)src)->v;
			case cH("StringW"):
				return w2s(((StringW*)src)->v);
			case cH("StringAndHash"):
				return ((StringAndHash*)src)->v;
			default:
				assert(0);
			}
		}

		return "";
	}

	void unserialize_value(TypeTag$ tag, uint hash, const char* name, const std::string& src, void* dst)
	{
		switch (tag)
		{
		case TypeTagEnumSingle:
			find_enum(hash, name)->find_item(src.c_str(), (int*)dst);
			break;
		case TypeTagEnumMulti:
		{
			auto v = 0;
			auto e = (EnumInfoPrivate*)find_enum(hash, name);
			auto sp = string_split(src, ';');
			for (auto& t : sp)
				v |= e->find_item(t.c_str())->value();
			*(int*)dst = v;
		}
			break;
		case TypeTagVariable:
			switch (hash)
			{
			case cH("bool"):
				*(bool*)dst = (src != "0");
				break;
			case cH("uint"):
				*(uint*)dst = std::stoul(src);
				break;
			case cH("int"):
				*(int*)dst = std::stoi(src);
				break;
				//case cH("Ivec2"): case cH("i2"):
				//	*(Ivec2*)dst = stoi2(src.c_str());
				//	break;
				//case cH("Ivec3"): case cH("i3"):
				//	*(Ivec3*)dst = stoi3(src.c_str());
				//	break;
				//case cH("Ivec4"): case cH("i4"):
				//	*(Ivec4*)dst = stoi4(src.c_str());
				//	break;
				//case cH("float"): case cH("f"):
				//	*(float*)dst = stof1(src.c_str());
				//	break;
				//case cH("Vec2f"): case cH("f2"):
				//	*(Vec2f*)dst = stof2(src.c_str());
				//	break;
				//case cH("Vec3f"): case cH("f3"):
				//	*(Vec3f*)dst = stof3(src.c_str());
				//	break;
				//case cH("Vec4f"): case cH("f4"):
				//	*(Vec4f*)dst = stof4(src.c_str());
				//	break;
				//case cH("uchar"): case cH("b"):
				//	*(uchar*)dst = stob1(src.c_str());
				//	break;
				//case cH("Vec2c"): case cH("b2"):
				//	*(Vec2c*)dst = stob2(src.c_str());
				//	break;
				//case cH("Vec3c"): case cH("b3"):
				//	*(Vec3c*)dst = stob3(src.c_str());
				//	break;
			case cH("Vec<4,uchar>"):
				*(Vec4c*)dst = stoi4(src.c_str());
				break;
			case cH("String"):
				*(String*)dst = src;
				break;
			case cH("StringW"):
				*(StringW*)dst = s2w(src);
				break;
			case cH("StringAndHash"):
				*(StringAndHash*)dst = src;
				break;
			default:
				assert(0);
			}
			break;
		}
	}

	struct FunctionInfoPrivate : FunctionInfo
	{
		std::wstring module_name;

		std::string name;
		void* rva;
		TypeInfoPrivate return_type;
		std::vector<TypeInfoPrivate> parameter_types;
		std::string code;
	};

	const wchar_t* FunctionInfo::module_name() const
	{
		return ((FunctionInfoPrivate*)this)->module_name.c_str();
	}

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
		std::wstring module_name;

		std::string name;
		int size;
		std::vector<std::unique_ptr<VariableInfoPrivate>> items;
		std::vector<std::unique_ptr<FunctionInfoPrivate>> functions;

		int item_find_pos;
		int func_find_pos;

		UdtInfoPrivate()
		{
			item_find_pos = 0;
			func_find_pos = 0;
		}

		VariableInfoPrivate* find_item(const char* name, int *out_idx)
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

		FunctionInfoPrivate* find_func(const char* name, int* out_idx)
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

	const wchar_t* UdtInfo::module_name() const
	{
		return ((UdtInfoPrivate*)this)->module_name.c_str();
	}

	const char* UdtInfo::name() const
	{
		return ((UdtInfoPrivate*)this)->name.c_str();
	}

	int UdtInfo::size() const
	{
		return ((UdtInfoPrivate*)this)->size;
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
		n->new_attr("id", std::to_string(id));
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

		SerializableNodePrivate() :
			type(Object),
			parent(nullptr),
			attr_find_pos(0),
			node_find_pos(0)
		{
		}

		SerializableAttribute* new_attr(const std::string& name, const std::string& value)
		{
			return insert_attr(-1, name, value);
		}

		SerializableAttribute* insert_attr(int idx, const std::string& name, const std::string& value)
		{
			if (idx == -1)
				idx = attrs.size();
			auto a = new SerializableAttributePrivate;
			a->name = name;
			a->value = value;
			attrs.emplace(attrs.begin() + idx, a);
			return a;
		}

		void remove_attr(int idx)
		{
			attrs.erase(attrs.begin() + idx);
		}

		void remove_attr(SerializableAttribute * a)
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

		void clear_attrs()
		{
			attrs.clear();
		}

		SerializableAttribute* find_attr(const std::string & name)
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

		void add_node(SerializableNodePrivate* n)
		{
			n->parent = this;
			nodes.emplace_back((SerializableNodePrivate*)n);
		}

		SerializableNode* new_node(const std::string & name)
		{
			return insert_node(-1, name);
		}

		SerializableNode* insert_node(int idx, const std::string & name)
		{
			if (idx == -1)
				idx = nodes.size();
			auto n = new SerializableNodePrivate;
			n->name = name;
			n->parent = this;
			nodes.emplace(nodes.begin() + idx, n);
			return n;
		}

		void remove_node(int idx)
		{
			nodes.erase(nodes.begin() + idx);
		}

		void remove_node(SerializableNode * n)
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

		void clear_nodes()
		{
			nodes.clear();
		}

		SerializableNode* find_node(const std::string & name)
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

		void serialize(UdtInfo* u, void* src, int precision)
		{
			for (auto i = 0; i < u->item_count(); i++)
			{
				auto item = u->item(i);
				auto type = item->type();

				switch (type->tag())
				{
				case TypeTagVariable:
					switch (type->hash())
					{
					case cH("Function"):

						break;
					default:
						if (item->default_value() && !compare(type->tag(), item->size(), src, item->default_value()))
						{
							auto n_item = new_node("item");
							n_item->new_attr("name", item->name());
							n_item->new_attr("value", serialize_value(type->tag(), type->hash(), type->name(), src, precision).v);
						}
					}
					break;
				}
			}
		}

		void unserialize(UdtInfo* u, void* dst)
		{
			for (auto& n_item : nodes)
			{
				if (n_item->name != "item")
					continue;

				auto item = u->find_item(n_item->find_attr("name")->value().c_str());
				auto type = item->type();

				switch (type->tag())
				{
				case TypeTagVariable:
					switch (type->hash())
					{
					case cH("Function"):

						break;
					default:
						unserialize_value(type->tag(), type->hash(), type->name(), n_item->find_attr("value")->value(), dst);
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
			pugi::xml_node n;
			switch (sn->type)
			{
			case SerializableNode::Cdata:
				n = dst.append_child(pugi::node_cdata);
				n.set_value(sn->value.c_str());
				break;
			case SerializableNode::Pcdata:
				n = dst.append_child(pugi::node_pcdata);
				n.set_value(sn->value.c_str());
				break;
			default:
				n = dst.append_child(sn->name.c_str());
			}
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
			else if (!src->value.empty())
				dst = src->value;
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

	void SerializableNode::save_json(const std::wstring& filename) const
	{
		std::ofstream file(filename);
		nlohmann::json doc;

		to_json(doc, (SerializableNodePrivate*)this);

		file << doc.dump(2);
		file.close();
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
			dst->set_value(src.is_string() ? src.get<std::string>() : src.dump());
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
		auto str = get_file_string(filename);
		if (str.empty())
			return nullptr;

		return create_from_json_string(str);
	}

	void SerializableNode::destroy(SerializableNode * n)
	{
		delete (SerializableNodePrivate*)n;
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
	static std::regex reg_temp(R"((\w)+\<(.)+\>)");
	static std::regex reg_usig(R"(\bunsigned\b)");
	static std::string str_func("Function");

	std::string format_name(const wchar_t* in, bool* pass_prefix = nullptr, bool* pass_$ = nullptr, std::string* attribute = nullptr)
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
			name.resize(pos_$);

			if (pass_$)
				* pass_$ = true;
		}
		if (name.compare(0, str_func.size(), str_func) == 0)
			name = "Function";
		name.erase(std::remove(name.begin(), name.end(), ' '));
		return name;
	}

	TypeInfoPrivate symbol_to_typeinfo(IDiaSymbol* symbol, const std::string& variable_attribute /* type varies with variable's attribute */)
	{
		DWORD dw;
		wchar_t* pwname;

		TypeInfoPrivate info;

		symbol->get_symTag(&dw);
		switch (dw)
		{
		case SymTagEnum:
		{
			info.tag = variable_attribute.find('m') != std::string::npos ? TypeTagEnumMulti : TypeTagEnumSingle;
			symbol->get_name(&pwname);
			info.name = format_name(pwname);
		}
			break;
		case SymTagBaseType:
		{
			info.tag = TypeTagVariable;
			info.name = base_type_name(symbol);
		}
			break;
		case SymTagPointerType: case SymTagArrayType:
		{
			info.tag = TypeTagPointer;
			IDiaSymbol* pointer_type;
			symbol->get_type(&pointer_type);
			pointer_type->get_symTag(&dw);
			switch (dw)
			{
			case SymTagBaseType:
				info.name = base_type_name(pointer_type);
				break;
			case SymTagPointerType:
				assert(0);
				break;
			case SymTagUDT:
				pointer_type->get_name(&pwname);
				info.name = format_name(pwname);
				break;
			}
			pointer_type->Release();
		}
			break;
		case SymTagUDT:
		{
			symbol->get_name(&pwname);
			info.tag = TypeTagVariable;
			info.name = format_name(pwname);
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
		
		info.hash = H(info.name.c_str());

		return info;
	}

	std::string serialize_typeinfo(const TypeInfoPrivate & src)
	{
		return std::string(get_type_tag_name(src.tag)) + ":" + src.name;
	}

	TypeInfoPrivate unserialize_typeinfo(const std::string& src)
	{
		TypeInfoPrivate info;

		auto sp = string_split(src, ':');

		auto e_tag = 0;
		for (auto s : tag_names)
		{
			if (sp[0] == s)
				break;
			e_tag++;
		}

		info.tag = (TypeTag$)e_tag;
		info.name = sp[1];
		info.hash = H(info.name.c_str());

		return info;
	}

	void symbol_to_function(IDiaSymbol* symbol, FunctionInfoPrivate* f, const std::string& attribute, CComPtr<IDiaSession> & session, std::map<DWORD, std::vector<std::string>>& source_files)
	{
		ULONG ul;
		ULONGLONG ull;
		DWORD dw;

		symbol->get_relativeVirtualAddress(&dw);
		f->rva = (void*)dw;

		IDiaSymbol* function_type;
		symbol->get_type(&function_type);

		IDiaEnumSymbols* parameters;
		function_type->findChildren(SymTagFunctionArgType, NULL, nsNone, &parameters);

		IDiaSymbol* return_type;
		function_type->get_type(&return_type);

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
						f->code += source_files[src_file_id][i];
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
		dst->new_attr("rva", std::to_string((uint)src->rva));
		dst->new_attr("return_type", serialize_typeinfo(src->return_type));
		if (!src->parameter_types.empty())
		{
			auto n_parameters = dst->new_node("parameters");
			n_parameters->set_type(SerializableNode::Array);
			for (auto& p : src->parameter_types)
			{
				auto n = n_parameters->new_node("");
				n->set_type(SerializableNode::Value);
				n->new_attr("", serialize_typeinfo(p));
			}
		}
		if (src->code.length() > 0)
			dst->new_attr("code", src->code);
	}

	void unserialize_function(SerializableNode * src, FunctionInfoPrivate * dst)
	{
		dst->name = src->find_node("name")->value();
		dst->rva = (void*)std::stoul(src->find_node("rva")->value().c_str());
		dst->return_type = unserialize_typeinfo(src->find_node("return_type")->value());
		auto n_parameters = src->find_node("parameters");
		if (n_parameters)
		{
			for (auto k = 0; k < n_parameters->node_count(); k++)
			{
				auto n = n_parameters->node(k);
				dst->parameter_types.push_back(unserialize_typeinfo(n->value()));
			}
		}
		auto n_code = src->find_node("code");
		if (n_code)
			dst->code = n_code->value();
	}

	struct TypeinfoDB
	{
		int level;
		std::map<uint, std::vector<std::unique_ptr<EnumInfoPrivate>>> enums;
		std::map<uint, std::vector<std::unique_ptr<UdtInfoPrivate>>> udts;
		std::map<uint, std::vector<std::unique_ptr<FunctionInfoPrivate>>> functions;
	};

	static std::vector<std::unique_ptr<TypeinfoDB>> typeinfo_dbs;

	TypeinfoDB* find_typeinfo_db(int level)
	{
		for (auto& db : typeinfo_dbs)
		{
			if (db->level == level)
				return db.get();
		}
		return nullptr;
	}

	template<class T, class U>
	Array<T*> get_typeinfo_objects(const std::map<uint, std::vector<std::unique_ptr<U>>>& map)
	{
		Array<T*> ret;
		ret.resize(map.size());
		auto i = 0;
		for (auto it = map.begin(); it != map.end(); it++)
		{
			for (auto& o : it->second)
				ret[i] = o.get();
			i++;
		}
		return ret;
	}

	template<class T>
	T* find_typeinfo_object(const std::map<uint, std::vector<std::unique_ptr<T>>>& map, uint name_hash, const char* name)
	{
		auto it = map.find(name_hash);
		if (it == map.end())
			return nullptr;
		if (it->second.size() == 1)
			return it->second.begin()->get();
		for (auto& i : it->second)
		{
			if (i->name == name)
				return i.get();
		}
		assert(0);
		return nullptr; // should not go here
	}

	Array<EnumInfo*> get_enums(int level)
	{
		auto db = find_typeinfo_db(level);
		return db ? get_typeinfo_objects<EnumInfo>(db->enums) : Array<EnumInfo*>();
	}

	EnumInfo* find_enum(uint name_hash, const char* name, int level)
	{
		if (level != -1)
		{
			auto db = find_typeinfo_db(level);
			return db ? find_typeinfo_object(db->enums, name_hash, name) : nullptr;
		}
		else
		{
			for (auto& db : typeinfo_dbs)
			{
				auto ret = find_typeinfo_object(db->enums, name_hash, name);
				if (ret)
					return ret;
			}
			return nullptr;
		}
	}

	Array<FunctionInfo*> get_functions(int level)
	{
		auto db = find_typeinfo_db(level);
		return db ? get_typeinfo_objects<FunctionInfo>(db->functions) : Array<FunctionInfo*>();
	}

	FunctionInfo* find_funcion(uint name_hash, const char* name, int level)
	{
		if (level != -1)
		{
			auto db = find_typeinfo_db(level);
			return db ? find_typeinfo_object(db->functions, name_hash, name) : nullptr;
		}
		else
		{
			for (auto& db : typeinfo_dbs)
			{
				auto ret = find_typeinfo_object(db->functions, name_hash, name);
				if (ret)
					return ret;
			}
			return nullptr;
		}
	}

	Array<UdtInfo*> get_udts(int level)
	{
		auto db = find_typeinfo_db(level);
		return db ? get_typeinfo_objects<UdtInfo>(db->udts) : Array<UdtInfo*>();
	}

	UdtInfo* find_udt(uint name_hash, const char* name, int level)
	{
		if (level != -1)
		{
			auto db = find_typeinfo_db(level);
			return db ? find_typeinfo_object(db->udts, name_hash, name) : nullptr;
		}
		else
		{
			for (auto& db : typeinfo_dbs)
			{
				auto ret = find_typeinfo_object(db->udts, name_hash, name);
				if (ret)
					return ret;
			}
			return nullptr;
		}
	}

	int typeinfo_free_level()
	{
		auto l = 0;
		while (true)
		{
			auto ok = true;
			for (auto& db : typeinfo_dbs)
			{
				if (db->level == l)
				{
					ok = false;
					break;
				}
			}
			if (ok)
				break;
			l++;
		}
		return l;
	}

	template<class T>
	void* pf2p(T f)
	{
		union
		{
			T f;
			void* p;
		}cvt;
		cvt.f = f;
		return cvt.p;
	}

	template<uint N, class T>
	struct BP_Vec
	{
		Vec<N, T> in;
		Vec<N, T> out;

		void update()
		{
			out = in;
		}
	};

	template<uint N, class T>
	void install_vec_udt(const char* name_suffix, const char* type_name)
	{
		auto u = new UdtInfoPrivate;
		u->name = "BP_Vec";
		u->name += name_suffix;
		u->size = sizeof(T) * N * 2 /* both in and out */;
		u->module_name = L"flame_foundation.dll";

		for (auto i = 0; i < N; i++)
		{
			auto v = new VariableInfoPrivate;
			v->type.tag = TypeTagVariable;
			v->type.name = type_name;
			v->type.hash = H(v->type.name.c_str());
			v->name = "xyzw"[i];
			v->attribute = "i";
			v->offset = sizeof(T) * i;
			v->size = sizeof(T);
			v->_init_default_value();
			u->items.emplace_back(v);
		}
		{
			auto v = new VariableInfoPrivate;
			v->type.tag = TypeTagVariable;
			v->type.name = "Vec";
			v->type.name += "<" + std::string(type_name) + ">";
			v->type.hash = H(v->type.name.c_str());
			v->name = "v";
			v->attribute = "o";
			v->offset = sizeof(T) * N;
			v->size = sizeof(T) * N;
			v->_init_default_value();
			u->items.emplace_back(v);
		}

		auto f = new FunctionInfoPrivate;
		f->name = "update";
		f->rva = pf2p(&BP_Vec<N, T>::update);
		f->return_type.tag = TypeTagVariable;
		f->return_type.name = "void";
		f->return_type.hash = H(f->return_type.name.c_str());

		u->functions.emplace_back(f);

		auto hash = H(u->name.c_str());
		auto db0 = find_typeinfo_db(0);
		assert(db0);
		auto it = db0->udts.find(hash);
		if (it == db0->udts.end())
			it = db0->udts.emplace(hash, std::vector<std::unique_ptr<UdtInfoPrivate>>()).first;
		it->second.emplace_back(u);
		
	}

	template<uint N, class T>
	struct BP_Array
	{
		T in[N];
		LNA<T> out;

		void initialize()
		{
			out.count = N;
			out.v = new T[N];
		}

		void update()
		{
			for (auto i = 0; i < N; i++)
				out.v[i] = in[i];
		}

		void finish()
		{
			delete[]out.v;
		}
	};

	template<uint N, class T>
	void install_array_udt(const char* name_suffix, const char* type_name)
	{
		auto u = new UdtInfoPrivate;
		u->name = "BP_Array_";
		u->name += name_suffix;
		u->size = sizeof(LNA<T>) + sizeof(T) * N;
		u->module_name = L"flame_foundation.dll";

		for (auto i = 0; i < N; i++)
		{
			auto v = new VariableInfoPrivate;
			v->type.tag = TypeTagVariable;
			v->type.name = type_name;
			v->type.hash = H(v->type.name.c_str());
			v->name = std::to_string(i + 1);
			v->attribute = "i";
			v->offset = sizeof(T) * i;
			v->size = sizeof(T);
			v->_init_default_value();
			u->items.emplace_back(v);
		}
		{
			auto v = new VariableInfoPrivate;
			v->type.tag = TypeTagVariable;
			v->type.name = "LNA";
			v->type.name += "<" + std::string(type_name) + ">";
			v->type.hash = H(v->type.name.c_str());
			v->name = "v";
			v->attribute = "o";
			v->offset = sizeof(T) * N;
			v->size = sizeof(LNA<T>);
			v->_init_default_value();
			u->items.emplace_back(v);
		}

		{
			auto f = new FunctionInfoPrivate;
			f->name = "initialize";
			f->rva = pf2p(&BP_Array<N, T>::initialize);
			f->return_type.tag = TypeTagVariable;
			f->return_type.name = "void";
			f->return_type.hash = H(f->return_type.name.c_str());

			u->functions.emplace_back(f);
		}
		{
			auto f = new FunctionInfoPrivate;
			f->name = "update";
			f->rva = pf2p(&BP_Array<N, T>::update);
			f->return_type.tag = TypeTagVariable;
			f->return_type.name = "void";
			f->return_type.hash = H(f->return_type.name.c_str());

			u->functions.emplace_back(f);
		}
		{
			auto f = new FunctionInfoPrivate;
			f->name = "finish";
			f->rva = pf2p(&BP_Array<N, T>::finish);
			f->return_type.tag = TypeTagVariable;
			f->return_type.name = "void";
			f->return_type.hash = H(f->return_type.name.c_str());

			u->functions.emplace_back(f);
		}

		auto hash = H(u->name.c_str());
		auto db0 = find_typeinfo_db(0);
		assert(db0);
		auto it = db0->udts.find(hash);
		if (it == db0->udts.end())
			it = db0->udts.emplace(hash, std::vector<std::unique_ptr<UdtInfoPrivate>>()).first;
		it->second.emplace_back(u);
	}

	void typeinfo_init_basic_bp_nodes()
	{
		install_vec_udt<1, float>("1f", "float");
		install_vec_udt<2, float>("2f", "float");
		install_vec_udt<3, float>("3f", "float");
		install_vec_udt<4, float>("4f", "float");

		install_array_udt<1, Vec4c>("1_4c", "Vec<4,uchar>");
	}

	void typeinfo_collect(const std::wstring & filename, int level)
	{
		auto db = find_typeinfo_db(level);
		if (!db)
		{
			db = new TypeinfoDB;
			db->level = level;
			typeinfo_dbs.emplace_back(db);
		}

		com_init();

		CComPtr<IDiaDataSource> dia_source;
		if (FAILED(CoCreateInstance(CLSID_DiaSource, NULL, CLSCTX_INPROC_SERVER, __uuidof(IDiaDataSource), (void**)& dia_source)))
		{
			printf("dia not found\n");
			assert(0);
			return;
		}
		if (FAILED(dia_source->loadDataFromPdb(ext_replace(filename, L".pdb").c_str())))
		{
			printf("pdb failed to open: %s\n", std::fs::path(filename).stem().string().c_str());
			assert(0);
			return;
		}
		CComPtr<IDiaSession> session;
		if (FAILED(dia_source->openSession(&session)))
		{
			printf("session failed to open\n");
			assert(0);
			return;
		}

		CComPtr<IDiaSymbol> global;
		if (FAILED(session->get_globalScope(&global)))
		{
			printf("failed to get global\n");
			assert(0);
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
			auto name = format_name(pwname, &pass_prefix, &pass_$);
			if (pass_prefix && pass_$ && name.find("unnamed") == std::string::npos)
			{
				auto hash = H(name.c_str());
				auto it = db->enums.find(hash);
				if (it == db->enums.end())
					it = db->enums.emplace(hash, std::vector<std::unique_ptr<EnumInfoPrivate>>()).first;
				auto found = false;
				for(auto& i : it->second)
				{
					if (i->name == name)
					{
						found = true;
						break;
					}
				}
				if (!found)
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

					e->module_name = filename;
					it->second.emplace_back(e);
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
			auto udt_name = format_name(pwname, &pass_prefix, &pass_$);
			std::regex_replace(udt_name, reg_usig, "u");
			if (pass_prefix && pass_$)
			{
				auto udt_hash = H(udt_name.c_str());
				auto it = db->udts.find(udt_hash);
				if (it == db->udts.end())
					it = db->udts.emplace(udt_hash, std::vector<std::unique_ptr<UdtInfoPrivate>>()).first;
				auto found = false;
				for (auto& i : it->second)
				{
					if (i->name == udt_name)
					{
						found = true;
						break;
					}
				}
				if (!found)
				{
					_udt->get_length(&ull);
					auto u = new UdtInfoPrivate;
					u->name = udt_name;
					u->size = (int)ull;
					u->module_name = filename;

					IDiaEnumSymbols* members;
					_udt->findChildren(SymTagData, NULL, nsNone, &members);
					IDiaSymbol* member;
					while (SUCCEEDED(members->Next(1, &member, &ul)) && (ul == 1))
					{
						member->get_name(&pwname);
						std::string attribute;
						auto name = format_name(pwname, nullptr, &pass_$, &attribute);
						if (pass_$)
						{
							IDiaSymbol* type;
							member->get_type(&type);

							auto i = new VariableInfoPrivate;
							i->type = symbol_to_typeinfo(type, attribute);
							i->name = name;
							i->attribute = attribute;
							member->get_offset(&l);
							i->offset = l;
							type->get_length(&ull);
							i->size = (int)ull;
							i->_init_default_value();

							type->Release();

							u->items.emplace_back(i);
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
						auto name = format_name(pwname, nullptr, &pass_$, &attribute);
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

										auto new_obj = malloc(u->size);
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

											for (auto& i : u->items)
											{
												if (i->default_value)
													memcpy(i->default_value, (char*)new_obj + i->offset, i->size);
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
									symbol_to_function(_function, f, attribute, session, source_files);

									u->functions.emplace_back(f);
								}
							}
						}
						_function->Release();
					}
					_functions->Release();

					u->module_name = filename;
					it->second.emplace_back(u);
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
			auto name = format_name(pwname, &pass_prefix, &pass_$, &attribute);
			if (pass_prefix && pass_$ && attribute.find("::") == std::string::npos /* not a member function */ )
			{
				auto hash = H(name.c_str());
				auto it = db->functions.find(hash);
				if (it == db->functions.end())
					it = db->functions.emplace(hash, std::vector<std::unique_ptr<FunctionInfoPrivate>>()).first;
				auto found = false;
				for (auto& i : it->second)
				{
					if (i->name == name)
					{
						found = true;
						break;
					}
				}
				if (!found)
				{
					auto f = new FunctionInfoPrivate;
					f->name = name;
					symbol_to_function(_function, f, attribute, session, source_files);

					f->module_name = filename;
					it->second.emplace_back(f);
				}
			}

			_function->Release();
		}
		_functions->Release();
	}

	void typeinfo_load(const std::wstring& filename, int level)
	{
		auto file = SerializableNode::create_from_json_file(filename);
		if (!file)
		{
			assert(0);
			return;
		}

		auto db = find_typeinfo_db(level);
		if (!db)
		{
			db = new TypeinfoDB;
			db->level = level;
			typeinfo_dbs.emplace_back(db);
		}

		auto n_enums = file->find_node("enums");
		for (auto i = 0; i < n_enums->node_count(); i++)
		{
			auto n_enum = n_enums->node(i);
			auto e = new EnumInfoPrivate;
			e->name = n_enum->find_node("name")->value();

			auto n_items = n_enum->find_node("items");
			for (auto j = 0; j < n_items->node_count(); j++)
			{
				auto n_item = n_items->node(j);
				auto i = new EnumItemPrivate;
				i->name = n_item->find_node("name")->value();
				i->value = std::stoi(n_item->find_node("value")->value());
				e->items.emplace_back(i);
			}

			auto hash = H(e->name.c_str());
			auto it = db->enums.find(hash);
			if (it == db->enums.end())
				it = db->enums.emplace(hash, std::vector<std::unique_ptr<EnumInfoPrivate>>()).first;
			it->second.emplace_back(e);
		}

		auto n_udts = file->find_node("udts");
		for (auto i = 0; i < n_udts->node_count(); i++)
		{
			auto n_udt = n_udts->node(i);
			auto u = new UdtInfoPrivate;
			u->name = n_udt->find_node("name")->value();
			u->size = std::stoi(n_udt->find_node("size")->value());
			u->module_name = s2w(n_udt->find_node("module_name")->value());

			auto n_items = n_udt->find_node("items");
			for (auto j = 0; j < n_items->node_count(); j++)
			{
				auto n_item = n_items->node(j);
				auto i = new VariableInfoPrivate;
				i->type = unserialize_typeinfo(n_item->find_node("type")->value());
				i->name = n_item->find_node("name")->value();
				i->attribute = n_item->find_node("attribute")->value();
				i->offset = std::stoi(n_item->find_node("offset")->value());
				i->size = std::stoi(n_item->find_node("size")->value());
				i->_init_default_value();
				if (i->default_value)
				{
					auto a_default_value = n_item->find_node("default_value");
					if (a_default_value)
						unserialize_value(i->type.tag, i->type.hash, i->type.name.c_str(), a_default_value->value(), i->default_value);
				}
				u->items.emplace_back(i);
			}

			auto n_functions = n_udt->find_node("functions");
			if (n_functions)
			{
				for (auto j = 0; j < n_functions->node_count(); j++)
				{
					auto n_function = n_functions->node(j);
					auto f = new FunctionInfoPrivate;
					unserialize_function(n_function, f);

					u->functions.emplace_back(f);
				}
			}

			auto hash = H(u->name.c_str());
			auto it = db->udts.find(hash);
			if (it == db->udts.end())
				it = db->udts.emplace(hash, std::vector<std::unique_ptr<UdtInfoPrivate>>()).first;
			it->second.emplace_back(u);
		}

		auto n_functions = file->find_node("functions");
		for (auto i = 0; i < n_functions->node_count(); i++)
		{
			auto n_function = n_functions->node(i);
			auto f = new FunctionInfoPrivate;
			unserialize_function(n_function, f);

			auto hash = H(f->name.c_str());
			auto it = db->functions.find(hash);
			if (it == db->functions.end())
				it = db->functions.emplace(hash, std::vector<std::unique_ptr<FunctionInfoPrivate>>()).first;
			it->second.emplace_back(f);
		}

		SerializableNode::destroy(file);
	}

	void typeinfo_save(const std::wstring & filename, int level)
	{
		auto file = SerializableNode::create("typeinfo");

		TypeinfoDB* db = nullptr;
		if (level != -1)
		{
			db = find_typeinfo_db(level);
			assert(db);
		}

		auto n_enums = file->new_node("enums");
		n_enums->set_type(SerializableNode::Array);
		{
			std::vector<EnumInfoPrivate*> sorted_enums;
			if (db)
			{
				for (auto& v : db->enums)
				{
					for (auto& e : v.second)
						sorted_enums.push_back(e.get());
				}
			}
			else
			{
				for (auto& db : typeinfo_dbs)
				{
					for (auto& v : db->enums)
					{
						for (auto& e : v.second)
							sorted_enums.push_back(e.get());
					}
				}
			}
			std::sort(sorted_enums.begin(), sorted_enums.end(), [](EnumInfoPrivate * a, EnumInfoPrivate * b) {
				return a->name < b->name;
			});
			for (auto& e : sorted_enums)
			{
				auto n_enum = n_enums->new_node("");
				n_enum->new_attr("name", e->name);

				auto n_items = n_enum->new_node("items");
				n_items->set_type(SerializableNode::Array);
				for (auto& i : e->items)
				{
					auto n_item = n_items->new_node("");
					n_item->new_attr("name", i->name);
					n_item->new_attr("value", std::to_string(i->value));
				}
			}
		}

		auto n_udts = file->new_node("udts");
		n_udts->set_type(SerializableNode::Array);
		{
			std::vector<UdtInfoPrivate*> sorted_udts;
			if (db)
			{
				for (auto& v : db->udts)
				{
					for (auto& e : v.second)
						sorted_udts.push_back(e.get());
				}
			}
			else
			{
				for (auto& db : typeinfo_dbs)
				{
					for (auto& v : db->udts)
					{
						for (auto& u : v.second)
							sorted_udts.push_back(u.get());
					}
				}
			}
			std::sort(sorted_udts.begin(), sorted_udts.end(), [](UdtInfoPrivate * a, UdtInfoPrivate * b) {
				return a->name < b->name;
			});
			for (auto& u : sorted_udts)
			{
				auto n_udt = n_udts->new_node("");
				n_udt->new_attr("name", u->name);
				n_udt->new_attr("size", std::to_string(u->size));
				n_udt->new_attr("module_name", w2s(u->module_name));

				auto n_items = n_udt->new_node("items");
				n_items->set_type(SerializableNode::Array);
				for (auto& i : u->items)
				{
					auto n_item = n_items->new_node("");
					const auto& type = i->type;
					n_item->new_attr("type", serialize_typeinfo(type));
					n_item->new_attr("name", i->name);
					n_item->new_attr("attribute", i->attribute);
					n_item->new_attr("offset", std::to_string(i->offset));
					n_item->new_attr("size", std::to_string(i->size));
					if (i->default_value)
					{
						auto default_value_str = serialize_value(type.tag, type.hash, type.name.c_str(), i->default_value, 1);
						if (default_value_str.size > 0)
							n_item->new_attr("default_value", default_value_str.v);
					}
				}

				auto n_functions = n_udt->new_node("functions");
				n_functions->set_type(SerializableNode::Array);
				for (auto& f : u->functions)
				{
					auto n_function = n_functions->new_node("");
					serialize_function(f.get(), n_function);
				}
			}
		}

		auto n_functions = file->new_node("functions");
		n_functions->set_type(SerializableNode::Array);
		{
			std::vector<FunctionInfoPrivate*> sorted_functions;
			if (db)
			{
				for (auto& v : db->functions)
				{
					for (auto& f : v.second)
						sorted_functions.push_back(f.get());
				}
			}
			else
			{
				for (auto& db : typeinfo_dbs)
				{
					for (auto& v : db->functions)
					{
						for (auto& f : v.second)
							sorted_functions.push_back(f.get());
					}
				}
			}
			std::sort(sorted_functions.begin(), sorted_functions.end(), [](FunctionInfoPrivate * a, FunctionInfoPrivate * b) {
				return a->name < b->name;
			});
			for (auto& f : sorted_functions)
			{
				auto n_function = n_functions->new_node("");
				serialize_function(f, n_function);
			}
		}

		file->save_json(filename);
		SerializableNode::destroy(file);
	}

	void typeinfo_clear(int level)
	{
		if (level == -1)
			typeinfo_dbs.clear();
		else
		{
			for (auto it = typeinfo_dbs.begin(); it != typeinfo_dbs.end(); it++)
			{
				if ((*it)->level == level)
				{
					typeinfo_dbs.erase(it);
					return;
				}
			}
			assert(0);
		}
	}
}
