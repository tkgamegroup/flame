#include <flame/foundation/serialize.h>

#include <pugixml.hpp>
#include <nlohmann/json.hpp>

#include <Windows.h>
#include <dia2.h>
#include <atlbase.h>
#include <DbgHelp.h>

namespace flame
{

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

	void SerializableAttribute::set_name(const std::string& name)
	{
		((SerializableAttributePrivate*)this)->name = name;
	}

	void SerializableAttribute::set_value(const std::string& value)
	{
		((SerializableAttributePrivate*)this)->value = value;
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

		void remove_attr(SerializableAttribute* a)
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

		SerializableAttribute* find_attr(const std::string& name)
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

		SerializableNode* new_node(const std::string& name)
		{
			return insert_node(-1, name);
		}

		SerializableNode* insert_node(int idx, const std::string& name)
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

		void remove_node(SerializableNode* n)
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

		SerializableNode* find_node(const std::string& name)
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

	void SerializableNode::set_name(const std::string& name)
	{
		((SerializableNodePrivate*)this)->name = name;
	}

	void SerializableNode::set_value(const std::string& value)
	{
		((SerializableNodePrivate*)this)->value = value;
	}

	SerializableAttribute* SerializableNode::new_attr(const std::string& name, const std::string& value)
	{
		return ((SerializableNodePrivate*)this)->new_attr(name, value);
	}

	SerializableAttribute* SerializableNode::insert_attr(int idx, const std::string& name, const std::string& value)
	{
		return ((SerializableNodePrivate*)this)->insert_attr(idx, name, value);
	}

	void SerializableNode::remove_attr(int idx)
	{
		((SerializableNodePrivate*)this)->remove_attr(idx);
	}

	void SerializableNode::remove_attr(SerializableAttribute* a)
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

	SerializableAttribute* SerializableNode::find_attr(const std::string& name)
	{
		return ((SerializableNodePrivate*)this)->find_attr(name);
	}

	void SerializableNode::add_node(SerializableNode* n)
	{
		((SerializableNodePrivate*)this)->add_node((SerializableNodePrivate*)n);
	}

	SerializableNode* SerializableNode::new_node(const std::string& name)
	{
		return ((SerializableNodePrivate*)this)->new_node(name);
	}

	SerializableNode* SerializableNode::insert_node(int idx, const std::string& name)
	{
		return ((SerializableNodePrivate*)this)->insert_node(idx, name);
	}

	void SerializableNode::remove_node(int idx)
	{
		((SerializableNodePrivate*)this)->remove_node(idx);
	}

	void SerializableNode::remove_node(SerializableNode* n)
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

	SerializableNode* SerializableNode::find_node(const std::string& name)
	{
		return ((SerializableNodePrivate*)this)->find_node(name);
	}

	static void from_xml(pugi::xml_node src, SerializableNode* dst)
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

	static void to_xml(pugi::xml_node dst, SerializableNodePrivate* src)
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

	SerializableNode* SerializableNode::create(const std::string& name)
	{
		auto n = new SerializableNodePrivate;
		n->name = name;

		return n;
	}

	SerializableNode* SerializableNode::create_from_xml_string(const std::string& str)
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

	SerializableNode* SerializableNode::create_from_xml_file(const std::wstring& filename)
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

	Mail<std::string> SerializableNode::to_xml_string(SerializableNode* n)
	{
		pugi::xml_document doc;
		auto rn = doc.append_child(n->name().c_str());

		to_xml(rn, (SerializableNodePrivate*)n);

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

		return new_mail(&writer.result);
	}

	Mail<std::string> SerializableNode::to_json_string(SerializableNode* n)
	{
		nlohmann::json doc;

		to_json(doc, (SerializableNodePrivate*)n);

		return new_mail(&doc.dump());
	}

	void SerializableNode::save_to_xml_file(SerializableNode* n, const std::wstring& filename)
	{
		pugi::xml_document doc;
		auto rn = doc.append_child(n->name().c_str());

		to_xml(rn, (SerializableNodePrivate*)n);

		doc.save_file(filename.c_str());
	}

	void SerializableNode::save_to_json_file(SerializableNode* n, const std::wstring& filename)
	{
		std::ofstream file(filename);
		nlohmann::json doc;

		to_json(doc, (SerializableNodePrivate*)n);

		file << doc.dump(2);
		file.close();
	}

	void SerializableNode::destroy(SerializableNode* n)
	{
		delete (SerializableNodePrivate*)n;
	}

	static const char* tag_names[] = {
		"enum_single",
		"enum_multi",
		"variable",
		"pointer",
		"attributeES",
		"attributeEM",
		"attributeV",
		"attributeP"
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

	const std::string& TypeInfo::name() const
	{
		return ((TypeInfoPrivate*)this)->name;
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
		uint name_hash;
		std::string decoration;
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

	const std::string& VariableInfo::name() const
	{
		return ((VariableInfoPrivate*)this)->name;
	}

	uint VariableInfo::name_hash() const
	{
		return ((VariableInfoPrivate*)this)->name_hash;
	}

	uint VariableInfo::offset() const
	{
		return ((VariableInfoPrivate*)this)->offset;
	}

	uint VariableInfo::size() const
	{
		return ((VariableInfoPrivate*)this)->size;
	}

	const std::string& VariableInfo::decoration() const
	{
		return ((VariableInfoPrivate*)this)->decoration;
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

	const std::string& EnumItem::name() const
	{
		return ((EnumItemPrivate*)this)->name;
	}

	int EnumItem::value() const
	{
		return ((EnumItemPrivate*)this)->value;
	}

	struct EnumInfoPrivate : EnumInfo
	{
		TypeinfoDatabase* db;

		std::string name;
		std::vector<std::unique_ptr<EnumItemPrivate>> items;
	};

	TypeinfoDatabase* EnumInfo::db() const
	{
		return ((EnumInfoPrivate*)this)->db;
	}

	const std::string& EnumInfo::name() const
	{
		return ((EnumInfoPrivate*)this)->name;
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
					* out_idx = items[i]->value;
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
		TypeinfoDatabase* db;

		std::string name;
		void* rva;
		TypeInfoPrivate return_type;
		std::vector<TypeInfoPrivate> parameter_types;
		std::string code_pos;
	};

	TypeinfoDatabase* FunctionInfo::db() const
	{
		return ((FunctionInfoPrivate*)this)->db;
	}

	const std::string& FunctionInfo::name() const
	{
		return ((FunctionInfoPrivate*)this)->name;
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

	const std::string& FunctionInfo::code_pos() const
	{
		return ((FunctionInfoPrivate*)this)->code_pos;
	}

	struct UdtInfoPrivate : UdtInfo
	{
		TypeinfoDatabase* db;

		std::string name;
		uint size;
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

	TypeinfoDatabase* UdtInfo::db() const
	{
		return ((UdtInfoPrivate*)this)->db;
	}

	const std::string& UdtInfo::name() const
	{
		return ((UdtInfoPrivate*)this)->name;
	}

	uint UdtInfo::size() const
	{
		return ((UdtInfoPrivate*)this)->size;
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

	VariableInfo* UdtInfo::add_variable(TypeTag$ tag, const std::string& type_name, const std::string& name, const std::string& decoration, uint offset, uint size)
	{
		auto v = new VariableInfoPrivate;
		v->type.set(tag, type_name);
		v->name = name;
		v->name_hash = H(name.c_str());
		v->decoration = decoration;
		v->offset = offset;
		v->size = size;
		v->default_value = nullptr;
		if (tag == TypeTagEnumSingle || tag == TypeTagEnumMulti || tag == TypeTagVariable ||
			tag == TypeTagAttributeES || tag == TypeTagAttributeEM || tag == TypeTagAttributeV)
		{
			static std::string vector_str("std::vector");
			static std::string string_str("std::string");
			if (type_name.compare(0, vector_str.size(), vector_str) != 0 &&
				type_name.compare(0, string_str.size(), string_str) != 0 &&
				decoration.find('o') == std::string::npos)
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
		f->db = db();
		f->name = name;
		f->rva = rva;
		f->return_type.set(return_type_tag, return_type_name);
		f->code_pos = code_pos;
		((UdtInfoPrivate*)this)->functions.emplace_back(f);
		return f;
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

	static std::string format_name(const wchar_t* in, bool* pass_prefix = nullptr, bool* pass_$ = nullptr, std::string* attribute = nullptr)
	{
		static std::string prefix("flame::");
		static std::string str_unsigned("unsigned ");
		static std::string str_enum("enum ");

		if (pass_prefix)
			* pass_prefix = false;
		if (pass_$)
			* pass_$ = false;

		auto str = w2s(in);

		if (pass_prefix)
		{
			if (str.compare(0, prefix.size(), prefix) == 0)
				* pass_prefix = true;
			else
				return "";
		}

		{
			auto pos = str.find(prefix);
			while (pos != std::string::npos)
			{
				str = str.replace(pos, prefix.size(), "");
				pos = str.find(prefix);
			}
		}
		{
			auto pos = str.find(str_unsigned);
			while (pos != std::string::npos)
			{
				str = str.replace(pos, str_unsigned.size(), "u");
				pos = str.find(str_unsigned);
			}
		}
		{
			auto pos = str.find(str_enum);
			while (pos != std::string::npos)
			{
				str = str.replace(pos, str_enum.size(), "");
				pos = str.find(str_enum);
			}
		}

		{
			static std::string eliminated_strs[] = {
				",std::allocator",
				",std::char_traits",
			};
			for (auto& s : eliminated_strs)
			{
				size_t pos;
				while ((pos = str.find(s)) != std::string::npos)
				{
					auto v = 0;
					auto l = s.size();
					do
					{
						auto ch = str[pos + l];
						if (ch == '<')
							v++;
						else if (ch == '>')
							v--;
						l++;
					} while (v > 0);
					str = str.replace(pos, l, "");
				}
			}
		}

		std::string head;
		std::string tail;
		auto pos_t = str.find('<');
		if (pos_t != std::string::npos)
		{
			head = std::string(str.begin(), str.begin() + pos_t);
			tail = std::string(str.begin() + pos_t, str.end());
		}
		else
			head = str;
		auto pos_$ = head.find('$');
		if (pos_$ != std::string::npos)
		{
			if (pass_$)
				* pass_$ = true;

			if (attribute)
				* attribute = std::string(head.begin() + pos_$ + 1, head.end());
			head.resize(pos_$);
		}
		else if (pass_$)
			return "";

		str = head + tail;

		str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
		str.erase(std::remove(str.begin(), str.end(), '$'), str.end());

		return tn_c2a(str);
	}

	static void symbol_to_typeinfo(IDiaSymbol* symbol, const std::string& variable_attribute /* type varies with variable's attribute */, TypeTag$& tag, std::string& name)
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
			pointer_type->Release();
		}
			break;
		case SymTagUDT:
		{
			symbol->get_name(&pwname);
			tag = TypeTagVariable;
			name = format_name(pwname);

			static std::string attr_str("Attribute");
			if (name.compare(0, attr_str.size(), attr_str) == 0 && name.size() > attr_str.size() + 1)
			{
				auto ch = name[attr_str.size()];
				if (ch == 'E')
				{
					tag = variable_attribute.find('m') != std::string::npos ? TypeTagAttributeEM : TypeTagAttributeES;
					name.erase(name.begin(), name.begin() + attr_str.size() + 2);
					name.erase(name.end() - 1);
				}
				else if (ch == 'V')
				{
					tag = TypeTagAttributeV;
					name.erase(name.begin(), name.begin() + attr_str.size() + 2);
					name.erase(name.end() - 1);
				}
				else if (ch == 'P')
				{
					tag = TypeTagAttributeP;
					name.erase(name.begin(), name.begin() + attr_str.size() + 2);
					name.erase(name.end() - 1);
				}
			}
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

	struct TypeinfoDatabasePrivate : TypeinfoDatabase
	{
		std::wstring module_name;

		std::map<uint, std::unique_ptr<EnumInfoPrivate>> enums;
		std::map<uint, std::unique_ptr<UdtInfoPrivate>> udts;
		std::map<uint, std::unique_ptr<FunctionInfoPrivate>> functions;
	};

	const std::wstring& TypeinfoDatabase::module_name() const
	{
		return ((TypeinfoDatabasePrivate*)this)->module_name;
	}

	template<class T, class U>
	 Mail<std::vector<T*>> get_typeinfo_objects(const std::map<uint, std::unique_ptr<U>>& map)
	{
		auto ret = new_mail<std::vector<T*>>();
		ret.p->resize(map.size());
		auto i = 0;
		for (auto it = map.begin(); it != map.end(); it++)
		{
			(*ret.p)[i] = it->second.get();
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

	Mail<std::vector<EnumInfo*>> TypeinfoDatabase::get_enums()
	{
		return get_typeinfo_objects<EnumInfo>(((TypeinfoDatabasePrivate*)this)->enums);
	}

	EnumInfo* TypeinfoDatabase::find_enum(uint name_hash)
	{
		return find_typeinfo_object(((TypeinfoDatabasePrivate*)this)->enums, name_hash);
	}

	EnumInfo* TypeinfoDatabase::add_enum(const std::string& name)
	{
		auto e = new EnumInfoPrivate;
		e->db = this;
		e->name = name;
		((TypeinfoDatabasePrivate*)this)->enums.emplace(H(name.c_str()), e);
		return e;
	}

	Mail<std::vector<FunctionInfo*>> TypeinfoDatabase::get_functions()
	{
		return get_typeinfo_objects<FunctionInfo>(((TypeinfoDatabasePrivate*)this)->functions);
	}

	FunctionInfo* TypeinfoDatabase::find_function(uint name_hash)
	{
		return find_typeinfo_object(((TypeinfoDatabasePrivate*)this)->functions, name_hash);
	}

	FunctionInfo* TypeinfoDatabase::add_function(const std::string& name, void* rva, TypeTag$ return_type_tag, const std::string& return_type_name, const std::string& code_pos)
	{
		auto f = new FunctionInfoPrivate;
		f->db = this;
		f->name = name;
		f->rva = rva;
		f->return_type.set(return_type_tag, return_type_name);
		f->code_pos = code_pos;
		((TypeinfoDatabasePrivate*)this)->functions.emplace(H(name.c_str()), f);
		return f;
	}

	Mail<std::vector<UdtInfo*>> TypeinfoDatabase::get_udts()
	{
		return get_typeinfo_objects<UdtInfo>(((TypeinfoDatabasePrivate*)this)->udts);
	}

	UdtInfo* TypeinfoDatabase::find_udt(uint name_hash)
	{
		return find_typeinfo_object(((TypeinfoDatabasePrivate*)this)->udts, name_hash);
	}

	UdtInfo* TypeinfoDatabase::add_udt(const std::string& name, uint size)
	{
		auto u = new UdtInfoPrivate;
		u->db = this;
		u->name = name;
		u->size = size;
		((TypeinfoDatabasePrivate*)this)->udts.emplace(H(name.c_str()), u);
		return u;
	}

	TypeinfoDatabase* TypeinfoDatabase::collect(const std::vector<TypeinfoDatabase*>& existed_dbs, const std::wstring& module_filename, const std::wstring& _pdb_filename)
	{
		com_init();

		CComPtr<IDiaDataSource> dia_source;
		if (FAILED(CoCreateInstance(CLSID_DiaSource, NULL, CLSCTX_INPROC_SERVER, __uuidof(IDiaDataSource), (void**)& dia_source)))
		{
			printf("dia not found\n");
			assert(0);
			return nullptr;
		}
		auto pdb_filename = _pdb_filename;
		if (pdb_filename.empty())
			pdb_filename = ext_replace(module_filename, L".pdb").wstring();
		if (FAILED(dia_source->loadDataFromPdb(pdb_filename.c_str())))
		{
			printf("pdb failed to open: %s\n", w2s(pdb_filename).c_str());
			assert(0);
			return nullptr;
		}
		CComPtr<IDiaSession> session;
		if (FAILED(dia_source->openSession(&session)))
		{
			printf("session failed to open\n");
			assert(0);
			return nullptr;
		}
		CComPtr<IDiaSymbol> global;
		if (FAILED(session->get_globalScope(&global)))
		{
			printf("failed to get global\n");
			assert(0);
			return nullptr;
		}

		auto db = new TypeinfoDatabasePrivate;
		db->module_name = module_filename;
		auto dbs = existed_dbs;
		dbs.push_back(db);

		{
			auto library = load_module(module_filename.c_str());
			if (library)
			{
				typedef void (*add_templates_func)(TypeinfoDatabase*);
				auto add_templates = (add_templates_func)GetProcAddress((HMODULE)library, "add_templates");
				if (add_templates)
					add_templates(db);

				free_module(library);
			}
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
					std::wstring source_file_name;
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

							source_file_name = fn;
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
						code = w2s(source_file_name) + "#" + std::to_string(min_line) + ":" + std::to_string(max_line);
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
				if (!::flame::find_enum(dbs, hash))
				{
					auto e = db->add_enum(name);

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

		// udts
		IDiaEnumSymbols* _udts;
		global->findChildren(SymTagUDT, NULL, nsNone, &_udts);
		IDiaSymbol* _udt;
		while (SUCCEEDED(_udts->Next(1, &_udt, &ul)) && (ul == 1))
		{
			_udt->get_name(&pwname);
			bool pass_prefix, pass_$;
			auto udt_name = format_name(pwname, &pass_prefix, &pass_$);
			if (pass_prefix && pass_$ && udt_name.find("(lambda_") == std::string::npos)
			{
				auto udt_hash = H(udt_name.c_str());
				if (!::flame::find_udt(dbs, udt_hash))
				{
					_udt->get_length(&ull);
					auto u = (UdtInfoPrivate*)db->add_udt(udt_name, ull);

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
							if (name == udt_name_nns)
								name = "ctor";
							else if (name[0] == '~')
								name = "dtor";

							void* rva; TypeTag$ return_type_tag; std::string return_type_name; std::string code;
							symbol_to_function(_function, attribute, rva, return_type_tag, return_type_name, code);
							if (rva)
							{
								auto f = (FunctionInfoPrivate*)u->add_function(name, rva, return_type_tag, return_type_name, code);
								symbol_to_parameters(_function, f);
							}
						}
						_function->Release();
					}
					_functions->Release();

					FunctionInfoPrivate* ctor = nullptr;
					FunctionInfoPrivate* dtor = nullptr;
					for (auto& f : u->functions)
					{
						if (f->name == "ctor" && f->parameter_types.empty())
							ctor = f.get();
						else if (f->name == "dtor")
							dtor = f.get();
						if (ctor && dtor)
							break;
					}
					if (ctor)
					{
						auto library = load_module(module_filename.c_str());
						if (library)
						{
							auto obj = malloc(u->size);
							memset(obj, 0, u->size);

							if (std::filesystem::path(module_filename).extension() == L".exe")
							{
								auto ulsize = 0UL;
								auto pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(library, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &ulsize);
								if (pImportDesc)
								{
									for (; pImportDesc->Name; pImportDesc++)
									{
										PSTR pszModName = (PSTR)((PBYTE)library + pImportDesc->Name);

										auto hImportDLL = LoadLibraryA(pszModName);
										assert(hImportDLL);

										auto pThunk = (PIMAGE_THUNK_DATA)((PBYTE)library + pImportDesc->FirstThunk);

										for (; pThunk->u1.Function; pThunk++)
										{
											FARPROC pfnNew = 0;
											size_t rva = 0;
											if (pThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG64)
											{
												size_t ord = IMAGE_ORDINAL64(pThunk->u1.Ordinal);

												PROC* ppfn = (PROC*)&pThunk->u1.Function;
												assert(ppfn);
												rva = (size_t)pThunk;

												char fe[100] = { 0 };
												sprintf_s(fe, 100, "#%u", ord);
												pfnNew = GetProcAddress(hImportDLL, (LPCSTR)ord);
												assert(pfnNew);
											}
											else
											{
												PROC* ppfn = (PROC*)&pThunk->u1.Function;
												assert(ppfn);
												rva = (size_t)pThunk;
												PSTR fName = (PSTR)library;
												fName += pThunk->u1.Function;
												fName += 2;
												if (!fName)
													break;
												pfnNew = GetProcAddress(hImportDLL, fName);
												assert(pfnNew);
											}

											auto hp = GetCurrentProcess();
											if (!WriteProcessMemory(hp, (LPVOID*)rva, &pfnNew, sizeof(pfnNew), NULL) && (ERROR_NOACCESS == GetLastError()))
											{
												DWORD dwOldProtect;
												if (VirtualProtect((LPVOID)rva, sizeof(pfnNew), PAGE_WRITECOPY, &dwOldProtect))
												{
													assert(WriteProcessMemory(GetCurrentProcess(), (LPVOID*)rva, &pfnNew, sizeof(pfnNew), NULL));
													assert(VirtualProtect((LPVOID)rva, sizeof(pfnNew), dwOldProtect, &dwOldProtect));
												}
											}
										}
									}
								}

								auto f = (void(*)(void*))GetProcAddress((HMODULE)library, "init_crt");
								auto ev = create_event(false);
								std::thread([&]() {
									f(ev);
								}).detach();
								wait_event(ev, -1);
								destroy_event(ev);
							}

							cmf(p2f<MF_v_v>((char*)library + (uint)(ctor->rva)), obj);
							for (auto& i : u->variables)
							{
								if (i->default_value)
									memcpy(i->default_value, (char*)obj + i->offset, i->size);
							}
							if (dtor)
								cmf(p2f<MF_v_v>((char*)library + (uint)(dtor->rva)), obj);

							free(obj);
							free_module(library);
						}
					}
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
			if (pass_prefix && pass_$ && attribute.find("::") == std::string::npos /* not a member function */)
			{
				auto hash = H(name.c_str());
				if (!::flame::find_function(dbs, hash))
				{
					void* rva; TypeTag$ return_type_tag; std::string return_type_name; std::string code;
					symbol_to_function(_function, attribute, rva, return_type_tag, return_type_name, code);
					if (rva)
					{
						auto f = db->add_function(name, rva, return_type_tag, return_type_name, code);
						symbol_to_parameters(_function, f);
					}
				}
			}

			_function->Release();
		}
		_functions->Release();

		return db;
	}

	TypeinfoDatabase* TypeinfoDatabase::load(const std::vector<TypeinfoDatabase*>& existed_dbs, const std::wstring& typeinfo_filename)
	{
		auto file = SerializableNode::create_from_xml_file(typeinfo_filename);
		if (!file)
		{
			assert(0);
			return nullptr;
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

		auto db = new TypeinfoDatabasePrivate;
		db->module_name = ext_replace(typeinfo_filename, L".dll");
		auto dbs = existed_dbs;
		dbs.push_back(db);

		auto n_enums = file->find_node("enums");
		for (auto i = 0; i < n_enums->node_count(); i++)
		{
			auto n_enum = n_enums->node(i);
			auto e = db->add_enum(n_enum->find_attr("name")->value());

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
			unserialize_parameters(n_function, db->add_function(name, rva, return_type_tag, return_type_name, code_pos));
		}

		auto n_udts = file->find_node("udts");
		for (auto i = 0; i < n_udts->node_count(); i++)
		{
			auto n_udt = n_udts->node(i);
			auto u = (UdtInfoPrivate*)db->add_udt(n_udt->find_attr("name")->value(), std::stoi(n_udt->find_attr("size")->value()));

			auto n_items = n_udt->find_node("variables");
			for (auto j = 0; j < n_items->node_count(); j++)
			{
				auto n_vari = n_items->node(j);
				TypeTag$ tag; std::string type_name;
				unserialize_typeinfo(n_vari->find_attr("type")->value(), tag, type_name);
				auto v = (VariableInfoPrivate*)u->add_variable(tag, type_name, n_vari->find_attr("name")->value(), n_vari->find_attr("decoration")->value(), std::stoi(n_vari->find_attr("offset")->value()), std::stoi(n_vari->find_attr("size")->value()));
				if (v->default_value)
				{
					auto a_default_value = n_vari->find_attr("default_value");
					if (a_default_value)
						unserialize_value(dbs, tag, v->type.hash, a_default_value->value(), v->default_value);
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

		return db;
	}

	void TypeinfoDatabase::save(const std::vector<TypeinfoDatabase*>& _dbs, TypeinfoDatabase* _db)
	{
		auto file = SerializableNode::create("typeinfo");

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

		auto db = (TypeinfoDatabasePrivate*)_db;
		auto dbs = _dbs;
		dbs.push_back(db);

		auto n_enums = file->new_node("enums");
		{
			for (auto& _e : db->enums)
			{
				auto e = _e.second.get();

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
			for (auto& f : db->functions)
			{
				auto n_function = n_functions->new_node("function");
				serialize_function(f.second.get(), n_function);
			}
		}

		auto n_udts = file->new_node("udts");
		{
			for (auto& _u : db->udts)
			{
				auto u = _u.second.get();

				auto n_udt = n_udts->new_node("udt");
				n_udt->new_attr("name", u->name);
				n_udt->new_attr("size", std::to_string(u->size));

				auto n_items = n_udt->new_node("variables");
				for (auto& v : u->variables)
				{
					auto n_vari = n_items->new_node("variable");
					const auto& type = v->type;
					n_vari->new_attr("type", type.serialize());
					n_vari->new_attr("name", v->name);
					n_vari->new_attr("decoration", v->decoration);
					n_vari->new_attr("offset", std::to_string(v->offset));
					n_vari->new_attr("size", std::to_string(v->size));
					if (v->default_value)
					{
						auto default_value_str = serialize_value(dbs, type.tag, type.hash, v->default_value, 1);
						if (!default_value_str.p->empty())
							n_vari->new_attr("default_value", *default_value_str.p);
						delete_mail(default_value_str);
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

		SerializableNode::save_to_xml_file(file, ext_replace(db->module_name, L".typeinfo"));
		SerializableNode::destroy(file);
	}

	void TypeinfoDatabase::destroy(TypeinfoDatabase* db)
	{
		delete (TypeinfoDatabasePrivate*)db;
	}

	Mail<std::string> serialize_value(const std::vector<TypeinfoDatabase*>& dbs, TypeTag$ tag, uint hash, const void* src, int precision)
	{
		auto ret = new_mail<std::string>();

		switch (tag)
		{
		case TypeTagAttributeES:
			src = (char*)src + sizeof(AttributeBase);
		case TypeTagEnumSingle:
		{
			auto e = find_enum(dbs, hash);
			assert(e);
			*(ret.p) = e->find_item(*(int*)src)->name();
		}
			break;
		case TypeTagAttributeEM:
			src = (char*)src + sizeof(AttributeBase);
		case TypeTagEnumMulti:
		{
			std::string str;
			auto e = find_enum(dbs, hash);
			assert(e);
			auto v = *(int*)src;
			for (auto i = 0; i < e->item_count(); i++)
			{
				if ((v & 1) == 1)
				{
					if (!str.empty())
						str += ";";
					str += e->find_item(1 << i)->name();
				}
				v >>= 1;
			}
			(*ret.p) = str;
		}
			break;
		case TypeTagAttributeV:
			src = (char*)src + sizeof(AttributeBase);
		case TypeTagVariable:
			switch (hash)
			{
			case cH("bool"):
				(*ret.p) = *(bool*)src ? "1" : "0";
				break;
			case cH("int"):
				(*ret.p) = std::to_string(*(int*)src);
				break;
			case cH("Vec(1+int)"):
				(*ret.p) = to_string(*(Vec1i*)src);
				break;
			case cH("Vec(2+int)"):
				(*ret.p) = to_string(*(Vec2i*)src);
				break;
			case cH("Vec(3+int)"):
				(*ret.p) = to_string(*(Vec3i*)src);
				break;
			case cH("Vec(4+int)"):
				(*ret.p) = to_string(*(Vec4i*)src);
				break;
			case cH("uint"):
				(*ret.p) = std::to_string(*(uint*)src);
				break;
			case cH("Vec(1+uint)"):
				(*ret.p) = to_string(*(Vec1u*)src);
				break;
			case cH("Vec(2+uint)"):
				(*ret.p) = to_string(*(Vec2u*)src);
				break;
			case cH("Vec(3+uint)"):
				(*ret.p) = to_string(*(Vec3u*)src);
				break;
			case cH("Vec(4+uint)"):
				(*ret.p) = to_string(*(Vec4u*)src);
				break;
			case cH("float"):
				(*ret.p) = to_string(*(float*)src, precision);
				break;
			case cH("Vec(1+float)"):
				(*ret.p) = to_string(*(Vec1f*)src, precision);
				break;
			case cH("Vec(2+float)"):
				(*ret.p) = to_string(*(Vec2f*)src, precision);
				break;
			case cH("Vec(3+float)"):
				(*ret.p) = to_string(*(Vec3f*)src, precision);
				break;
			case cH("Vec(4+float)"):
				(*ret.p) = to_string(*(Vec4f*)src, precision);
				break;
			case cH("uchar"):
				(*ret.p) = std::to_string(*(uchar*)src);
				break;
			case cH("Vec(1+uchar)"):
				(*ret.p) = to_string(*(Vec1c*)src);
				break;
			case cH("Vec(2+uchar)"):
				(*ret.p) = to_string(*(Vec2c*)src);
				break;
			case cH("Vec(3+uchar)"):
				(*ret.p) = to_string(*(Vec3c*)src);
				break;
			case cH("Vec(4+uchar)"):
				(*ret.p) = to_string(*(Vec4c*)src);
				break;
			case cH("std::basic_string(char)"):
				(*ret.p) = *(std::string*)src;
				break;
			case cH("std::basic_string(wchar_t)"):
				(*ret.p) = w2s(*(std::wstring*)src);
				break;
			default:
				assert(0);
			}
			break;
		}

		return ret;
	}

	void unserialize_value(const std::vector<TypeinfoDatabase*>& dbs, TypeTag$ tag, uint hash, const std::string& src, void* dst)
	{
		switch (tag)
		{
		case TypeTagAttributeES:
			dst = (char*)dst + sizeof(AttributeBase);
		case TypeTagEnumSingle:
		{
			auto e = find_enum(dbs, hash);
			assert(e);
			e->find_item(src, (int*)dst);
		}
			break;
		case TypeTagAttributeEM:
			dst = (char*)dst + sizeof(AttributeBase);
		case TypeTagEnumMulti:
		{
			auto v = 0;
			auto e = find_enum(dbs, hash);
			assert(e);
			auto sp = string_split(src, ';');
			for (auto& t : sp)
				v |= e->find_item(t)->value();
			*(int*)dst = v;
		}
			break;
		case TypeTagAttributeV:
			dst = (char*)dst + sizeof(AttributeBase);
		case TypeTagVariable:
			switch (hash)
			{
			case cH("bool"):
				*(bool*)dst = (src != "0");
				break;
			case cH("int"):
				*(int*)dst = std::stoi(src);
				break;
			case cH("Vec(1+int)"):
				*(Vec1u*)dst = std::stoi(src.c_str());
				break;
			case cH("Vec(2+int)"):
				*(Vec2u*)dst = stoi2(src.c_str());
				break;
			case cH("Vec(3+int)"):
				*(Vec3u*)dst = stoi3(src.c_str());
				break;
			case cH("Vec(4+int)"):
				*(Vec4u*)dst = stoi4(src.c_str());
				break;
			case cH("uint"):
				*(uint*)dst = std::stoul(src);
				break;
			case cH("Vec(1+uint)"):
				*(Vec1u*)dst = std::stoul(src.c_str());
				break;
			case cH("Vec(2+uint)"):
				*(Vec2u*)dst = stou2(src.c_str());
				break;
			case cH("Vec(3+uint)"):
				*(Vec3u*)dst = stou3(src.c_str());
				break;
			case cH("Vec(4+uint)"):
				*(Vec4u*)dst = stou4(src.c_str());
				break;
			case cH("float"):
				*(float*)dst = std::stof(src.c_str());
				break;
			case cH("Vec(1+float)"):
				*(Vec1f*)dst = std::stof(src.c_str());
				break;
			case cH("Vec(2+float)"):
				*(Vec2f*)dst = stof2(src.c_str());
				break;
			case cH("Vec(3+float)"):
				*(Vec3f*)dst = stof3(src.c_str());
				break;
			case cH("Vec(4+float)"):
				*(Vec4f*)dst = stof4(src.c_str());
				break;
			case cH("uchar"):
				*(uchar*)dst = std::stoul(src);
				break;
			case cH("Vec(1+uchar)"):
				*(Vec1c*)dst = std::stoul(src.c_str());
				break;
			case cH("Vec(2+uchar)"):
				*(Vec2c*)dst = stoc2(src.c_str());
				break;
			case cH("Vec(3+uchar)"):
				*(Vec3c*)dst = stoc3(src.c_str());
				break;
			case cH("Vec(4+uchar)"):
				*(Vec4c*)dst = stoc4(src.c_str());
				break;
			case cH("std::basic_string(char)"):
				*(std::string*)dst = src;
				break;
			case cH("std::basic_string(wchar_t)"):
				*(std::wstring*)dst = s2w(src);
				break;
			default:
				assert(0);
			}
			break;
		}
	}
}
