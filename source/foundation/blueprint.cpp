#include <flame/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/foundation/typeinfo.h>

namespace flame
{
	struct BPPrivate;
	struct NodePrivate;
	struct SlotPrivate;

	struct LibraryPrivate : BP::Library
	{
		std::wstring directory;
		void* module;
		TypeinfoDatabase* db;

		~LibraryPrivate();
	};

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
		int frame;
		void* data;

		std::vector<SlotPrivate*> links;

		std::string fail_message;

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
		bool active;

		void* object;
		void* module;

		void* dtor_addr;
		void* update_addr;

		std::vector<std::unique_ptr<SlotPrivate>> inputs;
		std::vector<std::unique_ptr<SlotPrivate>> outputs;

		uint order;
		bool in_pending_update;
		bool in_update_list;

		NodePrivate(BPPrivate* scene, const std::string& id, UdtInfo* udt, void* module);
		NodePrivate(BPPrivate* scene, const std::string& id, const std::string& type, uint size, 
			const std::vector<SlotDesc>& inputs, const std::vector<SlotDesc>& outputs, void* ctor_addr, void* dtor_addr, void* update_addr, bool active);
		~NodePrivate();

		SlotPrivate* find_input(const std::string& name) const;
		SlotPrivate* find_output(const std::string& name) const;

		void update();
	};

	struct BPLibraries : BP
	{
		std::vector<std::unique_ptr<LibraryPrivate>> libraries;
	};

	struct BPPrivate : BPLibraries
	{
		std::wstring filename;

		std::vector<std::unique_ptr<NodePrivate>> nodes;

		std::vector<NodePrivate*> active_nodes;
		std::vector<NodePrivate*> pending_update_nodes;
		std::list<NodePrivate*> update_list;
		bool need_update_hierarchy;

		std::vector<std::filesystem::path> used_resources;

		bool need_check_fail;
		Array<Slot*> failed_slots;

		BPPrivate()
		{
			frame = 0;
			time = 0.f;
			need_update_hierarchy = true;
			need_check_fail = false;
		}

		LibraryPrivate* add_library(const std::wstring& directory);
		void remove_library(LibraryPrivate* m);
		LibraryPrivate* find_library(const std::wstring& directory) const;

		bool check_or_create_id(std::string& id, const std::string& type) const;
		NodePrivate* add_node(const std::string& type, const std::string& id);
		void remove_node(NodePrivate* n);
		NodePrivate* find_node(const std::string& address) const;
		SlotPrivate* find_input(const std::string& address) const;
		SlotPrivate* find_output(const std::string& address) const;

		void clear();

		void add_to_pending_update(NodePrivate* n);
		void update();
	};

	LibraryPrivate::~LibraryPrivate()
	{
		TypeinfoDatabase::destroy(db);
	}

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
		{
			links.push_back(nullptr);
			frame = 0;
		}
		else /* if (type == Output) */
			frame = -1;
	}

	SlotPrivate::SlotPrivate(NodePrivate* node, IO io, uint index, VariableInfo* vi) :
		SlotPrivate(node, io, index, vi->type(), vi->name(),
			vi->offset(), vi->size(), vi->default_value())
	{
	}

	void SlotPrivate::set_data(const void* d)
	{
		frame = node->scene->frame;
		node->scene->add_to_pending_update(node);
		 
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

		frame = node->scene->frame;

		auto scene = node->scene;
		scene->need_update_hierarchy = true;
		scene->add_to_pending_update(node);
		if (target)
			scene->add_to_pending_update(target->node);

		return true;
	}

	StringA SlotPrivate::get_address() const
	{
		return StringA(node->id + "." + name);
	}

	NodePrivate::NodePrivate(BPPrivate* scene, const std::string& id, UdtInfo* udt, void* module) :
		scene(scene),
		id(id),
		type(udt->type()->name() + 2),
		udt(udt),
		active(false),
		module(module),
		order(0xffffffff),
		in_pending_update(false),
		in_update_list(false)
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
			auto f = find_not_null_and_only(udt->find_function("update"), udt->find_function("active_update"));
			assert(f.first && check_function(f.first, "D#void", { "D#uint" }));
			if (f.second == 1)
				active = true;
			update_addr = (char*)module + (uint)f.first->rva();
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
		const std::vector<SlotDesc>& _inputs, const std::vector<SlotDesc>& _outputs, void* ctor_addr, void* _dtor_addr, void* _update_addr, bool active) :
		scene(scene),
		id(id),
		type(type),
		udt(nullptr),
		active(active),
		module(nullptr),
		order(0xffffffff),
		in_pending_update(false),
		in_update_list(false)
	{
		pos = Vec2f(0.f);
		user_data = nullptr;

		object = malloc(size);
		memset(object, 0, size);

		auto thiz = this;
		memcpy(object, &thiz, sizeof(void*));
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
			in->fail_message.clear();

			auto out = in->links[0];
			if (out)
			{
				if (out->type->tag() == TypeData && in->type->tag() == TypePointer)
					memcpy(in->data, &out->data, sizeof(void*));
				else
					memcpy(in->data, out->data, in->size);
				if (out->frame > in->frame)
					in->frame = out->frame;
			}
		}

		cmf(p2f<MF_v_u>(update_addr), object, scene->frame);
	}

	static void on_node_removed(NodePrivate* n)
	{
		auto scene = n->scene;
		auto frame = scene->frame;

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
			{
				l->links[0] = nullptr;
				l->frame = frame;
				scene->add_to_pending_update(l->node);
			}
		}

		if (n->in_pending_update)
		{
			for (auto it = scene->pending_update_nodes.begin(); it != scene->pending_update_nodes.end(); it++)
			{
				if (*it == n)
				{
					scene->pending_update_nodes.erase(it);
					break;
				}
			}
		}

		if (n->active)
		{
			for (auto it = scene->active_nodes.begin(); it != scene->active_nodes.end(); it++)
			{
				if (*it == n)
				{
					scene->active_nodes.erase(it);
					break;
				}
			}
		}
	}

	LibraryPrivate* BPPrivate::add_library(const std::wstring& _dir)
	{
		auto dir = _dir;
		std::replace(dir.begin(), dir.end(), '\\', '/');
		if (dir.empty())
			return nullptr;
		if (dir.back() == '/')
			dir.erase(dir.begin() + dir.size() - 1);

		for (auto& l : libraries)
		{
			if (l->directory == dir)
				return nullptr;
		}

		std::wstring name;
		auto abs_dir = std::filesystem::path(filename).parent_path() / dir;
		if (!std::filesystem::exists(abs_dir))
			return nullptr;
		auto compile_otions = parse_ini_file(abs_dir / L"compile_otions.ini");
		for (auto& e : compile_otions.get_section_entries(""))
		{
			if (e.key == "name")
			{
				name = s2w(e.value) + L".dll";
				break;
			}
		}
		if (name.empty())
			return nullptr;

		std::wstring config_str;
#ifdef NDEBUG
		config_str = L"relwithdebinfo";
#else
		config_str = L"debug";
#endif

		auto abs_path = abs_dir / L"build" / config_str / name;
		if (!std::filesystem::exists(abs_path))
		{
			auto compiler_path = std::filesystem::path(get_app_path().str()) / L"compiler.exe";
			if (!std::filesystem::exists(compiler_path))
			{
				wprintf(L"cannot find library: %s, and cannot find compiler\n", dir.c_str());
				return nullptr;
			}
			auto last_curr_path = get_curr_path();
			set_curr_path(abs_dir.c_str());
			exec_and_redirect_to_std_output(nullptr, (wchar_t*)(compiler_path.wstring() + L" " + config_str).c_str());
			set_curr_path(last_curr_path.v);
		}

		auto module = load_module(abs_path.c_str());
		if (!module)
		{
			wprintf(L"cannot add library %s\n", dir.c_str());
			return nullptr;
		}

		auto m = new LibraryPrivate;
		m->directory = dir;
		m->module = module;
		m->db = TypeinfoDatabase::load(abs_path.c_str(), false, true);

		libraries.emplace_back(m);

		used_resources.push_back(dir + L"/build/{c}/" + name);

		return m;
	}

	void BPPrivate::remove_library(LibraryPrivate* _l)
	{
		for (auto it = libraries.begin(); it != libraries.end(); it++)
		{
			auto l = it->get();
			if (l == _l)
			{
				auto db = l->db;
				auto& nodes = this->nodes;
				for (auto n_it = nodes.begin(); n_it != nodes.end(); )
				{
					auto n = n_it->get();
					auto udt = n->udt;
					if (udt && udt->db() == db)
					{
						on_node_removed(n);
						n_it = nodes.erase(n_it);
					}
					else
						n_it++;
				}

				for (auto it = used_resources.begin(); it != used_resources.end(); it++)
				{
					if (*it == l->directory)
					{
						used_resources.erase(it);
						break;
					}
				}

				libraries.erase(it);

				return;
			}
		}
	}

	LibraryPrivate* BPPrivate::find_library(const std::wstring& directory) const
	{
		auto& libraries = ((BPPrivate*)this)->libraries;
		for (auto& l : libraries)
		{
			if (l->directory == directory)
				return l.get();
		}
		return nullptr;
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
				BP::Node* n;

				int in;
				int out;

				void update(uint frame)
				{
					if (n->input(0)->frame() > n->output(0)->frame())
					{
						out = in;
						n->output(0)->set_frame(frame);
					}
				}
			};
#pragma pack()
			n = new NodePrivate(this, id, type, sizeof(Dummy), {
					{TypeInfo::get(tag, enum_name.c_str()), "in", offsetof(Dummy, in), sizeof(Dummy::in), ""}
				}, {
					{TypeInfo::get(tag, enum_name.c_str()), "out", offsetof(Dummy, out), sizeof(Dummy::out), ""}
				}, nullptr, nullptr, f2v(&Dummy::update), false);
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
				BP::Node* n;

				uint type_hash;
				uint type_size;

				void dtor()
				{
					auto in = (char*)&type_size + sizeof(uint);
					auto out = (char*)&type_size + sizeof(uint) + type_size;
					basic_type_dtor(type_hash, in);
					basic_type_dtor(type_hash, out);
				}

				void update(uint frame)
				{
					auto in = (char*)&type_size + sizeof(uint);
					auto out = (char*)&type_size + sizeof(uint) + type_size;
					if (n->input(0)->frame() > n->output(0)->frame())
					{
						basic_type_copy(type_hash, in, out, type_size);
						n->output(0)->set_frame(frame);
					}
				}
			};
#pragma pack()
			auto type_hash = FLAME_HASH(parameters.c_str());
			auto type_size = basic_type_size(type_hash);
			n = new NodePrivate(this, id, type, sizeof(Dummy) + type_size * 2, {
					{TypeInfo::get(TypeData, parameters.c_str()), "in", sizeof(Dummy), type_size, ""}
				}, {
					{TypeInfo::get(TypeData, parameters.c_str()), "out", sizeof(Dummy) + type_size, type_size, ""}
				}, nullptr, f2v(&Dummy::dtor), f2v(&Dummy::update), false);
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
				BP::Node* n;

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

				void update(uint frame)
				{
					auto& out = *(Array<int>*)((char*)&size + sizeof(uint) + type_size * size);
					auto out_frame = n->output(0)->frame();
					if (out_frame == -1)
					{
						out.s = size;
						auto m_size = type_size * size;
						out.v = (int*)f_malloc(m_size);
						memset(out.v, 0, m_size);
					}
					auto is_out_updated = false;
					for (auto i = 0; i < size; i++)
					{
						auto v = (char*)&size + sizeof(uint) + type_size * i;
						if (n->input(0)->frame() > out_frame)
						{
							basic_type_copy(type_hash, v, (char*)out.v + type_size * i, type_size);
							is_out_updated = true;
						}
					}
					if (is_out_updated)
						n->output(0)->set_frame(frame);
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
				}, nullptr, f2v(&Dummy::dtor), f2v(&Dummy::update), false);
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
		if (n->active)
			active_nodes.push_back(n);
		need_update_hierarchy = true;

		return n;
	}

	void BPPrivate::remove_node(NodePrivate* _n)
	{
		for (auto it = nodes.begin(); it != nodes.end(); it++)
		{
			auto n = it->get();
			if (n == _n)
			{
				on_node_removed(n);
				nodes.erase(it);
				break;
			}
		}

		need_update_hierarchy = true;
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
		libraries.clear();
		nodes.clear();
		need_update_hierarchy = true;
		update_list.clear();
	}

	void BPPrivate::add_to_pending_update(NodePrivate* n)
	{
		if (n->in_pending_update)
			return;
		pending_update_nodes.push_back(n);
		n->in_pending_update = true;
	}

	static void add_to_hierarchy(BPPrivate* scn, NodePrivate* n, uint& order)
	{
		if (n->order != 0xffffffff)
			return;
		for (auto& i : n->inputs)
		{
			auto o = i->links[0];
			if (o)
			{
				auto nn = o->node;
				add_to_hierarchy(scn, nn, order);
			}
		}
		n->order = order++;
	}

	static void add_to_update_list(BPPrivate* scn, NodePrivate* n)
	{
		if (n->in_update_list)
			return;
		std::list<NodePrivate*>::iterator it;
		for (it = scn->update_list.begin(); it != scn->update_list.end(); it++)
		{
			if (n->order < (*it)->order)
				break;
		}
		scn->update_list.insert(it, n);
		n->in_update_list = true;
	}

	void BPPrivate::update()
	{
		if (need_update_hierarchy)
		{
			auto order = 0U;
			for (auto& n : nodes)
				add_to_hierarchy(this, n.get(), order);
		}

		for (auto n : active_nodes)
			add_to_update_list(this, n);
		for (auto n : pending_update_nodes)
		{
			add_to_update_list(this, n);
			n->in_pending_update = false;
		}
		pending_update_nodes.clear();

		need_check_fail =  failed_slots.s > 0;
		failed_slots.resize(0);
		
		while (!update_list.empty())
		{
			auto n = update_list.front();
			update_list.erase(update_list.begin());
			n->in_update_list = false;

			n->update();

			for (auto& o : n->outputs)
			{
				if (o->frame == frame)
				{
					for (auto& i : o->links)
						add_to_update_list(this, i->node);
				}
			}
		}

		if (need_check_fail)
		{
			for (auto& n : nodes)
			{
				for (auto& in : n->inputs)
				{
					if (!in->fail_message.empty())
						failed_slots.push_back(in.get());
				}
			}
		}

		frame++;
		time += looper().delta_time;
	}

	const wchar_t* BP::Library::directory() const
	{
		return ((LibraryPrivate*)this)->directory.c_str();
	}

	TypeinfoDatabase* BP::Library::db() const
	{
		return ((LibraryPrivate*)this)->db;
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

	int BP::Slot::frame() const
	{
		return ((SlotPrivate*)this)->frame;
	}

	void BP::Slot::set_frame(int frame)
	{
		((SlotPrivate*)this)->frame = frame;
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

	const char* BP::Slot::fail_message() const
	{
		return ((SlotPrivate*)this)->fail_message.c_str();
	}

	void BP::Slot::set_fail_message(const char* message)
	{
		auto thiz = (SlotPrivate*)this;
		thiz->fail_message = message;
		thiz->node->scene->need_check_fail = true;
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

	const wchar_t* BP::filename() const
	{
		return ((BPPrivate*)this)->filename.c_str();
	}

	uint BP::library_count() const
	{
		return ((BPPrivate*)this)->libraries.size();
	}

	BP::Library* BP::library(uint idx) const
	{
		return ((BPPrivate*)this)->libraries[idx].get();
	}

	BP::Library* BP::add_library(const wchar_t* directory)
	{
		return ((BPPrivate*)this)->add_library(directory);
	}

	void BP::remove_library(BP::Library* m)
	{
		((BPPrivate*)this)->remove_library((LibraryPrivate*)m);
	}

	BP::Library* BP::find_library(const wchar_t* directory) const
	{
		return ((BPPrivate*)this)->find_library(directory);
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
		auto thiz = (BPPrivate*)this;
		std::vector<TypeinfoDatabase*> dbs;
		for (auto& l : thiz->libraries)
			dbs.push_back(l->db);
		extra_global_db_count = dbs.size();
		extra_global_dbs = dbs.data();
		auto n = thiz->add_node(type, id);
		extra_global_db_count = 0;
		extra_global_dbs = nullptr;
		return n;
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

	Array<BP::Slot*> BP::failed_slots() const
	{
		return ((BPPrivate*)this)->failed_slots;
	}

	BP* BP::create_from_file(const wchar_t* filename, bool test_mode)
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

		std::vector<std::wstring> library_descs;

		for (auto n_library : file_root.child("libraries"))
			library_descs.push_back(s2w(n_library.attribute("directory").value()));

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
			auto id = n_node.attribute("id").value();
			if (!test_mode && SUS::starts_with(id, "test_"))
				continue;

			NodeDesc node;

			node.type = n_node.attribute("type").value();
			node.id = id;
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
			if (!test_mode && (SUS::starts_with(o_addr, "test_") || SUS::starts_with(i_addr, "test_")))
				continue;

			LinkDesc link;
			link.out_addr = o_addr;
			link.in_addr = i_addr;
			link_descs.push_back(link);
		}

		auto bp = new BPPrivate();
		bp->filename = filename;
		bp->test_mode = test_mode;

		for (auto& l_d : library_descs)
			bp->add_library(l_d);

		std::vector<TypeinfoDatabase*> dbs;
		for (auto& l : bp->libraries)
			dbs.push_back(l->db);
		extra_global_db_count = dbs.size();
		extra_global_dbs = dbs.data();
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
		extra_global_db_count = 0;
		extra_global_dbs = nullptr;

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

		auto n_libraries = file_root.append_child("libraries");
		for (auto& l : bp->libraries)
		{
			auto n_module = n_libraries.append_child("directory");
			n_module.append_attribute("directory").set_value(w2s(l->directory).c_str());
		}

		std::vector<TypeinfoDatabase*> dbs;
		for (auto& l : bp->libraries)
			dbs.push_back(l->db);
		extra_global_db_count = dbs.size();
		extra_global_dbs = dbs.data();
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
		extra_global_db_count = 0;
		extra_global_dbs = nullptr;

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
		BP::Node* n;

		FLAME_B0;
		FLAME_RV(int, x, i);
		FLAME_RV(int, y, i);

		FLAME_B1;
		FLAME_RV(Vec2i, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			auto out_frame = out_s()->frame();
			auto out_updated = false;
			if (x_s()->frame() > out_frame)
			{
				out[0] = x;
				out_updated = true;
			}
			if (y_s()->frame() > out_frame)
			{
				out[1] = y;
				out_updated = true;
			}
			if (out_updated)
				out_s()->set_frame(frame);
		}
	};

	struct FLAME_R(R_MakeVec3i)
	{
		BP::Node* n;

		FLAME_B0;
		FLAME_RV(int, x, i);
		FLAME_RV(int, y, i);
		FLAME_RV(int, z, i);

		FLAME_B1;
		FLAME_RV(Vec3i, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			auto out_frame = out_s()->frame();
			auto out_updated = false;
			if (x_s()->frame() > out_frame)
			{
				out[0] = x;
				out_updated = true;
			}
			if (y_s()->frame() > out_frame)
			{
				out[1] = y;
				out_updated = true;
			}
			if (z_s()->frame() > out_frame)
			{
				out[2] = z;
				out_updated = true;
			}
			if (out_updated)
				out_s()->set_frame(frame);
		}
	};

	struct FLAME_R(R_MakeVec4i)
	{
		BP::Node* n;

		FLAME_B0;
		FLAME_RV(int, x, i);
		FLAME_RV(int, y, i);
		FLAME_RV(int, z, i);
		FLAME_RV(int, w, i);

		FLAME_B1;
		FLAME_RV(Vec4i, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			auto out_frame = out_s()->frame();
			auto out_updated = false;
			if (x_s()->frame() > out_frame)
			{
				out[0] = x;
				out_updated = true;
			}
			if (y_s()->frame() > out_frame)
			{
				out[1] = y;
				out_updated = true;
			}
			if (z_s()->frame() > out_frame)
			{
				out[2] = z;
				out_updated = true;
			}
			if (w_s()->frame() > out_frame)
			{
				out[3] = w;
				out_updated = true;
			}
			if (out_updated)
				out_s()->set_frame(frame);
		}
	};

	struct FLAME_R(R_MakeVec2u)
	{
		BP::Node* n;

		FLAME_B0;
		FLAME_RV(uint, x, i);
		FLAME_RV(uint, y, i);

		FLAME_B1;
		FLAME_RV(Vec2u, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			auto out_frame = out_s()->frame();
			auto out_updated = false;
			if (x_s()->frame() > out_frame)
			{
				out[0] = x;
				out_updated = true;
			}
			if (y_s()->frame() > out_frame)
			{
				out[1] = y;
				out_updated = true;
			}
			if (out_updated)
				out_s()->set_frame(frame);
		}
	};

	struct FLAME_R(R_MakeVec3u)
	{
		BP::Node* n;

		FLAME_B0;
		FLAME_RV(uint, x, i);
		FLAME_RV(uint, y, i);
		FLAME_RV(uint, z, i);

		FLAME_B1;
		FLAME_RV(Vec3u, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			auto out_frame = out_s()->frame();
			auto out_updated = false;
			if (x_s()->frame() > out_frame)
			{
				out[0] = x;
				out_updated = true;
			}
			if (y_s()->frame() > out_frame)
			{
				out[1] = y;
				out_updated = true;
			}
			if (z_s()->frame() > out_frame)
			{
				out[2] = z;
				out_updated = true;
			}
			if (out_updated)
				out_s()->set_frame(frame);
		}
	};

	struct FLAME_R(R_MakeVec4u)
	{
		BP::Node* n;

		FLAME_B0;
		FLAME_RV(uint, x, i);
		FLAME_RV(uint, y, i);
		FLAME_RV(uint, z, i);
		FLAME_RV(uint, w, i);

		FLAME_B1;
		FLAME_RV(Vec4u, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			auto out_frame = out_s()->frame();
			auto out_updated = false;
			if (x_s()->frame() > out_frame)
			{
				out[0] = x;
				out_updated = true;
			}
			if (y_s()->frame() > out_frame)
			{
				out[1] = y;
				out_updated = true;
			}
			if (z_s()->frame() > out_frame)
			{
				out[2] = z;
				out_updated = true;
			}
			if (w_s()->frame() > out_frame)
			{
				out[3] = w;
				out_updated = true;
			}
			if (out_updated)
				out_s()->set_frame(frame);
		}
	};

	struct FLAME_R(R_MakeVec2f)
	{
		BP::Node* n;

		FLAME_B0;
		FLAME_RV(float, x, i);
		FLAME_RV(float, y, i);

		FLAME_B1;
		FLAME_RV(Vec2f, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			auto out_frame = out_s()->frame();
			auto out_updated = false;
			if (x_s()->frame() > out_frame)
			{
				out[0] = x;
				out_updated = true;
			}
			if (y_s()->frame() > out_frame)
			{
				out[1] = y;
				out_updated = true;
			}
			if (out_updated)
				out_s()->set_frame(frame);
		}
	};

	struct FLAME_R(R_MakeVec3f)
	{
		BP::Node* n;

		FLAME_B0;
		FLAME_RV(float, x, i);
		FLAME_RV(float, y, i);
		FLAME_RV(float, z, i);

		FLAME_B1;
		FLAME_RV(Vec3f, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			auto out_frame = out_s()->frame();
			auto out_updated = false;
			if (x_s()->frame() > out_frame)
			{
				out[0] = x;
				out_updated = true;
			}
			if (y_s()->frame() > out_frame)
			{
				out[1] = y;
				out_updated = true;
			}
			if (z_s()->frame() > out_frame)
			{
				out[2] = z;
				out_updated = true;
			}
			if (out_updated)
				out_s()->set_frame(frame);
		}
	};

	struct FLAME_R(R_MakeVec4f)
	{
		BP::Node* n;

		FLAME_B0;
		FLAME_RV(float, x, i);
		FLAME_RV(float, y, i);
		FLAME_RV(float, z, i);
		FLAME_RV(float, w, i);

		FLAME_B1;
		FLAME_RV(Vec4f, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			auto out_frame = out_s()->frame();
			auto out_updated = false;
			if (x_s()->frame() > out_frame)
			{
				out[0] = x;
				out_updated = true;
			}
			if (y_s()->frame() > out_frame)
			{
				out[1] = y;
				out_updated = true;
			}
			if (z_s()->frame() > out_frame)
			{
				out[2] = z;
				out_updated = true;
			}
			if (w_s()->frame() > out_frame)
			{
				out[3] = w;
				out_updated = true;
			}
			if (out_updated)
				out_s()->set_frame(frame);
		}
	};

	struct FLAME_R(R_MakeVec2c)
	{
		BP::Node* n;

		FLAME_B0;
		FLAME_RV(uchar, x, i);
		FLAME_RV(uchar, y, i);

		FLAME_B1;
		FLAME_RV(Vec2c, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			auto out_frame = out_s()->frame();
			auto out_updated = false;
			if (x_s()->frame() > out_frame)
			{
				out[0] = x;
				out_updated = true;
			}
			if (y_s()->frame() > out_frame)
			{
				out[1] = y;
				out_updated = true;
			}
			if (out_updated)
				out_s()->set_frame(frame);
		}
	};

	struct FLAME_R(R_MakeVec3c)
	{
		BP::Node* n;

		FLAME_B0;
		FLAME_RV(uchar, x, i);
		FLAME_RV(uchar, y, i);
		FLAME_RV(uchar, z, i);

		FLAME_B1;
		FLAME_RV(Vec3c, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			auto out_frame = out_s()->frame();
			auto out_updated = false;
			if (x_s()->frame() > out_frame)
			{
				out[0] = x;
				out_updated = true;
			}
			if (y_s()->frame() > out_frame)
			{
				out[1] = y;
				out_updated = true;
			}
			if (z_s()->frame() > out_frame)
			{
				out[2] = z;
				out_updated = true;
			}
			if (out_updated)
				out_s()->set_frame(frame);
		}
	};

	struct FLAME_R(R_MakeVec4c)
	{
		BP::Node* n;

		FLAME_B0;
		FLAME_RV(uchar, x, i);
		FLAME_RV(uchar, y, i);
		FLAME_RV(uchar, z, i);
		FLAME_RV(uchar, w, i);

		FLAME_B1;
		FLAME_RV(Vec4c, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			auto out_frame = out_s()->frame();
			auto out_updated = false;
			if (x_s()->frame() > out_frame)
			{
				out[0] = x;
				out_updated = true;
			}
			if (y_s()->frame() > out_frame)
			{
				out[1] = y;
				out_updated = true;
			}
			if (z_s()->frame() > out_frame)
			{
				out[2] = z;
				out_updated = true;
			}
			if (w_s()->frame() > out_frame)
			{
				out[3] = w;
				out_updated = true;
			}
			if (out_updated)
				out_s()->set_frame(frame);
		}
	};

	struct FLAME_R(R_FloatToUint)
	{
		BP::Node* n;

		FLAME_B0;
		FLAME_RV(float, in, i);

		FLAME_B1;
		FLAME_RV(uint, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			auto out_frame = out_s()->frame();
			if (in_s()->frame() > out_frame)
			{
				out = in;
				out_s()->set_frame(frame);
			}
		}
	};

	struct FLAME_R(R_Add)
	{
		BP::Node* n;

		FLAME_B0;
		FLAME_RV(float, a, i);
		FLAME_RV(float, b, i);

		FLAME_B1;
		FLAME_RV(float, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			auto out_frame = out_s()->frame();
			if (a_s()->frame() > out_frame || b_s()->frame() > out_frame)
			{
				out = a + b;
				out_s()->set_frame(frame);
			}
		}
	};

	struct FLAME_R(R_Multiple)
	{
		BP::Node* n;

		FLAME_B0;
		FLAME_RV(float, a, i);
		FLAME_RV(float, b, i);

		FLAME_B1;
		FLAME_RV(float, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			auto out_frame = out_s()->frame();
			if (a_s()->frame() > out_frame || b_s()->frame() > out_frame)
			{
				out = a * b;
				out_s()->set_frame(frame);
			}
		}
	};

	struct FLAME_R(R_Time)
	{
		BP::Node* n;

		FLAME_B1;
		FLAME_RV(float, delta, o);
		FLAME_RV(float, total, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			delta = looper().delta_time;
			total = n->scene()->time;
			delta_s()->set_frame(frame);
			total_s()->set_frame(frame);
		}
	};

	struct FLAME_R(R_LinearInterpolation1d)
	{
		BP::Node* n;

		FLAME_B0;
		FLAME_RV(float, a, i);
		FLAME_RV(float, b, i);
		FLAME_RV(float, t, i);

		FLAME_B1;
		FLAME_RV(float, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			auto out_frame = out_s()->frame();
			if (a_s()->frame() > out_frame || b_s()->frame() > out_frame || t_s()->frame() > out_frame)
			{
				out = a + (b - a) * clamp(t, 0.f, 1.f);
				out_s()->set_frame(frame);
			}
		}
	};

	struct FLAME_R(R_LinearInterpolation2d)
	{
		BP::Node* n;

		FLAME_B0;
		FLAME_RV(Vec2f, a, i);
		FLAME_RV(Vec2f, b, i);
		FLAME_RV(float, t, i);

		FLAME_B1;
		FLAME_RV(Vec2f, out, o);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{
			auto out_frame = out_s()->frame();
			if (a_s()->frame() > out_frame || b_s()->frame() > out_frame || t_s()->frame() > out_frame)
			{
				out = a + (b - a) * clamp(t, 0.f, 1.f);
				out_s()->set_frame(frame);
			}
		}
	};

	struct FLAME_R(R_KeyListener)
	{
		BP::Node* n;

		FLAME_B0;
		FLAME_RV(Key, key, i);

		FLAME_FOUNDATION_EXPORTS void FLAME_RF(update)(uint frame)
		{

		}
	};
}

