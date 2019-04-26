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
		NodePrivate* node;
		VariableInfo* variable_info;

		CommonData data;

		SlotPrivate*link;

		SlotPrivate(NodePrivate* _node, VariableInfo* _variable_info);
		bool set_link(SlotPrivate*target);

		String get_address() const;
	};

	struct NodePrivate : BP::Node
	{
		BPPrivate *bp;
		std::string id;
		UdtInfo *udt;
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

		void *_find_input_or_output(const std::string &name, int &input_or_output /* 0 or 1 */) const;
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

		std::vector<std::unique_ptr<NodePrivate>> nodes;
		std::vector<NodePrivate*> update_list;

		Vec2 pos;

		NodePrivate *add_node(const char *id, UdtInfo *udt);
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
		void tobin();
	};

	SlotPrivate::SlotPrivate(NodePrivate* _node, VariableInfo* _variable_info) :
		node(_node),
		variable_info(_variable_info),
		link(nullptr)
	{
		data = variable_info->default_value();
	}

	bool SlotPrivate::set_link(SlotPrivate* target)
	{
		if (target && !typefmt_compare(data.fmt, target->data.fmt))
			return false;
		if (link)
			link->link = nullptr;
		link = target;
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
				inputs.emplace_back(new SlotPrivate(this, v));
			if (attr.find('o') != std::string::npos)
				outputs.emplace_back(new SlotPrivate(this, v));
		}
	}

	NodePrivate::~NodePrivate()
	{
		for (auto &output : outputs)
		{
			auto link = output->link;
			if (link)
				link->link = nullptr;
		}
	}

	void *NodePrivate::_find_input_or_output(const std::string &name, int &input_or_output) const
	{
		auto udt_item = udt->find_item(name.c_str());
		if (!udt_item)
			return nullptr;
		auto udt_item_attribute = std::string(udt_item->attribute());
		if (udt_item_attribute.find('i') != std::string::npos)
		{
			for (auto &input : inputs)
			{
				if (name == input->variable_info->name())
				{
					input_or_output = 0;
					return input.get();
				}
			}
		}
		else if (udt_item_attribute.find('o') != std::string::npos)
		{
			for (auto &output : outputs)
			{
				if (name == output->variable_info->name())
				{
					input_or_output = 1;
					return output.get();
				}
			}
		}
	}

	SlotPrivate* NodePrivate::find_input(const std::string &name) const
	{
		int input_or_output;
		auto ret = _find_input_or_output(name, input_or_output);
		return input_or_output == 0 ? (SlotPrivate*)ret : nullptr;
	}

	SlotPrivate* NodePrivate::find_output(const std::string &name) const
	{
		int input_or_output;
		auto ret = _find_input_or_output(name, input_or_output);
		return input_or_output == 1 ? (SlotPrivate*)ret : nullptr;
	}

	void NodePrivate::report_order()
	{
		if (updated)
			return;

		for (auto &input : inputs)
		{
			auto link = input->link;
			if (link)
			{
				auto n = link->node;
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
			set(type->tag(), v->size(), input->link ? &input->link->data : &input->data, (char*)dummy + v->offset());
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
			get(type->tag(), v->size(), (char*)dummy + v->offset(), &output->data);
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
			set(type->tag(), v->size(), input->link ? &input->link->data : &input->data, (char*)dummy + v->offset());
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
			get(type->tag(), v->size(), (char*)dummy + v->offset(), &output->data);
		}

		updated = true;
	}

	NodePrivate *BPPrivate::add_node(const char *id, UdtInfo *udt)
	{
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

	SlotPrivate *BPPrivate::find_input(const std::string &address) const
	{
		auto sp = string_split(address, '.');
		auto n = find_node(sp[0]);
		if (!n)
			return nullptr;
		int input_or_output;
		auto ret = (SlotPrivate*)n->_find_input_or_output(sp[1], input_or_output);
		return input_or_output == 0 ? ret : nullptr;
	}

	SlotPrivate*BPPrivate::find_output(const std::string &address) const
	{
		auto sp = string_split(address, '.');
		auto n = find_node(sp[0]);
		if (!n)
			return nullptr;
		int input_or_output;
		auto ret = (SlotPrivate*)n->_find_input_or_output(sp[1], input_or_output);
		return input_or_output == 1 ? ret : nullptr;
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

		auto file = SerializableNode::create_from_xml_file(filename);
		if (!file || file->name() != "BP")
			return;

		for (auto i_n = 0; i_n < file->node_count(); i_n++)
		{
			auto n_node = file->node(i_n);
			if (n_node->name() == "node")
			{
				auto type = n_node->find_attr("type")->value();
				auto id = n_node->find_attr("id")->value();

				auto udt = find_udt(H(type.c_str()));
				if (!udt)
					continue;
				auto n = new NodePrivate(this, id, udt);
				auto a_pos = n_node->find_attr("pos");
				if (a_pos)
					n->position = stof2(a_pos->value().c_str());

				for (auto i_i = 0; i_i < n_node->node_count(); i_i++)
				{
					auto n_input = n_node->node(i_i);
					if (n_input->name() == "input")
					{
						auto name = n_input->find_attr("name")->value();
						auto udt_item = udt->find_item(name.c_str());
						if (udt_item && udt_item->type()->tag() != TypeTagPointer && 
							std::string(udt_item->attribute()).find('i') != std::string::npos)
						{
							for (auto& input : n->inputs)
							{
								auto v = input->variable_info;
								if (name == v->name())
								{
									auto type = v->type();
									unserialize_value(type->tag(), type->name_hash(), n_input->find_attr("value")->value(), &input->data.v);
									break;
								}
							}
						}
					}
				}

				nodes.emplace_back(n);
			}
			else if (n_node->name() == "link")
			{
				auto i = find_input(n_node->find_attr("in")->value());
				auto o = find_output(n_node->find_attr("out")->value());
				if (o && i)
					i->set_link(o);
			}
		}

		SerializableNode::destroy(file);
	}

	void BPPrivate::save(const wchar_t *_filename)
	{
		filename = _filename;

		auto file = SerializableNode::create("BP");

		for (auto &n : nodes)
		{
			auto n_node = file->new_node("node");
			n_node->new_attr("type", n->udt->name());
			n_node->new_attr("id", n->id);
			n_node->new_attr("pos", to_stdstring(n->position));
			for (auto &input : n->inputs)
			{
				auto v = input->variable_info;
				auto tag = v->type()->tag();
				auto hash = v->type()->name_hash();
				if (tag != TypeTagPointer)
				{
					if (!compare(tag, v->size(), &v->default_value(), &input->data.v))
					{
						auto n_input = n_node->new_node("input");
						n_input->new_attr("name", v->name());
						n_input->new_attr("value", serialize_value(tag, hash, &input->data.v, 2).v);
					}
				}
			}
		}
		for (auto &n : nodes)
		{
			for (auto &input : n->inputs)
			{
				if (input->link)
				{
					auto n_link = file->new_node("link");
					n_link->new_attr("in", input->get_address().v);
					n_link->new_attr("out", input->link->get_address().v);
				}
			}
		}

		file->save_xml(filename);
		SerializableNode::destroy(file);
	}

	CommonData &BP::Slot::data()
	{
		return ((SlotPrivate*)this)->data;
	}

	void BP::Slot::set_data(const CommonData &d)
	{
		((SlotPrivate*)this)->data = d;
	}

	BP::Slot* BP::Slot::link() const
	{
		return ((SlotPrivate*)this)->link;
	}

	bool BP::Slot::set_link(BP::Slot*target)
	{
		return ((SlotPrivate*)this)->set_link((SlotPrivate*)target);
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

	BP::Node *BP::add_node(const char *id, UdtInfo *udt)
	{
		return ((BPPrivate*)this)->add_node(id, udt);
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

	void BP_Socket4$::initialize$c()
	{
		v$o = new CommonData[4];
	}

	void BP_Socket4$::finish$c()
	{
		delete[]v$o;
	}

	void BP_Socket4$::update$c()
	{
		v$o[0] = v1$i;
		v$o[1] = v2$i;
		v$o[2] = v3$i;
		v$o[3] = v4$i;
	}

	BP_Socket4$ bp_socket4_unused;

	void BP_Int$::update$c()
	{
		v$o = v$i;
	}

	BP_Int$ bp_int_unused;

	void BP_Float$::update$c()
	{
		v$o = v$i;
	}

	BP_Float$ bp_float_unused;

	void BP_Bool$::update$c()
	{
		v$o = v$i;
	}

	BP_Bool$ bp_bool_unused;

	void BP_Vec2$::update$c()
	{
		v$o.x = x$i; 
		v$o.y = y$i;
	}

	BP_Vec2$ bp_vec2_unused;

	void BP_Vec3$::update$c()
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

