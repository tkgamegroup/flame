// MIT License
// 
// Copyright (c) 2018 wjs
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
	struct InputPrivate;
	struct OutputPrivate;

	struct ItemPrivate : BP::Item
	{
		InputPrivate *parent_i;
		OutputPrivate *parent_o;
		CommonData data;

		ItemPrivate *link;

		inline ItemPrivate(InputPrivate *_parent_i, OutputPrivate *_parent_o);
		inline bool set_link(ItemPrivate *target);
	};

	struct InputPrivate : BP::Input
	{
		NodePrivate *node;
		VaribleInfo *varible_info;
		std::vector<std::unique_ptr<ItemPrivate>> items;

		inline bool is_array() { auto tag = varible_info->tag(); return tag == VariableTagArrayOfVariable || tag == VariableTagArrayOfPointer; }
		inline InputPrivate(NodePrivate *_node, VaribleInfo *_varible_info);
		inline ItemPrivate *array_insert_item(int idx);
		inline void array_remove_item(int idx);
		inline void array_clear();
	};

	struct OutputPrivate : BP::Output
	{
		NodePrivate *node;
		VaribleInfo *varible_info;
		std::unique_ptr<ItemPrivate> item;

		inline OutputPrivate(NodePrivate *_node, VaribleInfo *_varible_info);
	};

	struct NodePrivate : BP::Node
	{
		BPPrivate *bp;
		std::string id;
		UDT *udt;
		std::vector<std::unique_ptr<InputPrivate>> inputs;
		std::vector<std::unique_ptr<OutputPrivate>> outputs;
		bool enable;

		inline NodePrivate(BPPrivate *_bp, const std::string &_id, UDT *_udt);
	};

	struct BPPrivate : BP
	{
		std::vector<std::unique_ptr<NodePrivate>> nodes;

		inline NodePrivate *add_node(uint hash);
		inline void remove_node(NodePrivate *n);
		inline NodePrivate *find_node(const std::string &id);

		inline ItemPrivate *find_item(const std::string &address);

		inline void clear();
		inline void load(const wchar_t *filename);
		inline void save(const wchar_t *filename);
	};

	ItemPrivate::ItemPrivate(InputPrivate *_parent_i, OutputPrivate *_parent_o) :
		parent_i(_parent_i),
		parent_o(_parent_o),
		link(nullptr)
	{
		data = (parent_i ? parent_i->varible_info : parent_o->varible_info)->default_value();
	}

	bool ItemPrivate::set_link(ItemPrivate *target)
	{
		if (!parent_i)
			return false;
		if (!typefmt_compare(data.fmt, target->data.fmt))
			return false;
		if (link)
			link->link = nullptr;
		link = target;
		if (link)
			link->link = this;
		return true;
	}

	InputPrivate::InputPrivate(NodePrivate *_node, VaribleInfo *_varible_info) :
		node(_node),
		varible_info(_varible_info)
	{
		if (!is_array())
			/* so we need an element stands for the content */
			items.emplace_back(new ItemPrivate(this, nullptr));
	}

	ItemPrivate *InputPrivate::array_insert_item(int idx)
	{
		if (!is_array())
			return nullptr;
		auto item = new ItemPrivate(this, nullptr);
		items.emplace(items.begin() + idx, item);
		return item;
	}

	void InputPrivate::array_remove_item(int idx)
	{
		if (!is_array())
			return;
		items.erase(items.begin() + idx);
	}

	void InputPrivate::array_clear()
	{
		if (!is_array())
			return;
		items.clear();
	}

	OutputPrivate::OutputPrivate(NodePrivate *_node, VaribleInfo *_varible_info) :
		node(_node),
		varible_info(_varible_info),
		item(new ItemPrivate(nullptr, this))
	{
	}

	NodePrivate::NodePrivate(BPPrivate *_bp, const std::string &_id, UDT *_udt) :
		bp(_bp),
		id(_id),
		udt(_udt)
	{
		for (auto i = 0; i < udt->item_count(); i++)
		{
			auto v = udt->item(i);
			auto attr = std::string(v->attribute());
			if (attr.find('i') != std::string::npos)
				inputs.emplace_back(new InputPrivate(this, v));
			if (attr.find('o') != std::string::npos)
				outputs.emplace_back(new OutputPrivate(this, v));
		}
	}

	NodePrivate *BPPrivate::add_node(uint hash)
	{
		auto udt = find_udt(hash);
		if (!udt)
			return nullptr;
		for (auto i = 0; i < nodes.size() + 1; i++)
		{
			auto try_id = "node_" + std::to_string(i);
			if (find_node(try_id))
				continue;
			auto n = new NodePrivate(this, try_id, udt);
			return n;
		}
		return nullptr;
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

	NodePrivate *BPPrivate::find_node(const std::string &id)
	{
		for (auto &n : nodes)
		{
			if (n->id == id)
				return n.get();
		}
		return nullptr;
	}

	ItemPrivate *BPPrivate::find_item(const std::string &adress)
	{
		auto sp = string_split(adress, '.');
		if (sp.size() < 2 || sp.size() > 3)
			return nullptr;
		auto n = find_node(sp[0]);
		if (!n)
			return nullptr;
		auto udt = n->udt;
		auto v_name = sp[1];
		auto udt_item_idx = udt->find_item_i(v_name.c_str());
		if (udt_item_idx < 0)
			return nullptr;
		auto udt_item = udt->item(udt_item_idx);
		auto udt_item_attribute = std::string(udt_item->attribute());
		if (udt_item_attribute.find('i') != std::string::npos)
		{
			for (auto &i : n->inputs)
			{
				if (v_name == i->varible_info->name())
				{
					if (sp.size() != 3)
						return nullptr;
					auto idx = std::stoi(sp[2]);
					if (idx < 0 || idx >= i->items.size())
						return nullptr;
					return i->items[idx].get();
				}
			}
		}
		else if (udt_item_attribute.find('o') != std::string::npos)
		{
			for (auto &o : n->outputs)
			{
				if (v_name == o->varible_info->name())
					return o->item.get();
			}
		}

		return nullptr;
	}

	void BPPrivate::clear()
	{
		nodes.clear();
	}

	void BPPrivate::load(const wchar_t *filename)
	{
		auto file = SerializableNode::create_from_xml(filename);
		if (!file)
			return;

		for (auto i_n = 0; i_n < file->node_count(); i_n++)
		{
			auto n_node = file->node(i_n);
			if (n_node->name() == "node")
			{
				auto id = n_node->find_attr("id")->value();
				auto type = n_node->find_attr("type")->value();

				auto udt = find_udt(H(type.c_str()));
				if (!udt)
					continue;
				auto n = new NodePrivate(this, id, udt);

				for (auto i_i = 0; i_i < n_node->node_count(); i_i++)
				{
					auto n_item = n_node->node(i_i);
					if (n_item->name() == "item")
					{
						auto name = n_item->find_attr("name")->value();
						auto udt_item_idx = udt->find_item_i(name.c_str());
						if (udt_item_idx > 0)
						{
							auto udt_item = udt->item(udt_item_idx);
							auto udt_item_attribute = std::string(udt_item->attribute());
							if (udt_item_attribute.find('i') != std::string::npos)
							{
								for (auto &i : n->inputs)
								{
									if (name == i->varible_info->name())
									{
										for (auto i_v = 0; i_v < n_item->node_count(); i_v++)
										{
											auto n_value = n_item->node(i_v);
											if (n_value->name() == "value")
											{
												auto v = new ItemPrivate(i.get(), nullptr);
												i->varible_info->unserialize_value(n_value->value(), &v->data, false);
												i->items.emplace_back(v);
											}
										}
										break;
									}
								}
							}
							else if (udt_item_attribute.find('o') != std::string::npos)
							{
								for (auto &o : n->outputs)
								{
									if (name == o->varible_info->name())
									{
										if (n_item->node_count() == 1)
										{
											auto n_value = n_item->node(0);
											if (n_value->name() == "value")
												o->varible_info->unserialize_value(n_value->value(), &o->item->data, false);
										}
										break;
									}
								}
							}
						}
					}
				}

				nodes.emplace_back(n);
			}
			else if (n_node->name() == "link")
			{
				auto o = find_item(n_node->find_attr("from")->value());
				auto i = find_item( n_node->find_attr("to")->value());
				if (o && i)
					i->set_link(o);
			}
		}

		SerializableNode::destroy(file);
	}

	void BPPrivate::save(const wchar_t *filename)
	{
		auto file = SerializableNode::create("BP");

		for (auto &n : nodes)
		{
			auto n_node = file->new_node("node");
			n_node->new_attr("id", n->id);
			n_node->new_attr("type", n->udt->name());
			for (auto &i : n->inputs)
			{
				auto n_item = n_node->new_node("item");
				n_item->new_attr("name", i->varible_info->name());
				for (auto &ii : i->items)
					n_item->new_attr("value", i->varible_info->serialize_value(&ii->data, false, 2).v);
			}
			for (auto &o : n->outputs)
			{
				auto n_item = n_node->new_node("item");
				n_item->new_attr("name", o->varible_info->name());
				n_item->new_attr("value", o->varible_info->serialize_value(&o->item->data, false, 2).v);
			}
		}
		for (auto &n : nodes)
		{
			for (auto &i : n->inputs)
			{
				auto idx = 0;
				for (auto &ii : i->items)
				{
					auto o = ii->link;
					if (o)
					{
						auto n_link = file->new_node("link");
						auto o_i = o->parent_o;
						n_link->new_attr("from", o_i->node->id + "." + o_i->varible_info->name());
						n_link->new_attr("to", n->id + "." + i->varible_info->name() + "." + std::to_string(idx));
					}
					idx++;
				}
			}
		}

		file->save_xml(filename);
		SerializableNode::destroy(file);
	}

	BP::Input *BP::Item::parent_i() const
	{
		return ((ItemPrivate*)this)->parent_i;
	}

	BP::Output *BP::Item::parent_o() const
	{
		return ((ItemPrivate*)this)->parent_o;
	}

	CommonData &BP::Item::data()
	{
		return ((ItemPrivate*)this)->data;
	}

	BP::Item *BP::Item::link() const
	{
		return ((ItemPrivate*)this)->link;
	}

	bool BP::Item::set_link(BP::Item *target)
	{
		return ((ItemPrivate*)this)->set_link((ItemPrivate*)target);
	}

	BP::Node *BP::Input::node() const
	{
		return ((InputPrivate*)this)->node;
	}

	VaribleInfo *BP::Input::varible_info() const
	{
		return ((InputPrivate*)this)->varible_info;
	}

	int BP::Input::array_item_count() const
	{
		return ((InputPrivate*)this)->items.size();
	}

	BP::Item *BP::Input::array_item(int idx) const
	{
		return ((InputPrivate*)this)->items[idx].get();
	}

	BP::Item *BP::Input::array_insert_item(int idx)
	{
		return ((InputPrivate*)this)->array_insert_item(idx);
	}

	void BP::Input::array_remove_item(int idx)
	{
		((InputPrivate*)this)->array_remove_item(idx);
	}

	void BP::Input::array_clear() const
	{
		((InputPrivate*)this)->array_clear();
	}

	BP::Node *BP::Output::node() const
	{
		return ((OutputPrivate*)this)->node;
	}

	VaribleInfo *BP::Output::varible_info() const
	{
		return ((OutputPrivate*)this)->varible_info;
	}

	BP::Item *BP::Output::item() const
	{
		return ((OutputPrivate*)this)->item.get();
	}

	BP *BP::Node::bp() const
	{
		return ((NodePrivate*)this)->bp;
	}

	const char *BP::Node::id() const
	{
		return ((NodePrivate*)this)->id.c_str();
	}

	UDT *BP::Node::udt() const
	{
		return ((NodePrivate*)this)->udt;
	}

	int BP::Node::input_count() const
	{
		return ((NodePrivate*)this)->inputs.size();
	}

	BP::Input *BP::Node::input(int idx) const
	{
		return ((NodePrivate*)this)->inputs[idx].get();
	}

	int BP::Node::output_count() const
	{
		return ((NodePrivate*)this)->outputs.size();
	}

	BP::Output *BP::Node::output(int idx) const
	{
		return ((NodePrivate*)this)->outputs[idx].get();
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

	BP::Node *BP::add_node(uint hash)
	{
		return ((BPPrivate*)this)->add_node(hash);
	}

	void BP::remove_node(BP::Node *n)
	{
		((BPPrivate*)this)->remove_node((NodePrivate*)n);
	}

	BP::Node *BP::find_node(const char *id) const
	{
		return ((BPPrivate*)this)->find_node(id);
	}

	BP::Item *BP::find_item(const char *adress)
	{
		return ((BPPrivate*)this)->find_item(adress);
	}

	void BP::clear()
	{
		((BPPrivate*)this)->clear();
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

	BP_Vec2 bp_vec2_unused;
}

