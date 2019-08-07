#include <flame/foundation/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/foundation/window.h>

namespace flame
{
	struct BPPrivate;
	struct NodePrivate;
	struct SlotPrivate;

	static BP::Environment bp_env;

	struct SlotPrivate : BP::Slot
	{
		Type type;
		NodePrivate* node;
		VariableInfo* variable_info;
		void* raw_data;

		std::vector<SlotPrivate*> links;

		SlotPrivate(Type _type, NodePrivate* _node, VariableInfo* _variable_info);

		void set_frame(int frame);
		void set_data(const void* data);
		bool link_to(SlotPrivate*target);

		Mail<std::string> get_address() const;
	};

	struct NodePrivate : BP::Node
	{
		BPPrivate *bp;
		std::string id;
		UdtInfo* udt;

		Vec2f pos;

		void* dummy; // represents the object

		void* ctor_addr;
		void* dtor_addr;
		void* update_addr;

		std::vector<std::unique_ptr<SlotPrivate>> inputs;
		std::vector<std::unique_ptr<SlotPrivate>> outputs;

		bool in_list;

		NodePrivate(BPPrivate *_bp, const std::string &_id, UdtInfo *_udt, void* module);
		~NodePrivate();

		SlotPrivate* find_input(const std::string &name) const;
		SlotPrivate* find_output(const std::string &name) const;

		void add_to_update_list();

		void update();
	};

	struct Dependency
	{
		std::wstring filename;
		std::wstring final_filename;
		void* module;
	};

	struct BPPrivate : BP
	{
		std::wstring filename;

		void* graphics_device;

		std::vector<Dependency> dependencies;
		void* bp_module;

		std::vector<std::unique_ptr<NodePrivate>> nodes;
		std::vector<NodePrivate*> update_list;

		BPPrivate();
		~BPPrivate();

		void add_dependency(const std::wstring& filename);
		void remove_dependency(const std::wstring& filename);

		NodePrivate *add_node(const std::string& id, const std::string& type_name);
		void remove_node(NodePrivate *n);

		NodePrivate* find_node(const std::string &id) const;
		SlotPrivate* find_input(const std::string &address) const;
		SlotPrivate* find_output(const std::string &address) const;

		void clear();

		void build_update_list();

		void update();

		void save(const std::wstring& filename);
	};

	SlotPrivate::SlotPrivate(Type _type, NodePrivate* _node, VariableInfo* _variable_info) :
		type(_type),
		node(_node),
		variable_info(_variable_info)
	{
		raw_data = (char*)node->dummy + variable_info->offset();

		if (type == Input)
			links.push_back(nullptr);
		else /* if (type == Output) */
			set_frame(-1);
	}

	void SlotPrivate::set_frame(int frame)
	{
		auto& p = *(AttributeV<int>*)raw_data;
		p.frame = frame;
	}

	void SlotPrivate::set_data(const void* d)
	{
		set_frame(app_frame());
		memcpy((char*)raw_data + sizeof(uint), d, variable_info->size() - sizeof(int));
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
			auto in_type = variable_info->type();
			auto out_type = target->variable_info->type();
			auto in_tag = in_type->tag();
			auto out_tag = out_type->tag();
			auto in_hash = in_type->hash();
			auto out_hash = out_type->hash();

			if (!(in_tag == out_tag && in_hash == out_hash) && !((out_tag == TypeTagAttributeV || out_tag == TypeTagAttributeP) && in_tag == TypeTagAttributeP && (out_hash == in_hash || in_hash == cH("void"))))
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
		target->links.push_back(this);

		set_frame(app_frame());

		node->bp->build_update_list();

		return true;
	}

	Mail<std::string> SlotPrivate::get_address() const
	{
		auto ret = new_mail<std::string>();
		(*ret.p) = node->id + "." + variable_info->name();
		return ret;
	}

	NodePrivate::NodePrivate(BPPrivate *_bp, const std::string& _id, UdtInfo *_udt, void* module) :
		bp(_bp),
		id(_id),
		udt(_udt),
		pos(0.f),
		in_list(false)
	{
		auto size = udt->size();
		dummy = malloc(size);
		memset(dummy, 0, size);

		ctor_addr = nullptr;
		{
			auto f = udt->find_function("ctor");
			if (f)
			{
				auto ret_t = f->return_type();
				if (ret_t->tag() == TypeTagVariable && ret_t->hash() == cH("void") && f->parameter_count() == 0)
					ctor_addr = (char*)module + (uint)f->rva();
			}
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
			if(f)
			{
				auto ret_t = f->return_type();
				if (ret_t->tag() == TypeTagVariable && ret_t->hash() == cH("void") && f->parameter_count() == 0)
					update_addr = (char*)module + (uint)f->rva();
			}
		}

		if (ctor_addr)
			cmf(p2f<MF_v_v>(ctor_addr), dummy);

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
				l->set_frame(app_frame());
				l->links[0] = nullptr;
			}
		}

		if (dtor_addr)
			cmf(p2f<MF_v_v>(dtor_addr), dummy);

		free(dummy);
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

	void NodePrivate::update()
	{
		for (auto& input : inputs)
		{
			auto v = input->variable_info;
			auto out = input->links[0];
			if (out)
			{
				if (out->variable_info->type()->tag() == TypeTagAttributeV && v->type()->tag() == TypeTagAttributeP)
				{
					memcpy(input->raw_data, out->raw_data, sizeof(int));
					auto p = (char*)out->raw_data + sizeof(int);
					memcpy((char*)input->raw_data + sizeof(int), &p, sizeof(void*));
				}
				else
					memcpy(input->raw_data, out->raw_data, v->size());
			}
		}

		cmf(p2f<MF_v_v>(update_addr), dummy);
	}

	BPPrivate::BPPrivate() :
		graphics_device(nullptr)
	{
	}

	BPPrivate::~BPPrivate()
	{
		for (auto& d : dependencies)
		{
			free_module(d.module);
			typeinfo_clear(ext_replace(d.final_filename, L".typeinfo"));
		}
		free_module(bp_module);
		typeinfo_clear(std::fs::path(filename).parent_path().wstring() + L"/build/debug/bp.typeinfo");
	}

	void BPPrivate::add_dependency(const std::wstring& filename)
	{
		for (auto& d : dependencies)
		{
			if (d.filename == filename)
				return;
		}
		Dependency d;
		d.filename = filename;
		d.final_filename = filename;
		d.module = load_module(filename);
		if (!d.module)
		{
			std::fs::path path(this->filename);
			d.final_filename = path.parent_path().wstring() + L"/" + filename;
			d.module = load_module(d.final_filename);
		}
		assert(d.module);
		typeinfo_load(ext_replace(d.final_filename, L".typeinfo"));
		dependencies.push_back(d);
	}

	void BPPrivate::remove_dependency(const std::wstring& filename)
	{
		for (auto it = dependencies.begin(); it != dependencies.end(); it++)
		{
			if (it->filename == filename)
			{
				auto& nodes = this->nodes;
				for (auto n_it = nodes.begin(); n_it != nodes.end(); )
				{
					if ((*n_it)->udt->module_name() == it->final_filename)
						n_it = nodes.erase(n_it);
					else
						n_it++;
				}

				free_module(it->module);
				typeinfo_clear(ext_replace(it->final_filename, L".typeinfo"));
				dependencies.erase(it);

				((BPPrivate*)this)->build_update_list();
				return;
			}
		}
	}

	NodePrivate* BPPrivate::add_node(const std::string& type_name, const std::string& id)
	{
		auto udt = find_udt(H(type_name.c_str()));

		if (!udt)
			return nullptr;

		void* module = nullptr;
		auto p_udt_module_name = std::fs::path(udt->module_name());
		for (auto& d : dependencies)
		{
			if (d.final_filename == p_udt_module_name)
			{
				module = d.module;
				break;
			}
		}
		if (!module)
		{
			if (p_udt_module_name == std::fs::path(filename).parent_path() / "build/debug/bp.dll")
				module = bp_module;
		}

		if (!module)
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
				if (find_node(s_id))
					continue;
			}
		}

		auto n = new NodePrivate(this, s_id, udt, module);
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

	void BPPrivate::update()
	{
		if (update_list.empty())
		{
			printf("no nodes or didn't call 'prepare'\n");
			return;
		}

		bp_env.path = std::fs::path(filename).parent_path().wstring();
		bp_env.graphics_device = graphics_device;

		for (auto &n : update_list)
			n->update();

		bp_env.path = L"";
		bp_env.graphics_device = nullptr;
	}

	void BPPrivate::save(const std::wstring& _filename)
	{
		filename = _filename;

		auto file = SerializableNode::create("BP");

		if (!dependencies.empty())
		{
			auto n_dependencies = file->new_node("dependencies");
			for (auto& d : dependencies)
				n_dependencies->new_node("dependency")->new_attr("v", (w2s(d.filename)));
		}

		auto n_nodes = file->new_node("nodes");
		for (auto& n : nodes)
		{
			auto n_node = n_nodes->new_node("node");
			n_node->new_attr("type", n->udt->name());
			n_node->new_attr("id", n->id);
			n_node->new_attr("pos", to_string(n->pos, 2));

			SerializableNode* n_datas = nullptr;
			for (auto& input : n->inputs)
			{
				if (input->links[0])
					continue;
				auto v = input->variable_info;
				auto type = v->type();
				if (v->default_value() && memcmp((char*)input->raw_data + sizeof(int), (char*)v->default_value() + sizeof(int), v->size() - sizeof(int)) != 0)
				{
					if (!n_datas)
						n_datas = n_node->new_node("datas");
					auto n_data = n_datas->new_node("data");
					n_data->new_attr("name", v->name());
					auto value = serialize_value(type->tag(), type->hash(), input->raw_data, 2);
					n_data->new_attr("value", *value.p);
					delete_mail(value);
				}
			}
		}

		auto n_links = file->new_node("links");
		for (auto& n : nodes)
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

		file->save_xml(filename);
		SerializableNode::destroy(file);
	}

	int BP::Slot::frame() const
	{
		return *(int*)((SlotPrivate*)this)->raw_data;
	}

	void* BP::Slot::data() const
	{
		return (char*)((SlotPrivate*)this)->raw_data + sizeof(int);
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

	Mail<std::string> BP::Slot::get_address() const
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

	const std::string& BP::Node::id() const
	{
		return ((NodePrivate*)this)->id;
	}

	UdtInfo *BP::Node::udt() const
	{
		return ((NodePrivate*)this)->udt;
	}

	Vec2f BP::Node::pos() const
	{
		return ((NodePrivate*)this)->pos;
	}

	void BP::Node::set_pos(const Vec2f& p)
	{
		((NodePrivate*)this)->pos = p;
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

	BP::Slot*BP::Node::find_input(const std::string& name) const
	{
		return ((NodePrivate*)this)->find_input(name);
	}

	BP::Slot*BP::Node::find_output(const std::string& name) const
	{
		return ((NodePrivate*)this)->find_output(name);
	}

	void BP::set_graphics_device(void* d)
	{
		((BPPrivate*)this)->graphics_device = d;
	}

	uint BP::dependency_count() const
	{
		return ((BPPrivate*)this)->dependencies.size();
	}

	Mail<std::wstring> BP::dependency(int idx) const
	{
		return new_mail(&(((BPPrivate*)this)->dependencies[idx].filename));
	}

	void BP::add_dependency(const std::wstring& filename)
	{
		((BPPrivate*)this)->add_dependency(filename);
	}

	void BP::remove_dependency(const std::wstring& filename)
	{
		((BPPrivate*)this)->remove_dependency(filename);
	}

	uint BP::node_count() const
	{
		return ((BPPrivate*)this)->nodes.size();
	}

	BP::Node *BP::node(int idx) const
	{
		return ((BPPrivate*)this)->nodes[idx].get();
	}

	BP::Node *BP::add_node(const std::string& type_name, const std::string& id)
	{
		return ((BPPrivate*)this)->add_node(type_name, id);
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

	void BP::clear()
	{
		((BPPrivate*)this)->clear();
	}

	void BP::update()
	{
		((BPPrivate*)this)->update();
	}

	void BP::save(const std::wstring& filename)
	{
		((BPPrivate*)this)->save(filename);
	}

	BP *BP::create_from_file(const std::wstring& filename)
	{
		auto s_filename = w2s(filename);
		auto ppath = std::fs::path(filename).parent_path();
		auto ppath_str = ppath.wstring();

		printf("begin to load bp: %s\n", s_filename.c_str());

		auto file = SerializableNode::create_from_xml_file(filename);
		if (!file)
		{
			printf("bp file does not exist, abort\n", s_filename.c_str());
			printf("end loading bp: %s\n", s_filename.c_str());
			return nullptr;
		}

		std::vector<std::pair<std::wstring, std::wstring>> dependencies;

		auto n_dependencies = file->find_node("dependencies");
		if (n_dependencies)
		{
			for (auto i_d = 0; i_d < n_dependencies->node_count(); i_d++)
			{
				std::pair<std::wstring, std::wstring> d;
				d.first = s2w(n_dependencies->node(i_d)->find_attr("v")->value());
				auto final_filename = d;
				if (!std::fs::exists(d.first))
					d.second = ppath_str + L"/" + d.first;
				else
					d.second = d.first;
				dependencies.push_back(d);
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
			auto a_pos = n_node->find_attr("pos");
			if (a_pos)
				node.pos = stof2(a_pos->value().c_str());
			else
				node.pos = Vec2f(0.f);

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
		auto lc = n_links->node_count();
		for (auto i_l = 0; i_l < n_links->node_count(); i_l++)
		{
			auto n_link = n_links->node(i_l);
			LinkDesc link;
			link.out_addr = n_link->find_attr("out")->value();
			link.in_addr = n_link->find_attr("in")->value();
			link_descs.push_back(link);
		}

		SerializableNode::destroy(file);

		auto templatecpp_path = ppath / L"template.cpp";
		if (!std::fs::exists(templatecpp_path) || std::fs::last_write_time(templatecpp_path) < std::fs::last_write_time(filename))
		{
			printf("generating template.cpp");

			std::ofstream templatecpp(templatecpp_path);
			templatecpp << "// THIS FILE IS AUTO GENERATED\n";
			templatecpp << "#include <flame/foundation/bp_node_template.h>\n";
			templatecpp << "using namespace flame;\n";
			templatecpp << "extern \"C\" __declspec(dllexport) void add_templates()\n";
			templatecpp << "{\n";
			templatecpp << "\tauto module = get_module_from_address(f2v(add_templates));\n";
			templatecpp << "\tauto module_name = get_module_name(module);\n";
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
					if (n.type[pos_t - 1] == 'N')
						templatecpp << std::string(n.type.begin(), n.type.begin() + pos_t);
					else
						templatecpp << tn_a2c(n.type);
					templatecpp << "::add_udt_info(*module_name.p, \"";
					templatecpp << std::string(n.type.begin() + pos_t, n.type.end());
					templatecpp << "\", module);\n";
				}
			}
			templatecpp << "\tdelete_mail(module_name);\n";
			templatecpp << "}\n";
			templatecpp.close();

			printf(" - done\n");
		}
		else
			printf("template.cpp up to data\n");

		auto cmakelists_path = ppath / L"CMakeLists.txt";
		if (!std::fs::exists(cmakelists_path) || std::fs::last_write_time(cmakelists_path) < std::fs::last_write_time(filename))
		{
			printf("generating cmakelists");

			std::ofstream cmakelists(cmakelists_path);
			cmakelists << "# THIS FILE IS AUTO GENERATED\n";
			cmakelists << "cmake_minimum_required(VERSION 3.4)\n";
			cmakelists << "project(bp)\n";
			cmakelists << "add_definitions(-W0 -std:c++latest)\n";
			cmakelists << "file(GLOB SOURCE_LIST \"*.c*\")\n";
			cmakelists << "add_library(bp SHARED ${SOURCE_LIST})\n";
			for (auto& d : dependencies)
			{
				cmakelists << "target_link_libraries(bp ${CMAKE_SOURCE_DIR}/../../bin/";
				cmakelists << w2s(ext_replace(d.second, L".lib"));
				cmakelists << ")\n";
			}
			cmakelists << "target_include_directories(bp PRIVATE ${CMAKE_SOURCE_DIR}/../../include)\n";
			cmakelists << "add_custom_command(TARGET bp POST_BUILD COMMAND ${CMAKE_SOURCE_DIR}/../../bin/typeinfogen ${CMAKE_SOURCE_DIR}/build/debug/bp.dll ";
			for (auto& d : dependencies)
				cmakelists << "${CMAKE_SOURCE_DIR}/../../bin/" + w2s(d.second) + " ";
			cmakelists << ")\n";
			cmakelists.close();

			printf(" - done\n");
		}
		else
			printf("cmakelists up to data\n");

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
				std::fs::remove(OUTPUT_FILE);

			#undef OUTPUT_FILE
		}

		auto bp = new BPPrivate();
		bp->filename = filename;

		for (auto& d : dependencies)
			bp->add_dependency(d.first);
		bp->bp_module = load_module(ppath_str + L"/build/debug/bp.dll");
		typeinfo_load(ppath_str + L"/build/debug/bp.typeinfo");

		for (auto& n_d : node_descs)
		{
			auto n = bp->add_node(n_d.type, n_d.id);
			n->set_pos(n_d.pos);
			for (auto& d_d : n_d.datas)
			{
				auto input = n->find_input(d_d.name);
				auto v = input->variable_info;
				auto type = v->type();
				if (v->default_value())
					unserialize_value(type->tag(), type->hash(), d_d.value, input->raw_data);
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
				printf("unable to link: %s - > %s\n", l_d.out_addr.c_str(), l_d.in_addr.c_str());
		}

		printf("end loading bp: %s\n", s_filename.c_str());

		return bp;
	}

	void BP::destroy(BP *bp)
	{
		delete(BPPrivate*)bp;
	}

	const BP::Environment& bp_environment()
	{
		return bp_env;
	}

	struct ArraySize$
	{
		AttributeP<void> array$i;

		AttributeV<uint> size$o;

		FLAME_FOUNDATION_EXPORTS void update$()
		{
			if (array$i.frame > size$o.frame)
			{
				if (array$i.v)
					size$o.v = ((std::vector<void*>*)array$i.v)->size();
				else
					size$o.v = 0;
				size$o.frame = array$i.frame;
			}
		}

	};
}

