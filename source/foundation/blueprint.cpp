#include <flame/serialize.h>
#include <flame/foundation/blueprint.h>

namespace flame
{
	struct BPPrivate;
	struct NodePrivate;
	struct SlotPrivate;

	struct ModulePrivate : BP::Module
	{
		std::wstring filename;
		std::filesystem::path absolute_filename;
		void* module;
		TypeinfoDatabase* db;

		ModulePrivate();
		~ModulePrivate();
	};

	struct PackagePrivate : BP::Package
	{
		BPPrivate* scene;
		std::string id;
		std::wstring filename;
		BPPrivate* bp;

		PackagePrivate(BPPrivate* scene, const std::string& id, BPPrivate* bp);
		~PackagePrivate();
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
		void* raw_data;

		std::vector<SlotPrivate*> links;

		SlotPrivate(NodePrivate* node, IO io, const TypeInfo* type, const std::string& name, uint offset, uint size, const std::string& default_value);
		SlotPrivate(NodePrivate* node, IO io, VariableInfo* vi);

		void set_frame(int frame);
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
		UdtInfo* udt;
		bool initiative;

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
		NodePrivate(BPPrivate* scene, const std::string& id, uint size, const std::vector<SlotDesc>& inputs, const std::vector<SlotDesc>& outputs, void* ctor_addr, void* dtor_addr, void* update_addr);
		~NodePrivate();

		SlotPrivate* find_input(const std::string& name) const;
		SlotPrivate* find_output(const std::string& name) const;

		void update();
	};

	struct BPDestroyDependence : BP
	{
		std::vector<std::unique_ptr<ModulePrivate>> modules;
		std::unique_ptr<ModulePrivate> self_module;
	};

	struct BPPrivate : BPDestroyDependence
	{
		std::wstring filename;

		PackagePrivate* package;
		BPPrivate* root;

		std::vector<std::unique_ptr<PackagePrivate>> packages;
		std::vector<ModulePrivate*> package_modules;

		std::vector<TypeinfoDatabase*> dbs;

		std::vector<std::unique_ptr<NodePrivate>> nodes;

		std::vector<SlotPrivate*> input_exports;
		std::vector<SlotPrivate*> output_exports;

		std::vector<NodePrivate*> initiative_nodes;
		std::vector<NodePrivate*> pending_update_nodes;
		std::list<NodePrivate*> update_list;
		bool need_update_hierarchy;

		BPPrivate()
		{
			frame = 0;
			time = 0.f;
			package = nullptr;
			root = this;
			need_update_hierarchy = true;
		}

		Module* add_module(const std::wstring& filename);
		void remove_module(ModulePrivate* m);
		ModulePrivate* find_module(const std::wstring& filename) const;

		PackagePrivate* add_package(const std::wstring& filename, const std::string& id);
		void remove_package(PackagePrivate* i);
		PackagePrivate* find_package(const std::string& id) const;
		void collect_package_modules();

		void collect_dbs();

		bool check_or_create_id(std::string& id) const;
		NodePrivate* add_node(const std::string& type, const std::string& type_name);
		NodePrivate* add_node(uint size, const std::vector<SlotDesc>& inputs, const std::vector<SlotDesc>& outputs, void* ctor_addr, void* dtor_addr, void* update_addr, const std::string& id);
		void remove_node(NodePrivate* n);
		NodePrivate* find_node(const std::string& address) const;
		SlotPrivate* find_input(const std::string& address) const;
		SlotPrivate* find_output(const std::string& address) const;

		void clear();

		void add_to_pending_update(NodePrivate* n);
		void remove_from_pending_update(NodePrivate* n);
		void update();
	};

	ModulePrivate::ModulePrivate()
	{
		pos = Vec2f(0.f);
		external = false;
		user_data = nullptr;
	}

	ModulePrivate::~ModulePrivate()
	{
		free_module(module);
		TypeinfoDatabase::destroy(db);
	}

	PackagePrivate::PackagePrivate(BPPrivate* scene, const std::string& id, BPPrivate* bp) :
		scene(scene),
		id(id),
		bp(bp)
	{
		pos = Vec2f(0.f);
		external = false;
		user_data = nullptr;

		bp->package = this;
	}

	PackagePrivate::~PackagePrivate()
	{
		BP::destroy(bp);
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
		raw_data = (char*)node->object + offset;
		user_data = nullptr;

		if (io == In)
			links.push_back(nullptr);
		else /* if (type == Output) */
			set_frame(-1);
	}

	SlotPrivate::SlotPrivate(NodePrivate* node, IO io, VariableInfo* vi) :
		SlotPrivate(node, io, vi->type(), vi->name(),
			vi->offset(), vi->size(), vi->default_value())
	{
	}

	void SlotPrivate::set_frame(int frame)
	{
		((AttributeBase*)raw_data)->frame = frame;
	}

	void SlotPrivate::set_data(const void* d)
	{
		set_frame(node->scene->frame);
		node->scene->root->add_to_pending_update(node);
		 
		type->copy_from(d, raw_data);
	}

	bool SlotPrivate::link_to(SlotPrivate* target)
	{
		assert(io == In);

		if (target)
		{
			if (target->io == In)
				return false;
			if (node == target->node) // same node
				return false;
			auto p = node->scene->package;
			if (p && p == target->node->scene->package) // same package
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
				(base_hash == target_type->base_hash() || base_hash == cH("void")))))
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

		set_frame(node->scene->frame);

		auto root = node->scene->root;
		root->need_update_hierarchy = true;
		root->add_to_pending_update(node);
		if (target)
			root->add_to_pending_update(target->node);

		return true;
	}

	StringA SlotPrivate::get_address() const
	{
		return StringA(node->id + "." + name);
	}

	NodePrivate::NodePrivate(BPPrivate* scene, const std::string& id, UdtInfo* udt, void* module) :
		scene(scene),
		id(id),
		udt(udt),
		initiative(false),
		module(module),
		order(0xffffffff),
		in_pending_update(false),
		in_update_list(false)
	{
		pos = Vec2f(0.f);
		external = false;
		user_data = nullptr;

		auto size = udt->size();
		object = malloc(size);
		memset(object, 0, size);

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
			if (f)
			{
				if (f->return_type()->hash() == TypeInfo::get_hash(TypeData, "void") && f->parameter_count() == 1 && f->parameter_type(0)->hash() == TypeInfo::get_hash(TypePointer, "BP"))
					update_addr = (char*)module + (uint)f->rva();
				assert(update_addr);
			}
		}

		for (auto i = 0; i < udt->variable_count(); i++)
		{
			auto v = udt->variable(i);
			std::string deco = v->decoration();
			auto ai = deco.find('i') != std::string::npos, ao = deco.find('o') != std::string::npos;
			if (!ai && !ao)
				continue;
			assert(!(ai && ao));
			assert(v->type()->is_attribute());
			if (ai)
				inputs.emplace_back(new SlotPrivate(this, BP::Slot::In, v));
			else /* if (ao) */
				outputs.emplace_back(new SlotPrivate(this, BP::Slot::Out, v));
		}
	}

	NodePrivate::NodePrivate(BPPrivate* scene, const std::string& id, uint size, const std::vector<SlotDesc>& _inputs, const std::vector<SlotDesc>& _outputs, void* ctor_addr, void* _dtor_addr, void* _update_addr) :
		scene(scene),
		id(id),
		udt(nullptr),
		initiative(false),
		module(nullptr),
		order(0xffffffff),
		in_pending_update(false),
		in_update_list(false)
	{
		pos = Vec2f(0.f);
		external = false;
		user_data = nullptr;

		object = malloc(size);
		memset(object, 0, size);

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
		auto root = scene->root;
		for (auto& o : outputs)
		{
			for (auto& l : o->links)
			{
				l->links[0] = nullptr;
				l->set_frame(scene->frame);
				root->add_to_pending_update(l->node);
			}
		}

		if (dtor_addr)
			cmf(p2f<MF_v_v>(dtor_addr), object);
		free(object);

		root->need_update_hierarchy = true;
		root->remove_from_pending_update(this);
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
		for (auto& input : inputs)
		{
			auto out = input->links[0];
			if (out)
			{
				auto ia = (AttributeBase*)input->raw_data;
				if (out->type->tag() == TypeData && input->type->tag() == TypePointer)
				{
					auto p = out->data();
					memcpy(input->data(), &p, sizeof(void*));
				}
				else
					memcpy(input->data(), out->data(), input->size - sizeof(AttributeBase));
				auto new_frame = ((AttributeBase*)out->raw_data)->frame;
				if (new_frame > ia->frame)
					ia->frame = new_frame;
			}
		}

		if (update_addr)
			cmf(p2f<MF_v_vp>(update_addr), object, scene);
	}

	BP::Module* BPPrivate::add_module(const std::wstring& fn)
	{
		if (fn == L"bp.dll")
			return nullptr;
		for (auto& m : modules)
		{
			if (m->filename == fn)
				return nullptr;
		}

		std::filesystem::path absolute_filename = fn;
		auto module = load_module(fn.c_str());
		if (module)
			absolute_filename = get_app_path().str() + L"/" + absolute_filename.wstring();
		else
		{
			std::filesystem::path path(filename);
			absolute_filename = path.parent_path() / fn;
			module = load_module(absolute_filename.c_str());
		}
		if (!module)
		{
			printf("cannot add module %s\n", w2s(fn).c_str());
			return nullptr;
		}
		absolute_filename = std::filesystem::canonical(absolute_filename);

		auto m = new ModulePrivate;
		m->filename = fn;
		m->absolute_filename = absolute_filename;
		m->module = module;
		std::vector<TypeinfoDatabase*> dbs;
		for (auto& m : modules)
			dbs.push_back(m->db);
		m->db = TypeinfoDatabase::load(dbs.size(), dbs.data(), absolute_filename.replace_extension(L".typeinfo").c_str());

		modules.emplace_back(m);
		collect_dbs();

		return m;
	}

	void BPPrivate::remove_module(ModulePrivate* m)
	{
		if (m->filename == L"bp.dll")
			return;

		for (auto it = modules.begin(); it != modules.end(); it++)
		{
			if (it->get() == m)
			{
				auto& nodes = this->nodes;
				for (auto n_it = nodes.begin(); n_it != nodes.end(); )
				{
					auto udt = (*n_it)->udt;
					if (udt && udt->db() == (*it)->db)
						n_it = nodes.erase(n_it);
					else
						n_it++;
				}

				modules.erase(it);
				collect_dbs();

				return;
			}
		}
	}

	ModulePrivate* BPPrivate::find_module(const std::wstring& filename) const
	{
		auto& modules = ((BPPrivate*)this)->modules;
		for (auto& m : modules)
		{
			if (m->filename == filename)
				return m.get();
		}
		return nullptr;
	}

	PackagePrivate* BPPrivate::add_package(const std::wstring& fn, const std::string& id)
	{
		std::string s_id;
		if (!id.empty())
		{
			s_id = id;
			if (find_package(s_id))
				return nullptr;
		}
		else
		{
			for (auto i = 0; i < packages.size() + 1; i++)
			{
				s_id = "package_" + std::to_string(i);
				if (!find_package(s_id))
					break;
			}
		}

		auto bp = (BPPrivate*)BP::create_from_file((std::filesystem::path(filename).parent_path().parent_path() / fn / L"bp").wstring().c_str(), true, root);
		if (!bp)
			return nullptr;

		auto p = new PackagePrivate(this, s_id, bp);
		p->filename = fn;
		packages.emplace_back(p);

		collect_package_modules();
		collect_dbs();
		root->need_update_hierarchy = true;

		return p;
	}

	void BPPrivate::remove_package(PackagePrivate* i)
	{
		for (auto it = packages.begin(); it != packages.end(); it++)
		{
			if (it->get() == i)
			{
				packages.erase(it);

				collect_package_modules();
				collect_dbs();
				root->need_update_hierarchy = true;

				return;
			}
		}
	}

	PackagePrivate* BPPrivate::find_package(const std::string& id) const
	{
		auto& packages = ((BPPrivate*)this)->packages;
		for (auto& p : packages)
		{
			if (p->id == id)
				return p.get();
		}
		return nullptr;
	}

	void BPPrivate::collect_package_modules()
	{
		package_modules.clear();
		auto had = [&](ModulePrivate* m) {
			for (auto& _m : modules)
			{
				if (_m->module == m->module)
					return true;
			}
			for (auto _m : package_modules)
			{
				if (_m->module == m->module)
					return true;
			}
			return false;
		};
		for (auto& p : packages)
		{
			auto bp = p->bp;
			for (auto& m : bp->modules)
			{
				if (!had(m.get()))
					package_modules.push_back(m.get());
			}
			if (!had(bp->self_module.get()))
				package_modules.push_back(bp->self_module.get());
			for (auto& m : bp->package_modules)
			{
				if (!had(m))
					package_modules.push_back(m);
			}
		}
	}

	void BPPrivate::collect_dbs()
	{
		dbs.clear();
		for (auto& m : modules)
			dbs.push_back(m->db);
		if (self_module)
			dbs.push_back(self_module->db);
		for (auto m : package_modules)
			dbs.push_back(m->db);
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
		auto type_hash = H(type.c_str());
		UdtInfo* udt = nullptr;
		void* module = nullptr;
		{
			for (auto& m : modules)
			{
				udt = m->db->find_udt(type_hash);
				if (udt)
				{
					module = m->module;
					break;
				}
			}
			if (!udt && self_module)
			{
				udt = self_module->db->find_udt(type_hash);
				if (udt)
					module = self_module->module;
			}
			if (!udt)
			{
				for (auto m : package_modules)
				{
					udt = m->db->find_udt(type_hash);
					if (udt)
					{
						module = m->module;
						break;
					}
				}
			}
		}

		if (!udt)
		{
			printf("cannot add node, type not found: %s\n", type.c_str());
			return nullptr;
		}

		std::string id = _id;
		if (!check_or_create_id(id))
		{
			printf("cannot add node, id repeated\n");
			return nullptr;
		}

		auto n = new NodePrivate(this, id, udt, module);
		nodes.emplace_back(n);

		root->need_update_hierarchy = true;

		return n;
	}

	NodePrivate* BPPrivate::add_node(uint size, const std::vector<SlotDesc>& inputs, const std::vector<SlotDesc>& outputs, void* ctor_addr, void* dtor_addr, void* update_addr, const std::string& _id)
	{
		std::string id = _id;
		if (!check_or_create_id(id))
		{
			printf("cannot add node, id repeated\n");
			return nullptr;
		}

		auto n = new NodePrivate(this, id, size, inputs, outputs, ctor_addr, dtor_addr, update_addr);
		nodes.emplace_back(n);

		root->need_update_hierarchy = true;

		return n;
	}

	void BPPrivate::remove_node(NodePrivate* n)
	{
		for (auto it = nodes.begin(); it != nodes.end(); it++)
		{
			if ((*it).get() == n)
			{
				nodes.erase(it);
				break;
			}
		}

		root->need_update_hierarchy = true;
	}

	NodePrivate* BPPrivate::find_node(const std::string& address) const
	{
		auto sp = ssplit(address, '.');
		switch (sp.size())
		{
		case 2:
			if (sp[0] != "*")
			{
				auto p = find_package(sp[0]);
				if (p)
					return p->bp->find_node(sp[1]);
				return nullptr;
			}
			for (auto& p : ((BPPrivate*)this)->packages)
			{
				auto n = p->bp->find_node(address);
				if (n)
					return n;
			}
			sp[0] = sp[1];
		case 1:
			for (auto& n : ((BPPrivate*)this)->nodes)
			{
				if (n->id == sp[0])
					return n.get();
			}
		}
		return nullptr;
	}

	SlotPrivate* BPPrivate::find_input(const std::string& address) const
	{
		auto sp = ssplit_lastone(address, '.');
		auto n = find_node(sp[0]);
		if (!n)
			return nullptr;
		return (SlotPrivate*)n->find_input(sp[1]);
	}

	SlotPrivate* BPPrivate::find_output(const std::string& address) const
	{
		auto sp = ssplit_lastone(address, '.');
		auto n = find_node(sp[0]);
		if (!n)
			return nullptr;
		return (SlotPrivate*)n->find_output(sp[1]);
	}

	void BPPrivate::clear()
	{
		packages.clear();
		nodes.clear();
		input_exports.clear();
		output_exports.clear();
		modules.clear();
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

	void BPPrivate::remove_from_pending_update(NodePrivate* n)
	{
		if (!n->in_pending_update)
			return;
		for (auto it = pending_update_nodes.begin(); it != pending_update_nodes.end(); it++)
		{
			if (*it == n)
			{
				pending_update_nodes.erase(it);
				n->in_pending_update = false;
				return;
			}
		}
	}

	static void collect_nodes(BPPrivate* scn, std::vector<NodePrivate*>& ns)
	{
		for (auto& n : scn->nodes)
		{
			n->order = 0xffffffff;
			ns.push_back(n.get());
		}
		for (auto& p : scn->packages)
			collect_nodes(p->bp, ns);
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
		assert(root == this);

		if (need_update_hierarchy)
		{
			std::vector<NodePrivate*> ns;
			collect_nodes(this, ns);

			auto order = 0U;
			for (auto n : ns)
				add_to_hierarchy(this, n, order);
		}

		for (auto n : initiative_nodes)
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
				if (o->frame() == frame)
				{
					for (auto& i : o->links)
						add_to_update_list(this, i->node);
				}
			}
		}

		frame++;
		time += looper().delta_time;
	}

	const wchar_t* BP::Module::filename() const
	{
		return ((ModulePrivate*)this)->filename.c_str();
	}

	void* BP::Module::module() const
	{
		return ((ModulePrivate*)this)->module;
	}

	TypeinfoDatabase* BP::Module::db() const
	{
		return ((ModulePrivate*)this)->db;
	}

	BP* BP::Package::bp() const
	{
		return ((PackagePrivate*)this)->bp;
	}

	BP* BP::Package::scene() const
	{
		return ((PackagePrivate*)this)->scene;
	}

	const char* BP::Package::id() const
	{
		return ((PackagePrivate*)this)->id.c_str();
	}

	void BP::Package::set_id(const char* id)
	{
		((PackagePrivate*)this)->id = id;
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

	BP::Node* BP::Slot::node() const
	{
		return ((SlotPrivate*)this)->node;
	}

	int BP::Slot::frame() const
	{
		return ((AttributeBase*)(((SlotPrivate*)this)->raw_data))->frame;
	}

	void BP::Slot::set_frame(int frame)
	{
		((SlotPrivate*)this)->set_frame(frame);
	}

	void* BP::Slot::data() const
	{
		return (char*)((SlotPrivate*)this)->raw_data + sizeof(AttributeBase);
	}

	void* BP::Slot::raw_data() const
	{
		return ((SlotPrivate*)this)->raw_data;
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

	void BP::Node::set_id(const char* id)
	{
		((NodePrivate*)this)->id = id;
	}

	UdtInfo* BP::Node::udt() const
	{
		return ((NodePrivate*)this)->udt;
	}

	bool BP::Node::initiative() const 
	{
		return ((NodePrivate*)this)->initiative;
	}

	void BP::Node::set_initiative(bool v)
	{
		auto thiz = (NodePrivate*)this;
		if (thiz->initiative == v)
			return;
		thiz->initiative = v;
		auto root = thiz->scene->root;
		if (!v)
		{
			for (auto it = root->initiative_nodes.begin(); it != root->initiative_nodes.end(); it++)
			{
				if (*it == thiz)
				{
					root->initiative_nodes.erase(it);
					break;
				}
			}
		}
		else
			root->initiative_nodes.push_back(thiz);
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

	BP::Package* BP::package() const
	{
		return ((BPPrivate*)this)->package;
	}

	uint BP::module_count() const
	{
		return ((BPPrivate*)this)->modules.size();
	}

	BP::Module* BP::module(uint idx) const
	{
		return ((BPPrivate*)this)->modules[idx].get();
	}

	BP::Module* BP::self_module() const
	{
		return ((BPPrivate*)this)->self_module.get();
	}

	BP::Module* BP::add_module(const wchar_t* filename)
	{
		return ((BPPrivate*)this)->add_module(filename);
	}

	void BP::remove_module(BP::Module* m)
	{
		((BPPrivate*)this)->remove_module((ModulePrivate*)m);
	}

	BP::Module* BP::find_module(const wchar_t* filename) const
	{
		return ((BPPrivate*)this)->find_module(filename);
	}

	uint BP::package_count() const
	{
		return ((BPPrivate*)this)->packages.size();
	}

	BP::Package* BP::package(uint idx) const
	{
		return ((BPPrivate*)this)->packages[idx].get();
	}

	BP::Package* BP::add_package(const wchar_t* filename, const char* id)
	{
		return ((BPPrivate*)this)->add_package(filename, id);
	}

	void BP::remove_package(Package* e)
	{
		((BPPrivate*)this)->remove_package((PackagePrivate*)e);
	}

	BP::Package* BP::find_package(const char* id) const
	{
		return ((BPPrivate*)this)->find_package(id);
	}

	uint BP::db_count() const
	{
		return ((BPPrivate*)this)->dbs.size();
	}

	const TypeinfoDatabase* const* BP::dbs() const
	{
		return ((BPPrivate*)this)->dbs.data();
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

	BP::Node* BP::find_node(const char* address) const
	{
		return ((BPPrivate*)this)->find_node(address);
	}

	BP::Slot* BP::find_input(const char* address) const
	{
		return ((BPPrivate*)this)->find_input(address);
	}

	BP::Slot* BP::find_output(const char* address) const
	{
		return ((BPPrivate*)this)->find_output(address);
	}

	uint BP::input_export_count() const
	{
		return ((BPPrivate*)this)->input_exports.size();
	}

	BP::Slot* BP::input_export(uint idx) const
	{
		return ((BPPrivate*)this)->input_exports[idx];
	}

	void BP::add_input_export(Slot* s)
	{
		auto& input_exports = ((BPPrivate*)this)->input_exports;
		for (auto& e : input_exports)
		{
			if (e == s)
				return;
		}
		input_exports.emplace_back((SlotPrivate*)s);
	}

	void BP::remove_input_export(Slot* s)
	{
		auto& input_exports = ((BPPrivate*)this)->input_exports;
		for (auto it = input_exports.begin(); it != input_exports.end(); it++)
		{
			if ((*it) == s)
			{
				input_exports.erase(it);
				return;
			}
		}
	}

	int BP::find_input_export(Slot* s) const
	{
		auto& input_exports = ((BPPrivate*)this)->input_exports;
		for (auto i = 0; i < input_exports.size(); i++)
		{
			if (input_exports[i] == s)
				return i;
		}
		return -1;
	}

	uint BP::output_export_count() const
	{
		return ((BPPrivate*)this)->output_exports.size();
	}

	BP::Slot* BP::output_export(uint idx) const
	{
		return ((BPPrivate*)this)->output_exports[idx];
	}

	void BP::add_output_export(Slot* s)
	{
		auto& output_exports = ((BPPrivate*)this)->output_exports;
		for (auto& e : output_exports)
		{
			if (e == s)
				return;
		}
		output_exports.emplace_back((SlotPrivate*)s);
	}

	void BP::remove_output_export(Slot* s)
	{
		auto& output_exports = ((BPPrivate*)this)->output_exports;
		for (auto it = output_exports.begin(); it != output_exports.end(); it++)
		{
			if ((*it) == s)
			{
				output_exports.erase(it);
				return;
			}
		}
	}

	int BP::find_output_export(Slot* s) const
	{
		auto& output_exports = ((BPPrivate*)this)->output_exports;
		for (auto i = 0; i < output_exports.size(); i++)
		{
			if (output_exports[i] == s)
				return i;
		}
		return -1;
	}

	void BP::clear()
	{
		((BPPrivate*)this)->clear();
	}

	void BP::update()
	{
		((BPPrivate*)this)->update();
	}

	BP* BP::create()
	{
		auto bp = new BPPrivate();
		bp->add_module(L"flame_foundation.dll");
		return bp;
	}

	BP* BP::create_from_file(const wchar_t* filename, bool no_compile, BP* root)
	{
		auto s_filename = w2s(filename);
		auto path = std::filesystem::path(filename);
		auto ppath = path.parent_path();
		auto ppath_str = ppath.wstring();
		auto ppath_slash_str = ppath_str.empty() ? L"" : ppath_str + L"/";

		printf("begin to load bp: %s\n", s_filename.c_str());

		pugi::xml_document file;
		pugi::xml_node file_root;
		if (!file.load_file(filename) || (file_root = file.first_child()).name() != std::string("BP"))
		{
			printf("bp file does not exist, abort\n", s_filename.c_str());
			printf("end loading bp: %s\n", s_filename.c_str());
			return nullptr;
		}

		if (!no_compile) // delete pervious created random pdbs
		{
			auto p = ppath / L"build/Debug";
			if (std::filesystem::exists(p))
			{
				std::vector<std::filesystem::path> pdbs;
				for (std::filesystem::directory_iterator end, it(p); it != end; it++)
				{
					if (!std::filesystem::is_directory(it->status()))
					{
						auto ext = it->path().extension().wstring();
						if (ext == L".pdb" && !is_file_occupied(it->path().wstring().c_str()))
							pdbs.push_back(it->path());
					}
				}
				for (auto& p : pdbs)
					std::filesystem::remove(p);
			}
		}

		struct ModuleDesc
		{
			std::wstring filename;
			Vec2f pos;
		};
		std::vector<ModuleDesc> module_descs;

		for (auto n_module : file_root.child("modules"))
		{
			ModuleDesc module;

			module.filename = s2w(n_module.attribute("filename").value());
			module.pos = stof2(n_module.attribute("pos").value());
			module_descs.push_back(module);
		}

		struct PackageDesc
		{
			std::wstring filename;
			std::string id;
			Vec2f pos;
		};
		std::vector<PackageDesc> package_descs; 

		for (auto n_package : file_root.child("packages"))
		{
			PackageDesc package;

			package.filename = s2w(n_package.attribute("filename").value());
			package.id = n_package.attribute("id").value();
			package.pos = stof2(n_package.attribute("pos").value());
			package_descs.push_back(package);
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
			bool initiative;
			Vec2f pos;
			std::vector<DataDesc> datas;
		};
		std::vector<NodeDesc> node_descs;

		for (auto n_node : file_root.child("nodes"))
		{
			NodeDesc node;

			node.type = n_node.attribute("type").value();
			node.id = n_node.attribute("id").value();
			node.initiative = n_node.attribute("initiative").as_int() == 1;
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

		std::vector<std::string> input_export_descs;
		std::vector<std::string> output_export_descs;

		for (auto n : file_root.child("exports").child("input"))
			input_export_descs.push_back(n.attribute("v").value());
		for (auto n : file_root.child("exports").child("output"))
			output_export_descs.push_back(n.attribute("v").value());

		auto bp = new BPPrivate();
		if (root)
			bp->root = (BPPrivate*)root;
		bp->filename = filename;

		for (auto& m_d : module_descs)
		{
			auto m = bp->add_module(m_d.filename);
			if (m)
				m->pos = m_d.pos;
		}

		for (auto& p_d : package_descs)
		{
			auto p = bp->add_package(p_d.filename, p_d.id);
			if (p)
				p->pos = p_d.pos;
		}

		if (!no_compile)
		{
			auto cmakelists_path = ppath / L"CMakeLists.txt";
			printf("generating cmakelists");

			std::ofstream cmakelists(cmakelists_path);
			cmakelists << "# THIS FILE IS AUTO GENERATED\n";
			cmakelists << "cmake_minimum_required(VERSION 3.4)\n";
			cmakelists << "project(bp)\n";
			cmakelists << "add_definitions(-W0 -std:c++latest)\n";
			cmakelists << "file(GLOB SOURCE_LIST \"*.c*\")\n";
			cmakelists << "add_library(bp SHARED ${SOURCE_LIST})\n";
			auto print_link_library = [&](ModulePrivate* m) {
				auto name = m->absolute_filename.replace_extension(L".lib").string();
				std::replace(name.begin(), name.end(), '\\', '/');
				cmakelists << "target_link_libraries(bp " << name << ")\n";
			};
			for (auto& m : bp->modules)
				print_link_library(m.get());
			for (auto& m : bp->package_modules)
				print_link_library(m);
			cmakelists << "target_include_directories(bp PRIVATE ${CMAKE_SOURCE_DIR}/../../include)\n";
			srand(::time(0));
			auto pdb_filename = std::to_string(::rand() % 100000);
			cmakelists << "set_target_properties(bp PROPERTIES PDB_NAME " << pdb_filename << ")\n";
			cmakelists << "add_custom_command(TARGET bp POST_BUILD COMMAND ${CMAKE_SOURCE_DIR}/../../bin/typeinfogen ${CMAKE_SOURCE_DIR}/build/debug/bp.dll ";
			auto print_typeinfogen_dependency = [&](ModulePrivate* m) {
				auto name = m->absolute_filename.string();
				std::replace(name.begin(), name.end(), '\\', '/');
				cmakelists << "-m" << name << " ";
			};
			for (auto& m : bp->modules)
				print_typeinfogen_dependency(m.get());
			for (auto& m : bp->package_modules)
				print_typeinfogen_dependency(m);
			cmakelists << "-p${CMAKE_SOURCE_DIR}/build/debug/" << pdb_filename << ".pdb)\n";
			cmakelists.close();

			printf(" - done\n");

			printf("cmaking:\n");
			std::wstring cmake_cmd(L"cmake ");
			if (ppath_str.empty())
				cmake_cmd += L" -B build";
			else
				cmake_cmd += L"-s " + ppath_str + L" -B " + ppath_str + L"/build";
			exec_and_redirect_to_std_output(nullptr, cmake_cmd.data());

			printf("compiling:\n");
			exec_and_redirect_to_std_output(L"", wsfmt(L"%s/Common7/IDE/devenv.com \"%s/build/bp.sln\" /build debug", s2w(VS_LOCATION).c_str(), (get_curr_path().str() + L"/" + ppath_slash_str).c_str()).data());
		}

		auto self_module_filename = ppath_slash_str + L"build/debug/bp.dll";
		auto self_module = load_module(self_module_filename.c_str());
		if (self_module)
		{
			auto m = new ModulePrivate;
			m->filename = self_module_filename;
			m->absolute_filename = std::filesystem::canonical(self_module_filename);
			m->module = self_module;
			m->db = TypeinfoDatabase::load(bp->dbs.size(), bp->dbs.data(), std::filesystem::path(self_module_filename).replace_extension(L".typeinfo").c_str());
			bp->self_module.reset(m);

			bp->collect_dbs();
		}

		for (auto& n_d : node_descs)
		{
			NodePrivate* n = nullptr;

			static SAL(prefix_enum, "D#Enum");
			static SAL(prefix_var, "D#Var");
			static SAL(prefix_array, "D#Array");
			if (n_d.type.compare(0, prefix_enum.l, prefix_enum.s) == 0)
			{
				std::string enum_name(n_d.type.begin() + prefix_enum.l + 1, n_d.type.end() - 1);
				struct Dummy
				{
					AttributeD<int> in;
					AttributeD<int> out;

					void update(BP* scene)
					{
						if (in.frame > out.frame)
						{
							out.v = in.v;
							out.frame = scene->frame;
						}
					}
				};
				n = bp->add_node(sizeof(Dummy), {
						{TypeInfo::get(TypeEnumSingle, enum_name.c_str(), true), "in", offsetof(Dummy, in), sizeof(Dummy::in), ""}
					}, {
						{TypeInfo::get(TypeEnumSingle, enum_name.c_str(), true), "out", offsetof(Dummy, out), sizeof(Dummy::in), ""}
					}, nullptr, nullptr, f2v(&Dummy::update), n_d.id);
			}
			else if (n_d.type.compare(0, prefix_var.l, prefix_var.s) == 0)
			{
				std::string type_name(n_d.type.begin() + prefix_var.l + 1, n_d.type.end() - 1);
				struct Dummy
				{
					uint type_hash;
					uint type_size;

					void dtor()
					{
						auto& in = *(AttributeD<int>*)((char*)&type_hash + sizeof(Dummy));
						auto& out = *(AttributeD<int>*)((char*)&type_hash + sizeof(Dummy) + sizeof(AttributeBase) + type_size);
						data_dtor(type_hash, &in.v);
						data_dtor(type_hash, &out.v);
					}

					void update(BP* scene)
					{
						auto& in = *(AttributeD<int>*)((char*)&type_hash + sizeof(Dummy));
						auto& out = *(AttributeD<int>*)((char*)&type_hash + sizeof(Dummy) + sizeof(AttributeBase) + type_size);
						if (in.frame > out.frame)
						{
							data_copy(type_hash, &in.v, &out.v, type_size);
							out.frame = scene->frame;
						}
					}
				};
				auto type_hash = H(type_name.c_str());
				auto type_size = data_size(type_hash);
				n = bp->add_node(sizeof(Dummy) + (sizeof(AttributeBase) + type_size) * 2, {
						{TypeInfo::get(TypeData, type_name.c_str(), true), "in", sizeof(Dummy), sizeof(AttributeBase) + type_size, ""}
					}, {
						{TypeInfo::get(TypeData, type_name.c_str(), true), "out", sizeof(Dummy) + sizeof(AttributeBase) + type_size, sizeof(AttributeBase) + type_size, ""}
					}, nullptr, f2v(&Dummy::dtor), f2v(&Dummy::update), n_d.id);
				auto obj = n->object;
				*(uint*)obj = type_hash;
				*(uint*)((char*)obj + sizeof(uint)) = type_size;
			}
			else if (n_d.type.compare(0, prefix_array.l, prefix_array.s) == 0)
			{
				auto parameters = ssplit(std::string(n_d.type.begin() + prefix_array.l + 1, n_d.type.end() - 1), '+');
				struct Dummy
				{
					uint type_hash;
					uint type_size;
					uint size;

					void dtor()
					{
						for (auto i = 0; i < size; i++)
							data_dtor(type_hash, &((AttributeD<int>*)((char*)&type_hash + sizeof(Dummy) + (sizeof(AttributeBase) + type_size) * i))->v);
						auto& out = *(AttributeD<Array<int>>*)((char*)&type_hash + sizeof(Dummy) + (sizeof(AttributeBase) + type_size) * size);
						for (auto i = 0; i < out.v.s; i++)
							data_dtor(type_hash, &out.v.v[i]);
						out.v.~Array();
					}

					void update(BP* scene)
					{
						auto& out = *(AttributeD<Array<int>>*)((char*)&type_hash + sizeof(Dummy) + (sizeof(AttributeBase) + type_size) * size);
						if (out.frame == -1)
						{
							out.v.s = size;
							auto m_size = type_size * size;
							out.v.v = (int*)f_malloc(m_size);
							memset(out.v.v, 0, m_size);
						}
						auto last_out_frame = out.frame;
						for (auto i = 0; i < size; i++)
						{
							auto& v = *(AttributeD<int>*)((char*)&type_hash + sizeof(Dummy) + (sizeof(AttributeBase) + type_size) * i);
							if (v.frame > last_out_frame)
							{
								data_copy(type_hash, &v.v, (char*)out.v.v + type_size * i, type_size);
								out.frame = scene->frame;
							}
						}
					}
				};
				auto tag = TypeData;
				auto type_name = parameters[1];
				auto base_name = type_name;
				if (type_name.back() == '*')
				{
					base_name.erase(base_name.end() - 1);
					tag = TypePointer;
				}
				auto type_hash = H(base_name.c_str());
				uint type_size = tag == TypeData ? data_size(type_hash) : sizeof(void*);
				auto size = stoi(parameters[0]);
				std::vector<SlotDesc> inputs;
				for (auto i = 0; i < size; i++)
				{
					inputs.push_back({
						TypeInfo::get(tag, base_name.c_str(), true), std::to_string(i),
						sizeof(Dummy) + (sizeof(AttributeBase) + type_size) * i, sizeof(AttributeBase) + type_size, ""
					});
				}
				n = bp->add_node(sizeof(Dummy) + (sizeof(AttributeBase) + type_size) * size, inputs, {
						{TypeInfo::get(TypeData, type_name.c_str(), true, true), "out", 
						sizeof(Dummy) + (sizeof(AttributeBase) + type_size) * size, sizeof(AttributeBase) + type_size, ""}
				}, nullptr, f2v(&Dummy::dtor), f2v(&Dummy::update), n_d.id);
				auto obj = n->object;
				*(uint*)obj = type_hash;
				*(uint*)((char*)obj + sizeof(uint)) = type_size;
				*(uint*)((char*)obj + sizeof(uint) + sizeof(uint)) = size;
			}
			else
				n = bp->add_node(n_d.type, n_d.id);
			if (n)
			{
				n->set_initiative(n_d.initiative);
				n->pos = n_d.pos;
				for (auto& d_d : n_d.datas)
				{
					auto input = n->find_input(d_d.name);
					auto type = input->type;
					auto tag = type->tag();
					if (!type->is_array() && (tag == TypeEnumSingle || tag == TypeEnumMulti || tag == TypeData))
						type->unserialize(bp->dbs, d_d.value, input->raw_data);
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

		for (auto& e_d : input_export_descs)
			bp->add_input_export(bp->find_input(e_d));
		for (auto& e_d : output_export_descs)
			bp->add_output_export(bp->find_output(e_d));

		printf("end loading bp: %s\n", s_filename.c_str());

		return bp;
	}

	void BP::save_to_file(BP* _bp, const wchar_t* filename)
	{
		auto bp = (BPPrivate*)_bp;

		bp->filename = filename;

		pugi::xml_document file;
		auto file_root = file.append_child("BP");

		auto n_modules = file_root.append_child("modules");
		for (auto& m : bp->modules)
		{
			if (m->external)
				continue;
			auto n_module = n_modules.append_child("module");
			n_module.append_attribute("filename").set_value(w2s(m->filename).c_str());
			n_module.append_attribute("pos").set_value(to_string(m->pos, 2).c_str());
		}

		if (!bp->packages.empty())
		{
			auto n_packages = file_root.append_child("packages");
			for (auto& p : bp->packages)
			{
				if (p->external)
					continue;

				auto n_import = n_packages.append_child("package");
				n_import.append_attribute("filename").set_value(w2s(p->filename).c_str());
				n_import.append_attribute("id").set_value(p->id.c_str());
				n_import.append_attribute("pos").set_value(to_string(p->pos, 2).c_str());
			}
		}

		std::vector<Module*> skipped_modules;
		for (auto& m : bp->modules)
		{
			if (m->external)
				skipped_modules.push_back(m.get());
		}
		auto n_nodes = file_root.append_child("nodes");
		for (auto& n : bp->nodes)
		{
			auto udt = n->udt;
			if (!udt || n->external)
				continue;
			auto skip = false;
			for (auto m : skipped_modules)
			{
				if (udt->db() == m->db())
				{
					skip = true;
					break;
				}
			}
			if (skip)
				continue;

			auto n_node = n_nodes.append_child("node");
			n_node.append_attribute("type").set_value(udt->type()->name());
			n_node.append_attribute("id").set_value(n->id.c_str());
			if (n->initiative)
				n_node.append_attribute("initiative").set_value(1);
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
					auto value_str = type->serialize(bp->dbs, input->data(), 2);
					if (value_str != input->default_value)
					{
						if (!n_datas)
							n_datas = n_node.append_child("datas");
						auto n_data = n_datas.append_child("data");
						n_data.append_attribute("name").set_value(input->name.c_str());
						n_data.append_attribute("value").set_value(type->serialize(bp->dbs, input->raw_data, 2).c_str());
					}
				}
			}
		}

		auto n_links = file_root.append_child("links");
		for (auto& p : bp->packages)
		{
			for (auto& in : p->bp->input_exports)
			{
				auto out = in->links[0];
				if (out)
				{
					auto n_link = n_links.append_child("link");
					auto out_bp = out->node->scene;
					n_link.append_attribute("out").set_value(((out_bp != bp ? out_bp->package->id + "." : "") + out->get_address().v).c_str());
					n_link.append_attribute("in").set_value((p->id + "." + in->get_address().v).c_str());
				}
			}
		}
		for (auto& n : bp->nodes)
		{
			if (n->external)
				continue;
			for (auto& in : n->inputs)
			{
				auto out = in->links[0];
				if (out && !out->node->external)
				{
					auto n_link = n_links.append_child("link");
					auto out_bp = out->node->scene;
					n_link.append_attribute("out").set_value(((out_bp != bp ? out_bp->package->id + "." : "") + out->get_address().v).c_str());
					n_link.append_attribute("in").set_value(in->get_address().v);
				}
			}
		}

		if (!bp->input_exports.empty() || !bp->output_exports.empty())
		{
			auto n_exports = file_root.append_child("exports");
			auto n_input_exports = n_exports.append_child("input");
			for (auto& e : bp->input_exports)
				n_input_exports.append_child("export").append_attribute("v").set_value(e->get_address().v);
			auto n_output_exports = n_exports.append_child("output");
			for (auto& e : bp->output_exports)
				n_output_exports.append_child("export").append_attribute("v").set_value(e->get_address().v);
		}

		file.save_file(filename);
	}

	void BP::destroy(BP *bp)
	{
		delete(BPPrivate*)bp;
	}

	struct Vec1i$
	{
		AttributeD<int> x$i;

		AttributeD<Vec1i> out$o;

		FLAME_FOUNDATION_EXPORTS void update$(BP* scene)
		{
			auto last_out_frame = out$o.frame;
			if (x$i.frame > last_out_frame)
			{
				out$o.v[0] = x$i.v;
				out$o.frame = scene->frame;
			}
		}
	};

	struct Vec2i$
	{
		AttributeD<int> x$i;
		AttributeD<int> y$i;

		AttributeD<Vec2i> out$o;

		FLAME_FOUNDATION_EXPORTS void update$(BP* scene)
		{
			auto last_out_frame = out$o.frame;
			if (x$i.frame > last_out_frame)
			{
				out$o.v[0] = x$i.v;
				out$o.frame = scene->frame;
			}
			if (y$i.frame > last_out_frame)
			{
				out$o.v[1] = y$i.v;
				out$o.frame = scene->frame;
			}
		}
	};

	struct Vec3i$
	{
		AttributeD<int> x$i;
		AttributeD<int> y$i;
		AttributeD<int> z$i;

		AttributeD<Vec3i> out$o;

		FLAME_FOUNDATION_EXPORTS void update$(BP* scene)
		{
			auto last_out_frame = out$o.frame;
			if (x$i.frame > last_out_frame)
			{
				out$o.v[0] = x$i.v;
				out$o.frame = scene->frame;
			}
			if (y$i.frame > last_out_frame)
			{
				out$o.v[1] = y$i.v;
				out$o.frame = scene->frame;
			}
			if (z$i.frame > last_out_frame)
			{
				out$o.v[2] = z$i.v;
				out$o.frame = scene->frame;
			}
		}
	};

	struct Vec4i$
	{
		AttributeD<int> x$i;
		AttributeD<int> y$i;
		AttributeD<int> z$i;
		AttributeD<int> w$i;

		AttributeD<Vec4i> out$o;

		FLAME_FOUNDATION_EXPORTS void update$(BP* scene)
		{
			auto last_out_frame = out$o.frame;
			if (x$i.frame > last_out_frame)
			{
				out$o.v[0] = x$i.v;
				out$o.frame = scene->frame;
			}
			if (y$i.frame > last_out_frame)
			{
				out$o.v[1] = y$i.v;
				out$o.frame = scene->frame;
			}
			if (z$i.frame > last_out_frame)
			{
				out$o.v[2] = z$i.v;
				out$o.frame = scene->frame;
			}
			if (w$i.frame > last_out_frame)
			{
				out$o.v[3] = w$i.v;
				out$o.frame = scene->frame;
			}
		}
	};

	struct Vec1u$
	{
		AttributeD<uint> x$i;

		AttributeD<Vec1u> out$o;

		FLAME_FOUNDATION_EXPORTS void update$(BP* scene)
		{
			auto last_out_frame = out$o.frame;
			if (x$i.frame > last_out_frame)
			{
				out$o.v[0] = x$i.v;
				out$o.frame = scene->frame;
			}
		}
	};

	struct Vec2u$
	{
		AttributeD<uint> x$i;
		AttributeD<uint> y$i;

		AttributeD<Vec2u> out$o;

		FLAME_FOUNDATION_EXPORTS void update$(BP* scene)
		{
			auto last_out_frame = out$o.frame;
			if (x$i.frame > last_out_frame)
			{
				out$o.v[0] = x$i.v;
				out$o.frame = scene->frame;
			}
			if (y$i.frame > last_out_frame)
			{
				out$o.v[1] = y$i.v;
				out$o.frame = scene->frame;
			}
		}
	};

	struct Vec3u$
	{
		AttributeD<uint> x$i;
		AttributeD<uint> y$i;
		AttributeD<uint> z$i;

		AttributeD<Vec3u> out$o;

		FLAME_FOUNDATION_EXPORTS void update$(BP* scene)
		{
			auto last_out_frame = out$o.frame;
			if (x$i.frame > last_out_frame)
			{
				out$o.v[0] = x$i.v;
				out$o.frame = scene->frame;
			}
			if (y$i.frame > last_out_frame)
			{
				out$o.v[1] = y$i.v;
				out$o.frame = scene->frame;
			}
			if (z$i.frame > last_out_frame)
			{
				out$o.v[2] = z$i.v;
				out$o.frame = scene->frame;
			}
		}
	};

	struct Vec4u$
	{
		AttributeD<uint> x$i;
		AttributeD<uint> y$i;
		AttributeD<uint> z$i;
		AttributeD<uint> w$i;

		AttributeD<Vec4u> out$o;

		FLAME_FOUNDATION_EXPORTS void update$(BP* scene)
		{
			auto last_out_frame = out$o.frame;
			if (x$i.frame > last_out_frame)
			{
				out$o.v[0] = x$i.v;
				out$o.frame = scene->frame;
			}
			if (y$i.frame > last_out_frame)
			{
				out$o.v[1] = y$i.v;
				out$o.frame = scene->frame;
			}
			if (z$i.frame > last_out_frame)
			{
				out$o.v[2] = z$i.v;
				out$o.frame = scene->frame;
			}
			if (w$i.frame > last_out_frame)
			{
				out$o.v[3] = w$i.v;
				out$o.frame = scene->frame;
			}
		}
	};

	struct Vec1f$
	{
		AttributeD<float> x$i;

		AttributeD<Vec1f> out$o;

		FLAME_FOUNDATION_EXPORTS void update$(BP* scene)
		{
			auto last_out_frame = out$o.frame;
			if (x$i.frame > last_out_frame)
			{
				out$o.v[0] = x$i.v;
				out$o.frame = scene->frame;
			}
		}
	};

	struct Vec2f$
	{
		AttributeD<float> x$i;
		AttributeD<float> y$i;

		AttributeD<Vec2f> out$o;

		FLAME_FOUNDATION_EXPORTS void update$(BP* scene)
		{
			auto last_out_frame = out$o.frame;
			if (x$i.frame > last_out_frame)
			{
				out$o.v[0] = x$i.v;
				out$o.frame = scene->frame;
			}
			if (y$i.frame > last_out_frame)
			{
				out$o.v[1] = y$i.v;
				out$o.frame = scene->frame;
			}
		}
	};

	struct Vec3f$
	{
		AttributeD<float> x$i;
		AttributeD<float> y$i;
		AttributeD<float> z$i;

		AttributeD<Vec3f> out$o;

		FLAME_FOUNDATION_EXPORTS void update$(BP* scene)
		{
			auto last_out_frame = out$o.frame;
			if (x$i.frame > last_out_frame)
			{
				out$o.v[0] = x$i.v;
				out$o.frame = scene->frame;
			}
			if (y$i.frame > last_out_frame)
			{
				out$o.v[1] = y$i.v;
				out$o.frame = scene->frame;
			}
			if (z$i.frame > last_out_frame)
			{
				out$o.v[2] = z$i.v;
				out$o.frame = scene->frame;
			}
		}
	};

	struct Vec4f$
	{
		AttributeD<float> x$i;
		AttributeD<float> y$i;
		AttributeD<float> z$i;
		AttributeD<float> w$i;

		AttributeD<Vec4f> out$o;

		FLAME_FOUNDATION_EXPORTS void update$(BP* scene)
		{
			auto last_out_frame = out$o.frame;
			if (x$i.frame > last_out_frame)
			{
				out$o.v[0] = x$i.v;
				out$o.frame = scene->frame;
			}
			if (y$i.frame > last_out_frame)
			{
				out$o.v[1] = y$i.v;
				out$o.frame = scene->frame;
			}
			if (z$i.frame > last_out_frame)
			{
				out$o.v[2] = z$i.v;
				out$o.frame = scene->frame;
			}
			if (w$i.frame > last_out_frame)
			{
				out$o.v[3] = w$i.v;
				out$o.frame = scene->frame;
			}
		}
	};

	struct Vec1c$
	{
		AttributeD<uchar> x$i;

		AttributeD<Vec1c> out$o;

		FLAME_FOUNDATION_EXPORTS void update$(BP* scene)
		{
			auto last_out_frame = out$o.frame;
			if (x$i.frame > last_out_frame)
			{
				out$o.v[0] = x$i.v;
				out$o.frame = scene->frame;
			}
		}
	};

	struct Vec2c$
	{
		AttributeD<uchar> x$i;
		AttributeD<uchar> y$i;

		AttributeD<Vec2c> out$o;

		FLAME_FOUNDATION_EXPORTS void update$(BP* scene)
		{
			auto last_out_frame = out$o.frame;
			if (x$i.frame > last_out_frame)
			{
				out$o.v[0] = x$i.v;
				out$o.frame = scene->frame;
			}
			if (y$i.frame > last_out_frame)
			{
				out$o.v[1] = y$i.v;
				out$o.frame = scene->frame;
			}
		}
	};

	struct Vec3c$
	{
		AttributeD<uchar> x$i;
		AttributeD<uchar> y$i;
		AttributeD<uchar> z$i;

		AttributeD<Vec3c> out$o;

		FLAME_FOUNDATION_EXPORTS void update$(BP* scene)
		{
			auto last_out_frame = out$o.frame;
			if (x$i.frame > last_out_frame)
			{
				out$o.v[0] = x$i.v;
				out$o.frame = scene->frame;
			}
			if (y$i.frame > last_out_frame)
			{
				out$o.v[1] = y$i.v;
				out$o.frame = scene->frame;
			}
			if (z$i.frame > last_out_frame)
			{
				out$o.v[2] = z$i.v;
				out$o.frame = scene->frame;
			}
		}
	};

	struct Vec4c$
	{
		AttributeD<uchar> x$i;
		AttributeD<uchar> y$i;
		AttributeD<uchar> z$i;
		AttributeD<uchar> w$i;

		AttributeD<Vec4c> out$o;

		FLAME_FOUNDATION_EXPORTS void update$(BP* scene)
		{
			auto last_out_frame = out$o.frame;
			if (x$i.frame > last_out_frame)
			{
				out$o.v[0] = x$i.v;
				out$o.frame = scene->frame;
			}
			if (y$i.frame > last_out_frame)
			{
				out$o.v[1] = y$i.v;
				out$o.frame = scene->frame;
			}
			if (z$i.frame > last_out_frame)
			{
				out$o.v[2] = z$i.v;
				out$o.frame = scene->frame;
			}
			if (w$i.frame > last_out_frame)
			{
				out$o.v[3] = w$i.v;
				out$o.frame = scene->frame;
			}
		}
	};

	struct F2U$
	{
		AttributeD<float> v$i;

		AttributeD<uint> out$o;

		FLAME_FOUNDATION_EXPORTS void update$(BP* scene)
		{
			if (v$i.frame > out$o.frame)
			{
				out$o.v = v$i.v;
				out$o.frame = scene->frame;
			}
		}
	};

	struct Add$
	{
		AttributeD<float> a$i;
		AttributeD<float> b$i;

		AttributeD<float> out$o;

		FLAME_FOUNDATION_EXPORTS void update$(BP* scene)
		{
			if (a$i.frame > out$o.frame || b$i.frame > out$o.frame)
			{
				out$o.v = a$i.v + b$i.v;
				out$o.frame = scene->frame;
			}
		}
	};

	struct Multiple$
	{
		AttributeD<float> a$i;
		AttributeD<float> b$i;

		AttributeD<float> out$o;

		FLAME_FOUNDATION_EXPORTS void update$(BP* scene)
		{
			if (a$i.frame > out$o.frame || b$i.frame > out$o.frame)
			{
				out$o.v = a$i.v * b$i.v;
				out$o.frame = scene->frame;
			}
		}
	};

	struct MakeVec2f$
	{
		AttributeD<float> x$i;
		AttributeD<float> y$i;

		AttributeD<Vec2f> out$o;

		FLAME_FOUNDATION_EXPORTS void update$(BP* scene)
		{
			if (x$i.frame > out$o.frame)
				out$o.v.x() = x$i.v;
			if (y$i.frame > out$o.frame)
				out$o.v.y() = y$i.v;
			out$o.frame = scene->frame;
		}
	};

	struct Time$
	{
		AttributeD<float> delta$o;
		AttributeD<float> total$o;

		FLAME_FOUNDATION_EXPORTS void update$(BP* scene)
		{
			delta$o.v = looper().delta_time;
			total$o.v = scene->time;
			delta$o.frame = scene->frame;
			total$o.frame = scene->frame;
		}
	};

	struct LinearInterpolation1d$
	{
		AttributeD<float> a$i;
		AttributeD<float> b$i;
		AttributeD<float> t$i;

		AttributeD<float> out$o;

		FLAME_FOUNDATION_EXPORTS void update$(BP* scene)
		{
			if (a$i.frame > out$o.frame || b$i.frame > out$o.frame || t$i.frame > out$o.frame)
			{
				if (t$i.v <= 0.f)
					out$o.v = a$i.v;
				else if (t$i.v >= 1.f)
					out$o.v = b$i.v;
				else
					out$o.v = a$i.v + (b$i.v - a$i.v) * t$i.v;
				out$o.frame = scene->frame;
			}
		}
	};

	struct LinearInterpolation2d$
	{
		AttributeD<Vec2f> a$i;
		AttributeD<Vec2f> b$i;
		AttributeD<float> t$i;

		AttributeD<Vec2f> out$o;

		FLAME_FOUNDATION_EXPORTS void update$(BP* scene)
		{
			if (a$i.frame > out$o.frame || b$i.frame > out$o.frame || t$i.frame > out$o.frame)
			{
				if (t$i.v <= 0.f)
					out$o.v = a$i.v;
				else if (t$i.v >= 1.f)
					out$o.v = b$i.v;
				else
					out$o.v = a$i.v + (b$i.v - a$i.v) * t$i.v;
				out$o.frame = scene->frame;
			}
		}
	};
}

