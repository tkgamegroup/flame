#include <flame/foundation/serialize.h>
#include <flame/foundation/blueprint.h>

namespace flame
{
	struct BPPrivate;
	struct NodePrivate;
	struct SlotPrivate;

	struct SlotPrivate : BP::Slot
	{
		Type type;
		NodePrivate* node;
		VariableInfo* variable_info;

		void* data;

		std::vector<SlotPrivate*> links;

		SlotPrivate(Type _type, NodePrivate* _node, VariableInfo* _variable_info);
		~SlotPrivate();

		void set_data(const void* data);
		bool link_to(SlotPrivate*target);

		String get_address() const;
	};

	struct NodePrivate : BP::Node
	{
		BPPrivate *bp;
		std::string id;
		UdtInfo* udt;
		bool udt_from_default_db;

		Vec2f position;

		void* module;
		FunctionInfo* update_function;

		std::vector<std::unique_ptr<SlotPrivate>> inputs;
		std::vector<std::unique_ptr<SlotPrivate>> outputs;

		bool in_list;
		bool changed;

		void* dummy; // represents the object

		NodePrivate(BPPrivate *_bp, const std::string &_id, UdtInfo *_udt);
		~NodePrivate();

		SlotPrivate* find_input(const std::string &name) const;
		SlotPrivate* find_output(const std::string &name) const;

		void add_to_update_list();

		void pass_change();

		void update(bool delta_time);
	};

	struct BPPrivate : BP
	{
		std::wstring filename;

		std::vector<std::pair<int, std::wstring>> extra_typeinfos;

		std::vector<std::unique_ptr<NodePrivate>> nodes;
		std::vector<NodePrivate*> update_list;

		Vec2f pos;

		~BPPrivate();

		NodePrivate *add_node(const char* id, const char* type_name);
		void remove_node(NodePrivate *n);

		NodePrivate* find_node(const std::string &id) const;
		SlotPrivate* find_input(const std::string &address) const;
		SlotPrivate* find_output(const std::string &address) const;

		void clear();

		void build_update_list();

		void update(float delta_time);

		void load(SerializableNode* src);
		void load(const std::wstring& filename);
		void save(SerializableNode* src);
		void save(const std::wstring& filename);
	};

	SlotPrivate::SlotPrivate(Type _type, NodePrivate* _node, VariableInfo* _variable_info) :
		type(_type),
		node(_node),
		variable_info(_variable_info),
		data(nullptr)
	{
		auto size = variable_info->size();
		data = new char[size];
		memset(data, 0, size);
		if (variable_info->default_value())
			set(data, variable_info->type()->tag(), size, variable_info->default_value());

		if (type == Input)
			links.push_back(nullptr);
	}

	SlotPrivate::~SlotPrivate()
	{
		delete[]data;
	}

	void SlotPrivate::set_data(const void* d)
	{
		memcpy(data, d, variable_info->size());
		node->pass_change();
	}

	bool SlotPrivate::link_to(SlotPrivate* target)
	{
		if (type == Output)
		{
			assert(0);
			return false;
		}

		if (target && (target->type == Input || !TypeInfo::equal(variable_info->type(), target->variable_info->type())))
			return false;
		if (links[0] == target)
			return true;

		if (links[0])
		{
			auto o = links[0];
			for (auto it = o->links.begin(); it != o->links.end(); it++)
			{
				if (*it == this)
				{
					o->links.erase(it);
					break;
				}
			}
		}

		links[0] = target;
		target->links.push_back(this);

		node->pass_change();

		node->bp->build_update_list();

		return true;
	}

	String SlotPrivate::get_address() const
	{
		return node->id + "." + variable_info->name();
	}

	NodePrivate::NodePrivate(BPPrivate *_bp, const std::string &_id, UdtInfo *_udt) :
		bp(_bp),
		id(_id),
		udt(_udt),
		position(0.f),
		module(nullptr),
		in_list(false),
		changed(true)
	{
		module = load_module(udt->module_name());
		
		update_function = nullptr;
		{
			auto f = udt->find_function("update");
			if(f)
			{
				auto ret_t = f->return_type();
				if (ret_t->tag() == TypeTagVariable && ret_t->hash() == cH("bool"))
				{
					if (f->parameter_count() == 1)
					{
						auto t = f->parameter_type(0);
						if (t->tag() == TypeTagVariable && t->hash() == cH("float"))
							update_function = f;
					}
				}
			}
		}
		assert(update_function);

		for (auto i = 0; i < udt->variable_count(); i++)
		{
			auto v = udt->variable(i);
			auto attr = std::string(v->attribute());
			if (attr.find('i') != std::string::npos)
				inputs.emplace_back(new SlotPrivate(SlotPrivate::Input, this, v));
			if (attr.find('o') != std::string::npos)
				outputs.emplace_back(new SlotPrivate(SlotPrivate::Output, this, v));
		}

		auto size = udt->size();
		dummy = malloc(size);
		memset(dummy, 0, size);
	}

	NodePrivate::~NodePrivate()
	{
		free_module(module);

		for (auto& i : inputs)
		{
			auto o = i->links[0];
			if (o)
			{
				for (auto it = o->links.begin(); it != o->links.end(); it++)
				{
					if (*it == i.get())
					{
						o->links.erase(it);
						break;
					}
				}
			}
		}
		for (auto& o : outputs)
		{
			for (auto& l : o->links)
				l->links[0] = nullptr;
		}

		free(dummy);

		changed = true;
		update(-1.f);
	}

	SlotPrivate* NodePrivate::find_input(const std::string &name) const
	{
		for (auto& input : inputs)
		{
			if (name == input->variable_info->name())
				return input.get();
		}
		return nullptr;
	}

	SlotPrivate* NodePrivate::find_output(const std::string &name) const
	{
		for (auto& output : outputs)
		{
			if (name == output->variable_info->name())
				return output.get();
		}
		return nullptr;
	}

	void NodePrivate::add_to_update_list()
	{
		if (in_list)
			return;

		for (auto& input : inputs)
		{
			auto o = input->links[0];
			if (o)
				o->node->add_to_update_list();
		}

		bp->update_list.push_back(this);

		in_list = true;
	}

	void NodePrivate::pass_change()
	{
		if (id == "make_cmd")
			int cut = 1;

		changed = true;
		for (auto& o : outputs)
		{
			for (auto& l : o->links)
				l->node->pass_change();
		}
	}

	void NodePrivate::update(bool delta_time)
	{
		if (!changed)
			return;

		for (auto& input : inputs)
		{
			auto v = input->variable_info;
			set((char*)dummy + v->offset(), v->type()->tag(), v->size(), input->links[0] ? input->links[0]->data : input->data);
		}

		struct Dummy { };
		typedef bool (Dummy:: * F)(float);
		union
		{
			void* p;
			F f;
		}cvt;
		cvt.p = (char*)module + (uint)update_function->rva();
		changed = (*((Dummy*)dummy).*cvt.f)(delta_time);

		for (auto& output : outputs)
		{
			auto v = output->variable_info;
			set(output->data, v->type()->tag(), v->size(), (char*)dummy + v->offset());
		}
	}

	BPPrivate::~BPPrivate()
	{
		for (auto& t : extra_typeinfos)
			typeinfo_clear(t.first);
	}

	NodePrivate *BPPrivate::add_node(const char* type_name, const char *id)
	{
		auto type_name_sp = string_split(std::string(type_name), '#');
		UdtInfo* udt = nullptr;
		auto udt_from_default_db = true;
		if (type_name_sp.size() == 1)
			udt = find_udt(H(type_name_sp[0].c_str()));
		else if (type_name_sp.size() == 2)
		{
			udt_from_default_db = false;
			auto fn = s2w(type_name_sp[0]);
			for (auto& t : extra_typeinfos)
			{
				if (t.second == fn)
				{
					udt = find_udt(H(type_name_sp[1].c_str()), t.first);
					assert(udt);
					break;
				}
			}
			if (!udt)
			{
				auto abs_fn = std::fs::path(filename).parent_path().wstring() + L"\\" + fn;
				auto fn_cpp = abs_fn + L".cpp";
				if (!std::fs::exists(fn_cpp))
				{
					assert(0);
					return nullptr;
				}
				auto fn_dll = abs_fn + L".dll";
				auto fn_ti = abs_fn + L".typeinfo";
				if (!std::fs::exists(fn_dll) || std::fs::last_write_time(fn_dll) < std::fs::last_write_time(fn_cpp))
				{
					auto compile_output = compile_to_dll({ fn_cpp }, { }, fn_dll);
					if (!std::fs::exists(fn_dll) || std::fs::last_write_time(fn_dll) < std::fs::last_write_time(fn_cpp))
					{
						printf("compile error:\n%s\n", compile_output.v);
						assert(0);
						return nullptr;
					}
					if (std::fs::exists(fn_ti))
						std::fs::remove(fn_ti);
				}
				if (!std::fs::exists(fn_ti) || std::fs::last_write_time(fn_ti) < std::fs::last_write_time(fn_dll))
				{
					typeinfo_collect(fn_dll, 99);
					typeinfo_save(fn_ti, 99);
					typeinfo_clear(99);
				}
				auto lv = typeinfo_free_level();
				typeinfo_load(fn_ti, lv);
				extra_typeinfos.emplace_back(lv, fn);
				udt = find_udt(H(type_name_sp[1].c_str()), lv);
			}
			if (!udt)
				return nullptr;
		}
		else
			return nullptr;

		if (!udt)
			return nullptr;

		std::string s_id;
		if (id)
		{
			s_id = id;
			if (find_node(s_id))
				return nullptr;
		}
		else
		{
			for (auto i = 0; i < nodes.size() + 1; i++)
			{
				s_id = "node_" + std::to_string(i);
				if (find_node(s_id))
					continue;
			}
		}

		auto n = new NodePrivate(this, s_id, udt);
		n->udt_from_default_db = udt_from_default_db;
		nodes.emplace_back(n);

		build_update_list();

		return n;
	}

	void BPPrivate::remove_node(NodePrivate *n)
	{
		for (auto it = nodes.begin(); it != nodes.end(); it++)
		{
			if ((*it).get() == n)
			{
				(*it)->pass_change();
				nodes.erase(it);
				break;
			}
		}

		build_update_list();
	}

	NodePrivate *BPPrivate::find_node(const std::string &id) const
	{
		for (auto &n : nodes)
		{
			if (n->id == id)
				return n.get();
		}
		return nullptr;
	}

	SlotPrivate* BPPrivate::find_input(const std::string &address) const
	{
		auto sp = string_split(address, '.');
		auto n = find_node(sp[0]);
		if (!n)
			return nullptr;
		return (SlotPrivate*)n->find_input(sp[1]);
	}

	SlotPrivate* BPPrivate::find_output(const std::string &address) const
	{
		auto sp = string_split(address, '.');
		auto n = find_node(sp[0]);
		if (!n)
			return nullptr;
		return (SlotPrivate*)n->find_output(sp[1]);
	}

	void BPPrivate::clear()
	{
		nodes.clear();
		update_list.clear();
	}

	void BPPrivate::build_update_list()
	{
		update_list.clear();
		for (auto &n : nodes)
			n->in_list = false;
		for (auto &n : nodes)
			n->add_to_update_list();
	}

	void BPPrivate::update(float delta_time)
	{
		if (update_list.empty())
		{
			printf("no nodes or didn't call 'prepare'\n");
			return;
		}

		for (auto &n : update_list)
			n->update(delta_time);
	}

	void BPPrivate::load(SerializableNode* src)
	{
		auto n_nodes = src->find_node("nodes");
		for (auto i_n = 0; i_n < n_nodes->node_count(); i_n++)
		{
			auto n_node = n_nodes->node(i_n);
			auto type = n_node->find_attr("type")->value();
			auto id = n_node->find_attr("id")->value();

			auto n = add_node(type.c_str(), id.c_str());
			if (!n)
			{
				printf("node \"%s\" with type \"%s\" add failed\n", id.c_str(), type.c_str());
				continue;
			}
			auto a_pos = n_node->find_attr("pos");
			if (a_pos)
				n->position = stof2(a_pos->value().c_str());

			auto n_datas = n_node->find_node("datas");
			for (auto i_d = 0; i_d < n_datas->node_count(); i_d++)
			{
				auto n_data = n_datas->node(i_d);
				auto input = n->find_input(n_data->find_attr("name")->value());
				auto v = input->variable_info;
				auto type = v->type();
				if (v->default_value())
					unserialize_value(type->tag(), type->hash(), n_data->find_attr("value")->value(), input->data);
			}
		}

		auto n_links = src->find_node("links");
		auto lc = n_links->node_count();
		for (auto i_l = 0; i_l < n_links->node_count(); i_l++)
		{
			auto n_link = n_links->node(i_l);
			auto o_address = n_link->find_attr("out")->value();
			auto i_address = n_link->find_attr("in")->value();
			auto o = find_output(o_address);
			auto i = find_input(i_address);
			if (o && i)
			{
				if (!i->link_to(o))
					printf("link type mismatch: %s - > %s\n", o_address.c_str(), i_address.c_str());
			}
			else
				printf("unable to link: %s - > %s\n", o_address.c_str(), i_address.c_str());
		}
	}
	
	void BPPrivate::load(const std::wstring& _filename)
	{
		filename = _filename;

		auto file = SerializableNode::create_from_xml_file(filename);
		if (!file)
			return;

		load(file);

		SerializableNode::destroy(file);
	}

	void BPPrivate::save(SerializableNode* dst)
	{
		auto ppath = std::fs::path(filename).parent_path().wstring() + L"\\";

		auto n_nodes = dst->new_node("nodes");
		for (auto& n : nodes)
		{
			auto n_node = n_nodes->new_node("node");
			auto u = n->udt;
			auto tn = std::string(u->name());
			if (!n->udt_from_default_db)
			{
				auto path = std::fs::path(u->module_name());
				auto src = (path.parent_path() / path.stem()).wstring();
				if (ppath.size() < src.size() && src.compare(0, ppath.size(), ppath.c_str()) == 0)
					src.erase(src.begin(), src.begin() + ppath.size());
				tn = w2s(src) + "#" + tn;
			}
			n_node->new_attr("type", tn);
			n_node->new_attr("id", n->id);
			n_node->new_attr("pos", to_string(n->position, 2));

			auto n_datas = n_node->new_node("datas");
			for (auto& input : n->inputs)
			{
				auto v = input->variable_info;
				auto type = v->type();
				if (v->default_value() && !compare(type->tag(), v->size(), v->default_value(), input->data))
				{
					auto n_data = n_datas->new_node("data");
					n_data->new_attr("name", v->name());
					n_data->new_attr("value", serialize_value(type->tag(), type->hash(), input->data, 2).v);
				}
			}
		}

		auto n_links = dst->new_node("links");
		for (auto& n : nodes)
		{
			for (auto& input : n->inputs)
			{
				if (input->links[0])
				{
					auto n_link = n_links->new_node("link");
					n_link->new_attr("out", input->links[0]->get_address().v);
					n_link->new_attr("in", input->get_address().v);
				}
			}
		}
	}

	void BPPrivate::save(const std::wstring& _filename)
	{
		filename = _filename;

		auto file = SerializableNode::create("BP");

		save(file);

		file->save_xml(filename);
		SerializableNode::destroy(file);
	}

	void* BP::Slot::data()
	{
		return ((SlotPrivate*)this)->data;
	}

	void BP::Slot::set_data(const void* d)
	{
		((SlotPrivate*)this)->set_data(d);
	}

	int BP::Slot::link_count() const
	{
		return ((SlotPrivate*)this)->links.size();
	}

	BP::Slot* BP::Slot::link(int idx) const
	{
		return ((SlotPrivate*)this)->links[idx];
	}

	bool BP::Slot::link_to(BP::Slot*target)
	{
		return ((SlotPrivate*)this)->link_to((SlotPrivate*)target);
	}

	String BP::Slot::get_address() const
	{
		return ((SlotPrivate*)this)->get_address();
	}

	BP::Node *BP::Slot::node() const
	{
		return ((SlotPrivate*)this)->node;
	}

	VariableInfo *BP::Slot::variable_info() const
	{
		return ((SlotPrivate*)this)->variable_info;
	}

	BP *BP::Node::bp() const
	{
		return ((NodePrivate*)this)->bp;
	}

	const char *BP::Node::id() const
	{
		return ((NodePrivate*)this)->id.c_str();
	}

	UdtInfo *BP::Node::udt() const
	{
		return ((NodePrivate*)this)->udt;
	}

	Vec2f BP::Node::position() const
	{
		return ((NodePrivate*)this)->position;
	}

	void BP::Node::set_position(const Vec2f& p)
	{
		((NodePrivate*)this)->position = p;
	}

	int BP::Node::input_count() const
	{
		return ((NodePrivate*)this)->inputs.size();
	}

	BP::Slot *BP::Node::input(int idx) const
	{
		return ((NodePrivate*)this)->inputs[idx].get();
	}

	int BP::Node::output_count() const
	{
		return ((NodePrivate*)this)->outputs.size();
	}

	BP::Slot*BP::Node::output(int idx) const
	{
		return ((NodePrivate*)this)->outputs[idx].get();
	}

	BP::Slot*BP::Node::find_input(const char *name) const
	{
		return ((NodePrivate*)this)->find_input(name);
	}

	BP::Slot*BP::Node::find_output(const char *name) const
	{
		return ((NodePrivate*)this)->find_output(name);
	}

	int BP::node_count() const
	{
		return ((BPPrivate*)this)->nodes.size();
	}

	BP::Node *BP::node(int idx) const
	{
		return ((BPPrivate*)this)->nodes[idx].get();
	}

	BP::Node *BP::add_node(const char* type_name, const char *id)
	{
		return ((BPPrivate*)this)->add_node(type_name, id);
	}

	void BP::remove_node(BP::Node *n)
	{
		((BPPrivate*)this)->remove_node((NodePrivate*)n);
	}

	BP::Node *BP::find_node(const char *id) const
	{
		return ((BPPrivate*)this)->find_node(id);
	}

	BP::Slot*BP::find_input(const char *address) const
	{
		return ((BPPrivate*)this)->find_input(address);
	}

	BP::Slot*BP::find_output(const char *address) const
	{
		return ((BPPrivate*)this)->find_output(address);
	}

	void BP::clear()
	{
		((BPPrivate*)this)->clear();
	}

	void BP::update(float delta_time)
	{
		((BPPrivate*)this)->update(delta_time);
	}

	void BP::load(SerializableNode* src)
	{
		((BPPrivate*)this)->load(src);
	}

	void BP::load(const std::wstring& filename)
	{
		((BPPrivate*)this)->load(filename);
	}

	void BP::save(SerializableNode* dst)
	{
		((BPPrivate*)this)->save(dst);
	}

	void BP::save(const std::wstring& filename)
	{
		((BPPrivate*)this)->save(filename);
	}

	BP *BP::create()
	{
		return new BPPrivate();
	}

	BP *BP::create_from_file(const std::wstring& filename)
	{
		if (!std::fs::exists(filename))
			return nullptr;

		auto bp = new BPPrivate();
		bp->load(filename);
		return bp;
	}

	void BP::destroy(BP *bp)
	{
		delete(BPPrivate*)bp;
	}
}

