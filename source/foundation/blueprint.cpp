#include <flame/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/foundation/typeinfo.h>

#include <flame/reflect_macros.h>

namespace flame
{
	struct BPPrivate;
	struct NodePrivate;
	struct SlotPrivate;

	struct LibraryPrivate : BP::Library
	{
		std::wstring filename;
		std::filesystem::path absolute_filename;
		void* module;
		TypeinfoDatabase* db;

		~LibraryPrivate();
	};

	struct SlotPrivate : BP::Slot
	{
		NodePrivate* node;
		IO io;
		const TypeInfo* type;
		std::string name;
		uint offset;
		uint size;
		std::string default_value;
		int frame;
		void* data;

		std::vector<SlotPrivate*> links;

		SlotPrivate(NodePrivate* node, IO io, const TypeInfo* type, const std::string& name, uint offset, uint size, const std::string& default_value);
		SlotPrivate(NodePrivate* node, IO io, VariableInfo* vi);

		void set_data(const void* data);
		bool can_link(SlotPrivate* target) const;
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

		BPPrivate()
		{
			frame = 0;
			time = 0.f;
			need_update_hierarchy = true;
		}

		void update_resources_file()
		{
			std::filesystem::path path(filename);
			path.replace_filename(L"bpres");
			std::ofstream file(path);
			for (auto& r : used_resources)
				file << r.string() << "\n";
			file.close();
		}

		LibraryPrivate* add_library(const std::wstring& filename);
		void remove_library(LibraryPrivate* m);
		LibraryPrivate* find_library(const std::wstring& filename) const;

		bool check_or_create_id(std::string& id) const;
		NodePrivate* add_node(const std::string& type, const std::string& id);
		void remove_node(NodePrivate* n);
		NodePrivate* find_node(const std::string& address) const;
		SlotPrivate* find_input(const std::string& address) const;
		SlotPrivate* find_output(const std::string& address) const;

		void clear();

		void add_to_pending_update(NodePrivate* n);
		void update();

		void report_used_resource(const std::wstring& filename);
	};

	LibraryPrivate::~LibraryPrivate()
	{
		TypeinfoDatabase::destroy(db);
	}

	SlotPrivate::SlotPrivate(NodePrivate* node, IO io, const TypeInfo* type, const std::string& name, uint offset, uint size, const std::string& default_value) :
		io(io),
		node(node),
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

	SlotPrivate::SlotPrivate(NodePrivate* node, IO io, VariableInfo* vi) :
		SlotPrivate(node, io, vi->type(), vi->name(),
			vi->offset(), vi->size(), vi->default_value())
	{
	}

	void SlotPrivate::set_data(const void* d)
	{
		frame = node->scene->frame;
		node->scene->add_to_pending_update(node);
		 
		type->copy_from(d, data);
	}

	bool SlotPrivate::can_link(SlotPrivate* target) const
	{
		assert(io == In);

		if (target)
		{
			if (target->io == In)
				return false;
			if (node == target->node) // same node
				return false;
		}

		if (links[0] == target)
			return true;

		if (target)
		{
			auto base_hash = type->base_hash();
			auto target_type = target->type;
			auto target_tag = target_type->tag();
			if (!(type == target_type || (type->tag() == TypePointer && (target_tag == TypeData || target_tag == TypePointer) &&
				(base_hash == target_type->base_hash() || base_hash == FLAME_CHASH("void")))))
				return false;
		}

		return true;
	}

	bool SlotPrivate::link_to(SlotPrivate* target)
	{
		assert(io == In);

		if (!can_link(target))
			return false;

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
		used_by_editor = false;
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
				inputs.emplace_back(new SlotPrivate(this, BP::Slot::In, v));
			else if (flags & VariableFlagOutput)
				outputs.emplace_back(new SlotPrivate(this, BP::Slot::Out, v));
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
		used_by_editor = false;
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

		for (auto& _i : _inputs)
		{
			auto i = new SlotPrivate(this, BP::Slot::In, _i.type, _i.name,
				_i.offset, _i.size, _i.default_value);
			inputs.emplace_back(i);
		}
		for (auto& _o : _outputs)
		{
			auto o = new SlotPrivate(this, BP::Slot::Out, _o.type, _o.name,
				_o.offset, _o.size, _o.default_value);
			outputs.emplace_back(o);
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

	LibraryPrivate* BPPrivate::add_library(const std::wstring& _fn)
	{
		for (auto& l : libraries)
		{
			if (l->filename == _fn)
				return nullptr;
		}

		auto fn = _fn;
		{
			std::wstring config_str;
#ifdef NDEBUG
			config_str = L"relwithdebinfo";
#else
			config_str = L"debug";
#endif
			static FLAME_SAL(str, L"{c}");
			auto pos = fn.find(str.s, 0, str.l);
			while (pos != std::wstring::npos)
			{
				fn = fn.replace(pos, str.l, config_str);
				pos = fn.find(str.s, 0, str.l);
			}
		}

		std::filesystem::path absolute_filename = std::filesystem::path(filename).parent_path() / fn;
		auto module = load_module(absolute_filename.c_str());
		if (!module)
		{
			printf("cannot add library %s\n", w2s(_fn).c_str());
			return nullptr;
		}
		absolute_filename = std::filesystem::canonical(absolute_filename);

		auto m = new LibraryPrivate;
		m->filename = _fn;
		m->absolute_filename = absolute_filename;
		m->module = module;
		m->db = TypeinfoDatabase::load(absolute_filename.c_str(), false, true);

		libraries.emplace_back(m);

		used_resources.push_back(_fn);
		update_resources_file();

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
					if (*it == l->filename)
					{
						used_resources.erase(it);
						break;
					}
				}
				update_resources_file();

				libraries.erase(it);

				return;
			}
		}
	}

	LibraryPrivate* BPPrivate::find_library(const std::wstring& filename) const
	{
		auto& libraries = ((BPPrivate*)this)->libraries;
		for (auto& l : libraries)
		{
			if (l->filename == filename)
				return l.get();
		}
		return nullptr;
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
			for (auto i = 0; i < nodes.size() + 1; i++)
			{
				id = "node_" + std::to_string(i);
				if (!find_node(id))
					break;
			}
		}
		return true;
	}

	NodePrivate* BPPrivate::add_node(const std::string& type, const std::string& _id)
	{
		std::string id = _id;
		if (!check_or_create_id(id))
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

		frame++;
		time += looper().delta_time;
	}

	void BPPrivate::report_used_resource(const std::wstring& _fn)
	{
		std::filesystem::path fn(_fn);
		fn = fn.lexically_relative(std::filesystem::path(filename).parent_path());

		for (auto& r : used_resources)
		{
			if (r == fn)
				return;
		}

		used_resources.push_back(fn);
		update_resources_file();
	}

	const wchar_t* BP::Library::filename() const
	{
		return ((LibraryPrivate*)this)->filename.c_str();
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

	bool BP::Slot::can_link(Slot* target) const
	{
		return ((SlotPrivate*)this)->can_link((SlotPrivate*)target);
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

	BP::Library* BP::add_library(const wchar_t* filename)
	{
		return ((BPPrivate*)this)->add_library(filename);
	}

	void BP::remove_library(BP::Library* m)
	{
		((BPPrivate*)this)->remove_library((LibraryPrivate*)m);
	}

	BP::Library* BP::find_library(const wchar_t* filename) const
	{
		return ((BPPrivate*)this)->find_library(filename);
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

	void BP::report_used_resource(const wchar_t* filename)
	{
		((BPPrivate*)this)->report_used_resource(filename);
	}

	BP* BP::create()
	{
		auto bp = new BPPrivate();
		bp->add_library(L"flame_foundation.dll");
		return bp;
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

		struct LibraryDesc
		{
			std::wstring filename;
		};
		std::vector<LibraryDesc> library_descs;

		for (auto n_library : file_root.child("libraries"))
		{
			LibraryDesc library;

			library.filename = s2w(n_library.attribute("filename").value());
			library_descs.push_back(library);
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
			LinkDesc link;
			link.out_addr = n_link.attribute("out").value();
			link.in_addr = n_link.attribute("in").value();
			link_descs.push_back(link);
		}

		auto bp = new BPPrivate();
		bp->filename = filename;
		bp->update_resources_file();

		for (auto& l_d : library_descs)
			bp->add_library(l_d.filename);

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
			auto n_module = n_libraries.append_child("library");
			n_module.append_attribute("filename").set_value(w2s(l->filename).c_str());
		}

		std::vector<TypeinfoDatabase*> dbs;
		for (auto& l : bp->libraries)
			dbs.push_back(l->db);
		extra_global_db_count = dbs.size();
		extra_global_dbs = dbs.data();
		auto n_nodes = file_root.append_child("nodes");
		for (auto& n : bp->nodes)
		{
			if (n->used_by_editor)
				continue;
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
				if (!type->is_array() && (tag == TypeEnumSingle || tag == TypeEnumMulti || tag == TypeData))
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
			if (n->used_by_editor)
				continue;
			for (auto& in : n->inputs)
			{
				auto out = in->links[0];
				if (out && !out->node->used_by_editor)
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

	struct R(R_MakeVec2i)
	{
		BP::Node* n;

		BASE0;
		RV(int, x, i);
		RV(int, y, i);

		BASE1;
		RV(Vec2i, out, o);

		FLAME_FOUNDATION_EXPORTS void RF(update)(uint frame)
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

	struct R(R_MakeVec3i)
	{
		BP::Node* n;

		BASE0;
		RV(int, x, i);
		RV(int, y, i);
		RV(int, z, i);

		BASE1;
		RV(Vec3i, out, o);

		FLAME_FOUNDATION_EXPORTS void RF(update)(uint frame)
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

	struct R(R_MakeVec4i)
	{
		BP::Node* n;

		BASE0;
		RV(int, x, i);
		RV(int, y, i);
		RV(int, z, i);
		RV(int, w, i);

		BASE1;
		RV(Vec4i, out, o);

		FLAME_FOUNDATION_EXPORTS void RF(update)(uint frame)
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

	struct R(R_MakeVec2u)
	{
		BP::Node* n;

		BASE0;
		RV(uint, x, i);
		RV(uint, y, i);

		BASE1;
		RV(Vec2u, out, o);

		FLAME_FOUNDATION_EXPORTS void RF(update)(uint frame)
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

	struct R(R_MakeVec3u)
	{
		BP::Node* n;

		BASE0;
		RV(uint, x, i);
		RV(uint, y, i);
		RV(uint, z, i);

		BASE1;
		RV(Vec3u, out, o);

		FLAME_FOUNDATION_EXPORTS void RF(update)(uint frame)
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

	struct R(R_MakeVec4u)
	{
		BP::Node* n;

		BASE0;
		RV(uint, x, i);
		RV(uint, y, i);
		RV(uint, z, i);
		RV(uint, w, i);

		BASE1;
		RV(Vec4u, out, o);

		FLAME_FOUNDATION_EXPORTS void RF(update)(uint frame)
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

	struct R(R_MakeVec2f)
	{
		BP::Node* n;

		BASE0;
		RV(float, x, i);
		RV(float, y, i);

		BASE1;
		RV(Vec2f, out, o);

		FLAME_FOUNDATION_EXPORTS void RF(update)(uint frame)
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

	struct R(R_MakeVec3f)
	{
		BP::Node* n;

		BASE0;
		RV(float, x, i);
		RV(float, y, i);
		RV(float, z, i);

		BASE1;
		RV(Vec3f, out, o);

		FLAME_FOUNDATION_EXPORTS void RF(update)(uint frame)
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

	struct R(R_MakeVec4f)
	{
		BP::Node* n;

		BASE0;
		RV(float, x, i);
		RV(float, y, i);
		RV(float, z, i);
		RV(float, w, i);

		BASE1;
		RV(Vec4f, out, o);

		FLAME_FOUNDATION_EXPORTS void RF(update)(uint frame)
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

	struct R(R_MakeVec2c)
	{
		BP::Node* n;

		BASE0;
		RV(uchar, x, i);
		RV(uchar, y, i);

		BASE1;
		RV(Vec2c, out, o);

		FLAME_FOUNDATION_EXPORTS void RF(update)(uint frame)
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

	struct R(R_MakeVec3c)
	{
		BP::Node* n;

		BASE0;
		RV(uchar, x, i);
		RV(uchar, y, i);
		RV(uchar, z, i);

		BASE1;
		RV(Vec3c, out, o);

		FLAME_FOUNDATION_EXPORTS void RF(update)(uint frame)
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

	struct R(R_MakeVec4c)
	{
		BP::Node* n;

		BASE0;
		RV(uchar, x, i);
		RV(uchar, y, i);
		RV(uchar, z, i);
		RV(uchar, w, i);

		BASE1;
		RV(Vec4c, out, o);

		FLAME_FOUNDATION_EXPORTS void RF(update)(uint frame)
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

	struct R(R_FloatToUint)
	{
		BP::Node* n;

		BASE0;
		RV(float, in, i);

		BASE1;
		RV(uint, out, o);

		FLAME_FOUNDATION_EXPORTS void RF(update)(uint frame)
		{
			auto out_frame = out_s()->frame();
			if (in_s()->frame() > out_frame)
			{
				out = in;
				out_s()->set_frame(frame);
			}
		}
	};

	struct R(R_Add)
	{
		BP::Node* n;

		BASE0;
		RV(float, a, i);
		RV(float, b, i);

		BASE1;
		RV(float, out, o);

		FLAME_FOUNDATION_EXPORTS void RF(update)(uint frame)
		{
			auto out_frame = out_s()->frame();
			if (a_s()->frame() > out_frame || b_s()->frame() > out_frame)
			{
				out = a + b;
				out_s()->set_frame(frame);
			}
		}
	};

	struct R(R_Multiple)
	{
		BP::Node* n;

		BASE0;
		RV(float, a, i);
		RV(float, b, i);

		BASE1;
		RV(float, out, o);

		FLAME_FOUNDATION_EXPORTS void RF(update)(uint frame)
		{
			auto out_frame = out_s()->frame();
			if (a_s()->frame() > out_frame || b_s()->frame() > out_frame)
			{
				out = a * b;
				out_s()->set_frame(frame);
			}
		}
	};

	struct R(R_Time)
	{
		BP::Node* n;

		BASE1;
		RV(float, delta, o);
		RV(float, total, o);

		FLAME_FOUNDATION_EXPORTS void RF(update)(uint frame)
		{
			delta = looper().delta_time;
			total = n->scene()->time;
			delta_s()->set_frame(frame);
			total_s()->set_frame(frame);
		}
	};

	struct R(R_LinearInterpolation1d)
	{
		BP::Node* n;

		BASE0;
		RV(float, a, i);
		RV(float, b, i);
		RV(float, t, i);

		BASE1;
		RV(float, out, o);

		FLAME_FOUNDATION_EXPORTS void RF(update)(uint frame)
		{
			auto out_frame = out_s()->frame();
			if (a_s()->frame() > out_frame || b_s()->frame() > out_frame || t_s()->frame() > out_frame)
			{
				out = a + (b - a) * clamp(t, 0.f, 1.f);
				out_s()->set_frame(frame);
			}
		}
	};

	struct R(R_LinearInterpolation2d)
	{
		BP::Node* n;

		BASE0;
		RV(Vec2f, a, i);
		RV(Vec2f, b, i);
		RV(float, t, i);

		BASE1;
		RV(Vec2f, out, o);

		FLAME_FOUNDATION_EXPORTS void RF(update)(uint frame)
		{
			auto out_frame = out_s()->frame();
			if (a_s()->frame() > out_frame || b_s()->frame() > out_frame || t_s()->frame() > out_frame)
			{
				out = a + (b - a) * clamp(t, 0.f, 1.f);
				out_s()->set_frame(frame);
			}
		}
	};

	struct R(R_KeyListener)
	{
		BP::Node* n;

		BASE0;
		RV(Key, key, i);

		FLAME_FOUNDATION_EXPORTS void RF(update)(uint frame)
		{

		}
	};
}

