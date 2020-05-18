#include <flame/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/foundation/typeinfo.h>

namespace flame
{
	struct BPPrivate;
	struct SlotPrivate;
	struct NodePrivate;
	struct GroupPrivate;
	struct BPPrivate;

	struct SlotPrivate : BP::Slot
	{
		Setter* setter;
		void* listener;

		SlotPrivate(BP::Unit* unit, IO io, uint index, TypeInfo* type, const std::string& name, uint offset, uint size, const void* default_value);
		SlotPrivate(BP::Unit* unit, IO io, uint index, VariableInfo* vi);
		~SlotPrivate();

		void set_data(const void* data);
		bool link_to(SlotPrivate* target);
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
		void* object;
		void* module;

		void* dtor_addr;
		void* update_addr;

		uint order;

		NodePrivate(BP::NodeType node_type, GroupPrivate* group, const std::string& id, UdtInfo* udt, void* module);
		NodePrivate(BP::NodeType node_type, GroupPrivate* group, const std::string& id, const std::string& type, uint size,
			const std::vector<SlotDesc>& inputs, const std::vector<SlotDesc>& outputs, void* ctor_addr, void* dtor_addr, void* update_addr);
		~NodePrivate();

		void update();
	};

	struct GroupPrivate : BP::Group
	{
		void* psignal;
		std::vector<NodePrivate*> update_list;
		bool need_rebuild_update_list;

		GroupPrivate(BPPrivate* scene, const std::string& id);
		~GroupPrivate();

		NodePrivate* add_node(const std::string& id, const std::string& type, BP::NodeType node_type);
		void remove_node(NodePrivate* n);

		void update();
	};

	struct BPPrivate : BP
	{
		BPPrivate();
		~BPPrivate();

		GroupPrivate* add_group(const std::string& id);
		void remove_group(GroupPrivate* n);

		void update();
	};

	SlotPrivate::SlotPrivate(BP::Unit* unit, IO _io, uint _index, TypeInfo* _type, const std::string& _name, uint _offset, uint _size, const void* _default_value) :
		setter(nullptr),
		listener(nullptr)
	{
		parent = unit;
		io = _io;
		index = _index;
		type = _type;
		name = _name;
		offset = _offset;
		size = _size;
		user_data = nullptr;

		if (_default_value)
		{
			default_value = new char[size];
			memcpy(default_value, _default_value, size);
		}
		else
			default_value = nullptr;

		data = unit->unit_type == BP::UnitNode ? (char*)((NodePrivate*)unit)->object + offset : (char*)&((GroupPrivate*)unit)->psignal;

		if (io == In)
			links.push_back(nullptr);
	}

	SlotPrivate::SlotPrivate(BP::Unit* unit, IO io, uint index, VariableInfo* vi) :
		SlotPrivate(unit, io, index, vi->type, vi->name.str(),
			vi->offset, vi->size, vi->default_value)
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
			type->copy_from(d, data, size);
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
			if (parent == target->parent)
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
			for (auto i = 0; i < o->links.s; i++)
			{
				if (o->links[i] == this)
				{
					o->links.remove(i);
					break;
				}
			}
			if (type->base_name == "ListenerHub")
				(*(ListenerHub<void(Capture&)>**)data)->remove(listener);
		}

		links[0] = target;
		if (target)
		{
			target->links.push_back(this);
			if (type->base_name == "ListenerHub")
			{
				auto p = target->data;
				memcpy(data, &p, sizeof(void*));
				listener = (*(ListenerHub<void(Capture&)>**)data)->add([](Capture& c) {
					c.thiz<BP::Group>()->update();
				}, Capture().set_thiz(parent));
			}
		}

		if (!target && type->tag == TypePointer)
			memset(data, 0, sizeof(void*));

		return true;
	}

	NodePrivate::NodePrivate(BP::NodeType _node_type, GroupPrivate* _group, const std::string& _id, UdtInfo* _udt, void* _module) :
		object(nullptr),
		module(_module),
		dtor_addr(nullptr),
		update_addr(nullptr),
		order(0xffffffff)
	{
		unit_type = BP::UnitNode;
		id = _id;
		pos = Vec2f(0.f);
		user_data = nullptr;
		node_type = _node_type;
		group = _group;
		type = _udt->name;
		udt = _udt;

		if (node_type == BP::NodeReal)
		{
			auto size = udt->size;
			object = malloc(size);
			memset(object, 0, size);

			{
				auto f = udt->find_function("ctor");
				if (f && f->parameters.s == 0)
					cmf(p2f<MF_v_v>((char*)module + (uint)f->rva), object);
			}

			{
				auto f = udt->find_function("dtor");
				if (f)
					dtor_addr = (char*)module + (uint)f->rva;
			}

			{
				auto f = udt->find_function("bp_update");
				assert(f && check_function(f, "D#void", {}));
				update_addr = (char*)module + (uint)f->rva;
			}

			for (auto v :udt->variables)
			{
				if (v->flags & VariableFlagOutput)
					outputs.push_back(new SlotPrivate(this, BP::Slot::Out, outputs.s, v));
				else
					inputs.push_back(new SlotPrivate(this, BP::Slot::In, inputs.s, v));
			}
		}
		else
		{
			{
				auto f = udt->find_function("get_linked_object");
				assert(f && check_function(f, ("P#" + udt->name.str()).c_str(), {}));
				object = cf(p2f<F_vp_v>((char*)module + (uint)f->rva));
				assert(object);
			}

			if (node_type == BP::NodeRefRead)
			{
				for (auto v : udt->variables)
					outputs.push_back(new SlotPrivate(this, BP::Slot::Out, outputs.s, v));
			}
			else if (node_type == BP::NodeRefWrite)
			{
				for (auto v: udt->variables)
				{
					auto type = v->type;
					if (type->tag != TypeData)
						continue;
					auto base_hash = type->base_hash;
					auto input = new SlotPrivate(this, BP::Slot::In, inputs.s, v);
					auto f_set = udt->find_function(("set_" + v->name.str()).c_str());
					if (f_set)
					{
						auto f_set_addr = (char*)module + (uint)f_set->rva;
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
						setter->s = group->scene;
						input->setter = setter;
					}
					memcpy(input->default_value, input->data, input->size);
					inputs.push_back(input);
				}
			}
		}
	}

	NodePrivate::NodePrivate(BP::NodeType _node_type, GroupPrivate* _group, const std::string& _id, const std::string& _type, uint size,
		const std::vector<SlotDesc>& _inputs, const std::vector<SlotDesc>& _outputs, void* ctor_addr, void* dtor_addr, void* update_addr) :
		module(nullptr),
		dtor_addr(dtor_addr),
		update_addr(update_addr),
		order(0xffffffff)
	{
		unit_type = BP::UnitNode;
		id = _id;
		pos = Vec2f(0.f);
		user_data = nullptr;
		node_type = _node_type;
		group = _group;
		type = _type;
		udt = nullptr;

		object = malloc(size);
		memset(object, 0, size);

		if (ctor_addr)
			cmf(p2f<MF_v_v>(ctor_addr), object);

		for (auto i = 0; i < _inputs.size(); i++)
		{
			auto& d = _inputs[i];
			inputs.push_back(new SlotPrivate(this, BP::Slot::In, i, (TypeInfo*)d.type, d.name, d.offset, d.size, nullptr));
		}
		for (auto i = 0; i < _outputs.size(); i++)
		{
			auto& d = _outputs[i];
			outputs.push_back(new SlotPrivate(this, BP::Slot::Out, i, (TypeInfo*)d.type, d.name, d.offset, d.size, nullptr));
		}
	}

	NodePrivate::~NodePrivate()
	{
		if (node_type == BP::NodeReal)
		{
			if (dtor_addr)
				cmf(p2f<MF_v_v>(dtor_addr), object);
			free(object);
		}
		for (auto in : inputs)
			delete (SlotPrivate*)in;
		for (auto out : outputs)
			delete (SlotPrivate*)out;
	}

	void NodePrivate::update()
	{
		for (auto _in : inputs)
		{
			auto in = (SlotPrivate*)_in;
			auto out = in->links[0];
			if (out)
			{
				if (out->type->tag == TypeData && in->type->tag == TypePointer)
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

	GroupPrivate::GroupPrivate(BPPrivate* _scene, const std::string& _id)
	{
		unit_type = BP::UnitGroup;
		id = _id;
		pos = Vec2f(0.f);
		user_data = nullptr;
		scene = _scene;

		signal = new SlotPrivate(this, BP::Slot::In, 0, (TypeInfo*)TypeInfo::get(TypePointer, "ListenerHub"), "signal", 0, sizeof(void*), "");

		need_rebuild_update_list = true;
	}

	GroupPrivate::~GroupPrivate()
	{
		delete (SlotPrivate*)signal;
		for (auto n : nodes)
			delete (NodePrivate*)n;
	}


	static bool check_or_create_id(GroupPrivate* group, std::string& id)
	{
		if (!id.empty())
		{
			if (group->find_node(id))
				return false;
		}
		else
		{
			id = std::to_string(::rand());
			while (group->find_node(id))
				id = std::to_string(::rand());
		}
		return true;
	}

	NodePrivate* GroupPrivate::add_node(const std::string& _id, const std::string& type, BP::NodeType node_type)
	{
		std::string id = _id;
		if (!check_or_create_id(this, id))
		{
			printf("cannot add node, id repeated\n");
			return nullptr;
		}

		NodePrivate* n = nullptr;

		std::string parameters;
		switch (BP::break_node_type(type, &parameters))
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
			n = new NodePrivate(BP::NodeReal, this, id, type, sizeof(Dummy), {
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
			n = new NodePrivate(BP::NodeReal, this, id, type, sizeof(Dummy), {
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
			n = new NodePrivate(BP::NodeReal, this, id, type, sizeof(Dummy) + type_size * 2, {
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
			n = new NodePrivate(BP::NodeReal, this, id, type, sizeof(Dummy) + type_size * size + sizeof(Array<int>), inputs, {
					{ TypeInfo::get(TypeData, type_name.c_str(), true), "out", sizeof(Dummy) + type_size * size, sizeof(Array<int>) }
				}, nullptr, f2v(&Dummy::dtor), f2v(&Dummy::update));
			auto& obj = *(Dummy*)n->object;
			obj.type_hash = type_hash;
			obj.type_size = type_size;
			obj.size = size;
		}
			break;
		}

		if (!n)
		{
			auto udt = find_udt(FLAME_HASH(type.c_str()));

			if (!udt)
			{
				printf("cannot add node, type: %s\n", type.c_str());
				return nullptr;
			}

			n = new NodePrivate(node_type, this, id, udt, udt->db->module);
		}

		n->guid = generate_guid();
		nodes.push_back(n);

		need_rebuild_update_list = true;

		return n;
	}

	void GroupPrivate::remove_node(NodePrivate* _n)
	{
		for (auto i = 0; i < nodes.s;i++)
		{
			auto n = nodes[i];
			if (n == _n)
			{
				for (auto in : n->inputs)
				{
					auto o = in->links[0];
					if (o)
					{
						for (auto j = 0; j < o->links.s; j++)
						{
							if (o->links[j] == in)
							{
								o->links.remove(j);
								break;
							}
						}
					}
				}
				for (auto o : n->outputs)
				{
					for (auto l : o->links)
						l->links[0] = nullptr;
				}
				delete (NodePrivate*)n;
				nodes.remove(i);
				break;
			}
		}

		need_rebuild_update_list = true;
	}

	static void get_order(NodePrivate* n, uint& order)
	{
		if (n->order != 0xffffffff)
			return;
		for (auto i : n->inputs)
		{
			auto o = i->links[0];
			if (o)
			{
				auto u = o->parent;
				if (u->unit_type == BP::UnitNode)
					get_order((NodePrivate*)u, order);
			}
		}
		n->order = order++;
	}

	static void build_update_list(GroupPrivate* g)
	{
		for (auto n : g->nodes)
			((NodePrivate*)n)->order = 0xffffffff;
		auto order = 0U;
		for (auto n : g->nodes)
			get_order((NodePrivate*)n, order);
		g->update_list.clear();
		for (auto n : g->nodes)
		{
			std::vector<NodePrivate*>::iterator it;
			for (it = g->update_list.begin(); it != g->update_list.end(); it++)
			{
				if (((NodePrivate*)n)->order < (*it)->order)
					break;
			}
			g->update_list.emplace(it, (NodePrivate*)n);
		}
		g->need_rebuild_update_list = false;
	}

	void GroupPrivate::update()
	{
		if (need_rebuild_update_list)
			build_update_list(this);

		for (auto n : update_list)
			n->update();
	}

	GroupPrivate* BPPrivate::add_group(const std::string& _id)
	{
		if (find_group(_id))
		{
			printf("cannot add group, id repeated\n");
			return nullptr;
		}

		auto g = new GroupPrivate(this, _id);
		g->guid = generate_guid();
		groups.push_back(g);

		return g;
	}

	void BPPrivate::remove_group(GroupPrivate* _g)
	{
		for (auto i = 0; i < groups.s; i++)
		{
			if (groups[i] == _g)
			{
				delete (GroupPrivate*)_g;
				groups.remove(i);
				break;
			}
		}
	}

	BPPrivate::BPPrivate()
	{
		time = 0.f;

		add_group("");
	}

	BPPrivate::~BPPrivate()
	{
		for (auto g : groups)
			delete (GroupPrivate*)g;
	}

	static float bp_time = 0.f;

	void BPPrivate::update()
	{
		bp_time = time;

		groups[0]->update();

		time += looper().delta_time;
	}

	void BP::Slot::set_data(const void* d)
	{
		((SlotPrivate*)this)->set_data(d);
	}

	bool BP::Slot::link_to(BP::Slot* target)
	{
		return ((SlotPrivate*)this)->link_to((SlotPrivate*)target);
	}

	BP::Node* BP::Group::add_node(const char* id, const char* type, NodeType node_type)
	{
		return ((GroupPrivate*)this)->add_node(id, type, node_type);
	}

	void BP::Group::remove_node(Node* n)
	{
		((GroupPrivate*)this)->remove_node((NodePrivate*)n);
	}

	void BP::Group::update()
	{
		((GroupPrivate*)this)->update();
	}

	BP::Group* BP::add_group(const char* id)
	{
		return ((BPPrivate*)this)->add_group(id);
	}

	void BP::remove_group(BP::Group *g)
	{
		((BPPrivate*)this)->remove_group((GroupPrivate*)g);
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

		auto bp = new BPPrivate();
		bp->filename = filename;

		for (auto n_group : file_root.child("groups"))
		{
			auto id = std::string(n_group.attribute("id").value());
			auto g = id.empty() ? bp->groups[0] : bp->add_group(id);
			if (g)
			{
				g->pos = stof2(n_group.attribute("pos").value());

				for (auto n_node : n_group.child("nodes"))
				{
					auto n = g->add_node(n_node.attribute("id").value(), n_node.attribute("type").value(), (NodeType)n_node.attribute("node_type").as_int());
					if (n)
					{
						n->pos = stof2(n_node.attribute("pos").value());
						for (auto n_data : n_node.child("datas"))
						{
							auto input = n->find_input(n_data.attribute("name").value());
							auto type = input->type;
							auto tag = type->tag;
							if (!type->is_array && (tag == TypeEnumSingle || tag == TypeEnumMulti || tag == TypeData))
								type->unserialize(n_data.attribute("value").value(), input->data);
						}
					}
				}

				for (auto n_link : n_group.child("links"))
				{
					auto o_addr = std::string(n_link.attribute("out").value());
					auto i_addr = std::string(n_link.attribute("in").value());

					auto o = g->find_output(o_addr);
					auto i = g->find_input(i_addr);
					if (o && i)
					{
						if (!i->link_to(o))
							printf("link type mismatch: %s - > %s\n", o_addr.c_str(), i_addr.c_str());
					}
					else
						printf("cannot link: %s - > %s\n", o_addr.c_str(), i_addr.c_str());
				}
			}
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

		auto n_groups = file_root.append_child("groups");
		for (auto& g : bp->groups)
		{
			auto n_group = n_groups.append_child("group");
			n_group.append_attribute("id").set_value(g->id.v);
			n_group.append_attribute("pos").set_value(to_string(g->pos).c_str());

			auto n_nodes = n_group.append_child("nodes");
			for (auto& n : g->nodes)
			{
				auto n_node = n_nodes.append_child("node");
				n_node.append_attribute("node_type").set_value(n->node_type);
				n_node.append_attribute("id").set_value(n->id.v);
				n_node.append_attribute("type").set_value(n->type.v);
				n_node.append_attribute("pos").set_value(to_string(n->pos).c_str());

				pugi::xml_node n_datas;
				for (auto& in : n->inputs)
				{
					if (in->links[0])
						continue;
					auto type = in->type;
					if (type->tag != TypePointer)
					{
						if (in->default_value && memcmp(in->default_value, in->data, in->size) == 0)
							continue;
						if (!n_datas)
							n_datas = n_node.append_child("datas");
						auto n_data = n_datas.append_child("data");
						n_data.append_attribute("name").set_value(in->name.v);
						n_data.append_attribute("value").set_value(type->serialize(in->data).c_str());
					}
				}
			}

			auto n_links = n_group.append_child("links");
			for (auto& n : g->nodes)
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
		}

		file.save_file(filename);

		//if (bp->need_rebuild_update_list)
		//	build_update_list(bp);
		//struct Var
		//{
		//	std::string type;
		//	std::string name;
		//	int ref_count;
		//};
		//struct Line
		//{
		//	int assign_var_idx; // -1 means it is not an assignment
		//	std::string content;
		//};
		//std::vector<Var> vars;
		//std::vector<Line> lines;
		//for (auto n : bp->update_list)
		//{
		//	auto var_id = [](SlotPrivate* s) {
		//		auto n = s->node;
		//		if (n->node_type != NodeReal)
		//			return std::string(n->udt->link_name()) + "->" + (s->setter ? "set_" : "") + s->name;
		//		return "_" + n->id + "_" + s->name;
		//	};

		//	switch (n->node_type)
		//	{
		//	case NodeRefWrite:
		//		for (auto& in : n->inputs)
		//		{
		//			auto out = (SlotPrivate*)in->links[0];
		//			std::string value;
		//			if (!out)
		//			{
		//				auto type = in->type;
		//				if (type->tag() == TypePointer || (in->default_value && memcmp(in->default_value, in->data, in->size) == 0))
		//					continue;
		//				value = type->serialize(in->data);
		//			}
		//			else
		//				value = var_id(out);
		//			auto line = var_id(in.get());
		//			if (in->setter)
		//				line += "(" + value + ");";
		//			else
		//				line += " = " + value + ";";
		//			lines.push_back({ -1, line });
		//		}
		//		break;
		//	case NodeReal:
		//	{
		//		std::string parameters;
		//		auto ntype = break_node_type(n->type, &parameters);
		//		if (ntype == 0)
		//		{
		//			auto f = n->udt->find_function("bp_update");
		//			assert(f && check_function(f, "D#void", {}));
		//			std::string function_code = f->code();
		//			for (auto& out : n->outputs)
		//			{
		//				auto id = var_id(out.get());

		//				vars.push_back({ out->type->get_cpp_name(), id, 0 });
		//				std::regex reg("\\b" + out->name + "\\b");
		//				function_code = std::regex_replace(function_code, reg, id);
		//			}
		//			for (auto& in : n->inputs)
		//			{
		//				auto out = (SlotPrivate*)in->links[0];
		//				std::string value;
		//				if (!out)
		//					value = in->type->serialize(in->data);
		//				else
		//					value = var_id(out);
		//				std::regex reg(R"(\b)" + in->name + R"(\b)");
		//				function_code = std::regex_replace(function_code, reg, value);
		//			}
		//			auto function_lines = SUS::split(function_code, '\n');
		//			for (auto& l : function_lines)
		//				lines.push_back({ -1, l });
		//		}
		//		else
		//		{
		//			switch (ntype)
		//			{
		//			case 'S':
		//			case 'M':
		//			{
		//				auto out_id = "_" + n->id + "_out";
		//				auto res_id = "_" + n->id + "_res";
		//				vars.push_back({ "uint", out_id, 0 });
		//				vars.push_back({ "float", res_id, 0 });
		//				std::string in_value;
		//				{
		//					auto in = n->inputs[0].get();
		//					auto out = (SlotPrivate*)in->links[0];
		//					in_value = out ? var_id(out) : std::to_string(*(int*)in->data);
		//					lines.push_back({ -1, out_id + " = " + in_value + ";" });
		//				}
		//				{
		//					auto in = n->inputs[1].get();
		//					auto out = (SlotPrivate*)in->links[0];
		//					if (ntype == 'S')
		//					{
		//						if (!out)
		//							lines.push_back({ -1, res_id + " = " + out_id + " == " + std::to_string(*(int*)in->data) + " ? 1.f : 0.f;" });
		//						else
		//							lines.push_back({ -1, res_id + " = " + in_value + " == " + var_id(out) + " ? 1.f : 0.f;" });
		//					}
		//					else
		//					{
		//						if (!out)
		//							lines.push_back({ -1, res_id + " = (" + out_id + " & " + std::to_string(*(int*)in->data) + ") ? 1.f : 0.f;" });
		//						else
		//							lines.push_back({ -1, res_id + " = (" + in_value + " & " + var_id(out) + ") ? 1.f : 0.f;" });
		//					}
		//				}
		//			}
		//				break;
		//			case 'V':
		//				assert(0); // WIP
		//				break;
		//			case 'A':
		//				assert(0); // WIP
		//				break;
		//			}
		//		}
		//	}
		//		break;
		//	}
		//}

		//for (auto i = 0; i < vars.size(); i++)
		//{
		//	std::regex reg(R"(^)" + vars[i].name + R"( = (.*);)");
		//	for (auto& l : lines)
		//	{
		//		if (l.assign_var_idx == -1)
		//		{
		//			std::smatch res;
		//			if (std::regex_search(l.content, res, reg))
		//			{
		//				l.content = res[1].str();
		//				l.assign_var_idx = i;
		//			}
		//		}
		//	}
		//}

		//for (auto i = 0; i < lines.size(); i++)
		//{
		//	auto& l = lines[i];
		//	if (l.assign_var_idx != -1)
		//	{
		//		auto& v = vars[l.assign_var_idx];
		//		std::regex reg(R"(\b)" + v.name + R"(\b)");
		//		auto ref_count = 0, last_ref_line = -1;
		//		for (auto j = i + 1; j < lines.size(); j++)
		//		{
		//			std::smatch res;
		//			auto str = lines[j].content;
		//			while (std::regex_search(str, res, reg))
		//			{
		//				last_ref_line = j;
		//				ref_count++;
		//				str = res.suffix();
		//			}
		//		}
		//		v.ref_count += ref_count;
		//		if (ref_count == 1)
		//		{
		//			lines[last_ref_line].content = std::regex_replace(lines[last_ref_line].content, reg, "(" + l.content + ")");
		//			v.ref_count--;
		//		}
		//		if (ref_count <= 1)
		//			l.content.clear();
		//	}
		//}

		//std::ofstream h_file(filename + std::wstring(L".h"));
		//h_file << "// THIS FILE IS AUTO-GENERATED\n";
		//for (auto& v : vars)
		//{
		//	if (v.ref_count == 0)
		//		continue;
		//	h_file << v.type + " " + v.name + ";\n";
		//}
		//for (auto& l : lines)
		//{
		//	if (l.content.empty())
		//		continue;
		//	if (l.assign_var_idx == -1)
		//		h_file << l.content + "\n";
		//	else
		//		h_file << vars[l.assign_var_idx].name + " = " + l.content + ";\n";
		//}
		//h_file.close();
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

	struct FLAME_R(R_Print)
	{
		FLAME_RV(StringA, text, i);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(bp_update)()
		{
			printf("%s\n", text.v);
		}
	};
}

