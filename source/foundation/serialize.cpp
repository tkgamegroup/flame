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

		SerializableNodePrivate() :
			type(Object),
			parent(nullptr)
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

		SerializableAttribute* find_attr(const std::string& name) const
		{
			for (auto& a : attrs)
			{
				if (a->name == name)
					return a.get();
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

		SerializableNode* find_node(const std::string& name) const
		{
			for (auto& n : nodes)
			{
				if (n->name == name)
					return n.get();
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

	SerializableAttribute* SerializableNode::find_attr(const std::string& name) const
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

	SerializableNode* SerializableNode::find_node(const std::string& name) const
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

	struct VariableInfoPrivate : VariableInfo
	{
		TypeInfo type;
		std::string name;
		uint name_hash;
		std::string decoration;
		uint offset, size;
		void* default_value;

		VariableInfoPrivate(const TypeInfo& type) :
			type(type)
		{
		}

		~VariableInfoPrivate()
		{
			delete default_value;
		}
	};

	const TypeInfo& VariableInfo::type() const
	{
		return ((VariableInfoPrivate*)this)->type;
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
		TypeInfo return_type;
		std::vector<TypeInfo> parameter_types;
		std::string code_pos;

		FunctionInfoPrivate(const TypeInfo& return_type) :
			return_type(return_type)
		{
		}
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

	const TypeInfo& FunctionInfo::return_type() const
	{
		return ((FunctionInfoPrivate*)this)->return_type;
	}

	uint FunctionInfo::parameter_count() const
	{
		return ((FunctionInfoPrivate*)this)->parameter_types.size();
	}

	const TypeInfo& FunctionInfo::parameter_type(uint idx) const
	{
		return ((FunctionInfoPrivate*)this)->parameter_types[idx];
	}

	void FunctionInfo::add_parameter(const TypeInfo& type)
	{
		((FunctionInfoPrivate*)this)->parameter_types.push_back(type);
	}

	const std::string& FunctionInfo::code_pos() const
	{
		return ((FunctionInfoPrivate*)this)->code_pos;
	}

	struct UdtInfoPrivate : UdtInfo
	{
		TypeinfoDatabase* db;

		TypeInfo type;
		uint size;
		std::vector<std::unique_ptr<VariableInfoPrivate>> variables;
		std::vector<std::unique_ptr<FunctionInfoPrivate>> functions;

		UdtInfoPrivate(const TypeInfo& type) :
			type(type)
		{
		}

		VariableInfoPrivate* find_vari(const std::string& name, int *out_idx)
		{
			for (auto i = 0; i < variables.size(); i++)
			{
				if (variables[i]->name == name)
				{
					if (out_idx)
						*out_idx = i;
					return variables[i].get();
				}
			}
			if (out_idx)
				*out_idx = -1;
			return nullptr;
		}

		FunctionInfoPrivate* find_func(const std::string& name, int* out_idx)
		{
			for (auto i = 0; i < functions.size(); i++)
			{
				if (functions[i]->name == name)
				{
					if (out_idx)
						*out_idx = i;
					return functions[i].get();
				}
			}
			if (out_idx)
				*out_idx = -1;
			return nullptr;
		}
	};

	TypeinfoDatabase* UdtInfo::db() const
	{
		return ((UdtInfoPrivate*)this)->db;
	}

	const TypeInfo& UdtInfo::type() const
	{
		return ((UdtInfoPrivate*)this)->type;
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

	VariableInfo* UdtInfo::add_variable(const TypeInfo& type, const std::string& name, const std::string& decoration, uint offset, uint size)
	{
		auto v = new VariableInfoPrivate(type);
		v->name = name;
		v->name_hash = H(name.c_str());
		v->decoration = decoration;
		v->offset = offset;
		v->size = size;
		v->default_value = nullptr;
		if (!type.is_vector && (type.tag == TypeEnumSingle || type.tag == TypeEnumMulti || type.tag == TypeData) &&
			type.base_hash != cH("std::string") &&
			type.base_hash != cH("std::wstring") &&
			type.base_hash != cH("StringA") &&
			type.base_hash != cH("StringW") &&
			decoration.find('o') == std::string::npos)
		{
			v->default_value = new char[size];
			memset(v->default_value, 0, size);
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

	FunctionInfo* UdtInfo::add_function(const std::string& name, void* rva, const TypeInfo& return_type, const std::string& code_pos)
	{
		auto f = new FunctionInfoPrivate(return_type);
		f->db = db();
		f->name = name;
		f->rva = rva;
		f->code_pos = code_pos;
		((UdtInfoPrivate*)this)->functions.emplace_back(f);
		return f;
	}

	static std::string base_type_name(IDiaSymbol* s)
	{
		DWORD baseType;
		s->get_baseType(&baseType);
		ULONGLONG len;
		s->get_length(&len);
		std::string name;
		switch (baseType)
		{
		case btVoid:
			return "void";
		case btChar:
			return "char";
		case btWChar:
			return "wchar_t";
		case btBool:
			return "bool";
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
		default:
			assert(0);
		}
	}

	static std::string format_name(const wchar_t* in, bool* pass_prefix = nullptr, bool* pass_$ = nullptr, std::string* attribute = nullptr)
	{
		static SAL(prefix, "flame::");
		static SAL(str_unsigned, "unsigned ");
		static SAL(str_int64, "__int64");
		static SAL(str_enum, "enum ");
		static SAL(str_string, "std::basic_string<char >");
		static SAL(str_wstring, "std::basic_string<wchar_t >");
		static SAL(str_stringa, "String<char>");
		static SAL(str_stringw, "String<wchar_t>");

		if (pass_prefix)
			*pass_prefix = false;
		if (pass_$)
			*pass_$ = false;

		auto str = w2s(in);

		if (pass_prefix)
		{
			if (str.compare(0, prefix.l, prefix.s) == 0)
				*pass_prefix = true;
			else
				return "";
		}

		{
			auto pos = str.find(prefix.s, 0, prefix.l);
			while (pos != std::string::npos)
			{
				str = str.replace(pos, prefix.l, "");
				pos = str.find(prefix.s, 0, prefix.l);
			}
		}
		{
			auto pos = str.find(str_unsigned.s, 0, str_unsigned.l);
			while (pos != std::string::npos)
			{
				str = str.replace(pos, str_unsigned.l, "u");
				pos = str.find(str_unsigned.s, 0, str_unsigned.l);
			}
		}
		{
			auto pos = str.find(str_int64.s, 0, str_int64.l);
			while (pos != std::string::npos)
			{
				str = str.replace(pos, str_int64.l, "longlong");
				pos = str.find(str_int64.s, 0, str_int64.l);
			}
		}
		{
			auto pos = str.find(str_enum.s, 0, str_enum.l);
			while (pos != std::string::npos)
			{
				str = str.replace(pos, str_enum.l, "");
				pos = str.find(str_enum.s, 0, str_enum.l);
			}
		}
		{
			static std::string eliminated_template_strs[] = {
				",std::allocator",
				",std::char_traits",
			};
			for (auto& s : eliminated_template_strs)
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
		{
			auto pos = str.find(str_string.s, 0, str_string.l);
			while (pos != std::string::npos)
			{
				str = str.replace(pos, str_string.l, "std::string");
				pos = str.find(str_string.s, 0, str_string.l);
			}
		}
		{
			auto pos = str.find(str_wstring.s, 0, str_wstring.l);
			while (pos != std::string::npos)
			{
				str = str.replace(pos, str_wstring.l, "std::wstring");
				pos = str.find(str_wstring.s, 0, str_wstring.l);
			}
		}
		{
			auto pos = str.find(str_stringa.s, 0, str_stringa.l);
			while (pos != std::string::npos)
			{
				str = str.replace(pos, str_stringa.l, "StringA");
				pos = str.find(str_stringa.s, 0, str_stringa.l);
			}
		}
		{
			auto pos = str.find(str_stringw.s, 0, str_stringw.l);
			while (pos != std::string::npos)
			{
				str = str.replace(pos, str_stringw.l, "StringW");
				pos = str.find(str_stringw.s, 0, str_stringw.l);
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
				*pass_$ = true;

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

	static TypeInfo symbol_to_typeinfo(IDiaSymbol* s_type, const std::string& decoration)
	{
		DWORD dw;
		wchar_t* pwname;

		s_type->get_symTag(&dw);
		switch (dw)
		{
		case SymTagEnum:
			s_type->get_name(&pwname);
			return TypeInfo(decoration.find('m') != std::string::npos ? TypeEnumMulti : TypeEnumSingle, format_name(pwname));
		case SymTagBaseType:
			return TypeInfo(TypeData, base_type_name(s_type));
		case SymTagPointerType:
		{
			std::string name;
			IDiaSymbol* pointer_type;
			s_type->get_type(&pointer_type);
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
			return TypeInfo(TypePointer, name);
		}
		case SymTagUDT:
		{
			s_type->get_name(&pwname);
			auto tag = TypeData;
			auto name = format_name(pwname);
			auto is_vector = false;
			auto is_attribute = false;

			SAL(vector_str, "std::vector");
			SAL(attribute_str, "Attribute");

			if (name.compare(0, attribute_str.l, attribute_str.s) == 0 && name.size() > attribute_str.l + 1)
			{
				is_attribute = true;
				switch (name[attribute_str.l])
				{
				case 'E':
					tag = decoration.find('m') != std::string::npos ? TypeEnumMulti : TypeEnumSingle;
					break;
				case 'D':
					break;
				case 'P':
					tag = TypePointer;
					break;
				}
				name.erase(name.begin(), name.begin() + attribute_str.l + 2);
				name.erase(name.end() - 1);
			}
			if (name.compare(0, vector_str.l, vector_str.s) == 0 && name.size() > vector_str.l + 1)
			{
				is_vector = true;
				name.erase(name.begin(), name.begin() + vector_str.l + 1);
				name.erase(name.end() - 1);
			}
			return TypeInfo(tag, name, is_attribute, is_vector);
		}
			break;
		case SymTagFunctionArgType:
		{
			IDiaSymbol* s_arg_type;
			s_type->get_type(&s_arg_type);
			auto ret = symbol_to_typeinfo(s_arg_type, "");
			s_arg_type->Release();
			return ret;
		}
		}
	}

	struct FunctionDesc
	{
		void* rva;
		TypeInfo ret_type;
		std::vector<TypeInfo> parameters;
	};

	void symbol_to_function(IDiaSymbol* s_function, FunctionDesc& desc)
	{
		DWORD dw;
		ULONG ul;

		s_function->get_relativeVirtualAddress(&dw);
		desc.rva = (void*)dw;
		if (!desc.rva)
			return;

		IDiaSymbol* s_function_type;
		s_function->get_type(&s_function_type);

		IDiaSymbol* s_return_type;
		s_function_type->get_type(&s_return_type);
		desc.ret_type = symbol_to_typeinfo(s_return_type, "");
		s_return_type->Release();

		/*
		if (rva && attribute.find('c') != std::string::npos)
		{
			s_function->get_length(&ull);
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
		*/

		IDiaEnumSymbols* s_parameters;
		s_function_type->findChildren(SymTagFunctionArgType, NULL, nsNone, &s_parameters);
		IDiaSymbol* s_parameter;
		while (SUCCEEDED(s_parameters->Next(1, &s_parameter, &ul)) && (ul == 1))
		{
			desc.parameters.push_back(symbol_to_typeinfo(s_parameter, ""));

			s_parameter->Release();
		}
		s_parameters->Release();

		s_function_type->Release();
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

	FunctionInfo* TypeinfoDatabase::add_function(const std::string& name, void* rva, const TypeInfo& return_type, const std::string& code_pos)
	{
		auto f = new FunctionInfoPrivate(return_type);
		f->db = this;
		f->name = name;
		f->rva = rva;
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

	UdtInfo* TypeinfoDatabase::add_udt(const TypeInfo& type, uint size)
	{
		auto u = new UdtInfoPrivate(type);
		u->db = this;
		u->size = size;
		((TypeinfoDatabasePrivate*)this)->udts.emplace(H(type.name.c_str()), u);
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
			pdb_filename = std::filesystem::path(module_filename).replace_extension(L".pdb");
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

		LONG l;
		ULONG ul;
		ULONGLONG ull;
		DWORD dw;
		wchar_t* pwname;

		std::vector<UdtInfo*> staging_string_symbols;

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

						auto item_name = w2s(pwname);
						if (!sendswith(item_name, std::string("Max")))
							e->add_item(item_name, v.lVal);

						item->Release();
					}
					items->Release();
				}
			}
			_enum->Release();
		}
		_enums->Release();

		auto find_udt_and_get_spcial_functions = [&](const TypeInfo& type, std::vector<std::pair<std::string, FunctionDesc>>& functions) {
			UdtInfo* u = nullptr;
			auto ok = false;

			IDiaEnumSymbols* _udts;
			global->findChildren(SymTagUDT, NULL, nsNone, &_udts);
			IDiaSymbol* _udt;
			while (SUCCEEDED(_udts->Next(1, &_udt, &ul)) && (ul == 1))
			{
				auto udt_type = symbol_to_typeinfo(_udt, "");
				if (!udt_type.is_attribute && udt_type.is_vector == type.is_vector && udt_type.base_hash == type.base_hash)
				{
					for (auto& f : functions)
						f.second.rva = nullptr;

					IDiaEnumSymbols* _functions;
					_udt->findChildren(SymTagFunction, NULL, nsNone, &_functions);
					IDiaSymbol* _function;
					while (SUCCEEDED(_functions->Next(1, &_function, &ul)) && (ul == 1))
					{
						_function->get_name(&pwname);
						auto name = w2s(pwname);
						for (auto& f : functions)
						{
							if (!f.second.rva && name == f.first)
							{
								FunctionDesc desc;
								symbol_to_function(_function, desc);
								if (desc.rva && desc.ret_type == f.second.ret_type &&
									desc.parameters == f.second.parameters)
									f.second.rva = desc.rva;
								break;
							}
						}

						_function->Release();
					}
					_functions->Release();

					ok = true;
					for (auto& f : functions)
					{
						if (!f.second.rva)
						{
							ok = false;
							break;
						}
					}
				}

				if (ok)
				{
					_udt->get_length(&ull);
					u = db->add_udt(TypeInfo(TypeData, type.base_name), ull);
					for (auto& f : functions)
					{
						auto _f = u->add_function(f.first, f.second.rva, f.second.ret_type, "");
						for (auto& p : f.second.parameters)
							_f->add_parameter(p);
					}
				}

				_udt->Release();

				if (ok)
					break;
			}
			_udts->Release();

			return u;
		};

		// udts
		IDiaEnumSymbols* _udts;
		global->findChildren(SymTagUDT, NULL, nsNone, &_udts);
		IDiaSymbol* _udt;
		while (SUCCEEDED(_udts->Next(1, &_udt, &ul)) && (ul == 1))
		{
			_udt->get_name(&pwname);
			bool pass_prefix, pass_$;
			auto udt_name = format_name(pwname, &pass_prefix, &pass_$);

			if (pass_prefix && pass_$ && udt_name.find("(unnamed") == std::string::npos && udt_name.find("(lambda_") == std::string::npos)
			{
				auto udt_hash = H(udt_name.c_str());
				if (!::flame::find_udt(dbs, udt_hash))
				{
					_udt->get_length(&ull);
					auto u = (UdtInfoPrivate*)db->add_udt(TypeInfo(TypeData, udt_name), ull);

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
							if (name[0] == '_')
								name.erase(name.begin());

							IDiaSymbol* s_type;
							_variable->get_type(&s_type);
 
							_variable->get_offset(&l);
							s_type->get_length(&ull);

							auto type = symbol_to_typeinfo(s_type, attribute);
							u->add_variable(type, name, attribute, l, ull);
							if (type.base_hash == cH("std::string") || type.base_hash == cH("std::wstring"))
							{
								auto is_new = true;
								for (auto u : staging_string_symbols)
								{
									if (u->type().base_hash == type.base_hash)
									{
										is_new = false;
										break;
									}
								}
								if (is_new)
								{
									std::vector<std::pair<std::string, FunctionDesc>> functions;
									functions.emplace_back("operator=", FunctionDesc{
										nullptr,
										TypeInfo(TypePointer, type.base_name),
										{ TypeInfo(TypePointer, type.base_name) }
									});
									auto u = find_udt_and_get_spcial_functions(type, functions);
									if (u)
										staging_string_symbols.push_back(u);
								}
							}

							s_type->Release();
						}
						_variable->Release();
					}
					_variables->Release();

					IDiaEnumSymbols* _functions;
					_udt->findChildren(SymTagFunction, NULL, nsNone, &_functions);
					IDiaSymbol* _function;
					auto name_no_namespace = udt_name;
					{
						auto pos = name_no_namespace.find_last_of(':');
						if (pos != std::string::npos)
							name_no_namespace.erase(name_no_namespace.begin(), name_no_namespace.begin() + pos + 1);
					}
					while (SUCCEEDED(_functions->Next(1, &_function, &ul)) && (ul == 1))
					{
						_function->get_name(&pwname);
						auto name = format_name(pwname, nullptr, &pass_$);
						if (pass_$)
						{
							if (name == name_no_namespace)
								name = "ctor";
							else if (name[0] == '~')
								name = "dtor";

							FunctionDesc desc;
							symbol_to_function(_function, desc);
							if (desc.rva)
							{
								auto f = (FunctionInfoPrivate*)u->add_function(name, desc.rva, desc.ret_type, "");
								for (auto& p : desc.parameters)
									f->add_parameter(p);
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

							if (false && std::filesystem::path(module_filename).extension() == L".exe")
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
					FunctionDesc desc;
					symbol_to_function(_function, desc);
					if (desc.rva)
					{
						auto f = db->add_function(name, desc.rva, desc.ret_type, "");
						for (auto& p : desc.parameters)
							f->add_parameter(p);
					}
				}
			}

			_function->Release();
		}
		_functions->Release();

		return db;
	}

	TypeinfoDatabase* TypeinfoDatabase::load(const std::vector<TypeinfoDatabase*>& _dbs, const std::wstring& typeinfo_filename)
	{
		auto file = SerializableNode::create_from_xml_file(typeinfo_filename);
		if (!file)
		{
			assert(0);
			return nullptr;
		}

		auto unserialize_function = [](SerializableNode* src, std::string& name, voidptr& rva, std::string& code) 
		->TypeInfo{
			name = src->find_attr("name")->value();
			rva = (void*)std::stoul(src->find_attr("rva")->value().c_str());
			auto ret = TypeInfo::from_str(src->find_attr("return_type")->value());
			auto n_code = src->find_node("code_pos");
			if (n_code)
				code = n_code->value();
			return ret;
		};

		auto unserialize_parameters = [](SerializableNode* src, FunctionInfo* f) {
			auto n_parameters = src->find_node("parameters");
			if (n_parameters)
			{
				for (auto i = 0; i < n_parameters->node_count(); i++)
					f->add_parameter(TypeInfo::from_str(n_parameters->node(i)->find_attr("type")->value()));
			}
		};

		auto db = new TypeinfoDatabasePrivate;
		db->module_name = std::filesystem::path(typeinfo_filename).replace_extension(L".dll");
		auto dbs = _dbs;
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
			std::string name; void* rva; std::string code_pos;
			auto type = unserialize_function(n_function, name, rva, code_pos);
			unserialize_parameters(n_function, db->add_function(name, rva, type, code_pos));
		}

		auto this_module = load_module(L"flame_foundation.dll");
		TypeinfoDatabase* this_db = nullptr;
		for (auto db : dbs)
		{
			if (std::filesystem::path(db->module_name()).filename() == L"flame_foundation.dll")
			{
				this_db = db;
				break;
			}
		}
		assert(this_module && this_db);
		auto n_udts = file->find_node("udts");
		for (auto i = 0; i < n_udts->node_count(); i++)
		{
			auto n_udt = n_udts->node(i);
			auto u = (UdtInfoPrivate*)db->add_udt(TypeInfo::from_str(n_udt->find_attr("name")->value()), std::stoi(n_udt->find_attr("size")->value()));

			auto n_items = n_udt->find_node("variables");
			for (auto j = 0; j < n_items->node_count(); j++)
			{
				auto n_vari = n_items->node(j);
				auto type = TypeInfo::from_str(n_vari->find_attr("type")->value());
				auto v = (VariableInfoPrivate*)u->add_variable(type, n_vari->find_attr("name")->value(), n_vari->find_attr("decoration")->value(), std::stoi(n_vari->find_attr("offset")->value()), std::stoi(n_vari->find_attr("size")->value()));
				if (v->default_value)
				{
					auto a_default_value = n_vari->find_attr("default_value");
					if (a_default_value)
						type.unserialize(dbs, a_default_value->value(), v->default_value, this_module, this_db);
				}
			}

			auto n_functions = n_udt->find_node("functions");
			if (n_functions)
			{
				for (auto j = 0; j < n_functions->node_count(); j++)
				{
					auto n_function = n_functions->node(j);
					std::string name; void* rva; std::string code_pos;
					unserialize_parameters(n_function, u->add_function(name, rva, unserialize_function(n_function, name, rva, code_pos), code_pos));
				}
			}
		}
		free_module(this_module);

		SerializableNode::destroy(file);

		return db;
	}

	void TypeinfoDatabase::save(const std::vector<TypeinfoDatabase*>& _dbs, TypeinfoDatabase* _db)
	{
		auto file = SerializableNode::create("typeinfo");

		auto serialize_function = [](FunctionInfoPrivate * src, SerializableNode * dst) {
			dst->new_attr("name", src->name);
			dst->new_attr("rva", std::to_string((uint)src->rva));
			dst->new_attr("return_type", src->return_type.name);
			if (!src->parameter_types.empty())
			{
				auto n_parameters = dst->new_node("parameters");
				for (auto& p : src->parameter_types)
					n_parameters->new_node("parameter")->new_attr("type", p.name);
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
				n_udt->new_attr("name", u->type.name);
				n_udt->new_attr("size", std::to_string(u->size));

				auto n_items = n_udt->new_node("variables");
				for (auto& v : u->variables)
				{
					auto n_vari = n_items->new_node("variable");
					const auto& type = v->type;
					n_vari->new_attr("type", type.name);
					n_vari->new_attr("name", v->name);
					n_vari->new_attr("decoration", v->decoration);
					n_vari->new_attr("offset", std::to_string(v->offset));
					n_vari->new_attr("size", std::to_string(v->size));
					if (v->default_value)
					{
						auto default_value_str = type.serialize(dbs, v->default_value, 1);
						if (!default_value_str.empty())
							n_vari->new_attr("default_value", default_value_str);
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

		SerializableNode::save_to_xml_file(file, std::filesystem::path(db->module_name).replace_extension(L".typeinfo"));
		SerializableNode::destroy(file);
	}

	void TypeinfoDatabase::destroy(TypeinfoDatabase* db)
	{
		delete (TypeinfoDatabasePrivate*)db;
	}
}
