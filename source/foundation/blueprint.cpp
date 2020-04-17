#include <flame/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/foundation/typeinfo.h>

namespace flame
{
	struct BPPrivate;
	struct NodePrivate;
	struct SlotPrivate;

	struct SlotPrivate : BP::Slot
	{
		NodePrivate* node;
		IO io;
		uint index;
		const TypeInfo* type;
		std::string name;
		uint offset;
		uint size;
		std::string default_value;
		void* data;

		std::vector<SlotPrivate*> links;

		SlotPrivate(NodePrivate* node, IO io, uint index, const TypeInfo* type, const std::string& name, uint offset, uint size, const std::string& default_value);
		SlotPrivate(NodePrivate* node, IO io, uint index, VariableInfo* vi);

		void set_data(const void* data);
		bool link_to(SlotPrivate* target);

		StringA get_address() const;
	};

	struct SlotDesc
	{
		const TypeInfo* type;
		std::string name;
		uint offset;
		uint size;
		std::string default_value;
	};

	struct NodePrivate : BP::Node
	{
		BPPrivate* scene;
		std::string id;
		std::string type;
		UdtInfo* udt;

		void* object;
		void* module;

		void* dtor_addr;
		void* update_addr;

		std::vector<std::unique_ptr<SlotPrivate>> inputs;
		std::vector<std::unique_ptr<SlotPrivate>> outputs;

		uint order;

		NodePrivate(BPPrivate* scene, const std::string& id, UdtInfo* udt, void* module);
		NodePrivate(BPPrivate* scene, const std::string& id, const std::string& type, uint size, 
			const std::vector<SlotDesc>& inputs, const std::vector<SlotDesc>& outputs, void* ctor_addr, void* dtor_addr, void* update_addr);
		~NodePrivate();

		SlotPrivate* find_input(const std::string& name) const;
		SlotPrivate* find_output(const std::string& name) const;

		void update();
	};

	struct BPPrivate : BP
	{
		std::wstring filename;

		std::vector<std::unique_ptr<NodePrivate>> nodes;

		std::vector<NodePrivate*> update_list;
		bool need_rebuild_update_list;

		std::vector<std::filesystem::path> used_resources;

		BPPrivate()
		{
			time = 0.f;
			need_rebuild_update_list = true;
		}

		bool check_or_create_id(std::string& id, const std::string& type) const;
		NodePrivate* add_node(const std::string& type, const std::string& id);
		void remove_node(NodePrivate* n);
		NodePrivate* find_node(const std::string& address) const;
		SlotPrivate* find_input(const std::string& address) const;
		SlotPrivate* find_output(const std::string& address) const;

		void clear();

		void update();
	};

	SlotPrivate::SlotPrivate(NodePrivate* node, IO io, uint index, const TypeInfo* type, const std::string& name, uint offset, uint size, const std::string& default_value) :
		node(node),
		io(io),
		index(index),
		type(type),
		name(name),
		offset(offset),
		size(size),
		default_value(default_value)
	{
		data = (char*)node->object + offset;
		user_data = nullptr;

		if (io == In)
			links.push_back(nullptr);
	}

	SlotPrivate::SlotPrivate(NodePrivate* node, IO io, uint index, VariableInfo* vi) :
		SlotPrivate(node, io, index, vi->type(), vi->name(),
			vi->offset(), vi->size(), vi->default_value())
	{
	}

	void SlotPrivate::set_data(const void* d)
	{
		type->copy_from(d, data);
	}

	bool SlotPrivate::link_to(SlotPrivate* target)
	{
		assert(io == In);

		if (links[0] == target)
			return true;

		if (target)
		{
			if (target->io == In)
				return false;
			if (node == target->node)
				return false;
		}

		if (target)
		{
			if (!can_link(type, target->type))
				return false;
		}

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
		if (target)
			target->links.push_back(this);

		if (!target && type->tag() == TypePointer)
		{
			void* p = nullptr;
			memcpy(data, &p, sizeof(void*));
		}

		return true;
	}

	StringA SlotPrivate::get_address() const
	{
		return StringA(node->id + "." + name);
	}

	static BP::Node* _current_node = nullptr;

	NodePrivate::NodePrivate(BPPrivate* scene, const std::string& id, UdtInfo* udt, void* module) :
		scene(scene),
		id(id),
		type(udt->type()->name() + 2),
		udt(udt),
		module(module),
		order(0xffffffff)
	{
		pos = Vec2f(0.f);
		user_data = nullptr;

		auto size = udt->size();
		object = malloc(size);
		memset(object, 0, size);

		auto thiz = this;
		memcpy(object, &thiz, sizeof(void*));
		{
			auto f = udt->find_function("ctor");
			if (f && f->parameter_count() == 0)
				cmf(p2f<MF_v_v>((char*)module + (uint)f->rva()), object);
		}

		dtor_addr = nullptr;
		{
			auto f = udt->find_function("dtor");
			if (f)
				dtor_addr = (char*)module + (uint)f->rva();
		}

		update_addr = nullptr;
		{
			auto f = udt->find_function("update");
			assert(f && check_function(f, "D#void", { "D#uint" }));
			update_addr = (char*)module + (uint)f->rva();
		}
		assert(update_addr);

		for (auto i = 0; i < udt->variable_count(); i++)
		{
			auto v = udt->variable(i);
			auto flags = v->flags();
			assert(flags);
			if (flags & VariableFlagInput)
				inputs.emplace_back(new SlotPrivate(this, BP::Slot::In, inputs.size(), v));
			else if (flags & VariableFlagOutput)
				outputs.emplace_back(new SlotPrivate(this, BP::Slot::Out, outputs.size(), v));
		}
	}

	NodePrivate::NodePrivate(BPPrivate* scene, const std::string& id, const std::string& type, uint size, 
		const std::vector<SlotDesc>& _inputs, const std::vector<SlotDesc>& _outputs, void* ctor_addr, void* _dtor_addr, void* _update_addr) :
		scene(scene),
		id(id),
		type(type),
		udt(nullptr),
		module(nullptr),
		order(0xffffffff)
	{
		pos = Vec2f(0.f);
		user_data = nullptr;

		object = malloc(size);
		memset(object, 0, size);

		if (ctor_addr)
			cmf(p2f<MF_v_v>(ctor_addr), object);

		dtor_addr = _dtor_addr;
		update_addr = _update_addr;
		assert(update_addr);

		for (auto i = 0; i < _inputs.size(); i++)
		{
			auto& d = _inputs[i];
			inputs.emplace_back(new SlotPrivate(this, BP::Slot::In, i, d.type, d.name, d.offset, d.size, d.default_value));
		}
		for (auto i = 0; i < _outputs.size(); i++)
		{
			auto& d = _outputs[i];
			outputs.emplace_back(new SlotPrivate(this, BP::Slot::Out, i, d.type, d.name, d.offset, d.size, d.default_value));
		}
	}

	NodePrivate::~NodePrivate()
	{
		if (dtor_addr)
			cmf(p2f<MF_v_v>(dtor_addr), object);
		free(object);
	}

	SlotPrivate* NodePrivate::find_input(const std::string& name) const
	{
		for (auto& input : ((NodePrivate*)this)->inputs)
		{
			if (name == input->name)
				return input.get();
		}
		return nullptr;
	}

	SlotPrivate* NodePrivate::find_output(const std::string& name) const
	{
		for (auto& output : ((NodePrivate*)this)->outputs)
		{
			if (name == output->name)
				return output.get();
		}
		return nullptr;
	}

	void NodePrivate::update()
	{
		for (auto& in : inputs)
		{
			auto out = in->links[0];
			if (out)
			{
				if (out->type->tag() == TypeData && in->type->tag() == TypePointer)
					memcpy(in->data, &out->data, sizeof(void*));
				else
					memcpy(in->data, out->data, in->size);
			}
		}

		_current_node = this;
		cmf(p2f<MF_v_v>(update_addr), object);
		_current_node = nullptr;
	}

	bool BPPrivate::check_or_create_id(std::string& id, const std::string& type) const
	{
		if (!id.empty())
		{
			if (find_node(id))
				return false;
		}
		else
		{
			auto prefix = type + "_";
			auto last_colon = prefix.find_last_of(L':');
			if (last_colon != std::string::npos)
				prefix = std::string(prefix.begin() + last_colon + 1, prefix.end());
			for (auto i = 0; i < nodes.size() + 1; i++)
			{
				id = prefix + std::to_string(i);
				if (!find_node(id))
					break;
			}
		}
		return true;
	}

	NodePrivate* BPPrivate::add_node(const std::string& type, const std::string& _id)
	{
		std::string id = _id;
		if (!check_or_create_id(id, type))
		{
			printf("cannot add node, id repeated\n");
			return nullptr;
		}

		NodePrivate* n = nullptr;

		auto add_enum_node = [&](TypeTag tag, const std::string& enum_name) {
#pragma pack(1)
			struct Dummy
			{
				int in;
				int out;

				void update()
				{
					out = in;
				}
			};
#pragma pack()
			n = new NodePrivate(this, id, type, sizeof(Dummy), {
					{TypeInfo::get(tag, enum_name.c_str()), "in", offsetof(Dummy, in), sizeof(Dummy::in), ""}
				}, {
					{TypeInfo::get(tag, enum_name.c_str()), "out", offsetof(Dummy, out), sizeof(Dummy::out), ""}
				}, nullptr, nullptr, f2v(&Dummy::update));
		};
		std::string parameters;
		switch (type_from_node_name(type, parameters))
		{
		case 'S':
			add_enum_node(TypeEnumSingle, parameters);
			break;
		case 'M':
			add_enum_node(TypeEnumMulti, parameters);
			break;
		case 'V':
		{
#pragma pack(1)
			struct Dummy
			{
				uint type_hash;
				uint type_size;

				void dtor()
				{
					auto in = (char*)&type_size + sizeof(uint);
					auto out = (char*)&type_size + sizeof(uint) + type_size;
					basic_type_dtor(type_hash, in);
					basic_type_dtor(type_hash, out);
				}

				void update()
				{
					auto in = (char*)&type_size + sizeof(uint);
					auto out = (char*)&type_size + sizeof(uint) + type_size;
					basic_type_copy(type_hash, in, out, type_size);
				}
			};
#pragma pack()
			auto type_hash = FLAME_HASH(parameters.c_str());
			auto type_size = basic_type_size(type_hash);
			n = new NodePrivate(this, id, type, sizeof(Dummy) + type_size * 2, {
					{TypeInfo::get(TypeData, parameters.c_str()), "in", sizeof(Dummy), type_size, ""}
				}, {
					{TypeInfo::get(TypeData, parameters.c_str()), "out", sizeof(Dummy) + type_size, type_size, ""}
				}, nullptr, f2v(&Dummy::dtor), f2v(&Dummy::update));
			auto obj = n->object;
			*(uint*)((char*)obj + sizeof(void*)) = type_hash;
			*(uint*)((char*)obj + sizeof(void*) + sizeof(uint)) = type_size;
		}
		break;
		case 'A':
		{
			auto sp = SUS::split(parameters, '+');
#pragma pack(1)
			struct Dummy
			{
				uint type_hash;
				uint type_size;
				uint size;

				void dtor()
				{
					for (auto i = 0; i < size; i++)
						basic_type_dtor(type_hash, (char*)&size + sizeof(uint) + type_size * i);
					auto& out = *(Array<int>*)((char*)&size + sizeof(uint) + type_size * size);
					for (auto i = 0; i < out.s; i++)
						basic_type_dtor(type_hash, (char*)out.v + type_size * i);
					f_free(out.v);
				}

				void update()
				{
					auto& out = *(Array<int>*)((char*)&size + sizeof(uint) + type_size * size);
					if (out.s != size)
					{
						out.s = size;
						auto m_size = type_size * size;
						out.v = (int*)f_malloc(m_size);
						memset(out.v, 0, m_size);
					}
					for (auto i = 0; i < size; i++)
					{
						auto v = (char*)&size + sizeof(uint) + type_size * i;
						basic_type_copy(type_hash, v, (char*)out.v + type_size * i, type_size);
					}
				}
			};
#pragma pack()
			auto tag = TypeData;
			auto type_name = sp[1];
			auto base_name = type_name;
			if (type_name.back() == '*')
			{
				base_name.erase(base_name.end() - 1);
				tag = TypePointer;
			}
			auto type_hash = FLAME_HASH(base_name.c_str());
			uint type_size = tag == TypeData ? basic_type_size(type_hash) : sizeof(void*);
			auto size = stoi(sp[0]);
			std::vector<SlotDesc> inputs;
			for (auto i = 0; i < size; i++)
			{
				inputs.push_back({
					TypeInfo::get(tag, base_name.c_str()), std::to_string(i),
					sizeof(Dummy) + type_size * i, type_size, ""
					});
			}
			n = new NodePrivate(this, id, type, sizeof(Dummy) + type_size * size + sizeof(Array<int>), inputs, {
					{ TypeInfo::get(TypeData, type_name.c_str(), true), "out", sizeof(Dummy) + type_size * size, sizeof(Array<int>), "" }
				}, nullptr, f2v(&Dummy::dtor), f2v(&Dummy::update));
			auto obj = n->object;
			*(uint*)((char*)obj + sizeof(void*)) = type_hash;
			*(uint*)((char*)obj + sizeof(void*) + sizeof(uint)) = type_size;
			*(uint*)((char*)obj + sizeof(void*) + sizeof(uint) + sizeof(uint)) = size;
		}
		break;
		default:
			auto udt = find_udt(FLAME_HASH(("D#" + type).c_str()));

			if (!udt)
			{
				printf("cannot add node, type: %s\n", type.c_str());
				return nullptr;
			}

			n = new NodePrivate(this, id, udt, udt->db()->module());
		}

		nodes.emplace_back(n);

		need_rebuild_update_list = true;

		return n;
	}

	void BPPrivate::remove_node(NodePrivate* _n)
	{
		for (auto it = nodes.begin(); it != nodes.end(); it++)
		{
			auto n = it->get();
			if (n == _n)
			{
				for (auto& i : n->inputs)
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
				for (auto& o : n->outputs)
				{
					for (auto& l : o->links)
						l->links[0] = nullptr;
				}
				nodes.erase(it);
				break;
			}
		}

		need_rebuild_update_list = true;
	}

	NodePrivate* BPPrivate::find_node(const std::string& id) const
	{
		for (auto& n : ((BPPrivate*)this)->nodes)
		{
			if (n->id == id)
				return n.get();
		}
		return nullptr;
	}

	SlotPrivate* BPPrivate::find_input(const std::string& address) const
	{
		auto sp = SUS::split_lastone(address, '.');
		auto n = find_node(sp[0]);
		if (!n)
			return nullptr;
		return (SlotPrivate*)n->find_input(sp[1]);
	}

	SlotPrivate* BPPrivate::find_output(const std::string& address) const
	{
		auto sp = SUS::split_lastone(address, '.');
		auto n = find_node(sp[0]);
		if (!n)
			return nullptr;
		return (SlotPrivate*)n->find_output(sp[1]);
	}

	void BPPrivate::clear()
	{
		nodes.clear();
		need_rebuild_update_list = true;
		update_list.clear();
	}

	static void get_order(BPPrivate* scn, NodePrivate* n, uint& order)
	{
		if (n->order != 0xffffffff)
			return;
		for (auto& i : n->inputs)
		{
			auto o = i->links[0];
			if (o)
			{
				auto nn = o->node;
				get_order(scn, nn, order);
			}
		}
		n->order = order++;
	}

	void BPPrivate::update()
	{
		if (need_rebuild_update_list)
		{
			for (auto& n : nodes)
				n->order = 0xffffffff;
			auto order = 0U;
			for (auto& n : nodes)
				get_order(this, n.get(), order);
			update_list.clear();
			for (auto& n : nodes)
			{
				std::vector<NodePrivate*>::iterator it;
				for (it = update_list.begin(); it != update_list.end(); it++)
				{
					if (n->order < (*it)->order)
						break;
				}
				update_list.emplace(it, n.get());
			}
		}

		for (auto n : update_list)
			n->update();

		time += looper().delta_time;
	}

	BP::Node* BP::Slot::node() const
	{
		return ((SlotPrivate*)this)->node;
	}

	BP::Slot::IO BP::Slot::io() const
	{
		return ((SlotPrivate*)this)->io;
	}

	uint BP::Slot::index() const
	{
		return ((SlotPrivate*)this)->index;
	}

	const TypeInfo* BP::Slot::type() const
	{
		return ((SlotPrivate*)this)->type;
	}

	const char* BP::Slot::name() const
	{
		return ((SlotPrivate*)this)->name.c_str();
	}

	uint BP::Slot::offset() const
	{
		return ((SlotPrivate*)this)->offset;
	}

	uint BP::Slot::size() const
	{
		return ((SlotPrivate*)this)->size;
	}

	const char* BP::Slot::default_value() const
	{
		return ((SlotPrivate*)this)->default_value.c_str();
	}

	void* BP::Slot::data() const
	{
		return ((SlotPrivate*)this)->data;
	}

	void BP::Slot::set_data(const void* d)
	{
		((SlotPrivate*)this)->set_data(d);
	}

	uint BP::Slot::link_count() const
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

	StringA BP::Slot::get_address() const
	{
		return ((SlotPrivate*)this)->get_address();
	}

	BP *BP::Node::scene() const
	{
		return ((NodePrivate*)this)->scene;
	}

	const char* BP::Node::id() const
	{
		return ((NodePrivate*)this)->id.c_str();
	}

	bool BP::Node::set_id(const char* id)
	{
		if (!id || !id[0])
			return false;
		auto thiz = (NodePrivate*)this;
		if (thiz->id == id)
			return true;
		for (auto& n : thiz->scene->nodes)
		{
			if (n->id == id)
				return false;
		}
		thiz->id = id;
		return true;
	}

	const char* BP::Node::type() const
	{
		return ((NodePrivate*)this)->type.c_str();
	}

	UdtInfo* BP::Node::udt() const
	{
		return ((NodePrivate*)this)->udt;
	}

	uint BP::Node::input_count() const
	{
		return ((NodePrivate*)this)->inputs.size();
	}

	BP::Slot *BP::Node::input(uint idx) const
	{
		return ((NodePrivate*)this)->inputs[idx].get();
	}

	uint BP::Node::output_count() const
	{
		return ((NodePrivate*)this)->outputs.size();
	}

	BP::Slot* BP::Node::output(uint idx) const
	{
		return ((NodePrivate*)this)->outputs[idx].get();
	}

	BP::Slot* BP::Node::find_input(const char* name) const
	{
		return ((NodePrivate*)this)->find_input(name);
	}

	BP::Slot* BP::Node::find_output(const char* name) const
	{
		return ((NodePrivate*)this)->find_output(name);
	}

	BP::Node* BP::Node::current()
	{
		return _current_node;
	}

	const wchar_t* BP::filename() const
	{
		return ((BPPrivate*)this)->filename.c_str();
	}

	uint BP::node_count() const
	{
		return ((BPPrivate*)this)->nodes.size();
	}

	BP::Node* BP::node(uint idx) const
	{
		return ((BPPrivate*)this)->nodes[idx].get();
	}

	BP::Node* BP::add_node(const char* type, const char* id)
	{
		return ((BPPrivate*)this)->add_node(type, id);
	}

	void BP::remove_node(BP::Node *n)
	{
		((BPPrivate*)this)->remove_node((NodePrivate*)n);
	}

	BP::Node* BP::find_node(const char* id) const
	{
		return ((BPPrivate*)this)->find_node(id);
	}

	BP::Slot* BP::find_input(const char* address) const
	{
		return ((BPPrivate*)this)->find_input(address);
	}

	BP::Slot* BP::find_output(const char* address) const
	{
		return ((BPPrivate*)this)->find_output(address);
	}

	void BP::clear()
	{
		((BPPrivate*)this)->clear();
	}

	void BP::update()
	{
		((BPPrivate*)this)->update();
	}

	BP* BP::create_from_file(const wchar_t* filename)
	{
		auto s_filename = w2s(filename);
		auto path = std::filesystem::path(filename);
		auto ppath = path.parent_path();
		auto ppath_str = ppath.wstring();

		printf("begin to load bp: %s\n", s_filename.c_str());

		pugi::xml_document file;
		pugi::xml_node file_root;
		if (!file.load_file(filename) || (file_root = file.first_child()).name() != std::string("BP"))
		{
			printf("bp file does not exist, abort\n", s_filename.c_str());
			printf("end loading bp: %s\n", s_filename.c_str());
			return nullptr;
		}

		struct DataDesc
		{
			std::string name;
			std::string value;
		};
		struct NodeDesc
		{
			std::string type;
			std::string id;
			Vec2f pos;
			std::vector<DataDesc> datas;
		};
		std::vector<NodeDesc> node_descs;

		for (auto n_node : file_root.child("nodes"))
		{
			NodeDesc node;

			node.type = n_node.attribute("type").value();
			node.id = n_node.attribute("id").value();
			node.pos = stof2(n_node.attribute("pos").value());

			for (auto n_data : n_node.child("datas"))
			{
				DataDesc data;
				data.name = n_data.attribute("name").value();
				data.value = n_data.attribute("value").value();
				node.datas.push_back(data);
			}

			node_descs.push_back(node);
		}

		struct LinkDesc
		{
			std::string out_addr;
			std::string in_addr;
		};
		std::vector<LinkDesc> link_descs;

		for (auto n_link : file_root.child("links"))
		{
			auto o_addr = n_link.attribute("out").value();
			auto i_addr = n_link.attribute("in").value();

			LinkDesc link;
			link.out_addr = o_addr;
			link.in_addr = i_addr;
			link_descs.push_back(link);
		}

		auto bp = new BPPrivate();
		bp->filename = filename;

		for (auto& n_d : node_descs)
		{
			auto n = bp->add_node(n_d.type, n_d.id);
			if (n)
			{
				n->pos = n_d.pos;
				for (auto& d_d : n_d.datas)
				{
					auto input = n->find_input(d_d.name);
					auto type = input->type;
					auto tag = type->tag();
					if (!type->is_array() && (tag == TypeEnumSingle || tag == TypeEnumMulti || tag == TypeData))
						type->unserialize(d_d.value, input->data);
				}
			}
		}

		for (auto& l_d : link_descs)
		{
			auto o = bp->find_output(l_d.out_addr);
			auto i = bp->find_input(l_d.in_addr);
			if (o && i)
			{
				if (!i->link_to(o))
					printf("link type mismatch: %s - > %s\n", l_d.out_addr.c_str(), l_d.in_addr.c_str());
			}
			else
				printf("cannot link: %s - > %s\n", l_d.out_addr.c_str(), l_d.in_addr.c_str());
		}

		printf("end loading bp: %s\n", s_filename.c_str());

		return bp;
	}

	void BP::save_to_file(BP* _bp, const wchar_t* filename)
	{
		auto bp = (BPPrivate*)_bp;

		bp->filename = filename;

		pugi::xml_document file;
		auto file_root = file.append_child("BP");

		auto n_nodes = file_root.append_child("nodes");
		for (auto& n : bp->nodes)
		{
			auto udt = n->udt;

			auto n_node = n_nodes.append_child("node");
			n_node.append_attribute("type").set_value(n->type.c_str());
			n_node.append_attribute("id").set_value(n->id.c_str());
			n_node.append_attribute("pos").set_value(to_string(n->pos, 2).c_str());

			pugi::xml_node n_datas;
			for (auto& input : n->inputs)
			{
				if (input->links[0])
					continue;
				auto type = input->type;
				auto tag = type->tag();
				if (!type->is_array() && tag != TypePointer)
				{
					auto value_str = type->serialize(input->data, 2);
					if (value_str != input->default_value)
					{
						if (!n_datas)
							n_datas = n_node.append_child("datas");
						auto n_data = n_datas.append_child("data");
						n_data.append_attribute("name").set_value(input->name.c_str());
						n_data.append_attribute("value").set_value(type->serialize(input->data, 2).c_str());
					}
				}
			}
		}

		auto n_links = file_root.append_child("links");
		for (auto& n : bp->nodes)
		{
			for (auto& in : n->inputs)
			{
				auto out = in->links[0];
				if (out)
				{
					auto n_link = n_links.append_child("link");
					n_link.append_attribute("out").set_value(out->get_address().v);
					n_link.append_attribute("in").set_value(in->get_address().v);
				}
			}
		}

		file.save_file(filename);
	}

	void BP::destroy(BP *bp)
	{
		delete(BPPrivate*)bp;
	}

	struct FLAME_R(R_MakeVec2i)
	{
		FLAME_RV(int, x);
		FLAME_RV(int, y);

		FLAME_RV(Vec2i, out);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			out[0] = x;
			out[1] = y;
		}
	};

	struct FLAME_R(R_MakeVec3i)
	{
		FLAME_RV(int, x);
		FLAME_RV(int, y);
		FLAME_RV(int, z);

		FLAME_RV(Vec3i, out);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			out[0] = x;
			out[1] = y;
			out[2] = z;
		}
	};

	struct FLAME_R(R_MakeVec4i)
	{
		FLAME_RV(int, x);
		FLAME_RV(int, y);
		FLAME_RV(int, z);
		FLAME_RV(int, w);

		FLAME_RV(Vec4i, out);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			out[0] = x;
			out[1] = y;
			out[2] = z;
			out[3] = w;
		}
	};

	struct FLAME_R(R_MakeVec2u)
	{
		FLAME_RV(uint, x);
		FLAME_RV(uint, y);

		FLAME_RV(Vec2u, out);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			out[0] = x;
			out[1] = y;
		}
	};

	struct FLAME_R(R_MakeVec3u)
	{
		FLAME_RV(uint, x);
		FLAME_RV(uint, y);
		FLAME_RV(uint, z);

		FLAME_RV(Vec3u, out);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			out[0] = x;
			out[1] = y;
			out[2] = z;
		}
	};

	struct FLAME_R(R_MakeVec4u)
	{
		FLAME_RV(uint, x);
		FLAME_RV(uint, y);
		FLAME_RV(uint, z);
		FLAME_RV(uint, w);

		FLAME_RV(Vec4u, out);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			out[0] = x;
			out[1] = y;
			out[2] = z;
			out[3] = w;
		}
	};

	struct FLAME_R(R_MakeVec2f)
	{
		FLAME_RV(float, x);
		FLAME_RV(float, y);

		FLAME_RV(Vec2f, out);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			out[0] = x;
			out[1] = y;
		}
	};

	struct FLAME_R(R_MakeVec3f)
	{
		FLAME_RV(float, x);
		FLAME_RV(float, y);
		FLAME_RV(float, z);

		FLAME_RV(Vec3f, out);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			out[0] = x;
			out[1] = y;
			out[2] = z;
		}
	};

	struct FLAME_R(R_MakeVec4f)
	{
		FLAME_RV(float, x);
		FLAME_RV(float, y);
		FLAME_RV(float, z);
		FLAME_RV(float, w);

		FLAME_RV(Vec4f, out);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			out[0] = x;
			out[1] = y;
			out[2] = z;
			out[3] = w;
		}
	};

	struct FLAME_R(R_MakeVec2c)
	{
		FLAME_RV(uchar, x);
		FLAME_RV(uchar, y);

		FLAME_RV(Vec2c, out);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			out[0] = x;
			out[1] = y;
		}
	};

	struct FLAME_R(R_MakeVec3c)
	{
		FLAME_RV(uchar, x);
		FLAME_RV(uchar, y);
		FLAME_RV(uchar, z);

		FLAME_RV(Vec3c, out);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			out[0] = x;
			out[1] = y;
			out[2] = z;
		}
	};

	struct FLAME_R(R_MakeVec4c)
	{
		FLAME_RV(uchar, x);
		FLAME_RV(uchar, y);
		FLAME_RV(uchar, z);
		FLAME_RV(uchar, w);

		FLAME_RV(Vec4c, out);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			out[0] = x;
			out[1] = y;
			out[2] = z;
			out[3] = w;
		}
	};

	struct FLAME_R(R_Add)
	{
		FLAME_RV(float, a);
		FLAME_RV(float, b);

		FLAME_RV(float, out);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			out = a + b;
		}
	};

	struct FLAME_R(R_Multiple)
	{
		FLAME_RV(float, a);
		FLAME_RV(float, b);

		FLAME_RV(float, out);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			out = a * b;
		}
	};

	struct FLAME_R(R_Time)
	{
		FLAME_RV(float, delta);
		FLAME_RV(float, total);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			delta = looper().delta_time;
			total = BP::Node::current()->scene()->time;
		}
	};

	struct FLAME_R(R_Linear1d)
	{
		FLAME_RV(float, a);
		FLAME_RV(float, b);
		FLAME_RV(float, t);

		FLAME_RV(float, out);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			out = a + (b - a) * clamp(t, 0.f, 1.f);
		}
	};

	struct FLAME_R(R_Linear2d)
	{
		FLAME_RV(Vec2f, a);
		FLAME_RV(Vec2f, b);
		FLAME_RV(float, t);

		FLAME_RV(Vec2f, out);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			out = a + (b - a) * clamp(t, 0.f, 1.f);
		}
	};

	struct FLAME_R(R_KeyListener)
	{
		FLAME_RV(Key, key);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{

		}
	};
}

