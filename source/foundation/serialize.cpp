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
		"pointer",
		"any"
	};

	const char* get_name(TypeTag$ tag)
	{
		return tag_names[tag];
	}

	struct TypeInfoPrivate : TypeInfo
	{
		TypeTag$ tag;
		std::string name;
		uint hash;

		void set(TypeTag$ _tag, const std::string& _name)
		{
			tag = _tag;
			name = _name;
			hash = H(name.c_str());
		}

		std::string serialize() const
		{
			return std::string(get_name(tag)) + "#" + name;
		}
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

	void unserialize_typeinfo(const std::string& src, TypeTag$& tag, std::string& name)
	{
		auto sp = string_split(src, '#');

		auto e_tag = 0;
		for (auto s : tag_names)
		{
			if (sp[0] == s)
				break;
			e_tag++;
		}

		tag = (TypeTag$)e_tag;
		name = sp[1];
	}

	struct TypeinfoDB;

	struct VariableInfoPrivate : VariableInfo
	{
		TypeInfoPrivate type;
		std::string name;
		std::string attribute;
		uint offset, size;
		void* default_value;

		~VariableInfoPrivate()
		{
			delete default_value;
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

	uint VariableInfo::offset() const
	{
		return ((VariableInfoPrivate*)this)->offset;
	}

	uint VariableInfo::size() const
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
	};

	const char* EnumInfo::name() const
	{
		return ((EnumInfoPrivate*)this)->name.c_str();
	}

	uint EnumInfo::item_count() const
	{
		return ((EnumInfoPrivate*)this)->items.size();
	}

	EnumItem* EnumInfo::item(int idx) const
	{
		return ((EnumInfoPrivate*)this)->items[idx].get();
	}

	EnumItem* EnumInfo::find_item(const std::string& name, int* out_idx) const
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

	EnumItem* EnumInfo::add_item(const std::string& name, int value)
	{
		auto i = new EnumItemPrivate;
		i->name = name;
		i->value = value;
		((EnumInfoPrivate*)this)->items.emplace_back(i);
		return i;
	}

	struct FunctionInfoPrivate : FunctionInfo
	{
		std::string name;
		void* rva;
		TypeInfoPrivate return_type;
		std::vector<TypeInfoPrivate> parameter_types;
		std::string code_pos;
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

	uint FunctionInfo::parameter_count() const
	{
		return ((FunctionInfoPrivate*)this)->parameter_types.size();
	}

	const TypeInfo* FunctionInfo::parameter_type(uint idx) const
	{
		return &(((FunctionInfoPrivate*)this)->parameter_types[idx]);
	}

	void FunctionInfo::add_parameter(TypeTag$ tag, const std::string& type_name)
	{
		TypeInfoPrivate t;
		t.set(tag, type_name);
		((FunctionInfoPrivate*)this)->parameter_types.push_back(t);
	}

	const char* FunctionInfo::code_pos() const
	{
		return ((FunctionInfoPrivate*)this)->code_pos.c_str();
	}

	struct UdtInfoPrivate : UdtInfo
	{
		std::string name;
		uint size;
		std::wstring module_name;
		std::vector<std::unique_ptr<VariableInfoPrivate>> variables;
		std::vector<std::unique_ptr<FunctionInfoPrivate>> functions;

		uint item_find_pos;
		uint func_find_pos;

		UdtInfoPrivate()
		{
			item_find_pos = 0;
			func_find_pos = 0;
		}

		VariableInfoPrivate* find_vari(const std::string& name, int *out_idx)
		{
			if (variables.empty())
			{
				if (out_idx)
					* out_idx = -1;
				return nullptr;
			}

			auto p = item_find_pos;
			while (true)
			{
				auto item = variables[item_find_pos].get();
				if (item->name == name)
				{
					auto t = item_find_pos;
					item_find_pos++;
					if (item_find_pos >= variables.size())
						item_find_pos = 0;
					if (out_idx)
						* out_idx = t;
					return item;
				}
				item_find_pos++;
				if (item_find_pos >= variables.size())
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

		FunctionInfoPrivate* find_func(const std::string& name, int* out_idx)
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

	uint UdtInfo::size() const
	{
		return ((UdtInfoPrivate*)this)->size;
	}

	const wchar_t* UdtInfo::module_name() const
	{
		return ((UdtInfoPrivate*)this)->module_name.c_str();
	}

	uint UdtInfo::variable_count() const
	{
		return ((UdtInfoPrivate*)this)->variables.size();
	}

	VariableInfo* UdtInfo::variable(uint idx) const
	{
		return ((UdtInfoPrivate*)this)->variables[idx].get();
	}

	VariableInfo* UdtInfo::find_variable(const std::string& name, int *out_idx) const
	{
		return ((UdtInfoPrivate*)this)->find_vari(name, out_idx);
	}

	VariableInfo* UdtInfo::add_variable(TypeTag$ tag, const std::string& type_name, const std::string& name, const std::string& attribute, uint offset, uint size)
	{
		auto v = new VariableInfoPrivate;
		v->type.set(tag, type_name);
		v->name = name;
		v->attribute = attribute;
		v->offset = offset;
		v->size = size;
		v->default_value = nullptr;
		if (tag == TypeTagEnumSingle || tag == TypeTagEnumMulti || tag == TypeTagVariable)
		{
			const std::string ignore_types[] = {
				"void*",
				"Array",
				"DynamicArray",
				"String",
				"Function"
			};
			auto skip = false;
			if (tag == TypeTagVariable)
			{
				for (auto& t : ignore_types)
				{
					if (type_name.compare(0, t.size(), t) == 0)
					{
						skip = true;
						break;
					}
				}
			}
			if (!skip)
			{
				v->default_value = new char[size];
				memset(v->default_value, 0, size);
			}
		}
		((UdtInfoPrivate*)this)->variables.emplace_back(v);
		return v;
	}

	uint UdtInfo::function_count() const
	{
		return ((UdtInfoPrivate*)this)->functions.size();
	}

	FunctionInfo* UdtInfo::function(uint idx) const
	{
		return ((UdtInfoPrivate*)this)->functions[idx].get();
	}

	FunctionInfo* UdtInfo::find_function(const std::string& name, int* out_idx) const
	{
		return ((UdtInfoPrivate*)this)->find_func(name, out_idx);
	}

	FunctionInfo* UdtInfo::add_function(const std::string& name, void* rva, TypeTag$ return_type_tag, const std::string& return_type_name, const std::string& code_pos)
	{
		auto f = new FunctionInfoPrivate;
		f->name = name;
		f->rva = rva;
		f->return_type.set(return_type_tag, return_type_name);
		f->code_pos = code_pos;
		((UdtInfoPrivate*)this)->functions.emplace_back(f);
		return f;
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
		case TypeTagAny:
			*(void**)dst = (void*)src;
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

	String serialize_value(TypeTag$ tag, uint hash, const void* src, int precision)
	{
		switch (tag)
		{
		case TypeTagEnumSingle:
			return find_enum(hash)->find_item(*(int*)src)->name();
		case TypeTagEnumMulti:
		{
			std::string ret;
			auto e = (EnumInfoPrivate*)find_enum(hash);
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
			case cH("float"):
				return to_string(*(float*)src, precision);
			case cH("uint"):
				return std::to_string(*(uint*)src);
			case cH("int"):
				return std::to_string(*(int*)src);
			case cH("bool"):
				return *(bool*)src ? "1" : "0";
			case cH("Vec~1~float"):
				return to_string(*(Vec1f*)src, precision);
			case cH("Vec~2~float"):
				return to_string(*(Vec2f*)src, precision);
			case cH("Vec~3~float"):
				return to_string(*(Vec3f*)src, precision);
			case cH("Vec~4~float"):
				return to_string(*(Vec4f*)src, precision);
			case cH("Vec~1~uint"):
				return to_string(*(Vec1u*)src);
			case cH("Vec~2~uint"):
				return to_string(*(Vec2u*)src);
			case cH("Vec~3~uint"):
				return to_string(*(Vec3u*)src);
			case cH("Vec~4~uint"):
				return to_string(*(Vec4u*)src);
				//case cH("Ivec2"): case cH("i2"):
				//	return to_string(*(Ivec2*)src);
				//case cH("Ivec3"): case cH("i3"):
				//	return to_string(*(Ivec3*)src);
				//case cH("Ivec4"): case cH("i4"):
				//	return to_string(*(Ivec4*)src);
				//case cH("uchar"): case cH("b"):
				//	return to_string(*(uchar*)src);
				//case cH("Vec2c"): case cH("b2"):
				//	return to_string(*(Vec2c*)src);
				//case cH("Vec3c"): case cH("b3"):
				//	return to_string(*(Vec3c*)src);
			case cH("Vec~4~uchar"):
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

	void unserialize_value(TypeTag$ tag, uint hash, const std::string& src, void* dst)
	{
		switch (tag)
		{
		case TypeTagEnumSingle:
			find_enum(hash)->find_item(src, (int*)dst);
			break;
		case TypeTagEnumMulti:
		{
			auto v = 0;
			auto e = (EnumInfoPrivate*)find_enum(hash);
			auto sp = string_split(src, ';');
			for (auto& t : sp)
				v |= e->find_item(t)->value();
			*(int*)dst = v;
		}
		break;
		case TypeTagVariable:
			switch (hash)
			{
			case cH("float"):
				*(float*)dst = std::stof(src.c_str());
				break;
			case cH("uint"):
				*(uint*)dst = std::stoul(src);
				break;
			case cH("int"):
				*(int*)dst = std::stoi(src);
				break;
			case cH("bool"):
				*(bool*)dst = (src != "0");
				break;
			case cH("Vec~1~float"):
				*(Vec1f*)dst = std::stof(src.c_str());
				break;
			case cH("Vec~2~float"):
				*(Vec2f*)dst = stof2(src.c_str());
				break;
			case cH("Vec~3~float"):
				*(Vec3f*)dst = stof3(src.c_str());
				break;
			case cH("Vec~4~float"):
				*(Vec4f*)dst = stof4(src.c_str());
				break;
			case cH("Vec~1~uint"):
				*(Vec1u*)dst = std::stof(src.c_str());
				break;
			case cH("Vec~2~uint"):
				*(Vec2u*)dst = stou2(src.c_str());
				break;
			case cH("Vec~3~uint"):
				*(Vec3u*)dst = stou3(src.c_str());
				break;
			case cH("Vec~4~uint"):
				*(Vec4u*)dst = stou4(src.c_str());
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
			case cH("Vec~4~uchar"):
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
			for (auto i = 0; i < u->variable_count(); i++)
			{
				auto vari = u->variable(i);
				auto type = vari->type();

				if (type->tag() == TypeTagVariable)
				{
					if (type->hash() == cH("Function"))
						;
					else
					{
						if (vari->default_value() && !compare(type->tag(), vari->size(), src, vari->default_value()))
						{
							auto n_item = new_node("item");
							n_item->new_attr("name", vari->name());
							n_item->new_attr("value", serialize_value(type->tag(), type->hash(), src, precision).v);
						}
					}
				}
			}
		}

		void unserialize(UdtInfo* u, void* dst)
		{
			for (auto& n_item : nodes)
			{
				if (n_item->name != "item")
					continue;

				auto vari = u->find_variable(n_item->find_attr("name")->value());
				auto type = vari->type();

				if (type->tag() == TypeTagVariable)
				{
					if (type->hash() == cH("Function"))
						;
					else
						unserialize_value(type->tag(), type->hash(), n_item->find_attr("value")->value(), dst);
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
				auto c = it.value();
				if (!c.is_object() && !c.is_array())
					dst->new_attr(it.key(), c.is_string() ? c.get<std::string>() : c.dump());
				else
				{
					auto node = dst->new_node(it.key());
					from_json(c, node);
				}
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

	std::string format_name(const wchar_t* in, bool* pass_prefix = nullptr, bool* pass_$ = nullptr, std::string* attribute = nullptr)
	{
		static std::string prefix("flame::");
		static std::regex reg_token(R"([\w\s\_\$\:\*]+)");
		static std::string str_unsigned("unsigned");

		if (pass_prefix)
			* pass_prefix = false;
		if (pass_$)
			* pass_$ = false;

		//auto tokens = string_regex_split(std::string("flame::Array<flame::Vec<4, unsigned char>>"), reg_token);
		auto tokens = string_regex_split(w2s(in), reg_token);
		if (tokens.empty())
			return "";

		if (tokens[0].compare(0, prefix.size(), prefix) == 0)
		{
			if (pass_prefix)
				* pass_prefix = true;
		}
		if (tokens[0] == "Function")
			return tokens[0];
		auto pos_$ = tokens[0].find('$');
		if (pos_$ != std::string::npos)
		{
			if (attribute)
				* attribute = std::string(tokens[0].c_str() + pos_$ + 1);
			tokens[0].resize(pos_$);

			if (pass_$)
				* pass_$ = true;
		}

		for (auto& t : tokens)
		{
			{
				auto pos = t.find(prefix);
				if (pos != std::string::npos)
					t = t.replace(pos, prefix.size(), "");
			}
			{
				auto pos = t.find(str_unsigned);
				if (pos != std::string::npos)
					t = t.replace(pos, str_unsigned.size(), "u");
			}

			t.erase(std::remove(t.begin(), t.end(), ' '), t.end());
			t.erase(std::remove(t.begin(), t.end(), '$'), t.end());
		}

		auto name = tokens[0];
		for (auto i = 1; i < tokens.size(); i++)
		{
			if (!tokens[i].empty())
				name += "~" + tokens[i];
		}
		return name;
	}

	void symbol_to_typeinfo(IDiaSymbol* symbol, const std::string& variable_attribute /* type varies with variable's attribute */, TypeTag$& tag, std::string& name)
	{
		DWORD dw;
		wchar_t* pwname;

		symbol->get_symTag(&dw);
		switch (dw)
		{
		case SymTagEnum:
		{
			tag = variable_attribute.find('m') != std::string::npos ? TypeTagEnumMulti : TypeTagEnumSingle;
			symbol->get_name(&pwname);
			name = format_name(pwname);
		}
			break;
		case SymTagBaseType:
		{
			tag = TypeTagVariable;
			name = base_type_name(symbol);
		}
			break;
		case SymTagPointerType:
		{
			tag = TypeTagPointer;
			IDiaSymbol* pointer_type;
			symbol->get_type(&pointer_type);
			pointer_type->get_symTag(&dw);
			switch (dw)
			{
			case SymTagBaseType:
				name = base_type_name(pointer_type);
				break;
			case SymTagPointerType:
				assert(0);
				break;
			case SymTagUDT:
				pointer_type->get_name(&pwname);
				name = format_name(pwname);
				break;
			}
			if (variable_attribute.find('a') != std::string::npos && name == "void")
				tag = TypeTagAny;
			pointer_type->Release();
		}
			break;
		case SymTagUDT:
		{
			symbol->get_name(&pwname);
			tag = TypeTagVariable;
			name = format_name(pwname);
		}
			break;
		case SymTagFunctionArgType:
		{
			IDiaSymbol* type;
			symbol->get_type(&type);
			symbol_to_typeinfo(type, "", tag, name);
			type->Release();
		}
			break;
		}
	}

	struct TypeinfoDB
	{
		uint level;
		std::map<uint, std::unique_ptr<EnumInfoPrivate>> enums;
		std::map<uint, std::unique_ptr<UdtInfoPrivate>> udts;
		std::map<uint, std::unique_ptr<FunctionInfoPrivate>> functions;
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

	TypeinfoDB* find_or_create_typeinfo_db(int level)
	{
		auto db = find_typeinfo_db(level);
		if (!db)
		{
			db = new TypeinfoDB;
			db->level = level;
			typeinfo_dbs.emplace_back(db);
		}
		return db;
	}

	template<class T, class U>
	DynamicArray<T*> get_typeinfo_objects(const std::map<uint, std::unique_ptr<U>>& map)
	{
		DynamicArray<T*> ret;
		ret.resize(map.size());
		auto i = 0;
		for (auto it = map.begin(); it != map.end(); it++)
		{
			ret[i] = it->second.get();
			i++;
		}
		return ret;
	}

	template<class T>
	T* find_typeinfo_object(const std::map<uint, std::unique_ptr<T>>& map, uint name_hash)
	{
		auto it = map.find(name_hash);
		if (it == map.end())
			return nullptr;
		return it->second.get();
	}

	DynamicArray<EnumInfo*> get_enums(int level)
	{
		auto db = find_typeinfo_db(level);
		return db ? get_typeinfo_objects<EnumInfo>(db->enums) : DynamicArray<EnumInfo*>();
	}

	EnumInfo* find_enum(uint name_hash, int level)
	{
		if (level != -1)
		{
			auto db = find_typeinfo_db(level);
			return db ? find_typeinfo_object(db->enums, name_hash) : nullptr;
		}
		else
		{
			for (auto& db : typeinfo_dbs)
			{
				auto ret = find_typeinfo_object(db->enums, name_hash);
				if (ret)
					return ret;
			}
			return nullptr;
		}
	}

	EnumInfo* add_enum(uint level, const std::string& name)
	{
		auto db = find_or_create_typeinfo_db(level);
		auto e = new EnumInfoPrivate;
		e->name = name;
		db->enums.emplace(H(name.c_str()), e);
		return e;
	}

	DynamicArray<FunctionInfo*> get_functions(int level)
	{
		auto db = find_typeinfo_db(level);
		return db ? get_typeinfo_objects<FunctionInfo>(db->functions) : DynamicArray<FunctionInfo*>();
	}

	FunctionInfo* find_function(uint name_hash, int level)
	{
		if (level != -1)
		{
			auto db = find_typeinfo_db(level);
			return db ? find_typeinfo_object(db->functions, name_hash) : nullptr;
		}
		else
		{
			for (auto& db : typeinfo_dbs)
			{
				auto ret = find_typeinfo_object(db->functions, name_hash);
				if (ret)
					return ret;
			}
			return nullptr;
		}
	}

	FunctionInfo* add_function(uint level, const std::string& name, void* rva, TypeTag$ return_type_tag, const std::string& return_type_name, const std::string& code_pos)
	{
		auto db = find_or_create_typeinfo_db(level);
		auto f = new FunctionInfoPrivate;
		f->name = name;
		f->rva = rva;
		f->return_type.set(return_type_tag, return_type_name);
		f->code_pos = code_pos;
		db->functions.emplace(H(name.c_str()), f);
		return f;
	}

	DynamicArray<UdtInfo*> get_udts(int level)
	{
		auto db = find_typeinfo_db(level);
		return db ? get_typeinfo_objects<UdtInfo>(db->udts) : DynamicArray<UdtInfo*>();
	}

	UdtInfo* find_udt(uint name_hash, int level)
	{
		if (level != -1)
		{
			auto db = find_typeinfo_db(level);
			return db ? find_typeinfo_object(db->udts, name_hash) : nullptr;
		}
		else
		{
			for (auto& db : typeinfo_dbs)
			{
				auto ret = find_typeinfo_object(db->udts, name_hash);
				if (ret)
					return ret;
			}
			return nullptr;
		}
	}

	UdtInfo* add_udt(uint level, const std::string& name, uint size, const std::wstring& module_name)
	{
		auto db = find_or_create_typeinfo_db(level);
		auto u = new UdtInfoPrivate;
		u->name = name;
		u->size = size;
		u->module_name = module_name;
		db->udts.emplace(H(name.c_str()), u);
		return u;
	}

	uint typeinfo_free_level()
	{
		auto l = 0U;
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

	static void* this_module;

	template<class T>
	void* pf2p(T f)
	{
		union
		{
			T f;
			void* p;
		}cvt;
		cvt.f = f;
		return (void*)((char*)cvt.p - this_module);
	}

	template<uint N, class T>
	struct BP_Vec
	{
		T in[N];
		Vec<N, T> out;

		bool update(float delta_time)
		{
			for (auto i = 0; i < N; i++)
				out.v_[i] = in[i];

			return false;
		}
	};

	template<uint N, class T>
	void add_vec_udt(const std::string& name_suffix, const std::string& type_name)
	{
		typedef BP_Vec<N, T> VecType;

		auto u = add_udt(0, "Vec" + name_suffix, sizeof(BP_Vec<N, T>), L"flame_foundation.dll");

		for (auto i = 0; i < N; i++)
			u->add_variable(TypeTagVariable, type_name, std::string(1, "xyzw"[i]), "i", sizeof(T) * i, sizeof(T));
		u->add_variable(TypeTagVariable, "Vec~" + std::to_string(N) + "~" + std::string(type_name), "v", "o", offsetof(VecType, out), sizeof(VecType::out));

		u->add_function("update", pf2p(&BP_Vec<N, T>::update), TypeTagVariable, "bool", "")->add_parameter(TypeTagVariable, "float");
	}

	template<uint N, class T>
	struct BP_Array
	{
		T in[N];
		Array<T> out;

		bool update(float delta_time)
		{
			if (delta_time >= 0.f)
			{
				if (out.size != N)
				{
					delete[]out.v;
					out.size = N;
					out.v = new T[N];
				}
				for (auto i = 0; i < N; i++)
					out.v[i] = in[i];
			}
			else
				delete[]out.v;

			return false;
		}
	};

	struct ArraySize$
	{
		void* array$ia;

		uint size$o;

		FLAME_FOUNDATION_EXPORTS bool update$(float delta_time)
		{
			if (array$ia)
				size$o = ((Array<int>*)array$ia)->size;

			return false;
		}

	}bp_array_size_unused;

	struct ArrayInsertBeforeForEachItem_vp$
	{
		Array<void*> array$i;
		void* v$i;

		Array<void*> array$o;

		FLAME_FOUNDATION_EXPORTS bool update$(float delta_time)
		{
			if (delta_time >= 0.f)
			{
				if (array$o.size != array$i.size * 2)
				{
					delete[]array$o.v;
					array$o.size = array$i.size * 2;
					array$o.v = array$o.size > 0 ? new void* [array$o.size] : nullptr;
				}
				if (array$i.v)
				{
					for (auto i = 0; i < array$i.size; i++)
					{
						array$o.v[i * 2 + 0] = v$i;
						array$o.v[i * 2 + 1] = array$i.v[i];
					}
				}
			}
			else
				delete[]array$o.v;

			return false;
		}

	}bp_array_insert_before_for_each_item_unused;

	template<uint N, class T>
	void add_array_udt(const std::string& name_suffix, const std::string& type_name)
	{
		typedef BP_Array<N, T> ArrayType;

		auto u = add_udt(0, "Array_" + name_suffix, sizeof(BP_Array<N, T>), L"flame_foundation.dll");
		
		auto is_pointer = false;
		auto s_type_name = type_name;
		if (*s_type_name.rbegin() == '*')
		{
			s_type_name.erase(s_type_name.end() - 1);
			is_pointer = true;
		}

		for (auto i = 0; i < N; i++)
			u->add_variable(is_pointer ? TypeTagPointer : TypeTagVariable, s_type_name, std::to_string(i + 1), "i", sizeof(T) * i, sizeof(T));
		u->add_variable(TypeTagVariable, "Array~" + type_name, "v", "o", offsetof(ArrayType, out), sizeof(ArrayType::out));

		u->add_function("update", pf2p(&BP_Array<N, T>::update), TypeTagVariable, "bool", "")->add_parameter(TypeTagVariable, "float");
	}

	void typeinfo_init_basic_bp_nodes()
	{
		this_module = load_module(L"flame_foundation.dll");

		add_vec_udt<1, float>("1f", "float");
		add_vec_udt<2, float>("2f", "float");
		add_vec_udt<3, float>("3f", "float");
		add_vec_udt<4, float>("4f", "float");

		add_array_udt<1, uint>("1_u", "uint");
		add_array_udt<1, Vec4c>("1_4c", "Vec~4~uchar");
		add_array_udt<1, voidptr>("1_vp", "void*");

		add_array_udt<2, uint>("2_u", "uint");
		add_array_udt<2, Vec4c>("2_4c", "Vec~4~uchar");
		add_array_udt<2, voidptr>("2_vp", "void*");

		free_module(this_module);
	}

	void typeinfo_collect(const std::wstring& filename, int level)
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

		auto symbol_to_function = [&](IDiaSymbol* symbol, const std::string& attribute, voidptr& rva, TypeTag$& return_type_tag, std::string& return_type_name, std::string& code) {
			symbol->get_relativeVirtualAddress(&dw);
			rva = (void*)dw;

			IDiaSymbol* function_type;
			symbol->get_type(&function_type);

			IDiaSymbol* return_type;
			function_type->get_type(&return_type);
			symbol_to_typeinfo(return_type, "", return_type_tag, return_type_name);
			return_type->Release();

			function_type->Release();

			if (rva && attribute.find('c') != std::string::npos)
			{
				symbol->get_length(&ull);
				IDiaEnumLineNumbers* lines;

				if (SUCCEEDED(session->findLinesByRVA(dw, (DWORD)ull, &lines)))
				{
					IDiaLineNumber* line;
					DWORD src_file_id = -1;
					std::wstring filename;
					DWORD line_num;

					uint min_line = 1000000;
					uint max_line = 0;

					while (SUCCEEDED(lines->Next(1, &line, &ul)) && (ul == 1))
					{
						if (src_file_id == -1)
						{
							line->get_sourceFileId(&src_file_id);

							BSTR fn;
							IDiaSourceFile* source_file;
							line->get_sourceFile(&source_file);
							source_file->get_fileName(&fn);
							source_file->Release();

							filename = fn;
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
						code = w2s(filename) + "#" + std::to_string(min_line) + ":" + std::to_string(max_line);
				}
			}
		};

		auto symbol_to_parameters = [&](IDiaSymbol* symbol, FunctionInfo* f) {
			IDiaSymbol* function_type;
			symbol->get_type(&function_type);

			IDiaEnumSymbols* parameters;
			function_type->findChildren(SymTagFunctionArgType, NULL, nsNone, &parameters);
			IDiaSymbol* parameter;
			while (SUCCEEDED(parameters->Next(1, &parameter, &ul)) && (ul == 1))
			{
				TypeTag$ tag; std::string name;
				symbol_to_typeinfo(parameter, "", tag, name);
				f->add_parameter(tag, name);

				parameter->Release();
			}
			parameters->Release();

			function_type->Release();
		};

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
				if (!find_enum(hash))
				{
					auto e = add_enum(level, name);

					IDiaEnumSymbols* items;
					_enum->findChildren(SymTagNull, NULL, nsNone, &items);
					IDiaSymbol* item;
					while (SUCCEEDED(items->Next(1, &item, &ul)) && (ul == 1))
					{
						VARIANT v;
						ZeroMemory(&v, sizeof(v));
						item->get_name(&pwname);
						item->get_value(&v);

						e->add_item(w2s(pwname), v.lVal);

						item->Release();
					}
					items->Release();
				}
			}
			_enum->Release();
		}
		_enums->Release();

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
			if (pass_prefix && pass_$ && attribute.find("::") == std::string::npos /* not a member function */)
			{
				auto hash = H(name.c_str());
				if (!find_function(hash))
				{
					void* rva; TypeTag$ return_type_tag; std::string return_type_name; std::string code;
					symbol_to_function(_function, attribute, rva, return_type_tag, return_type_name, code);
					auto f = add_function(level, name, rva, return_type_tag, return_type_name, code);
					symbol_to_parameters(_function, f);
				}
			}

			_function->Release();
		}
		_functions->Release();

		// udts
		IDiaEnumSymbols* _udts;
		global->findChildren(SymTagUDT, NULL, nsNone, &_udts);
		IDiaSymbol* _udt;
		while (SUCCEEDED(_udts->Next(1, &_udt, &ul)) && (ul == 1))
		{
			_udt->get_name(&pwname);
			bool pass_prefix, pass_$;
			auto udt_name = format_name(pwname, &pass_prefix, &pass_$);
			if (pass_prefix && pass_$)
			{
				auto udt_hash = H(udt_name.c_str());
				if (!find_udt(udt_hash))
				{
					_udt->get_length(&ull);
					auto u = (UdtInfoPrivate*)add_udt(level, udt_name, ull, filename);

					IDiaEnumSymbols* _variables;
					_udt->findChildren(SymTagData, NULL, nsNone, &_variables);
					IDiaSymbol* _variable;
					while (SUCCEEDED(_variables->Next(1, &_variable, &ul)) && (ul == 1))
					{
						_variable->get_name(&pwname);
						std::string attribute;
						auto name = format_name(pwname, nullptr, &pass_$, &attribute);
						if (pass_$)
						{
							IDiaSymbol* type;
							_variable->get_type(&type);

							TypeTag$ tag; std::string type_name; 
							symbol_to_typeinfo(type, attribute, tag, type_name);
							_variable->get_offset(&l);
							type->get_length(&ull);

							u->add_variable(tag, type_name, name, attribute, l, ull);

							type->Release();
						}
						_variable->Release();
					}
					_variables->Release();

					IDiaEnumSymbols* _functions;
					_udt->findChildren(SymTagFunction, NULL, nsNone, &_functions);
					IDiaSymbol* _function;
					auto udt_name_nns = udt_name;
					{
						auto pos = udt_name_nns.find_last_of(':');
						if (pos != std::string::npos)
							udt_name_nns.erase(udt_name_nns.begin(), udt_name_nns.begin() + pos + 1);
					}
					while (SUCCEEDED(_functions->Next(1, &_function, &ul)) && (ul == 1))
					{
						_function->get_name(&pwname);
						std::string attribute;
						auto name = format_name(pwname, nullptr, &pass_$, &attribute);
						if (pass_$)
						{
							if (name[0] != '~')
							{
								if (name == udt_name_nns)
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

											for (auto& i : u->variables)
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
									void* rva; TypeTag$ return_type_tag; std::string return_type_name; std::string code;
									symbol_to_function(_function, attribute, rva, return_type_tag, return_type_name, code);
									auto f = (FunctionInfoPrivate*)u->add_function(name, rva, return_type_tag, return_type_name, code);
									symbol_to_parameters(_function, f);
								}
							}
						}
						_function->Release();
					}
					_functions->Release();
				}
			}
			_udt->Release();
		}
		_udts->Release();
	}

	void typeinfo_load(const std::wstring& filename, int level)
	{
		auto file = SerializableNode::create_from_xml_file(filename);
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

		auto unserialize_function = [](SerializableNode* src, std::string& name, voidptr& rva, TypeTag$& return_type_tag, std::string& return_type_name, std::string& code) {
			name = src->find_attr("name")->value();
			rva = (void*)std::stoul(src->find_attr("rva")->value().c_str());
			unserialize_typeinfo(src->find_attr("return_type")->value(), return_type_tag, return_type_name);
			auto n_code = src->find_node("code_pos");
			if (n_code)
				code = n_code->value();
		};

		auto unserialize_parameters = [](SerializableNode* src, FunctionInfo* f) {
			auto n_parameters = src->find_node("parameters");
			if (n_parameters)
			{
				for (auto i = 0; i < n_parameters->node_count(); i++)
				{
					auto n = n_parameters->node(i);
					TypeTag$ tag; std::string name;
					unserialize_typeinfo(n->find_attr("type")->value(), tag, name);
					f->add_parameter(tag, name);
				}
			}
		};

		auto n_enums = file->find_node("enums");
		for (auto i = 0; i < n_enums->node_count(); i++)
		{
			auto n_enum = n_enums->node(i);
			auto e = add_enum(level, n_enum->find_attr("name")->value());

			auto n_items = n_enum->find_node("items");
			for (auto j = 0; j < n_items->node_count(); j++)
			{
				auto n_item = n_items->node(j);
				e->add_item(n_item->find_attr("name")->value(), std::stoi(n_item->find_attr("value")->value()));
			}
		}

		auto n_functions = file->find_node("functions");
		for (auto i = 0; i < n_functions->node_count(); i++)
		{
			auto n_function = n_functions->node(i);
			std::string name; void* rva; TypeTag$ return_type_tag; std::string return_type_name; std::string code_pos;
			unserialize_function(n_function, name, rva, return_type_tag, return_type_name, code_pos);
			unserialize_parameters(n_function, add_function(level, name, rva, return_type_tag, return_type_name, code_pos));
		}

		auto n_udts = file->find_node("udts");
		for (auto i = 0; i < n_udts->node_count(); i++)
		{
			auto n_udt = n_udts->node(i);
			auto u = add_udt(level, n_udt->find_attr("name")->value(), std::stoi(n_udt->find_attr("size")->value()), s2w(n_udt->find_attr("module_name")->value()));

			auto n_items = n_udt->find_node("variables");
			for (auto j = 0; j < n_items->node_count(); j++)
			{
				auto n_vari = n_items->node(j);
				TypeTag$ tag; std::string type_name;
				unserialize_typeinfo(n_vari->find_attr("type")->value(), tag, type_name);
				auto v = (VariableInfoPrivate*)u->add_variable(tag, type_name, n_vari->find_attr("name")->value(), n_vari->find_attr("attribute")->value(), std::stoi(n_vari->find_attr("offset")->value()), std::stoi(n_vari->find_attr("size")->value()));
				if (v->default_value)
				{
					auto a_default_value = n_vari->find_attr("default_value");
					if (a_default_value)
						unserialize_value(tag, v->type.hash, a_default_value->value(), v->default_value);
				}
			}

			auto n_functions = n_udt->find_node("functions");
			if (n_functions)
			{
				for (auto j = 0; j < n_functions->node_count(); j++)
				{
					auto n_function = n_functions->node(j);
					std::string name; void* rva; TypeTag$ return_type_tag; std::string return_type_name; std::string code_pos;
					unserialize_function(n_function, name, rva, return_type_tag, return_type_name, code_pos);
					unserialize_parameters(n_function, u->add_function(name, rva, return_type_tag, return_type_name, code_pos));
				}
			}
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

		auto serialize_function = [](FunctionInfoPrivate * src, SerializableNode * dst) {
			dst->new_attr("name", src->name);
			dst->new_attr("rva", std::to_string((uint)src->rva));
			dst->new_attr("return_type", src->return_type.serialize());
			if (!src->parameter_types.empty())
			{
				auto n_parameters = dst->new_node("parameters");
				for (auto& p : src->parameter_types)
					n_parameters->new_node("parameter")->new_attr("type", p.serialize());
			}
			if (src->code_pos.length() > 0)
				dst->new_node("code_pos")->set_value(src->code_pos);
		};

		auto n_enums = file->new_node("enums");
		{
			std::vector<EnumInfoPrivate*> sorted_enums;
			if (db)
			{
				for (auto& e : db->enums)
					sorted_enums.push_back(e.second.get());
			}
			else
			{
				for (auto& db : typeinfo_dbs)
				{
					for (auto& e : db->enums)
						sorted_enums.push_back(e.second.get());
				}
			}
			std::sort(sorted_enums.begin(), sorted_enums.end(), [](EnumInfoPrivate * a, EnumInfoPrivate * b) {
				return a->name < b->name;
			});
			for (auto& e : sorted_enums)
			{
				auto n_enum = n_enums->new_node("enum");
				n_enum->new_attr("name", e->name);

				auto n_items = n_enum->new_node("items");
				for (auto& i : e->items)
				{
					auto n_item = n_items->new_node("item");
					n_item->new_attr("name", i->name);
					n_item->new_attr("value", std::to_string(i->value));
				}
			}
		}

		auto n_functions = file->new_node("functions");
		{
			std::vector<FunctionInfoPrivate*> sorted_functions;
			if (db)
			{
				for (auto& f : db->functions)
					sorted_functions.push_back(f.second.get());
			}
			else
			{
				for (auto& db : typeinfo_dbs)
				{
					for (auto& f : db->functions)
						sorted_functions.push_back(f.second.get());
				}
			}
			std::sort(sorted_functions.begin(), sorted_functions.end(), [](FunctionInfoPrivate* a, FunctionInfoPrivate* b) {
				return a->name < b->name;
				});
			for (auto& f : sorted_functions)
			{
				auto n_function = n_functions->new_node("function");
				serialize_function(f, n_function);
			}
		}

		auto n_udts = file->new_node("udts");
		{
			std::vector<UdtInfoPrivate*> sorted_udts;
			if (db)
			{
				for (auto& u : db->udts)
					sorted_udts.push_back(u.second.get());
			}
			else
			{
				for (auto& db : typeinfo_dbs)
				{
					for (auto& u : db->udts)
						sorted_udts.push_back(u.second.get());
				}
			}
			std::sort(sorted_udts.begin(), sorted_udts.end(), [](UdtInfoPrivate * a, UdtInfoPrivate * b) {
				return a->name < b->name;
			});
			for (auto& u : sorted_udts)
			{
				auto n_udt = n_udts->new_node("udt");
				n_udt->new_attr("name", u->name);
				n_udt->new_attr("size", std::to_string(u->size));
				n_udt->new_attr("module_name", w2s(u->module_name));

				auto n_items = n_udt->new_node("variables");
				for (auto& v : u->variables)
				{
					auto n_vari = n_items->new_node("variable");
					const auto& type = v->type;
					n_vari->new_attr("type", type.serialize());
					n_vari->new_attr("name", v->name);
					n_vari->new_attr("attribute", v->attribute);
					n_vari->new_attr("offset", std::to_string(v->offset));
					n_vari->new_attr("size", std::to_string(v->size));
					if (v->default_value)
					{
						auto default_value_str = serialize_value(type.tag, type.hash, v->default_value, 1);
						if (default_value_str.size > 0)
							n_vari->new_attr("default_value", default_value_str.v);
					}
				}

				auto n_functions = n_udt->new_node("functions");
				for (auto& f : u->functions)
				{
					auto n_function = n_functions->new_node("function");
					serialize_function(f.get(), n_function);
				}
			}
		}

		file->save_xml(filename);
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

	void typeinfo_check_update()
	{
		// typeinfo collect must do by order, because it only record the first entry
		std::vector<std::wstring> pdbs = {
			L"flame_foundation.dll",
			L"flame_network.dll",
			L"flame_graphics.dll",
			//L"flame_sound.dll",
			//L"flame_universe.dll",
		};

		printf("typeinfo update begin\n");

		auto id = 0;
		for (auto& fn : pdbs)
		{
			if (!std::fs::exists(fn))
				continue;

			auto w_dst = ext_replace(fn, L".typeinfo");
			auto dst = w2s(w_dst);
			if (!std::fs::exists(w_dst) || std::fs::last_write_time(w_dst) < std::fs::last_write_time(fn))
			{
				if (fn == L"flame_foundation.dll")
					typeinfo_init_basic_bp_nodes();

				printf("generating: %s\n", dst.c_str());

				typeinfo_collect(fn, id);
				typeinfo_save(w_dst, id);

				printf("ok\n");
			}
			else
			{
				typeinfo_load(w_dst, id);

				printf("up-to-data: %s\n", dst.c_str());
			}
			id++;
		}

		typeinfo_clear();

		printf("typeinfo update end\n");
	}
}
