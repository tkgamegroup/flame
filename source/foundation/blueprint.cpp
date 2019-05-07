// MIT License
// 
// Copyright (c) 2019 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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

		Vec2 position;

		FunctionInfo* initialize_function;
		FunctionInfo* finish_function;
		FunctionInfo* update_function;

		std::vector<std::unique_ptr<SlotPrivate>> inputs;
		std::vector<std::unique_ptr<SlotPrivate>> outputs;

		bool enable;

		bool updated;
		void* dummy; // represents the object

		NodePrivate(BPPrivate *_bp, const std::string &_id, UdtInfo *_udt);
		~NodePrivate();

		SlotPrivate* find_input(const std::string &name) const;
		SlotPrivate* find_output(const std::string &name) const;

		void report_order(); // use by BP's prepare update, report update order from dependencies
		void initialize();
		void finish();
		void update();
	};

	struct BPPrivate : BP
	{
		std::wstring filename;

		std::vector<std::pair<int, std::wstring>> extra_typeinfos;

		std::vector<std::unique_ptr<NodePrivate>> nodes;
		std::vector<NodePrivate*> update_list;

		Vec2 pos;

		~BPPrivate();

		NodePrivate *add_node(const char* id, const char* type_name);
		void remove_node(NodePrivate *n);

		NodePrivate* find_node(const std::string &id) const;
		SlotPrivate* find_input(const std::string &address) const;
		SlotPrivate* find_output(const std::string &address) const;

		void clear();

		void initialize();
		void finish();

		void update();

		void load(const wchar_t *filename);
		void save(const wchar_t *filename);
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
		auto vtype = variable_info->type();
		if (vtype->tag() != TypeTagPointer && vtype->name_hash() != cH("VoidPtrs"))
			set(data, variable_info->type()->tag(), size, &variable_info->default_value());

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
		enable(true),
		updated(false),
		dummy(nullptr)
	{
		initialize_function = udt->find_function("initialize");
		finish_function = udt->find_function("finish");
		update_function = udt->find_function("update");

		for (auto i = 0; i < udt->item_count(); i++)
		{
			auto v = udt->item(i);
			auto attr = std::string(v->attribute());
			if (attr.find('i') != std::string::npos)
				inputs.emplace_back(new SlotPrivate(SlotPrivate::Input, this, v));
			if (attr.find('o') != std::string::npos)
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
				l->links[0] = nullptr;
		}

		if (dummy)
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

	void NodePrivate::report_order()
	{
		if (updated)
			return;

		for (auto &input : inputs)
		{
			auto o = input->links[0];
			if (o)
			{
				auto n = o->node;
				if (!n->updated)
					n->report_order();
			}
		}

		bp->update_list.push_back(this);

		updated = true;
	}

	void NodePrivate::initialize()
	{
		for (auto& input : inputs)
		{
			auto v = input->variable_info;
			auto type = v->type();
			set((char*)dummy + v->offset(), type->tag(), v->size(), input->links[0] ? input->links[0]->data : input->data);
		}

		if (initialize_function)
		{
			auto library = load_module(udt->module_name());
			if (library)
			{
				struct Dummy { };
				typedef void (Dummy:: * F)();
				union
				{
					void* p;
					F f;
				}cvt;
				cvt.p = (char*)library + (uint)initialize_function->rva();
				(*((Dummy*)dummy).*cvt.f)();

				free_module(library);
			}
		}

		for (auto& output : outputs)
		{
			auto v = output->variable_info;
			auto type = v->type();
			set(output->data, type->tag(), v->size(), (char*)dummy + v->offset());
		}
	}

	void NodePrivate::finish()
	{
		if (finish_function)
		{
			auto library = load_module(udt->module_name());
			if (library)
			{
				struct Dummy { };
				typedef void (Dummy:: * F)();
				union
				{
					void* p;
					F f;
				}cvt;
				cvt.p = (char*)library + (uint)finish_function->rva();
				(*((Dummy*)dummy).*cvt.f)();

				free_module(library);
			}
		}
	}

	void NodePrivate::update()
	{
		if (updated)
			return;

		for (auto& input : inputs)
		{
			auto v = input->variable_info;
			auto type = v->type();
			set((char*)dummy + v->offset(), type->tag(), v->size(), input->links[0] ? input->links[0]->data : input->data);
		}

		if (update_function)
		{
			auto library = load_module(udt->module_name());
			if (library)
			{
				struct Dummy { };
				typedef void (Dummy:: * F)();
				union
				{
					void* p;
					F f;
				}cvt;
				cvt.p = (char*)library + (uint)update_function->rva();
				(*((Dummy*)dummy).*cvt.f)();

				free_module(library);
			}
		}

		for (auto& output : outputs)
		{
			auto v = output->variable_info;
			auto type = v->type();
			set(output->data, type->tag(), v->size(), (char*)dummy + v->offset());
		}

		updated = true;
	}

	BPPrivate::~BPPrivate()
	{
		for (auto& t : extra_typeinfos)
			typeinfo_clear(t.first);
	}

	NodePrivate *BPPrivate::add_node(const char* type_name, const char *id)
	{
		auto type_name_sp = string_split(std::string(type_name), ':');
		UdtInfo* udt = nullptr;
		if (type_name_sp.size() == 1)
			udt = find_udt(H(type_name_sp[0].c_str()));
		else if (type_name_sp.size() == 2)
		{
			auto fn = s2w(type_name_sp[0]);
			for (auto& t : extra_typeinfos)
			{
				if (t.second == fn)
				{
					udt = find_udt(H(type_name_sp[1].c_str()));
					break;
				}
			}
			if (!udt)
			{
				auto abs_fn = std::filesystem::path(filename).parent_path().wstring() + L"\\" + fn;
				auto fn_cpp = abs_fn + L".cpp";
				if (!std::filesystem::exists(fn_cpp))
				{
					assert(0);
					return nullptr;
				}
				auto fn_dll = abs_fn + L".dll";
				auto fn_ti = abs_fn + L".typeinfo";
				if (!std::filesystem::exists(fn_dll) || std::filesystem::last_write_time(fn_dll) < std::filesystem::last_write_time(fn_cpp))
				{
					auto compile_output = compile_to_dll({ fn_cpp }, { L"flame_foundation.lib", L"flame_graphics.lib" }, fn_dll);
					if (!std::filesystem::exists(fn_dll) || std::filesystem::last_write_time(fn_dll) < std::filesystem::last_write_time(fn_cpp))
					{
						printf("compile error:\n%s\n", compile_output.v);
						assert(0);
						return nullptr;
					}
					if (std::filesystem::exists(fn_ti))
						std::filesystem::remove(fn_ti);
				}
				if (!std::filesystem::exists(fn_ti) || std::filesystem::last_write_time(fn_ti) < std::filesystem::last_write_time(fn_dll))
				{
					typeinfo_collect(fn_dll, 99);
					typeinfo_save(fn_ti, 99);
					typeinfo_clear(99);
				}
				auto lv = typeinfo_free_level();
				typeinfo_load(fn_ti, lv);
				extra_typeinfos.emplace_back(lv, fn);
				udt = find_udt(H(type_name_sp[1].c_str()));
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
				s_id = "node_" + to_stdstring(i);
				if (find_node(s_id))
					continue;
			}
		}
		auto n = new NodePrivate(this, s_id, udt);
		nodes.emplace_back(n);
		return n;
	}

	void BPPrivate::remove_node(NodePrivate *n)
	{
		for (auto it = nodes.begin(); it != nodes.end(); it++)
		{
			if ((*it).get() == n)
			{
				nodes.erase(it);
				return;
			}
		}
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
	}

	void BPPrivate::initialize()
	{
		for (auto &n : nodes)
		{
			if (!n->dummy)
			{
				auto udt = n->udt;
				auto size = udt->size();
				n->dummy = malloc(size);
				memset(n->dummy, 0, size);
			}
		}

		update_list.clear();
		for (auto &n : nodes)
			n->updated = false;
		for (auto &n : nodes)
			n->report_order();

		for (auto& n : update_list)
			n->initialize();
	}

	void BPPrivate::finish()
	{
		for (auto &n : nodes)
		{
			auto dummy = n->dummy;
			if (dummy)
			{
				n->finish();

				free(dummy);
				n->dummy = nullptr;
			}
		}

		update_list.clear();
	}

	void BPPrivate::update()
	{
		if (update_list.empty())
		{
			printf("no nodes or didn't call 'prepare'\n");
			return;
		}

		for (auto &n : update_list)
			n->updated = false;

		for (auto &n : update_list)
			n->update();
	}
	
	void BPPrivate::load(const wchar_t *_filename)
	{
		filename = _filename;

		auto file = SerializableNode::create_from_json_file(filename);
		if (!file)
			return;

		auto n_nodes = file->find_node("nodes");
		for (auto i_n = 0; i_n < n_nodes->node_count(); i_n++)
		{
			auto n_node = n_nodes->node(i_n);
			auto type = n_node->find_node("type")->value();
			auto id = n_node->find_node("id")->value();

			auto n = add_node(type.c_str(), id.c_str());
			if (!n)
			{
				printf("node \"%s\" with type \"%s\" add failed\n", id.c_str(), type.c_str());
				continue;
			}
			auto a_pos = n_node->find_node("pos");
			if (a_pos)
				n->position = stof2(a_pos->value().c_str());

			auto n_datas = n_node->find_node("datas");
			for (auto i_d = 0; i_d < n_datas->node_count(); i_d++)
			{
				auto n_data = n_datas->node(i_d);
				auto input = n->find_input(n_data->find_node("name")->value());
				auto type = input->variable_info->type();
				if (type->tag() != TypeTagPointer)
					unserialize_value(type->tag(), type->name_hash(), n_data->find_node("value")->value(), input->data);
			}
		}

		auto n_links = file->find_node("links");
		auto lc = n_links->node_count();
		for (auto i_l = 0; i_l < n_links->node_count(); i_l++)
		{
			auto n_link = n_links->node(i_l);
			auto o_address = n_link->find_node("out")->value();
			auto i_address = n_link->find_node("in")->value();
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

		SerializableNode::destroy(file);
	}

	void BPPrivate::save(const wchar_t *_filename)
	{
		filename = _filename;
		auto ppath = std::filesystem::path(filename).parent_path().wstring() + L"\\";

		auto file = SerializableNode::create("BP");

		auto n_nodes = file->new_node("nodes");
		n_nodes->set_type(SerializableNode::Array);
		for (auto &n : nodes)
		{
			auto n_node = n_nodes->new_node("");
			auto u = n->udt;
			auto tn = std::string(u->name());
			if (u->level() != 0)
			{
				auto src = std::filesystem::path(u->module_name()).wstring();
				if (ppath.size() < src.size() && src.compare(0, ppath.size(), ppath.c_str()) == 0)
					src.erase(src.begin(), src.begin() + ppath.size());
				tn = w2s(src) + ":" + tn;
			}
			n_node->new_attr("type", tn);
			n_node->new_attr("id", n->id);
			n_node->new_attr("pos", to_stdstring(n->position));

			auto n_datas = n_node->new_node("datas");
			n_datas->set_type(SerializableNode::Array);
			for (auto &input : n->inputs)
			{
				auto v = input->variable_info;
				auto type = v->type();
				if (type->tag() != TypeTagPointer && type->name_hash() != cH("VoidPtrs") && !compare(type->tag(), v->size(), &v->default_value(), input->data))
				{
					auto n_data = n_datas->new_node("");
					n_data->new_attr("name", v->name());
					n_data->new_attr("value", serialize_value(type->tag(), type->name_hash(), input->data, 2).v);
				}
			}
		}

		auto n_links = file->new_node("links");
		n_links->set_type(SerializableNode::Array);
		for (auto &n : nodes)
		{
			for (auto &input : n->inputs)
			{
				if (input->links[0])
				{
					auto n_link = n_links->new_node("");
					n_link->new_attr("out", input->links[0]->get_address().v);
					n_link->new_attr("in", input->get_address().v);
				}
			}
		}

		file->save_json(filename);
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

	Vec2 BP::Node::position() const
	{
		return ((NodePrivate*)this)->position;
	}

	void BP::Node::set_position(const Vec2& p)
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

	bool BP::Node::enable() const
	{
		return ((NodePrivate*)this)->enable;
	}

	void BP::Node::set_enable(bool enable) const
	{
		((NodePrivate*)this)->enable = enable;
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

	void BP::initialize()
	{
		((BPPrivate*)this)->initialize();
	}

	void BP::finish()
	{
		((BPPrivate*)this)->finish();
	}

	void BP::update()
	{
		((BPPrivate*)this)->update();
	}

	void BP::save(const wchar_t *filename)
	{
		((BPPrivate*)this)->save(filename);
	}

	BP *BP::create()
	{
		return new BPPrivate();
	}

	BP *BP::create_from_file(const wchar_t *filename)
	{
		if (!std::filesystem::exists(filename))
			return nullptr;

		auto bp = new BPPrivate();
		bp->load(filename);
		return bp;
	}

	void BP::destroy(BP *bp)
	{
		delete(BPPrivate*)bp;
	}

	void BP_LNA1Bvec4$::initialize$c()
	{
		v$o.count = 1;
		v$o.v = new Bvec4[1];
	}

	void BP_LNA1Bvec4$::finish$c()
	{
		v$o.count = 0;
		delete[]v$o.v;
	}

	void BP_LNA1Bvec4$::update$c()
	{
		v$o.v[0] = v$i;
	}

	BP_LNA1Bvec4$ bp_lna1bvec4_unused;

	void BP_Vec2$::update$c()
	{
		v$o.x = x$i; 
		v$o.y = y$i;
	}

	BP_Vec2$ bp_vec2_unused;

	void BP_Vec3$::update$c(float a)
	{
		v$o.x = x$i;
		v$o.y = y$i;
		v$o.z = z$i;
	}

	BP_Vec3$ bp_vec3_unused;

	void BP_Vec4$::update$c()
	{
		v$o.x = x$i;
		v$o.y = y$i;
		v$o.z = z$i;
		v$o.w = w$i;
	}

	BP_Vec4$ bp_vec4_unused;

	void BP_Ivec2$::update$c()
	{
		v$o.x = x$i;
		v$o.y = y$i;
	}

	BP_Ivec2$ bp_ivec2_unused;

	void BP_Ivec3$::update$c()
	{
		v$o.x = x$i;
		v$o.y = y$i;
		v$o.z = z$i;
	}

	BP_Ivec3$ bp_ivec3_unused;

	void BP_Ivec4$::update$c()
	{
		v$o.x = x$i;
		v$o.y = y$i;
		v$o.z = z$i;
		v$o.w = w$i;
	}

	BP_Ivec4$ bp_ivec4_unused;

	void BP_Bvec2$::update$c()
	{
		v$o.x = x$i;
		v$o.y = y$i;
	}

	BP_Bvec2$ bp_bvec2_unused;

	void BP_Bvec3$::update$c()
	{
		v$o.x = x$i;
		v$o.y = y$i;
		v$o.z = z$i;
	}

	BP_Bvec3$ bp_bvec3_unused;

	void BP_Bvec4$::update$c()
	{
		v$o.x = x$i;
		v$o.y = y$i;
		v$o.z = z$i;
		v$o.w = w$i;
	}

	BP_Bvec4$ bp_bvec4_unused;
}

