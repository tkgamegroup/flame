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
	BP::Node *BP::add_node(uint hash)
	{
		auto n = new Node;
		auto u = find_udt(hash);
		n->udt = u;

		for (auto i = 0; i < u->item_count(); i++)
		{
			auto item = u->item(i);
			switch (item->tag())
			{
			case VariableTagEnumSingle:
			{
				auto ie = new ItemEnum;
				ie->name = item->name();
				ie->type = ItemTypeEnum;
				ie->e = find_enum(item->type_hash());
				ie->v = item->default_value().v.i[0];
				n->items.push_back(ie);
			}
				break;
			case VariableTagEnumMulti:
				break;
			case VariableTagVariable:
			{
				auto iv = new ItemVarible;
				iv->name = item->name();
				iv->type = ItemTypeVariable;
				iv->v = item;
				iv->d = item->default_value();
				n->items.push_back(iv);
			}
				break;
			case VariableTagArrayOfPointer:
				break;
			}
		}

		nodes.push_back(n);

		return n;
	}

	void BP::save(const wchar_t *filename)
	{
		auto file = SerializableNode::create("BP");

		for (auto i = 0; i < nodes.size; i++)
		{
			auto n = nodes[i];
			auto u = n->udt;

			auto n_nd = file->new_node("node");
			n_nd->new_attr("id", n->id.v);
			n_nd->new_attr("type", u->name());
			for (auto i_i = 0; i_i < n->items.size; i_i++)
			{
				auto item = n->items[i_i];
				auto n_it = n_nd->new_node("item");
				n_it->new_attr("name", item->name.v);

				switch (item->type)
				{
				case ItemTypeEnum:
					break;
				case ItemTypeVariable:
					break;
				case ItemTypeArrayOfPointer:
					break;
				}
			}
		}

		file->save_xml(filename);
		SerializableNode::destroy(file);
	}

	BP *BP::create()
	{
		return new BP;
	}

	BP *BP::create_from_file(const wchar_t *filename)
	{
		std::vector<std::pair<Node**, std::string>> defer_links;

		auto file = SerializableNode::create_from_xml(filename);

		for (auto i_n = 0; i_n < file->node_count(); i_n++)
		{
			auto n_nd = file->node(i_n);
			if (n_nd->name() == "node")
			{
				auto id = n_nd->find_attr("id")->value();
				auto type = n_nd->find_attr("type")->value();
				auto n = new Node;
				auto u = find_udt(H(type.c_str()));
				n->udt = u;
				n->id = id;

				//for (auto i_s = 0; i_s < n_nd->node_count(); i_s++)
				//{
				//	auto n_sl = n_nd->node(i_s);
				//	if (n_sl->name() == "slot")
				//	{
				//		auto name = n_sl->find_attr("name")->value();
				//		auto pos = n->u->find_item_i(name.c_str());

				//		for (auto i_i = 0; i_i < n_sl->node_count(); i_i++)
				//		{
				//			auto n_it = n_sl->node(i_i);
				//			if (n_it->name() == "item")
				//			{
				//				auto a_ln = n_it->find_attr("link");
				//				auto a_vl = n_it->find_attr("value");

				//				if (a_ln)
				//					defer_links.emplace_back(n->insls[pos]->items[i_i].get(), a_ln->value());
				//				else if (a_vl)
				//					n->insls[pos]->items[i_i]->d;
				//			}
				//		}
				//	}
				//}

				//nodes.emplace_back(n);
			}
		}

		//for (auto &l : defer_links)
		//{
		//	Node *node = nullptr;
		//	for (auto &n : nodes)
		//	{
		//		if (n->id == l.second)
		//		{
		//			node = n.get();
		//			break;
		//		}
		//	}
		//	l.first->n = node;
		//}

		SerializableNode::destroy(file);

		return nullptr;
	}

	void BP::destroy(BP *s)
	{
		delete s;
	}
}

