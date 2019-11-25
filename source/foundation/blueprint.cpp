#include <flame/foundation/serialize.h>
#include <flame/foundation/blueprint.h>

namespace flame
{
	struct BPPrivate;
	struct NodePrivate;
	struct SlotPrivate;

	static BP::Environment* p_env;

	struct ModulePrivate : BP::Module
	{
		std::wstring filename;
		std::wstring absolute_filename;
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
		Type type;
		void* raw_data;
		VariableInfo* vi;

		std::vector<SlotPrivate*> links;

		SlotPrivate(Type type, NodePrivate* node, VariableInfo* variable_info);

		void set_frame(int frame);
		void set_data(const void* data);
		bool link_to(SlotPrivate* target);

		Mail<std::string> get_address() const;
	};

	struct NodePrivate : BP::Node
	{
		BPPrivate* scene;
		std::string id;
		UdtInfo* udt;
		bool initiative;

		void* object;

		void* dtor_addr;
		void* update_addr;

		std::vector<std::unique_ptr<SlotPrivate>> inputs;
		std::vector<std::unique_ptr<SlotPrivate>> outputs;

		uint order;
		bool in_pending_update;
		bool in_update_list;

		NodePrivate(BPPrivate* scene, const std::string& id, UdtInfo* udt, void* module);
		~NodePrivate();

		void update();
	};

	struct BPDestroyDependence : BP
	{
		std::vector<std::unique_ptr<ModulePrivate>> modules;
		std::unique_ptr<ModulePrivate> self_module;
	};

	struct BPPrivate : BPDestroyDependence
	{
		PackagePrivate* package;
		BPPrivate* root;

		std::vector<std::unique_ptr<PackagePrivate>> packages;
		std::vector<ModulePrivate*> package_modules;

		std::vector<std::unique_ptr<NodePrivate>> nodes;

		std::vector<SlotPrivate*> input_exports;
		std::vector<SlotPrivate*> output_exports;

		Environment env;

		std::vector<NodePrivate*> initiative_nodes;
		std::vector<NodePrivate*> pending_update_nodes;
		std::list<NodePrivate*> update_list;
		bool need_update_hierarchy;

		BPPrivate()
		{
			package = nullptr;
			root = this;
			env.time = 0.f;
			need_update_hierarchy = true;
		}

		Module* add_module(const std::wstring& filename);
		void remove_module(ModulePrivate* m);

		PackagePrivate* add_package(const std::wstring& filename, const std::string& id);
		void remove_package(PackagePrivate* i);
		void collect_package_modules();

		void collect_dbs();

		NodePrivate* add_node(const std::string& type, const std::string& type_name);
		void remove_node(NodePrivate* n);

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

	SlotPrivate::SlotPrivate(Type _type, NodePrivate* _node, VariableInfo* _variable_info) :
		node(_node)
	{
		type = _type;
		vi = _variable_info;
		raw_data = (char*)node->object + vi->offset();
		user_data = nullptr;

		if (type == Input)
			links.push_back(nullptr);
		else /* if (type == Output) */
			set_frame(-1);
	}

	void SlotPrivate::set_frame(int frame)
	{
		((AttributeBase*)raw_data)->frame = frame;
	}

	void SlotPrivate::set_data(const void* d)
	{
		set_frame(looper().frame);
		node->scene->root->add_to_pending_update(node);
		 
		if (vi->type().tag == TypeTagAttributeV)
		{
			switch (vi->type().hash)
			{
			case cH("std::basic_string(char)"):
				*(std::string*)((char*)raw_data + sizeof(AttributeBase)) = *(std::string*)d;
				return;
			case cH("std::basic_string(wchar_t)"):
				*(std::wstring*)((char*)raw_data + sizeof(AttributeBase)) = *(std::wstring*)d;
				return;
			}
		}
		memcpy(data(), d, vi->size() - sizeof(AttributeBase));
	}

	bool SlotPrivate::link_to(SlotPrivate* target)
	{
		assert(type == Input);

		if (target)
		{
			if (target->type == Input)
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
			auto in_type = vi->type();
			auto out_type = target->vi->type();

			if (![&]() {
				if (in_type.tag == out_type.tag && in_type.hash == out_type.hash)
					return true;
				if (in_type.tag == TypeTagAttributeP)
				{
					if (out_type.tag == TypeTagAttributeV || out_type.tag == TypeTagAttributeP)
					{
						if ((out_type.hash == in_type.hash || in_type.hash == cH("void")))
							return true;
						#define PREFIX "std::vector"
						if (in_type.name.compare(0, strlen(PREFIX), PREFIX) == 0)
						{
							((AttributeBase*)raw_data)->twist = 1;
							return true;
						}
						#undef PREFIX
					}
				}
				return false;
			}())
				return false;
		}
		else
			((AttributeBase*)raw_data)->twist = 0;

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

		set_frame(looper().frame);

		auto root = node->scene->root;
		root->need_update_hierarchy = true;
		root->add_to_pending_update(node);
		if (target)
			root->add_to_pending_update(target->node);

		return true;
	}

	Mail<std::string> SlotPrivate::get_address() const
	{
		auto ret = new_mail<std::string>();
		(*ret.p) = node->id + "." + vi->name();
		return ret;
	}

	NodePrivate::NodePrivate(BPPrivate* scene, const std::string& id, UdtInfo* udt, void* module) :
		scene(scene),
		id(id),
		udt(udt),
		initiative(false),
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
				if (f->return_type().tag == TypeTagVariable && f->return_type().hash == cH("void") && f->parameter_count() == 0)
					update_addr = (char*)module + (uint)f->rva();
				assert(update_addr);
			}
		}

		for (auto i = 0; i < udt->variable_count(); i++)
		{
			auto v = udt->variable(i);
			auto& deco = v->decoration();
			auto ai = deco.find('i') != std::string::npos, ao = deco.find('o') != std::string::npos;
			if (!ai && !ao)
				continue;
			assert(!(ai && ao));
			assert(v->type().tag == TypeTagAttributeES || v->type().tag == TypeTagAttributeEM || v->type().tag == TypeTagAttributeV || v->type().tag == TypeTagAttributeP);
			if (ai)
				inputs.emplace_back(new SlotPrivate(SlotPrivate::Input, this, v));
			else /* if (ao) */
				outputs.emplace_back(new SlotPrivate(SlotPrivate::Output, this, v));
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
				l->set_frame(looper().frame);
				root->add_to_pending_update(l->node);
			}
		}

		if (dtor_addr)
			cmf(p2f<MF_v_v>(dtor_addr), object);
		free(object);

		root->need_update_hierarchy = true;
		root->remove_from_pending_update(this);
	}

	void NodePrivate::update()
	{
		for (auto& input : inputs)
		{
			auto out = input->links[0];
			if (out)
			{
				auto iv = input->vi;
				auto ia = (AttributeBase*)input->raw_data;
				auto ot = out->vi->type().tag;
				if ((ia->twist == 1 && ot == TypeTagAttributeV) || (ot == TypeTagAttributeV && iv->type().tag == TypeTagAttributeP))
				{
					auto p = out->data();
					memcpy(input->data(), &p, sizeof(void*));
				}
				else
					memcpy(input->data(), out->data(), iv->size() - sizeof(AttributeBase));
				auto new_frame = ((AttributeBase*)out->raw_data)->frame;
				if (new_frame > ia->frame)
					ia->frame = new_frame;
			}
		}

		if (update_addr)
			cmf(p2f<MF_v_v>(update_addr), object);
	}

	BP::Module* BPPrivate::add_module(const std::wstring& filename)
	{
		if (filename == L"bp.dll")
			return nullptr;
		for (auto& m : modules)
		{
			if (m->filename == filename)
				return nullptr;
		}

		auto absolute_filename = filename;
		auto module = load_module(filename);
		if (!module)
		{
			std::filesystem::path path(env.filename);
			absolute_filename = path.parent_path().wstring() + L"/" + filename;
			module = load_module(absolute_filename);
		}
		if (!module)
		{
			printf("cannot add module %s\n", w2s(filename).c_str());
			return nullptr;
		}

		auto m = new ModulePrivate;
		m->filename = filename;
		m->absolute_filename = absolute_filename;
		m->module = module;
		std::vector<TypeinfoDatabase*> dbs;
		for (auto& m : modules)
			dbs.push_back(m->db);
		m->db = TypeinfoDatabase::load(dbs, std::filesystem::path(absolute_filename).replace_extension(L".typeinfo"));

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
					if ((*n_it)->udt->db() == (*it)->db)
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

	PackagePrivate* BPPrivate::add_package(const std::wstring& _filename, const std::string& id)
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

		auto bp = (BPPrivate*)BP::create_from_file((std::filesystem::path(env.filename).parent_path().parent_path() / _filename / L"bp").wstring(), true, root);
		if (!bp)
			return nullptr;

		auto p = new PackagePrivate(this, s_id, bp);
		p->filename = _filename;
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
		env.dbs.clear();
		for (auto& m : modules)
			env.dbs.push_back(m->db);
		if (self_module)
			env.dbs.push_back(self_module->db);
		for (auto m : package_modules)
			env.dbs.push_back(m->db);
	}

	NodePrivate* BPPrivate::add_node(const std::string& type, const std::string& id)
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

		std::string s_id;
		if (!id.empty())
		{
			s_id = id;
			if (find_node(s_id))
			{
				printf("cannot add node, id repeated\n");
				return nullptr;
			}
		}
		else
		{
			for (auto i = 0; i < nodes.size() + 1; i++)
			{
				s_id = "node_" + std::to_string(i);
				if (!find_node(s_id))
					break;
			}
		}

		auto n = new NodePrivate(this, s_id, udt, module);
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
			add_to_update_list(this, n);
		pending_update_nodes.clear();
		
		while (!update_list.empty())
		{
			auto n = update_list.front();
			update_list.erase(update_list.begin());
			n->in_update_list = false;

			p_env = &n->scene->env;
			p_env->time = env.time;
			n->update();

			auto frame = looper().frame;
			for (auto& o : n->outputs)
			{
				if (o->frame() == frame)
				{
					for (auto& i : o->links)
						add_to_update_list(this, i->node);
				}
			}
		}

		env.time += looper().delta_time;
	}

	const std::wstring& BP::Module::filename() const
	{
		return ((ModulePrivate*)this)->filename;
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

	const std::string& BP::Package::id() const
	{
		return ((PackagePrivate*)this)->id;
	}

	void BP::Package::set_id(const std::string& id)
	{
		((PackagePrivate*)this)->id = id;
	}

	BP::Slot::Type BP::Slot::type() const
	{
		return ((SlotPrivate*)this)->type;
	}

	BP::Node* BP::Slot::node() const
	{
		return ((SlotPrivate*)this)->node;
	}

	VariableInfo* BP::Slot::vi() const
	{
		return ((SlotPrivate*)this)->vi;
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

	Mail<std::string> BP::Slot::get_address() const
	{
		return ((SlotPrivate*)this)->get_address();
	}

	BP *BP::Node::scene() const
	{
		return ((NodePrivate*)this)->scene;
	}

	const std::string& BP::Node::id() const
	{
		return ((NodePrivate*)this)->id;
	}

	void BP::Node::set_id(const std::string& id)
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

	BP::Slot* BP::Node::find_input(const std::string& name) const
	{
		for (auto& input : ((NodePrivate*)this)->inputs)
		{
			if (name == input->vi->name())
				return input.get();
		}
		return nullptr;
	}

	BP::Slot* BP::Node::find_output(const std::string& name) const
	{
		for (auto& output : ((NodePrivate*)this)->outputs)
		{
			if (name == output->vi->name())
				return output.get();
		}
		return nullptr;
	}

	void BP::set_time(float t)
	{
		((BPPrivate*)this)->env.time = t;
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

	BP::Module* BP::add_module(const std::wstring& filename)
	{
		return ((BPPrivate*)this)->add_module(filename);
	}

	void BP::remove_module(BP::Module* m)
	{
		((BPPrivate*)this)->remove_module((ModulePrivate*)m);
	}

	BP::Module* BP::find_module(const std::wstring& filename) const
	{
		auto& modules = ((BPPrivate*)this)->modules;
		for (auto& m : modules)
		{
			if (m->filename == filename)
				return m.get();
		}
		return nullptr;
	}

	uint BP::package_count() const
	{
		return ((BPPrivate*)this)->packages.size();
	}

	BP::Package* BP::package(uint idx) const
	{
		return ((BPPrivate*)this)->packages[idx].get();
	}

	BP::Package* BP::add_package(const std::wstring& filename, const std::string& id)
	{
		return ((BPPrivate*)this)->add_package(filename, id);
	}

	void BP::remove_package(Package* e)
	{
		((BPPrivate*)this)->remove_package((PackagePrivate*)e);
	}

	BP::Package* BP::find_package(const std::string& id) const
	{
		auto& packages = ((BPPrivate*)this)->packages;
		for (auto& p : packages)
		{
			if (p->id == id)
				return p.get();
		}
		return nullptr;
	}

	const std::vector<TypeinfoDatabase*> BP::dbs() const
	{
		return ((BPPrivate*)this)->env.dbs;
	}

	uint BP::node_count() const
	{
		return ((BPPrivate*)this)->nodes.size();
	}

	BP::Node *BP::node(uint idx) const
	{
		return ((BPPrivate*)this)->nodes[idx].get();
	}

	BP::Node *BP::add_node(const std::string& type, const std::string& id)
	{
		return ((BPPrivate*)this)->add_node(type, id);
	}

	void BP::remove_node(BP::Node *n)
	{
		((BPPrivate*)this)->remove_node((NodePrivate*)n);
	}

	BP::Node *BP::find_node(const std::string& address) const
	{
		auto sp = string_split(address, '.');
		switch (sp.size())
		{
		case 2:
			if (sp[0] != "*")
			{
				auto p = (PackagePrivate*)find_package(sp[0]);
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

	BP::Slot* BP::find_input(const std::string& address) const
	{
		auto sp = string_last_first_split(address, '.');
		auto n = find_node(sp[0]);
		if (!n)
			return nullptr;
		return (SlotPrivate*)n->find_input(sp[1]);
	}

	BP::Slot* BP::find_output(const std::string& address) const
	{
		auto sp = string_last_first_split(address, '.');
		auto n = find_node(sp[0]);
		if (!n)
			return nullptr;
		return (SlotPrivate*)n->find_output(sp[1]);
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

	BP* BP::create_from_file(const std::wstring& filename, bool no_compile, BP* root)
	{
		auto s_filename = w2s(filename);
		auto path = std::filesystem::path(filename);
		auto ppath = path.parent_path();
		auto ppath_str = ppath.wstring();

		printf("begin to load bp: %s\n", s_filename.c_str());

		auto file = SerializableNode::create_from_xml_file(filename);
		if (!file)
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
						if (ext == L".pdb" && !is_file_occupied(it->path().wstring()))
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

		auto n_modules = file->find_node("modules");
		for (auto i_m = 0; i_m < n_modules->node_count(); i_m++)
		{
			ModuleDesc module;

			auto n_module = n_modules->node(i_m);
			module.filename = s2w(n_module->find_attr("filename")->value());
			module.pos = stof2(n_module->find_attr("pos")->value().c_str());
			module_descs.push_back(module);
		}

		struct PackageDesc
		{
			std::wstring filename;
			std::string id;
			Vec2f pos;
		};
		std::vector<PackageDesc> package_descs; 

		auto n_packages = file->find_node("packages");
		if (n_packages)
		{
			for (auto i_p = 0; i_p < n_packages->node_count(); i_p++)
			{
				PackageDesc package;

				auto n_package = n_packages->node(i_p);
				package.filename = s2w(n_package->find_attr("filename")->value());
				package.id = n_package->find_attr("id")->value();
				package.pos = stof2(n_package->find_attr("pos")->value().c_str());
				package_descs.push_back(package);
			}
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

		auto n_nodes = file->find_node("nodes");
		for (auto i_n = 0; i_n < n_nodes->node_count(); i_n++)
		{
			NodeDesc node;

			auto n_node = n_nodes->node(i_n);
			node.type = n_node->find_attr("type")->value();
			node.id = n_node->find_attr("id")->value();
			{
				auto a = n_node->find_attr("initiative");
				node.initiative = a ? a->value() == "1" : false;
			}
			node.pos = stof2(n_node->find_attr("pos")->value().c_str());

			auto n_datas = n_node->find_node("datas");
			if (n_datas)
			{
				for (auto i_d = 0; i_d < n_datas->node_count(); i_d++)
				{
					auto n_data = n_datas->node(i_d);
					DataDesc data;
					data.name = n_data->find_attr("name")->value();
					data.value = n_data->find_attr("value")->value();
					node.datas.push_back(data);
				}
			}

			node_descs.push_back(node);
		}

		struct LinkDesc
		{
			std::string out_addr;
			std::string in_addr;
		};
		std::vector<LinkDesc> link_descs;

		auto n_links = file->find_node("links");
		for (auto i_l = 0; i_l < n_links->node_count(); i_l++)
		{
			auto n_link = n_links->node(i_l);
			LinkDesc link;
			link.out_addr = n_link->find_attr("out")->value();
			link.in_addr = n_link->find_attr("in")->value();
			link_descs.push_back(link);
		}

		std::vector<std::string> input_export_descs;
		std::vector<std::string> output_export_descs;

		auto n_exports = file->find_node("exports");
		if (n_exports)
		{
			auto n_input_exports = n_exports->find_node("input");
			for (auto i_e = 0; i_e < n_input_exports->node_count(); i_e++)
				input_export_descs.push_back(n_input_exports->node(i_e)->find_attr("v")->value());
			auto n_output_exports = n_exports->find_node("output");
			for (auto i_e = 0; i_e < n_output_exports->node_count(); i_e++)
				output_export_descs.push_back(n_output_exports->node(i_e)->find_attr("v")->value());
		}

		SerializableNode::destroy(file);

		auto bp = new BPPrivate();
		if (root)
			bp->root = (BPPrivate*)root;
		bp->env.filename = filename;

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
			auto templatecpp_path = ppath / L"template.cpp";
			if (!std::filesystem::exists(templatecpp_path) || std::filesystem::last_write_time(templatecpp_path) < std::filesystem::last_write_time(filename))
			{
				printf("generating template.cpp");

				std::ofstream templatecpp(templatecpp_path);
				templatecpp << "// THIS FILE IS AUTO GENERATED\n";
				templatecpp << "#include <flame/foundation/bp_node_template.h>\n";
				templatecpp << "using namespace flame;\n";
				templatecpp << "extern \"C\" __declspec(dllexport) void add_templates(TypeinfoDatabase* db)\n";
				templatecpp << "{\n";
				templatecpp << "\tauto module = get_module_from_address(f2v(add_templates));\n";
				std::vector<std::string> all_templates;
				for (auto& n : node_descs)
				{
					auto pos_t = n.type.find('(');
					if (pos_t != std::string::npos)
					{
						auto found = false;
						for (auto& t : all_templates)
						{
							if (t == n.type)
							{
								found = true;
								break;
							}
						}
						if (found)
							continue;
						all_templates.push_back(n.type);

						templatecpp << "\tBP_";
						if (find_enum(bp->env.dbs, H(std::string(n.type.begin() + pos_t + 1, n.type.end() - 1).c_str())))
							templatecpp << std::string(n.type.begin(), n.type.begin() + pos_t) + "<int>";
						else
							templatecpp << tn_a2c(n.type);
						templatecpp << "::add_udt_info(db, \"";
						templatecpp << std::string(n.type.begin() + pos_t, n.type.end());
						templatecpp << "\", module);\n";
					}
				}
				templatecpp << "}\n";
				templatecpp.close();

				printf(" - done\n");
			}
			else
				printf("template.cpp up to data\n");

			auto cmakelists_path = ppath / L"CMakeLists.txt";
			printf("generating cmakelists");

			std::ofstream cmakelists(cmakelists_path);
			cmakelists << "# THIS FILE IS AUTO GENERATED\n";
			cmakelists << "cmake_minimum_required(VERSION 3.4)\n";
			cmakelists << "project(bp)\n";
			cmakelists << "add_definitions(-W0 -std:c++latest)\n";
			cmakelists << "file(GLOB SOURCE_LIST \"*.c*\")\n";
			cmakelists << "add_library(bp SHARED ${SOURCE_LIST})\n";
			for (auto& m : bp->modules)
			{
				cmakelists << "target_link_libraries(bp ${CMAKE_SOURCE_DIR}/../../bin/";
				cmakelists << std::filesystem::path(m->absolute_filename).replace_extension(L".lib").string();
				cmakelists << ")\n";
			}
			cmakelists << "target_include_directories(bp PRIVATE ${CMAKE_SOURCE_DIR}/../../include)\n";
			srand(::time(0));
			auto pdb_filename = std::to_string(::rand() % 100000);
			cmakelists << "set_target_properties(bp PROPERTIES PDB_NAME " + pdb_filename + ")\n";
			cmakelists << "add_custom_command(TARGET bp POST_BUILD COMMAND ${CMAKE_SOURCE_DIR}/../../bin/typeinfogen ${CMAKE_SOURCE_DIR}/build/debug/bp.dll ";
			for (auto& m : bp->modules)
				cmakelists << "-m${CMAKE_SOURCE_DIR}/../../bin/" + w2s(m->absolute_filename) + " ";
			cmakelists << "-p${CMAKE_SOURCE_DIR}/build/debug/" + pdb_filename + ".pdb)\n";
			cmakelists.close();

			printf(" - done\n");

			printf("cmaking:\n");
			exec_and_redirect_to_std_output(L"", L"cmake -S " + ppath_str + L" -B " + ppath_str + L"/build");

			printf("compiling:\n");
			auto curr_path = get_curr_path();
			exec_and_redirect_to_std_output(L"", s2w(VS_LOCATION) + L"/Common7/IDE/devenv.com \"" + *curr_path.p + L"/" + ppath_str + L"/build/bp.sln\" /build debug");
			delete_mail(curr_path);
		}

		auto self_module_filename = ppath_str + L"/build/debug/bp.dll";
		auto self_module = load_module(self_module_filename);
		if (self_module)
		{
			auto m = new ModulePrivate;
			m->filename = self_module_filename;
			m->absolute_filename = self_module_filename;
			m->module = self_module;
			m->db = TypeinfoDatabase::load(bp->env.dbs, std::filesystem::path(self_module_filename).replace_extension(L".typeinfo"));
			bp->self_module.reset(m);

			bp->collect_dbs();
		}

		for (auto& n_d : node_descs)
		{
			auto n = bp->add_node(n_d.type, n_d.id);
			if (n)
			{
				n->set_initiative(n_d.initiative);
				n->pos = n_d.pos;
				for (auto& d_d : n_d.datas)
				{
					auto input = n->find_input(d_d.name);
					auto v = input->vi();
					if (v->default_value())
						unserialize_value(bp->env.dbs, v->type().tag, v->type().hash, d_d.value, input->raw_data());
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

	void BP::save_to_file(BP* _bp, const std::wstring& filename)
	{
		auto bp = (BPPrivate*)_bp;

		bp->env.filename = filename;

		auto file = SerializableNode::create("BP");

		auto n_modules = file->new_node("modules");
		for (auto& m : bp->modules)
		{
			if (m->external)
				continue;
			auto n_module = n_modules->new_node("module");
			n_module->new_attr("filename", w2s(m->filename));
			n_module->new_attr("pos", to_string(m->pos, 2));
		}

		if (!bp->packages.empty())
		{
			auto n_packages = file->new_node("packages");
			for (auto& p : bp->packages)
			{
				if (p->external)
					continue;

				auto n_import = n_packages->new_node("package");
				n_import->new_attr("filename", w2s(p->filename));
				n_import->new_attr("id", p->id);
				n_import->new_attr("pos", to_string(p->pos, 2));
			}
		}

		std::vector<Module*> skipped_modules;
		for (auto& m : bp->modules)
		{
			if (m->external)
				skipped_modules.push_back(m.get());
		}
		auto n_nodes = file->new_node("nodes");
		for (auto& n : bp->nodes)
		{
			if (n->external)
				continue;
			auto udt = n->udt;
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

			auto n_node = n_nodes->new_node("node");
			n_node->new_attr("type", udt->name());
			n_node->new_attr("id", n->id);
			if (n->initiative)
				n_node->new_attr("initiative", "1");
			n_node->new_attr("pos", to_string(n->pos, 2));

			SerializableNode* n_datas = nullptr;
			for (auto& input : n->inputs)
			{
				if (input->links[0])
					continue;
				auto v = input->vi;
				if (v->default_value() && memcmp(input->data(), (char*)v->default_value() + sizeof(AttributeBase), v->size() - sizeof(AttributeBase)) != 0)
				{
					if (!n_datas)
						n_datas = n_node->new_node("datas");
					auto n_data = n_datas->new_node("data");
					n_data->new_attr("name", v->name());
					auto value = serialize_value(bp->env.dbs, v->type().tag, v->type().hash, input->raw_data, 2);
					n_data->new_attr("value", *value.p);
					delete_mail(value);
				}
			}
		}

		auto n_links = file->new_node("links");
		for (auto& p : bp->packages)
		{
			for (auto& in : p->bp->input_exports)
			{
				auto out = in->links[0];
				if (out)
				{
					auto n_link = n_links->new_node("link");
					auto out_addr = out->get_address();
					auto in_addr = in->get_address();
					auto out_bp = out->node->scene;
					n_link->new_attr("out", (out_bp != bp ? out_bp->package->id + "." : "") + *out_addr.p);
					n_link->new_attr("in", p->id + "." + *in_addr.p);
					delete_mail(out_addr);
					delete_mail(in_addr);
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
					auto n_link = n_links->new_node("link");
					auto out_addr = out->get_address();
					auto in_addr = in->get_address();
					auto out_bp = out->node->scene;
					n_link->new_attr("out", (out_bp != bp ? out_bp->package->id + "." : "") + *out_addr.p);
					n_link->new_attr("in", *in_addr.p);
					delete_mail(out_addr);
					delete_mail(in_addr);
				}
			}
		}

		if (!bp->input_exports.empty() || !bp->output_exports.empty())
		{
			auto n_exports = file->new_node("exports");
			{
				auto n_input_exports = n_exports->new_node("input");
				for (auto& e : bp->input_exports)
				{
					auto addr = e->get_address();
					auto n_export = n_input_exports->new_node("export")->new_attr("v", *addr.p);
					delete_mail(addr);
				}
				auto n_output_exports = n_exports->new_node("output");
				for (auto& e : bp->output_exports)
				{
					auto addr = e->get_address();
					auto n_export = n_output_exports->new_node("export")->new_attr("v", *addr.p);
					delete_mail(addr);
				}
			}
		}

		SerializableNode::save_to_xml_file(file, filename);
		SerializableNode::destroy(file);
	}

	void BP::destroy(BP *bp)
	{
		delete(BPPrivate*)bp;
	}

	const BP::Environment& bp_env()
	{
		return *p_env;
	}

	struct F2U$
	{
		AttributeV<float> v$i;

		AttributeV<uint> out$o;

		FLAME_FOUNDATION_EXPORTS void update$()
		{
			if (v$i.frame > out$o.frame)
			{
				out$o.v = v$i.v;
				out$o.frame = looper().frame;
			}
		}
	};

	struct Add$
	{
		AttributeV<float> a$i;
		AttributeV<float> b$i;

		AttributeV<float> out$o;

		FLAME_FOUNDATION_EXPORTS void update$()
		{
			if (a$i.frame > out$o.frame || b$i.frame > out$o.frame)
			{
				out$o.v = a$i.v + b$i.v;
				out$o.frame = looper().frame;
			}
		}
	};

	struct Multiple$
	{
		AttributeV<float> a$i;
		AttributeV<float> b$i;

		AttributeV<float> out$o;

		FLAME_FOUNDATION_EXPORTS void update$()
		{
			if (a$i.frame > out$o.frame || b$i.frame > out$o.frame)
			{
				out$o.v = a$i.v * b$i.v;
				out$o.frame = looper().frame;
			}
		}
	};

	struct MakeVec2f$
	{
		AttributeV<float> x$i;
		AttributeV<float> y$i;

		AttributeV<Vec2f> out$o;

		FLAME_FOUNDATION_EXPORTS void update$()
		{
			if (x$i.frame > out$o.frame)
				out$o.v.x() = x$i.v;
			if (y$i.frame > out$o.frame)
				out$o.v.y() = y$i.v;
			out$o.frame = looper().frame;
		}
	};

	struct Time$
	{
		AttributeV<float> delta$o;
		AttributeV<float> total$o;

		FLAME_FOUNDATION_EXPORTS void update$()
		{
			delta$o.v = looper().delta_time;
			total$o.v = bp_env().time;
			auto frame = looper().frame;
			delta$o.frame = frame;
			total$o.frame = frame;
		}
	};

	struct LinearInterpolation1d$
	{
		AttributeV<float> a$i;
		AttributeV<float> b$i;
		AttributeV<float> t$i;

		AttributeV<float> out$o;

		FLAME_FOUNDATION_EXPORTS void update$()
		{
			if (a$i.frame > out$o.frame || b$i.frame > out$o.frame || t$i.frame > out$o.frame)
			{
				if (t$i.v <= 0.f)
					out$o.v = a$i.v;
				else if (t$i.v >= 1.f)
					out$o.v = b$i.v;
				else
					out$o.v = a$i.v + (b$i.v - a$i.v) * t$i.v;
				out$o.frame = looper().frame;
			}
		}
	};

	struct LinearInterpolation2d$
	{
		AttributeV<Vec2f> a$i;
		AttributeV<Vec2f> b$i;
		AttributeV<float> t$i;

		AttributeV<Vec2f> out$o;

		FLAME_FOUNDATION_EXPORTS void update$()
		{
			if (a$i.frame > out$o.frame || b$i.frame > out$o.frame || t$i.frame > out$o.frame)
			{
				if (t$i.v <= 0.f)
					out$o.v = a$i.v;
				else if (t$i.v >= 1.f)
					out$o.v = b$i.v;
				else
					out$o.v = a$i.v + (b$i.v - a$i.v) * t$i.v;
				out$o.frame = looper().frame;
			}
		}
	};
}

