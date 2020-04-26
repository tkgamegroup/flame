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
		void* default_value;
		void* data;
		Setter* setter;

		std::vector<SlotPrivate*> links;

		SlotPrivate(NodePrivate* node, IO io, uint index, const TypeInfo* type, const std::string& name, uint offset, uint size, const void* default_value);
		SlotPrivate(NodePrivate* node, IO io, uint index, VariableInfo* vi);
		~SlotPrivate();

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
	};

	struct NodePrivate : BP::Node
	{
		BPPrivate* scene;
		BP::ObjectType object_type;
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

		NodePrivate(BPPrivate* scene, BP::ObjectType object_type, const std::string& id, UdtInfo* udt, void* module);
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

		bool check_or_create_id(std::string& id) const;
		NodePrivate* add_node(const std::string& id, const std::string& type, BP::ObjectType object_type);
		void remove_node(NodePrivate* n);
		NodePrivate* find_node(const std::string& address) const;
		SlotPrivate* find_input(const std::string& address) const;
		SlotPrivate* find_output(const std::string& address) const;

		void clear();

		void update();
	};

	SlotPrivate::SlotPrivate(NodePrivate* node, IO io, uint index, const TypeInfo* type, const std::string& name, uint offset, uint size, const void* _default_value) :
		node(node),
		io(io),
		index(index),
		type(type),
		name(name),
		offset(offset),
		size(size),
		setter(nullptr)
	{
		user_data = nullptr;

		if (_default_value)
		{
			default_value = new char[size];
			memcpy(default_value, _default_value, size);
		}
		else
			default_value = nullptr;

		data = (char*)node->object + offset;

		if (io == In)
			links.push_back(nullptr);
	}

	SlotPrivate::SlotPrivate(NodePrivate* node, IO io, uint index, VariableInfo* vi) :
		SlotPrivate(node, io, index, vi->type(), vi->name(),
			vi->offset(), vi->size(), vi->default_value())
	{
	}

	SlotPrivate::~SlotPrivate()
	{
		delete[] default_value;
		delete setter;
	}

	void SlotPrivate::set_data(const void* d)
	{
		if (!setter)
			type->copy_from(d, data);
		else
			setter->set(d);
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

	NodePrivate::NodePrivate(BPPrivate* scene, BP::ObjectType object_type, const std::string& id, UdtInfo* udt, void* module) :
		scene(scene),
		object_type(object_type),
		id(id),
		type(udt->name()),
		udt(udt),
		object(nullptr),
		module(module),
		dtor_addr(nullptr),
		update_addr(nullptr),
		order(0xffffffff)
	{
		pos = Vec2f(0.f);
		user_data = nullptr;

		if (object_type == BP::ObjectReal)
		{
			auto size = udt->size();
			object = malloc(size);
			memset(object, 0, size);

			{
				auto f = udt->find_function("ctor");
				if (f && f->parameter_count() == 0)
					cmf(p2f<MF_v_v>((char*)module + (uint)f->rva()), object);
			}

			{
				auto f = udt->find_function("dtor");
				if (f)
					dtor_addr = (char*)module + (uint)f->rva();
			}

			{
				auto f = udt->find_function("bp_update");
				assert(f && check_function(f, "D#void", {}));
				update_addr = (char*)module + (uint)f->rva();
			}
			assert(update_addr);

			for (auto i = 0; i < udt->variable_count(); i++)
			{
				auto v = udt->variable(i);
				if (v->flags() & VariableFlagOutput)
					outputs.emplace_back(new SlotPrivate(this, BP::Slot::Out, outputs.size(), v));
				else
					inputs.emplace_back(new SlotPrivate(this, BP::Slot::In, inputs.size(), v));
			}
		}
		else
		{
			{
				auto f = udt->find_function("get_linked_object");
				assert(f && check_function(f, (std::string("P#") + udt->name()).c_str(), {}));
				object = cf(p2f<F_vp_v>((char*)module + (uint)f->rva()));
				assert(object);
			}

			if (object_type == BP::ObjectRefRead)
			{
				for (auto i = 0; i < udt->variable_count(); i++)
				{
					auto v = udt->variable(i);
					outputs.emplace_back(new SlotPrivate(this, BP::Slot::Out, outputs.size(), v));
				}
			}
			else if (object_type == BP::ObjectRefWrite)
			{
				for (auto i = 0; i < udt->variable_count(); i++)
				{
					auto v = udt->variable(i);
					auto type = v->type();
					if (type->tag() != TypeData)
						continue;
					auto base_hash = type->base_hash();
					auto input = new SlotPrivate(this, BP::Slot::In, inputs.size(), v);
					auto f_set = udt->find_function((std::string("set_") + v->name()).c_str());
					if (f_set)
					{
						auto f_set_addr = (char*)module + (uint)f_set->rva();
						Setter* setter = nullptr;
						switch (base_hash)
						{
							case FLAME_CHASH("bool"):
								setter = new Setter_t<bool>;
								break;
							case FLAME_CHASH("int"):
								setter = new Setter_t<int>;
								break;
							case FLAME_CHASH("flame::Vec(2+int)"):
								setter = new Setter_t<Vec2i>;
								break;
							case FLAME_CHASH("flame::Vec(3+int)"):
								setter = new Setter_t<Vec3i>;
								break;
							case FLAME_CHASH("flame::Vec(4+int)"):
								setter = new Setter_t<Vec4i>;
								break;
							case FLAME_CHASH("uint"):
								setter = new Setter_t<uint>;
								break;
							case FLAME_CHASH("flame::Vec(2+uint)"):
								setter = new Setter_t<Vec2u>;
								break;
							case FLAME_CHASH("flame::Vec(3+uint)"):
								setter = new Setter_t<Vec3u>;
								break;
							case FLAME_CHASH("flame::Vec(4+unt)"):
								setter = new Setter_t<Vec4u>;
								break;
							case FLAME_CHASH("float"):
								setter = new Setter_t<float>;
								break;
							case FLAME_CHASH("flame::Vec(2+float)"):
								setter = new Setter_t<Vec2f>;
								break;
							case FLAME_CHASH("flame::Vec(3+float)"):
								setter = new Setter_t<Vec3f>;
								break;
							case FLAME_CHASH("flame::Vec(4+float)"):
								setter = new Setter_t<Vec4f>;
								break;
							case FLAME_CHASH("uchar"):
								setter = new Setter_t<uchar>;
								break;
							case FLAME_CHASH("flame::Vec(2+uchar)"):
								setter = new Setter_t<Vec2c>;
								break;
							case FLAME_CHASH("flame::Vec(3+uchar)"):
								setter = new Setter_t<Vec3c>;
								break;
							case FLAME_CHASH("flame::Vec(4+uchar)"):
								setter = new Setter_t<Vec4c>;
								break;
						}
						setter->o = object;
						setter->f = f_set_addr;
						setter->s = scene;
						input->setter = setter;
					}
					memcpy(input->default_value, input->data, input->size);
					inputs.emplace_back(input);
				}
			}
		}
	}

	NodePrivate::NodePrivate(BPPrivate* scene, const std::string& id, const std::string& type, uint size, 
		const std::vector<SlotDesc>& _inputs, const std::vector<SlotDesc>& _outputs, void* ctor_addr, void* dtor_addr, void* update_addr) :
		scene(scene),
		object_type(BP::ObjectReal),
		id(id),
		type(type),
		udt(nullptr),
		module(nullptr),
		dtor_addr(dtor_addr),
		update_addr(update_addr),
		order(0xffffffff)
	{
		pos = Vec2f(0.f);
		user_data = nullptr;

		object = malloc(size);
		memset(object, 0, size);

		if (ctor_addr)
			cmf(p2f<MF_v_v>(ctor_addr), object);

		assert(update_addr);

		for (auto i = 0; i < _inputs.size(); i++)
		{
			auto& d = _inputs[i];
			inputs.emplace_back(new SlotPrivate(this, BP::Slot::In, i, d.type, d.name, d.offset, d.size, nullptr));
		}
		for (auto i = 0; i < _outputs.size(); i++)
		{
			auto& d = _outputs[i];
			outputs.emplace_back(new SlotPrivate(this, BP::Slot::Out, i, d.type, d.name, d.offset, d.size, nullptr));
		}
	}

	NodePrivate::~NodePrivate()
	{
		if (object_type == BP::ObjectReal)
		{
			if (dtor_addr)
				cmf(p2f<MF_v_v>(dtor_addr), object);
			free(object);
		}
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
				{
					if (in->setter)
						in->setter->set(out->data);
					else
						memcpy(in->data, out->data, in->size);
				}
			}
		}

		if (update_addr)
			cmf(p2f<MF_v_v>(update_addr), object);
	}

	bool BPPrivate::check_or_create_id(std::string& id) const
	{
		if (!id.empty())
		{
			if (find_node(id))
				return false;
		}
		else
		{
			id = std::to_string(::rand());
			while (find_node(id))
				id = std::to_string(::rand());
		}
		return true;
	}

	NodePrivate* BPPrivate::add_node(const std::string& _id, const std::string& type, BP::ObjectType object_type)
	{
		std::string id = _id;
		if (!check_or_create_id(id))
		{
			printf("cannot add node, id repeated\n");
			return nullptr;
		}

		NodePrivate* n = nullptr;

		if (object_type == ObjectReal)
		{
			std::string parameters;
			switch (break_node_type(type, &parameters))
			{
			case 'S':
			{
#pragma pack(1)
				struct Dummy
				{
					int in;
					int chk;

					int out;
					float res;

					void update()
					{
						out = in;
						res = in == chk ? 1.f : 0.f;
					}
				};
#pragma pack()
				n = new NodePrivate(this, id, type, sizeof(Dummy), {
						{TypeInfo::get(TypeEnumSingle, parameters.c_str()), "in", offsetof(Dummy, in), sizeof(Dummy::in) },
						{TypeInfo::get(TypeEnumSingle, parameters.c_str()), "chk", offsetof(Dummy, chk), sizeof(Dummy::chk) }
					}, {
						{TypeInfo::get(TypeEnumSingle, parameters.c_str()), "out", offsetof(Dummy, out), sizeof(Dummy::out) },
						{TypeInfo::get(TypeData, "float"), "res", offsetof(Dummy, res), sizeof(Dummy::res) }
					}, nullptr, nullptr, f2v(&Dummy::update));
			}
				break;
			case 'M':
			{
#pragma pack(1)
				struct Dummy
				{
					int in;
					int chk;

					int out;
					float res;

					void update()
					{
						out = in;
						res = (in & chk) ? 1.f : 0.f;
					}
				};
#pragma pack()
				n = new NodePrivate(this, id, type, sizeof(Dummy), {
						{TypeInfo::get(TypeEnumMulti, parameters.c_str()), "in", offsetof(Dummy, in), sizeof(Dummy::in) },
						{TypeInfo::get(TypeEnumSingle, parameters.c_str()), "chk", offsetof(Dummy, chk), sizeof(Dummy::chk) }
					}, {
						{TypeInfo::get(TypeEnumMulti, parameters.c_str()), "out", offsetof(Dummy, out), sizeof(Dummy::out) },
						{TypeInfo::get(TypeData, "float"), "res", offsetof(Dummy, res), sizeof(Dummy::res) }
					}, nullptr, nullptr, f2v(&Dummy::update));
			}
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
						{TypeInfo::get(TypeData, parameters.c_str()), "in", sizeof(Dummy), type_size }
					}, {
						{TypeInfo::get(TypeData, parameters.c_str()), "out", sizeof(Dummy) + type_size, type_size }
					}, nullptr, f2v(&Dummy::dtor), f2v(&Dummy::update));
				auto& obj = *(Dummy*)n->object;
				obj.type_hash = type_hash;
				obj.type_size = type_size;
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
							f_free(out.v);
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
						sizeof(Dummy) + type_size * i, type_size
						});
				}
				n = new NodePrivate(this, id, type, sizeof(Dummy) + type_size * size + sizeof(Array<int>), inputs, {
						{ TypeInfo::get(TypeData, type_name.c_str(), true), "out", sizeof(Dummy) + type_size * size, sizeof(Array<int>) }
					}, nullptr, f2v(&Dummy::dtor), f2v(&Dummy::update));
				auto& obj = *(Dummy*)n->object;
				obj.type_hash = type_hash;
				obj.type_size = type_size;
				obj.size = size;
			}
				break;
			}
		}
		if (!n)
		{
			auto udt = find_udt(FLAME_HASH(type.c_str()));

			if (!udt)
			{
				printf("cannot add node, type: %s\n", type.c_str());
				return nullptr;
			}

			n = new NodePrivate(this, object_type, id, udt, udt->db()->module());
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

	static float bp_time = 0.f;

	static void get_order(NodePrivate* n, uint& order)
	{
		if (n->order != 0xffffffff)
			return;
		for (auto& i : n->inputs)
		{
			auto o = i->links[0];
			if (o)
			{
				auto nn = o->node;
				get_order(nn, order);
			}
		}
		n->order = order++;
	}

	static void build_update_list(BPPrivate* s)
	{
		for (auto& n : s->nodes)
			n->order = 0xffffffff;
		auto order = 0U;
		for (auto& n : s->nodes)
			get_order(n.get(), order);
		s->update_list.clear();
		for (auto& n : s->nodes)
		{
			std::vector<NodePrivate*>::iterator it;
			for (it = s->update_list.begin(); it != s->update_list.end(); it++)
			{
				if (n->order < (*it)->order)
					break;
			}
			s->update_list.emplace(it, n.get());
		}
		s->need_rebuild_update_list = false;
	}

	void BPPrivate::update()
	{
		if (need_rebuild_update_list)
			build_update_list(this);

		bp_time = time;

		for (auto n : update_list)
			n->update();

		time += looper().delta_time;
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

	const void* BP::Slot::default_value() const
	{
		return ((SlotPrivate*)this)->default_value;
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

	BP* BP::Node::scene() const
	{
		return ((NodePrivate*)this)->scene;
	}

	BP::ObjectType BP::Node::object_type() const
	{
		return ((NodePrivate*)this)->object_type;
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

	BP::Node* BP::add_node(const char* id, const char* type, ObjectType object_type)
	{
		return ((BPPrivate*)this)->add_node(id, type, object_type);
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
			ObjectType object_type;
			std::string id;
			std::string type;
			Vec2f pos;
			std::vector<DataDesc> datas;
		};
		std::vector<NodeDesc> node_descs;

		for (auto n_node : file_root.child("nodes"))
		{
			NodeDesc node;

			node.object_type = (ObjectType)n_node.attribute("object_type").as_int();
			node.id = n_node.attribute("id").value();
			node.type = n_node.attribute("type").value();
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
			auto n = bp->add_node(n_d.id, n_d.type, n_d.object_type);
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
			n_node.append_attribute("object_type").set_value(n->object_type);
			n_node.append_attribute("id").set_value(n->id.c_str());
			n_node.append_attribute("type").set_value(n->type.c_str());
			n_node.append_attribute("pos").set_value(to_string(n->pos).c_str());

			pugi::xml_node n_datas;
			for (auto& in : n->inputs)
			{
				if (in->links[0])
					continue;
				auto type = in->type;
				if (type->tag() != TypePointer)
				{
					if (in->default_value && memcmp(in->default_value, in->data, in->size) == 0)
						continue;
					if (!n_datas)
						n_datas = n_node.append_child("datas");
					auto n_data = n_datas.append_child("data");
					n_data.append_attribute("name").set_value(in->name.c_str());
					n_data.append_attribute("value").set_value(type->serialize(in->data).c_str());
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

		if (bp->need_rebuild_update_list)
			build_update_list(bp);
		struct Var
		{
			std::string type;
			std::string name;
			int ref_count;
		};
		struct Line
		{
			int assign_var_idx; // -1 means it is not an assignment
			std::string content;
		};
		std::vector<Var> vars;
		std::vector<Line> lines;
		for (auto n : bp->update_list)
		{
			auto var_id = [](SlotPrivate* s) {
				auto n = s->node;
				if (n->object_type != ObjectReal)
					return std::string(n->udt->link_name()) + "->" + (s->setter ? "set_" : "") + s->name;
				return "_" + n->id + "_" + s->name;
			};

			switch (n->object_type)
			{
			case ObjectRefWrite:
				for (auto& in : n->inputs)
				{
					auto out = (SlotPrivate*)in->links[0];
					std::string value;
					if (!out)
					{
						auto type = in->type;
						if (type->tag() == TypePointer || (in->default_value && memcmp(in->default_value, in->data, in->size) == 0))
							continue;
						value = type->serialize(in->data);
					}
					else
						value = var_id(out);
					auto line = var_id(in.get());
					if (in->setter)
						line += "(" + value + ");";
					else
						line += " = " + value + ";";
					lines.push_back({ -1, line });
				}
				break;
			case ObjectReal:
			{
				std::string parameters;
				auto ntype = break_node_type(n->type, &parameters);
				if (ntype == 0)
				{
					auto f = n->udt->find_function("bp_update");
					assert(f && check_function(f, "D#void", {}));
					std::string function_code = f->code();
					for (auto& out : n->outputs)
					{
						auto id = var_id(out.get());

						vars.push_back({ out->type->get_cpp_name(), id, 0 });
						std::regex reg("\\b" + out->name + "\\b");
						function_code = std::regex_replace(function_code, reg, id);
					}
					for (auto& in : n->inputs)
					{
						auto out = (SlotPrivate*)in->links[0];
						std::string value;
						if (!out)
							value = in->type->serialize(in->data);
						else
							value = var_id(out);
						std::regex reg(R"(\b)" + in->name + R"(\b)");
						function_code = std::regex_replace(function_code, reg, value);
					}
					auto function_lines = SUS::split(function_code, '\n');
					for (auto& l : function_lines)
						lines.push_back({ -1, l });
				}
				else
				{
					switch (ntype)
					{
					case 'S':
					case 'M':
					{
						auto out_id = "_" + n->id + "_out";
						auto res_id = "_" + n->id + "_res";
						vars.push_back({ "uint", out_id, 0 });
						vars.push_back({ "float", res_id, 0 });
						std::string in_value;
						{
							auto in = n->inputs[0].get();
							auto out = (SlotPrivate*)in->links[0];
							in_value = out ? var_id(out) : std::to_string(*(int*)in->data);
							lines.push_back({ -1, out_id + " = " + in_value + ";" });
						}
						{
							auto in = n->inputs[1].get();
							auto out = (SlotPrivate*)in->links[0];
							if (ntype == 'S')
							{
								if (!out)
									lines.push_back({ -1, res_id + " = " + out_id + " == " + std::to_string(*(int*)in->data) + " ? 1.f : 0.f;" });
								else
									lines.push_back({ -1, res_id + " = " + in_value + " == " + var_id(out) + " ? 1.f : 0.f;" });
							}
							else
							{
								if (!out)
									lines.push_back({ -1, res_id + " = (" + out_id + " & " + std::to_string(*(int*)in->data) + ") ? 1.f : 0.f;" });
								else
									lines.push_back({ -1, res_id + " = (" + in_value + " & " + var_id(out) + ") ? 1.f : 0.f;" });
							}
						}
					}
						break;
					case 'V':
						assert(0); // WIP
						break;
					case 'A':
						assert(0); // WIP
						break;
					}
				}
			}
				break;
			}
		}

		for (auto i = 0; i < vars.size(); i++)
		{
			std::regex reg(R"(^)" + vars[i].name + R"( = (.*);)");
			for (auto& l : lines)
			{
				if (l.assign_var_idx == -1)
				{
					std::smatch res;
					if (std::regex_search(l.content, res, reg))
					{
						l.content = res[1].str();
						l.assign_var_idx = i;
					}
				}
			}
		}

		for (auto i = 0; i < lines.size(); i++)
		{
			auto& l = lines[i];
			if (l.assign_var_idx != -1)
			{
				auto& v = vars[l.assign_var_idx];
				std::regex reg(R"(\b)" + v.name + R"(\b)");
				auto ref_count = 0, last_ref_line = -1;
				for (auto j = i + 1; j < lines.size(); j++)
				{
					std::smatch res;
					auto str = lines[j].content;
					while (std::regex_search(str, res, reg))
					{
						last_ref_line = j;
						ref_count++;
						str = res.suffix();
					}
				}
				v.ref_count += ref_count;
				if (ref_count == 1)
				{
					lines[last_ref_line].content = std::regex_replace(lines[last_ref_line].content, reg, "(" + l.content + ")");
					v.ref_count--;
				}
				if (ref_count <= 1)
					l.content.clear();
			}
		}

		std::ofstream h_file(filename + std::wstring(L".h"));
		h_file << "// THIS FILE IS AUTO-GENERATED\n";
		for (auto& v : vars)
		{
			if (v.ref_count == 0)
				continue;
			h_file << v.type + " " + v.name + ";\n";
		}
		for (auto& l : lines)
		{
			if (l.content.empty())
				continue;
			if (l.assign_var_idx == -1)
				h_file << l.content + "\n";
			else
				h_file << vars[l.assign_var_idx].name + " = " + l.content + ";\n";
		}
		h_file.close();
	}

	void BP::destroy(BP *bp)
	{
		delete(BPPrivate*)bp;
	}

	struct FLAME_R(R_MakeVec2i)
	{
		FLAME_RV(int, x, i);
		FLAME_RV(int, y, i);

		FLAME_RV(Vec2i, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = Vec2i(x, y);
		}
	};

	struct FLAME_R(R_BreakVec2i)
	{
		FLAME_RV(Vec2i, in, i);

		FLAME_RV(int, x, o);
		FLAME_RV(int, y, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			x = in[0];
			y = in[1];
		}
	};

	struct FLAME_R(R_MakeVec3i)
	{
		FLAME_RV(int, x, i);
		FLAME_RV(int, y, i);
		FLAME_RV(int, z, i);

		FLAME_RV(Vec3i, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = Vec3i(x, y, z);
		}
	};

	struct FLAME_R(R_BreakVec3i)
	{
		FLAME_RV(Vec3i, in, i);

		FLAME_RV(int, x, o);
		FLAME_RV(int, y, o);
		FLAME_RV(int, z, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			x = in[0];
			y = in[1];
			z = in[2];
		}
	};

	struct FLAME_R(R_MakeVec4i)
	{
		FLAME_RV(int, x, i);
		FLAME_RV(int, y, i);
		FLAME_RV(int, z, i);
		FLAME_RV(int, w, i);

		FLAME_RV(Vec4i, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = Vec4i(x, y, z, w);
		}
	};

	struct FLAME_R(R_BreakVec4i)
	{
		FLAME_RV(Vec4i, in, i);

		FLAME_RV(int, x, o);
		FLAME_RV(int, y, o);
		FLAME_RV(int, z, o);
		FLAME_RV(int, w, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			x = in[0];
			y = in[1];
			z = in[2];
			w = in[3];
		}
	};

	struct FLAME_R(R_MakeVec2u)
	{
		FLAME_RV(uint, x, i);
		FLAME_RV(uint, y, i);

		FLAME_RV(Vec2u, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = Vec2u(x, y);
		}
	};

	struct FLAME_R(R_BreakVec2u)
	{
		FLAME_RV(Vec2u, in, i);

		FLAME_RV(uint, x, o);
		FLAME_RV(uint, y, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			x = in[0];
			y = in[1];
		}
	};

	struct FLAME_R(R_MakeVec3u)
	{
		FLAME_RV(uint, x, i);
		FLAME_RV(uint, y, i);
		FLAME_RV(uint, z, i);

		FLAME_RV(Vec3u, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = Vec3u(x, y, z);
		}
	};

	struct FLAME_R(R_BreakVec3u)
	{
		FLAME_RV(Vec3u, in, i);

		FLAME_RV(uint, x, o);
		FLAME_RV(uint, y, o);
		FLAME_RV(uint, z, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			x = in[0];
			y = in[1];
			z = in[2];
		}
	};

	struct FLAME_R(R_MakeVec4u)
	{
		FLAME_RV(uint, x, i);
		FLAME_RV(uint, y, i);
		FLAME_RV(uint, z, i);
		FLAME_RV(uint, w, i);

		FLAME_RV(Vec4u, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = Vec4u(x, y, z, w);
		}
	};

	struct FLAME_R(R_BreakVec4u)
	{
		FLAME_RV(Vec4u, in, i);

		FLAME_RV(uint, x, o);
		FLAME_RV(uint, y, o);
		FLAME_RV(uint, z, o);
		FLAME_RV(uint, w, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			x = in[0];
			y = in[1];
			z = in[2];
			w = in[3];
		}
	};

	struct FLAME_R(R_MakeVec2f)
	{
		FLAME_RV(float, x, i);
		FLAME_RV(float, y, i);

		FLAME_RV(Vec2f, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = Vec2f(x, y);
		}
	};

	struct FLAME_R(R_BreakVec2f)
	{
		FLAME_RV(Vec2f, in, i);

		FLAME_RV(float, x, o);
		FLAME_RV(float, y, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			x = in[0];
			y = in[1];
		}
	};

	struct FLAME_R(R_MakeVec3f)
	{
		FLAME_RV(float, x, i);
		FLAME_RV(float, y, i);
		FLAME_RV(float, z, i);

		FLAME_RV(Vec3f, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = Vec3f(x, y, z);
		}
	};

	struct FLAME_R(R_BreakVec3f)
	{
		FLAME_RV(Vec3f, in, i);

		FLAME_RV(float, x, o);
		FLAME_RV(float, y, o);
		FLAME_RV(float, z, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			x = in[0];
			y = in[1];
			z = in[2];
		}
	};

	struct FLAME_R(R_MakeVec4f)
	{
		FLAME_RV(float, x, i);
		FLAME_RV(float, y, i);
		FLAME_RV(float, z, i);
		FLAME_RV(float, w, i);

		FLAME_RV(Vec4f, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = Vec4f(x, y, z, w);
		}
	};

	struct FLAME_R(R_BreakVec4f)
	{
		FLAME_RV(Vec4f, in, i);

		FLAME_RV(float, x, o);
		FLAME_RV(float, y, o);
		FLAME_RV(float, z, o);
		FLAME_RV(float, w, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			x = in[0];
			y = in[1];
			z = in[2];
			w = in[3];
		}
	};

	struct FLAME_R(R_MakeVec2c)
	{
		FLAME_RV(uchar, x, i);
		FLAME_RV(uchar, y, i);

		FLAME_RV(Vec2c, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = Vec2c(x, y);
		}
	};

	struct FLAME_R(R_BreakVec2c)
	{
		FLAME_RV(Vec2c, in, i);

		FLAME_RV(uchar, x, o);
		FLAME_RV(uchar, y, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			x = in[0];
			y = in[1];
		}
	};

	struct FLAME_R(R_MakeVec3c)
	{
		FLAME_RV(uchar, x, i);
		FLAME_RV(uchar, y, i);
		FLAME_RV(uchar, z, i);

		FLAME_RV(Vec3c, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = Vec3c(x, y, z);
		}
	};

	struct FLAME_R(R_BreakVec3c)
	{
		FLAME_RV(Vec3c, in, i);

		FLAME_RV(uchar, x, o);
		FLAME_RV(uchar, y, o);
		FLAME_RV(uchar, z, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			x = in[0];
			y = in[1];
			z = in[2];
		}
	};

	struct FLAME_R(R_MakeVec4c)
	{
		FLAME_RV(uchar, x, i);
		FLAME_RV(uchar, y, i);
		FLAME_RV(uchar, z, i);
		FLAME_RV(uchar, w, i);

		FLAME_RV(Vec4c, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = Vec4c(x, y, z, w);
		}
	};

	struct FLAME_R(R_BreakVec4c)
	{
		FLAME_RV(Vec4c, in, i);

		FLAME_RV(uchar, x, o);
		FLAME_RV(uchar, y, o);
		FLAME_RV(uchar, z, o);
		FLAME_RV(uchar, w, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			x = in[0];
			y = in[1];
			z = in[2];
			w = in[3];
		}
	};

	struct FLAME_R(R_Add)
	{
		FLAME_RV(float, a, i);
		FLAME_RV(float, b, i);

		FLAME_RV(float, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = a + b;
		}
	};

	struct FLAME_R(R_Multiple)
	{
		FLAME_RV(float, a, i);
		FLAME_RV(float, b, i);

		FLAME_RV(float, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = a * b;
		}
	};

	struct FLAME_R(R_Time)
	{
		FLAME_RV(float, delta, o);
		FLAME_RV(float, total, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			delta = looper().delta_time;
			total = bp_time;
		}
	};

	struct FLAME_R(R_Sin)
	{
		FLAME_RV(float, t, i);

		FLAME_RV(float, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = sin(t * M_PI / 180.f);
		}
	};

	struct FLAME_R(R_Linear1d)
	{
		FLAME_RV(float, a, i);
		FLAME_RV(float, b, i);
		FLAME_RV(float, t, i);

		FLAME_RV(float, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = a + (b - a) * clamp(t, 0.f, 1.f);
		}
	};

	struct FLAME_R(R_Linear2d)
	{
		FLAME_RV(Vec2f, a, i);
		FLAME_RV(Vec2f, b, i);
		FLAME_RV(float, t, i);

		FLAME_RV(Vec2f, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = a + (b - a) * clamp(t, 0.f, 1.f);
		}
	};

	struct FLAME_R(R_Trace)
	{
		FLAME_RV(float, target, i);
		FLAME_RV(float, step, i);
		FLAME_RV(float, v, i);

		FLAME_RV(float, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			out = v + min(abs(target - v), step) * sign(target - v);
		}
	};

	struct FLAME_R(R_KeyListener)
	{
		FLAME_RV(Key, key, i);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{

		}
	};
}

