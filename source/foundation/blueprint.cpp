#include <flame/foundation/serialize.h>
#include <flame/foundation/blueprint.h>

namespace flame
{
	struct BPPrivate;
	struct NodePrivate;
	struct SlotPrivate;

	static BP::Environment _bp_env;

	struct ModulePrivate : BP::Module
	{
		std::wstring filename;
		std::wstring absolute_filename;
		void* module;
		TypeinfoDatabase* db;

		ModulePrivate();
		~ModulePrivate();
	};

	struct ImportPrivate : BP::Import
	{
		std::wstring filename;
		BP* bp;
		std::string id;

		ImportPrivate();
		~ImportPrivate();
	};

	struct SlotPrivate : BP::Slot
	{
		Type type;
		NodePrivate* node;
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
		BPPrivate* bp;
		std::string id;
		UdtInfo* udt;

		void* dummy; // represents the object

		void* dtor_addr;
		void* update_addr;

		std::vector<std::unique_ptr<SlotPrivate>> inputs;
		std::vector<std::unique_ptr<SlotPrivate>> outputs;

		bool in_list;

		NodePrivate(BPPrivate* _bp, const std::string& _id, UdtInfo* _udt, void* module);
		~NodePrivate();

		SlotPrivate* find_input(const std::string& name) const;
		SlotPrivate* find_output(const std::string& name) const;

		void add_to_update_list();

		void update();
	};

	struct ExportPrivate : BP::Export
	{
		SlotPrivate* slot;
		std::string alias;
	};

	struct BPModules : BP
	{
		std::vector<std::unique_ptr<ModulePrivate>> modules;
		std::unique_ptr<ModulePrivate> self_module;
	};

	struct BPPrivate : BPModules
	{
		std::wstring filename;

		std::vector<std::unique_ptr<NodePrivate>> nodes;

		std::vector<std::unique_ptr<ImportPrivate>> impts;
		std::vector<std::unique_ptr<ExportPrivate>> expts;

		std::vector<NodePrivate*> update_list;

		BPPrivate();

		Module* add_module(const std::wstring& filename);
		void remove_module(Module* m);

		ImportPrivate* add_impt(const std::wstring& filename, const std::string& id);
		void remove_impt(ImportPrivate* i);
		ImportPrivate* find_impt(const std::string& id) const;

		NodePrivate* add_node(uint type_hash, const std::string& type_name);
		void remove_node(NodePrivate* n);
		NodePrivate* find_node(const std::string& id) const;
		SlotPrivate* find_input(const std::string& address) const;
		SlotPrivate* find_output(const std::string& address) const;

		ExportPrivate* add_expt(SlotPrivate* output, const std::string& alias);
		void remove_expt(ExportPrivate* e);
		ExportPrivate* find_expt(const std::string& alias) const;

		void clear();

		void build_update_list();

		void update();
	};

	ModulePrivate::ModulePrivate()
	{
		pos = Vec2f(0.f);
		placed = false;
	}

	ModulePrivate::~ModulePrivate()
	{
		free_module(module);
		TypeinfoDatabase::destroy(db);
	}

	ImportPrivate::ImportPrivate()
	{
		pos = Vec2f(0.f);
	}

	ImportPrivate::~ImportPrivate()
	{
		BP::destroy(bp);
	}

	SlotPrivate::SlotPrivate(Type _type, NodePrivate* _node, VariableInfo* _variable_info) :
		node(_node)
	{
		type = _type;
		vi = _variable_info;
		raw_data = (char*)node->dummy + vi->offset();

		if (type == Input)
			links.push_back(nullptr);
		else /* if (type == Output) */
			set_frame(-1);
	}

	void SlotPrivate::set_frame(int frame)
	{
		auto& p = *(AttributeBase*)raw_data;
		p.frame = frame;
	}

	void SlotPrivate::set_data(const void* d)
	{
		set_frame(looper().frame);
		auto type = vi->type();
		if (type->tag() == TypeTagAttributeV)
		{
			switch (type->hash())
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
		if (type == Output)
		{
			assert(0);
			return false;
		}

		if (target && target->type == Input)
			return false;

		if (links[0] == target)
			return true;

		if (target)
		{
			auto in_type = vi->type();
			auto out_type = target->vi->type();
			auto in_tag = in_type->tag();
			auto out_tag = out_type->tag();
			auto in_hash = in_type->hash();
			auto out_hash = out_type->hash();

			if (![&]() {
				if (in_tag == out_tag && in_hash == out_hash)
					return true;
				if (in_tag == TypeTagAttributeP)
				{
					if (out_tag == TypeTagAttributeV || out_tag == TypeTagAttributeP)
					{
						if ((out_hash == in_hash || in_hash == cH("void")))
							return true;
						#define PREFIX "std::vector"
						if (in_type->name().compare(0, strlen(PREFIX), PREFIX) == 0)
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

		node->bp->build_update_list();

		return true;
	}

	Mail<std::string> SlotPrivate::get_address() const
	{
		auto ret = new_mail<std::string>();
		(*ret.p) = node->id + "." + vi->name();
		return ret;
	}

	NodePrivate::NodePrivate(BPPrivate* _bp, const std::string& _id, UdtInfo* _udt, void* module) :
		bp(_bp),
		id(_id),
		in_list(false)
	{
		udt = _udt;
		pos = Vec2f(0.f);

		auto size = udt->size();
		dummy = malloc(size);
		memset(dummy, 0, size);

		{
			auto f = udt->find_function("ctor");
			if (f && f->parameter_count() == 0)
				cmf(p2f<MF_v_v>((char*)module + (uint)f->rva()), dummy);
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
			assert(f);
			if (f)
			{
				auto ret_t = f->return_type();
				if (ret_t->tag() == TypeTagVariable && ret_t->hash() == cH("void") && f->parameter_count() == 0)
					update_addr = (char*)module + (uint)f->rva();
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
			auto tag = v->type()->tag();
			assert(tag == TypeTagAttributeES || tag == TypeTagAttributeEM || tag == TypeTagAttributeV || tag == TypeTagAttributeP);
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
		for (auto& o : outputs)
		{
			for (auto& l : o->links)
			{
				l->set_frame(looper().frame);
				l->links[0] = nullptr;
			}
		}

		if (dtor_addr)
			cmf(p2f<MF_v_v>(dtor_addr), dummy);
		free(dummy);
	}

	SlotPrivate* NodePrivate::find_input(const std::string& name) const
	{
		for (auto& input : inputs)
		{
			if (name == input->vi->name())
				return input.get();
		}
		return nullptr;
	}

	SlotPrivate* NodePrivate::find_output(const std::string& name) const
	{
		for (auto& output : outputs)
		{
			if (name == output->vi->name())
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

	void NodePrivate::update()
	{
		for (auto& input : inputs)
		{
			auto out = input->links[0];
			if (out)
			{
				auto iv = input->vi;
				auto ia = (AttributeBase*)input->raw_data;
				auto ot = out->vi->type()->tag();
				if ((ia->twist == 1 && ot == TypeTagAttributeV) || (ot == TypeTagAttributeV && iv->type()->tag() == TypeTagAttributeP))
				{
					auto p = out->data();
					memcpy(input->data(), &p, sizeof(void*));
				}
				else
					memcpy(input->data(), out->data(), iv->size() - sizeof(AttributeBase));
				ia->frame = ((AttributeBase*)out->raw_data)->frame;
			}
		}

		cmf(p2f<MF_v_v>(update_addr), dummy);
	}

	BPPrivate::BPPrivate()
	{
		graphics_device = nullptr;

		expts_node_pos = Vec2f(0.f);
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
			std::filesystem::path path(this->filename);
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
		m->db = TypeinfoDatabase::load(dbs, ext_replace(absolute_filename, L".typeinfo"));

		modules.emplace_back(m);

		return m;
	}

	void BPPrivate::remove_module(Module* m)
	{
		if (filename == L"bp.dll")
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

				build_update_list();
				return;
			}
		}
	}

	ImportPrivate* BPPrivate::add_impt(const std::wstring& _filename, const std::string& id)
	{
		std::string s_id;
		if (!id.empty())
		{
			s_id = id;
			if (find_impt(s_id))
				return nullptr;
		}
		else
		{
			for (auto i = 0; i < impts.size() + 1; i++)
			{
				s_id = "import_" + std::to_string(i);
				if (!find_impt(s_id))
					break;
			}
		}

		auto bp = BP::create_from_file((std::filesystem::path(filename).parent_path().parent_path() / _filename / L"bp").wstring(), true);
		if (!bp)
			return nullptr;

		auto i = new ImportPrivate;
		i->filename = _filename;
		i->bp = bp;
		i->id = s_id;
		impts.emplace_back(i);

		return i;
	}

	void BPPrivate::remove_impt(ImportPrivate* i)
	{
		for (auto it = impts.begin(); it != impts.end(); it++)
		{
			if (it->get() == i)
			{
				impts.erase(it);
				return;
			}
		}
	}

	ImportPrivate* BPPrivate::find_impt(const std::string& id) const
	{
		for (auto& i : impts)
		{
			if (i->id == id)
				return i.get();
		}
		return nullptr;
	}

	NodePrivate* BPPrivate::add_node(uint type_hash, const std::string& id)
	{
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
		}

		if (!udt)
			return nullptr;

		std::string s_id;
		if (!id.empty())
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
				if (!find_node(s_id))
					break;
			}
		}

		auto n = new NodePrivate(this, s_id, udt, module);
		nodes.emplace_back(n);

		build_update_list();

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

		build_update_list();
	}

	NodePrivate* BPPrivate::find_node(const std::string& id) const
	{
		for (auto& n : nodes)
		{
			if (n->id == id)
				return n.get();
		}
		return nullptr;
	}

	SlotPrivate* BPPrivate::find_input(const std::string& address) const
	{
		auto sp = string_split(address, '.');
		auto n = find_node(sp[0]);
		if (!n)
			return nullptr;
		return (SlotPrivate*)n->find_input(sp[1]);
	}

	SlotPrivate* BPPrivate::find_output(const std::string& address) const
	{
		auto sp = string_split(address, '.');
		auto n = find_node(sp[0]);
		if (!n)
			return nullptr;
		return (SlotPrivate*)n->find_output(sp[1]);
	}

	ExportPrivate* BPPrivate::add_expt(SlotPrivate* s, const std::string& alias)
	{
		if (alias.empty())
			return nullptr;
		for (auto& e : expts)
		{
			if (e->slot == s || e->alias == alias)
				return nullptr;
		}

		auto e = new ExportPrivate;
		e->slot = s;
		e->alias = alias;
		if (s->type == Slot::Input)
		{
			std::vector<std::unique_ptr<ExportPrivate>>::iterator it;
			for (it = expts.begin(); it != expts.end(); it++)
			{
				if ((*it)->slot->type == Slot::Output)
					break;
			}
			expts.emplace(it, e);
		}
		else
			expts.emplace_back(e);

		return e;
	}

	void BPPrivate::remove_expt(ExportPrivate* e)
	{
		for (auto it = expts.begin(); it != expts.end(); it++)
		{
			if (it->get() == e)
			{
				expts.erase(it);
				return;
			}
		}
	}

	ExportPrivate* BPPrivate::find_expt(const std::string& alias) const
	{
		for (auto& e : expts)
		{
			if (e->alias == alias)
				return e.get();
		}
		return nullptr;
	}

	void BPPrivate::clear()
	{
		nodes.clear();
		update_list.clear();
	}

	void BPPrivate::build_update_list()
	{
		update_list.clear();
		for (auto& n : nodes)
			n->in_list = false;
		for (auto& n : nodes)
			n->add_to_update_list();
	}

	void BPPrivate::update()
	{
		if (update_list.empty())
			return;

		_bp_env.path = std::filesystem::path(filename).parent_path().wstring();
		_bp_env.graphics_device = graphics_device;
		_bp_env.dbs.clear();
		for (auto& m : modules)
			_bp_env.dbs.push_back(m->db);
		if (self_module)
			_bp_env.dbs.push_back(self_module->db);

		for (auto& n : update_list)
			n->update();

		_bp_env.path = L"";
		_bp_env.graphics_device = nullptr;
		_bp_env.dbs.clear();
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

	BP* BP::Import::bp() const
	{
		return ((ImportPrivate*)this)->bp;
	}

	const std::string& BP::Import::id() const
	{
		return ((ImportPrivate*)this)->id;
	}

	void BP::Import::set_id(const std::string& id)
	{
		((ImportPrivate*)this)->id = id;
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
		return *(int*)((SlotPrivate*)this)->raw_data;
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

	BP *BP::Node::bp() const
	{
		return ((NodePrivate*)this)->bp;
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

	BP::Slot*BP::Node::output(uint idx) const
	{
		return ((NodePrivate*)this)->outputs[idx].get();
	}

	BP::Slot*BP::Node::find_input(const std::string& name) const
	{
		return ((NodePrivate*)this)->find_input(name);
	}

	BP::Slot*BP::Node::find_output(const std::string& name) const
	{
		return ((NodePrivate*)this)->find_output(name);
	}

	BP::Slot* BP::Export::slot() const
	{
		return ((ExportPrivate*)this)->slot;
	}

	const std::string& BP::Export::alias() const
	{
		return ((ExportPrivate*)this)->alias;
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
		((BPPrivate*)this)->remove_module(m);
	}

	uint BP::impt_count() const
	{
		return ((BPPrivate*)this)->impts.size();
	}

	BP::Import* BP::impt(uint idx) const
	{
		return ((BPPrivate*)this)->impts[idx].get();
	}

	BP::Import* BP::add_impt(const std::wstring& filename, const std::string& id)
	{
		return ((BPPrivate*)this)->add_impt(filename, id);
	}

	void BP::remove_impt(Import* e)
	{
		((BPPrivate*)this)->remove_impt((ImportPrivate*)e);
	}

	BP::Import* BP::find_impt(const std::string& id) const
	{
		return ((BPPrivate*)this)->find_impt(id);
	}

	uint BP::node_count() const
	{
		return ((BPPrivate*)this)->nodes.size();
	}

	BP::Node *BP::node(uint idx) const
	{
		return ((BPPrivate*)this)->nodes[idx].get();
	}

	BP::Node *BP::add_node(uint type_hash, const std::string& id)
	{
		return ((BPPrivate*)this)->add_node(type_hash, id);
	}

	void BP::remove_node(BP::Node *n)
	{
		((BPPrivate*)this)->remove_node((NodePrivate*)n);
	}

	BP::Node *BP::find_node(const std::string& id) const
	{
		return ((BPPrivate*)this)->find_node(id);
	}

	BP::Slot*BP::find_input(const std::string& address) const
	{
		return ((BPPrivate*)this)->find_input(address);
	}

	BP::Slot*BP::find_output(const std::string& address) const
	{
		return ((BPPrivate*)this)->find_output(address);
	}

	uint BP::expt_count() const
	{
		return ((BPPrivate*)this)->expts.size();
	}

	BP::Export* BP::expt(uint idx) const
	{
		return ((BPPrivate*)this)->expts[idx].get();
	}

	BP::Export* BP::add_expt(Slot* s, const std::string& alias)
	{
		return ((BPPrivate*)this)->add_expt((SlotPrivate*)s, alias);
	}

	void BP::remove_expt(Export* e)
	{
		((BPPrivate*)this)->remove_expt((ExportPrivate*)e);
	}

	BP::Export* BP::find_expt(const std::string& alias) const
	{
		return ((BPPrivate*)this)->find_expt(alias);
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

	static std::vector<std::filesystem::path> loaded_bps; // track locked pdbs

	BP *BP::create_from_file(const std::wstring& filename, bool no_compile)
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

		auto loaded_before = false;
		for (auto& l : loaded_bps)
		{
			if (l == path)
			{
				loaded_before = true;
				break;
			}
		}
		loaded_bps.push_back(path);
		if (!no_compile && !loaded_before) // delete pervious created random pdbs
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
						if (ext == L".pdb")
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

		struct ImportDesc
		{
			std::wstring filename;
			std::string id;
			Vec2f pos;
		};
		std::vector<ImportDesc> import_descs; 

		auto n_imports = file->find_node("imports");
		if (n_imports)
		{
			for (auto i_i = 0; i_i < n_imports->node_count(); i_i++)
			{
				ImportDesc import;

				auto n_import = n_imports->node(i_i);
				import.filename = s2w(n_import->find_attr("filename")->value());
				import.id = n_import->find_attr("id")->value();
				import.pos = stof2(n_import->find_attr("pos")->value().c_str());
				import_descs.push_back(import);
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

		struct ExportDesc
		{
			bool is_in;
			std::string out_addr;
			std::string alias;
		};
		std::vector<ExportDesc> export_descs;

		auto n_exports = file->find_node("exports");
		auto expts_node_pos = stof2(n_exports->find_attr("pos")->value().c_str());
		for (auto i_e = 0; i_e < n_exports->node_count(); i_e++)
		{
			auto n_export = n_exports->node(i_e);
			ExportDesc expt;
			expt.is_in = n_export->find_attr("type")->value() == "in";
			expt.out_addr = n_export->find_attr("slot")->value();
			expt.alias = n_export->find_attr("alias")->value();
			export_descs.push_back(expt);
		}

		SerializableNode::destroy(file);

		auto bp = new BPPrivate();
		bp->filename = filename;

		for (auto& m_d : module_descs)
		{
			auto m = bp->add_module(m_d.filename);
			if (m)
				m->pos = m_d.pos;
		}

		for (auto& i_d : import_descs)
		{
			auto i = bp->add_impt(i_d.filename, i_d.id);
			if (i)
				i->pos = i_d.pos;
		}

		std::vector<TypeinfoDatabase*> dbs;
		for (auto& m : bp->modules)
			dbs.push_back(m->db);

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
						if (find_enum(dbs, H(std::string(n.type.begin() + pos_t + 1, n.type.end() - 1).c_str())))
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
				cmakelists << w2s(ext_replace(m->absolute_filename, L".lib"));
				cmakelists << ")\n";
			}
			cmakelists << "target_include_directories(bp PRIVATE ${CMAKE_SOURCE_DIR}/../../include)\n";
			srand(time(0));
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
			{
#define OUTPUT_FILE L"compile_log.txt"

				auto curr_path = get_curr_path();
				exec(s2w(VS_LOCATION) + L"/Common7/IDE/devenv.exe", L"\"" + *curr_path.p + L"/" + ppath_str + L"/build/bp.sln\" /build debug /out " OUTPUT_FILE, true);
				delete_mail(curr_path);

				auto content = get_file_string(OUTPUT_FILE);
				printf("%s\n", content.c_str());

				if (!content.empty())
					std::filesystem::remove(OUTPUT_FILE);

#undef OUTPUT_FILE
			}
		}

		auto self_module_filename = ppath_str + L"/build/debug/bp.dll";
		auto self_module = load_module(self_module_filename);
		if (self_module)
		{
			auto m = new ModulePrivate;
			m->filename = self_module_filename;
			m->absolute_filename = self_module_filename;
			m->module = self_module;
			m->db = TypeinfoDatabase::load(dbs, ext_replace(self_module_filename, L".typeinfo"));
			bp->self_module.reset(m);
		}

		for (auto& n_d : node_descs)
		{
			auto n = bp->add_node(H(n_d.type.c_str()), n_d.id);
			if (n)
			{
				n->pos = n_d.pos;
				for (auto& d_d : n_d.datas)
				{
					auto input = n->find_input(d_d.name);
					auto v = input->vi;
					auto type = v->type();
					if (v->default_value())
						unserialize_value(dbs, type->tag(), type->hash(), d_d.value, input->raw_data);
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

		bp->expts_node_pos = expts_node_pos;
		for(auto& e_d : export_descs)
		{
			SlotPrivate* slot = nullptr;
			if (e_d.is_in)
				slot = bp->find_input(e_d.out_addr);
			else
				slot = bp->find_output(e_d.out_addr);
			if (slot)
				bp->add_expt(slot, e_d.alias);
			else
				printf("cannot find slot: %s\n", e_d.out_addr.c_str());
		}

		printf("end loading bp: %s\n", s_filename.c_str());

		return bp;
	}

	void BP::save_to_file(BP* _bp, const std::wstring& filename)
	{
		auto bp = (BPPrivate*)_bp;

		bp->filename = filename;

		auto file = SerializableNode::create("BP");

		auto n_modules = file->new_node("modules");
		for (auto& m : bp->modules)
		{
			if (m->placed)
				continue;
			auto n_module = n_modules->new_node("module");
			n_module->new_attr("filename", w2s(m->filename));
			n_module->new_attr("pos", to_string(m->pos, 2));
		}

		if (!bp->impts.empty())
		{
			auto n_imports = file->new_node("imports");
			for (auto& i : bp->impts)
			{
				auto n_import = n_imports->new_node("import");
				n_import->new_attr("filename", w2s(i->filename));
				n_import->new_attr("id", i->id);
				n_import->new_attr("pos", to_string(i->pos, 2));
			}
		}

		std::vector< TypeinfoDatabase*> dbs;
		for (auto& m : bp->modules)
			dbs.push_back(m->db);
		if (bp->self_module)
			dbs.push_back(bp->self_module->db);

		std::vector<Module*> skipped_modules;
		for (auto& m : bp->modules)
		{
			if (m->placed)
				skipped_modules.push_back(m.get());
		}
		auto n_nodes = file->new_node("nodes");
		for (auto& n : bp->nodes)
		{
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
			n_node->new_attr("pos", to_string(n->pos, 2));

			SerializableNode* n_datas = nullptr;
			for (auto& input : n->inputs)
			{
				if (input->links[0])
					continue;
				auto v = input->vi;
				auto type = v->type();
				if (v->default_value() && memcmp(input->data(), (char*)v->default_value() + sizeof(AttributeBase), v->size() - sizeof(AttributeBase)) != 0)
				{
					if (!n_datas)
						n_datas = n_node->new_node("datas");
					auto n_data = n_datas->new_node("data");
					n_data->new_attr("name", v->name());
					auto value = serialize_value(dbs, type->tag(), type->hash(), input->raw_data, 2);
					n_data->new_attr("value", *value.p);
					delete_mail(value);
				}
			}
		}

		auto n_links = file->new_node("links");
		for (auto& n : bp->nodes)
		{
			for (auto& input : n->inputs)
			{
				if (input->links[0])
				{
					auto n_link = n_links->new_node("link");
					auto out_addr = input->links[0]->get_address();
					auto in_addr = input->get_address();
					n_link->new_attr("out", *out_addr.p);
					n_link->new_attr("in", *in_addr.p);
					delete_mail(out_addr);
					delete_mail(in_addr);
				}
			}
		}

		auto n_exports = file->new_node("exports");
		n_exports->new_attr("pos", to_string(bp->expts_node_pos, 2));
		{
			for (auto& e : bp->expts)
			{
				auto s = e->slot;

				auto n_export = n_exports->new_node("export");
				n_export->new_attr("type", s->type == Slot::Input ? "in" : "out");
				auto out_addr = s->get_address();
				n_export->new_attr("slot", *out_addr.p);
				n_export->new_attr("alias", e->alias);
				delete_mail(out_addr);
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
		return _bp_env;
	}
}

