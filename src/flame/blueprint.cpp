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

#include "blueprint.h"
#include "typeinfo.h"
#include "file.h"
#include "serialize.h"

#include <vector>
#include <string>
#include <memory>
#include <assert.h>

namespace flame
{
	namespace blueprint
	{
		struct InSlotPrivate : InSlot
		{
			std::vector< std::unique_ptr<InSlotItem>> items;

			inline InSlotPrivate()
			{
				items.emplace_back(new InSlotItem);
			}
		};

		int InSlot::item_count() const
		{
			return ((InSlotPrivate*)this)->items.size();
		}

		InSlotItem *InSlot::item(int idx) const
		{
			return ((InSlotPrivate*)this)->items[idx].get();
		}

		struct NodePrivate : Node
		{
			Type type;
			std::string id;
		};

		Node::Type Node::type() const
		{
			return ((NodePrivate*)this)->type;
		}

		const char *Node::id() const
		{
			return ((NodePrivate*)this)->id.c_str();
		}

		struct NodeInputEnumSinglePrivate : NodePrivate
		{
			typeinfo::cpp::EnumType *enumeration;
			int idx;

			inline NodeInputEnumSinglePrivate(typeinfo::cpp::EnumType *e)
			{
				type = TypeInputEnumSingle;
				enumeration = e;
			}

			inline void set_idx(int _idx)
			{
				idx = _idx;
			}

			inline void set_value(int value)
			{
				idx = enumeration->find_item(value);
			}
		};

		typeinfo::cpp::EnumType *NodeInputEnumSingle::enumeration() const
		{
			return ((NodeInputEnumSinglePrivate*)this)->enumeration;
		}

		int NodeInputEnumSingle::idx() const
		{
			return ((NodeInputEnumSinglePrivate*)this)->idx;
		}

		struct NodeUDTPrivate : NodePrivate
		{
			typeinfo::cpp::UDT *udt;
			std::vector< std::unique_ptr<InSlotPrivate>> insls;
			std::vector< std::unique_ptr<OutSlot>> outsls;

			inline NodeUDTPrivate(typeinfo::cpp::UDT *u)
			{
				type = TypeUDT;
				udt = u;
				insls.resize(udt->item_count());
				for (auto i = 0; i < insls.size(); i++)
					insls[i].reset(new InSlotPrivate);
				outsls.resize(1);
				outsls[0].reset(new OutSlot);
			}
		};

		typeinfo::cpp::UDT *NodeUDT::udt() const
		{
			return ((NodeUDTPrivate*)this)->udt;
		}

		int NodeUDT::insl_count() const
		{
			return ((NodeUDTPrivate*)this)->insls.size();
		}

		InSlot *NodeUDT::insl(int idx) const
		{
			return ((NodeUDTPrivate*)this)->insls[idx].get();
		}

		int NodeUDT::outsl_count() const
		{
			return ((NodeUDTPrivate*)this)->outsls.size();
		}

		OutSlot *NodeUDT::outsl(int idx) const
		{
			return ((NodeUDTPrivate*)this)->outsls[idx].get();
		}

		struct ScenePrivate : Scene
		{
			std::vector<std::unique_ptr<NodePrivate>> nodes;

			inline ScenePrivate()
			{
			}

			inline ScenePrivate(const wchar_t *filename)
			{
				std::vector<std::pair<InSlotItem*, std::string>> defer_links;

				auto xml = XmlFile::create_from_file(filename);
				auto rn = xml->root_node;

				for (auto i_n = 0; i_n < rn->node_count(); i_n++)
				{
					auto n_nd = rn->node(i_n);
					if (n_nd->name() == "node_input_enum_single")
					{
						auto id = n_nd->find_attr("id")->value();
						auto type = n_nd->find_attr("type")->value();
						auto n = new NodeInputEnumSinglePrivate(typeinfo::cpp::find_enumeration(H(type.c_str())));
						n->id = id;

						auto n_it = n_nd->node(0);
						if (n_it->name() == "item")
						{
							auto a_vl = n_it->find_attr("value");
							if (a_vl)
								n->set_idx(n->enumeration->find_item(a_vl->value().c_str()));
						}
					}
					else if (n_nd->name() == "node_udt")
					{
						auto id = n_nd->find_attr("id")->value();
						auto type = n_nd->find_attr("type")->value();
						auto n = new NodeUDTPrivate(typeinfo::cpp::find_udt(H(type.c_str())));
						n->id = id;

						for (auto i_s = 0; i_s < n_nd->node_count(); i_s++)
						{
							auto n_sl = n_nd->node(i_s);
							if (n_sl->name() == "slot")
							{
								auto name = n_sl->find_attr("name")->value();
								auto pos = n->udt->find_item_i(name.c_str());

								for (auto i_i = 0; i_i < n_sl->node_count(); i_i++)
								{
									auto n_it = n_sl->node(i_i);
									if (n_it->name() == "item")
									{
										auto a_ln = n_it->find_attr("link");
										auto a_vl = n_it->find_attr("value");

										if (a_ln)
											defer_links.emplace_back(n->insls[pos]->items[i_i].get(), a_ln->value());
										else if (a_vl)
											n->insls[pos]->items[i_i]->d;
									}
								}
							}
						}

						nodes.emplace_back(n);
					}
				}

				for (auto &l : defer_links)
				{
					Node *node = nullptr;
					for (auto &n : nodes)
					{
						if (n->id == l.second)
						{
							node = n.get();
							break;
						}
					}
					l.first->n = node;
				}

				XmlFile::destroy(xml);
			}

			inline Node *add_node_input_enum_single(uint enum_hash, int value)
			{
				auto n = new NodeInputEnumSinglePrivate(typeinfo::cpp::find_enumeration(enum_hash));
				n->id = "Node " + to_stdstring((int)nodes.size());

				n->set_value(value);

				nodes.emplace_back(n);

				return n;
			}

			inline Node *add_node_udt(uint hash)
			{
				auto n = new NodeUDTPrivate(typeinfo::cpp::find_udt(hash));
				n->id = "Node " + to_stdstring((int)nodes.size());

				nodes.emplace_back(n);

				return n;
			}

			inline void save(const wchar_t *filename)
			{
				auto xml = XmlFile::create("BP");
				auto rn = xml->root_node;

				for (auto &_n : nodes)
				{
					switch (_n->type)
					{
					case Node::TypeInputEnumSingle:
					{
						auto n = (NodeInputEnumSinglePrivate*)_n.get();
						auto e = n->enumeration;

						auto n_nd = rn->new_node("node_input_enum_single");
						n_nd->new_attr("id", n->id);
						n_nd->new_attr("type", e->name());

						auto n_it = n_nd->new_node("item");
						n_it->new_attr("value", e->serialize_value(true, n->idx).v);
					}
						break;
					case Node::TypeUDT:
					{
						auto n = (NodeUDTPrivate*)_n.get();
						auto u = n->udt;

						auto n_nd = rn->new_node("node_udt");
						n_nd->new_attr("id", n->id);
						n_nd->new_attr("type", u->name());
						for (auto i_s = 0; i_s < u->item_count(); i_s++)
						{
							auto u_item = u->item(i_s);
							auto sl = n->insls[i_s].get();
							auto n_sl = n_nd->new_node("slot");
							n_sl->new_attr("name", u_item->name());

							for (auto &item : sl->items)
							{
								auto n_it = n_sl->new_node("item");
								if (item->n)
									n_it->new_attr("link", ((NodePrivate*)(item->n))->id);
								else
									n_it->new_attr("value", u_item->serialize_value(&item->d, false).v);
							}
						}
					}
						break;
					}
				}

				xml->save(filename);
				XmlFile::destroy(xml);
			}
		};

		Node *Scene::add_node_input_enum_single(uint enum_hash, int value) 
		{
			return ((ScenePrivate*)this)->add_node_input_enum_single(enum_hash, value);
		}

		Node *Scene::add_node_udt(uint hash)
		{
			return ((ScenePrivate*)this)->add_node_udt(hash);
		}

		void Scene::save(const wchar_t *filename)
		{
			((ScenePrivate*)this)->save(filename);
		}

		Scene *Scene::create()
		{
			return new ScenePrivate;
		}

		Scene *Scene::create_from_file(const wchar_t *filename)
		{
			return new ScenePrivate(filename);
		}

		void Scene::destroy(Scene *s)
		{
			delete (ScenePrivate*)s;
		}
/*
		struct ScenePrivate : Scene
		{
			std::vector<std::unique_ptr<NodePrivate>> ns;
			std::vector<std::unique_ptr<Link>> ls;

			inline SlotTemplate *load_slot(XmlNode *src)
			{
				auto dst = new SlotTemplate;

				for (auto i = 0; i < src->attr_count(); i++)
				{
					auto a = src->attr(i);
					switch (H(a->name().c_str()))
					{
					case cH("link"):
						switch (H(a->value().c_str()))
						{
						case cH("single"):
							dst->link_type = SlotLinkSingle;
							break;
						case cH("multi"):
							dst->link_type = SlotLinkMulti;
							break;
						}
						break;
					case cH("name"):
						dst->name = a->value();
						break;
					case cH("enum_name"):
						dst->s_val = a->value().c_str();
						break;
					case cH("default"):
						switch (dst->type)
						{
						case SlotTypeInt:
							dst->i_val[0] = stoi1(a->value());
							break;
						case SlotTypeInt2:
							dst->i_val = Ivec4(stoi2(a->value()), 0, 0);
							break;
						case SlotTypeInt3:
							dst->i_val = Ivec4(stoi3(a->value()), 0);
							break;
						case SlotTypeInt4:
							dst->i_val = stoi4(a->value());
							break;
						case SlotTypeFloat:
							dst->i_val[0] = stof1(a->value());
							break;
						case SlotTypeFloat2:
							dst->f_val = Vec4(stof2(a->value()), 0, 0);
							break;
						case SlotTypeFloat3:
							dst->f_val = Vec4(stof3(a->value()), 0);
							break;
						case SlotTypeFloat4:
							dst->f_val = stof4(a->value());
							break;
						case SlotTypeBool:
							dst->b_val = (a->value() == "true");
							break;
						case SlotTypeStr: case SlotTypeFile:
							dst->s_val = a->value().c_str();
							break;
						case SlotTypeSingle:
						{
							auto e = typeinfo::cpp::find_enumeration(H(dst->s_val.data));
							auto idx = e->find_item(a->value().c_str());
							if (idx != -1)
								dst->i_val[0] = e->item(idx)->value();
						}
							break;
						case SlotTypeMulti:
						{
							auto sp = string_split(a->value(), ';');
							auto v = 0;
							auto e = typeinfo::cpp::find_enumeration(H(dst->s_val.data));
							for (auto &s : sp)
							{
								auto idx = e->find_item(s.c_str());
								if (idx != -1)
									v |= e->item(idx)->value();
							}
							dst->i_val[0] = v;
						}
							break;
						}
					}
				}

				return dst;
			}

			inline Node *add_node_udt(const char *name)
			{
				auto nt = find_template(name);
				if (!nt)
					return nullptr;

				auto n = new NodePrivate(nt);
				n->s = this;

				ns.emplace_back(n);
				return n;
			}

			inline void remove_node(Node *n)
			{
				for (auto &s : ((NodePrivate*)n)->inputslots)
				{
					for (auto l : s->links)
					{
						auto &ls = ((SlotPrivate*)l->out_slot)->links;
						for (auto it = ls.begin(); it != ls.end();)
						{
							if ((*it)->in_slot->node() == n)
								it = ls.erase(it);
							else
								it++;
						}
					}
				}
				for (auto &s : ((NodePrivate*)n)->outputslots)
				{
					for (auto l : s->links)
					{
						auto &ls = ((SlotPrivate*)l->in_slot)->links;
						for (auto it = ls.begin(); it != ls.end();)
						{
							if ((*it)->out_slot->node() == n)
								it = ls.erase(it);
							else
								it++;
						}
					}
				}
				for (auto it = ls.begin(); it != ls.end();)
				{
					auto &l = *it;
					if (l->out_slot->node() == n || l->in_slot->node() == n)
						it = ls.erase(it);
					else
						it++;
				}
				for (auto it = ns.begin(); it != ns.end();)
				{
					if (it->get() == n)
						it = ns.erase(it);
					else
						it++;
				}
			}

			inline Link *add_link(Slot *out_sl, Slot *in_sl)
			{
				if (out_sl->node() == in_sl->node())
					return nullptr;

				for (auto &l : ls)
				{
					if (l->out_slot == out_sl && l->in_slot == in_sl)
						return nullptr;
				}

				auto l = new Link;
				l->out_slot = out_sl;
				l->in_slot = in_sl;

				((SlotPrivate*)out_sl)->links.push_back(l);
				((SlotPrivate*)in_sl)->links.push_back(l);
				ls.emplace_back(l);

				return l;
			}

			inline void remove_link(Link *l)
			{
				{
					auto &ls = ((SlotPrivate*)l->out_slot)->links;
					for (auto it = ls.begin(); it != ls.end(); it++)
					{
						if ((*it) == l)
						{
							ls.erase(it);
							break;
						}
					}
				}
				{
					auto &ls = ((SlotPrivate*)l->in_slot)->links;
					for (auto it = ls.begin(); it != ls.end(); it++)
					{
						if ((*it) == l)
						{
							ls.erase(it);
							break;
						}
					}
				}
				for (auto it = ls.begin(); it != ls.end(); it++)
				{
					if (it->get() == l)
					{
						ls.erase(it);
						break;
					}
				}
			}

			inline void clear()
			{
				ls.clear();
				ns.clear();
			}

			inline void load(const wchar_t *filename)
			{
				auto xml = XmlFile::create_from_file(filename);
				if (xml)
				{
					clear();

					auto rn = xml->root_node;

					auto n_nodes = rn->find_node("nodes");
					if (n_nodes)
					{
						for (auto i = 0; i < n_nodes->node_count(); i++)
						{
							auto n = n_nodes->node(i);
							if (n->name() != "node")
								continue;

							auto dst = new NodePrivate(find_template(n->find_attr("type_name")->value().c_str()));
							dst->s = this;
							ns.emplace_back(dst);

							dst->name = n->find_attr("name")->value().c_str();

							auto n_insl = n->find_node("input_slots");
							if (n_insl)
							{
								for (auto j = 0; j < n_insl->node_count(); j++)
								{
									auto n = n_insl->node(j);

									auto id = stoi1(n->find_attr("id")->value());
									auto slot = dst->inputslot(id);
									auto value = n->find_attr("value")->value();
									switch (slot->type)
									{
									case SlotTypeInt: case SlotTypeSingle: case SlotTypeMulti:
										slot->i_val[0] = stoi1(value);
										break;
									case SlotTypeInt2:
										slot->i_val = Ivec4(stoi2(value), 0, 0);
										break;
									case SlotTypeFloat:
										slot->f_val[0] = stof1(value);
										break;
									case SlotTypeBool:
										slot->b_val = stoi1(value);
										break;
									case SlotTypeStr:
										slot->s_val = value.c_str();
										break;
									}
								}
							}
						}
					}

					auto n_links = rn->find_node("links");
					if (n_links)
					{
						for (auto i = 0; i < n_links->node_count(); i++)
						{
							auto n = n_links->node(i);
							if (n->name() != "link")
								continue;

							int out_node, out_slot, in_node, in_slot;
							for (auto j = 0; j < n->attr_count(); j++)
							{
								auto a = n->attr(j);

								switch (H(a->name().c_str()))
								{
								case cH("out_node"):
									out_node = stoi1(a->value());
									break;
								case cH("out_slot"):
									out_slot = stoi1(a->value());
									break;
								case cH("in_node"):
									in_node = stoi1(a->value());
									break;
								case cH("in_slot"):
									in_slot = stoi1(a->value());
									break;
								}
							}

							add_link(ns[out_node]->outputslots[out_slot].get(),
								ns[in_node]->inputslots[in_slot].get());
						}
					}

					XmlFile::destroy(xml);
				}
			}

			inline int node_idx(NodePrivate *n) const
			{
				for (auto i = 0; i < ns.size(); i++)
				{
					if (ns[i].get() == n)
						return i;
				}
				return -1;
			}

			inline void save(const wchar_t *filename)
			{
				auto n_nodes = rn->new_node("nodes");
				for (auto &src : ns)
				{
					auto n = n_nodes->new_node("node");
					n->new_attr("type_name", src->type_name.data);
					n->new_attr("name", src->name.data);

					auto n_insl = n->new_node("input_slots");
					for (auto i = 0; i < src->inputslot_count(); i++)
					{
						auto slot = src->inputslot(i);
						auto n = n_insl->new_node("slot");

						switch (slot->type)
						{
						case SlotTypeInt: case SlotTypeSingle: case SlotTypeMulti:
							n->new_attr("id", to_string(i));
							n->new_attr("value", to_string(slot->i_val[0]));
							break;
						case SlotTypeInt2:
							n->new_attr("id", to_string(i));
							n->new_attr("value", to_string(Ivec2(slot->i_val)));
							break;
						case SlotTypeFloat:
							n->new_attr("id", to_string(i));
							n->new_attr("value", to_string(slot->f_val[0]));
							break;
						case SlotTypeBool:
							n->new_attr("id", to_string(i));
							n->new_attr("value", to_string((int)slot->b_val));
							break;
						case SlotTypeStr:
							n->new_attr("id", to_string(i));
							n->new_attr("value", slot->s_val.data);
							break;
						case SlotTypePtr:
							n->new_attr("id", to_string(i));
							n->new_attr("value", "");
							break;
						}
					}

					auto n_outsl = n->new_node("output_slots");
				}

				auto n_links = rn->new_node("links");
				for (auto &src : ls)
				{
					auto n = n_links->new_node("link");
					n->new_attr("out_node", to_string(node_idx((NodePrivate*)src->out_slot->node())));
					n->new_attr("out_slot", to_string(((NodePrivate*)src->out_slot->node())->outsl_idx(src->out_slot)));
					n->new_attr("in_node", to_string(node_idx((NodePrivate*)src->in_slot->node())));
					n->new_attr("in_slot", to_string(((NodePrivate*)src->in_slot->node())->insl_idx(src->in_slot)));
				}
			}
		};

		Node *Scene::add_node_udt(const char *name)
		{
			auto udt = typeinfo::cpp::find_udt(H(name));
			if (!udt)
				return nullptr;

			return ((ScenePrivate*)this)->add_node_udt(name);
		}

		void Scene::remove_node(Node *n)
		{
			((ScenePrivate*)this)->remove_node(n);
		}

		Link *Scene::add_link(Slot *out_sl, Slot *in_sl)
		{
			return ((ScenePrivate*)this)->add_link(out_sl, in_sl);
		}

		void Scene::remove_link(Link *l)
		{
			((ScenePrivate*)this)->remove_link(l);
		}

		void Scene::clear()
		{
			((ScenePrivate*)this)->clear();
		}

		void Scene::load(const wchar_t *filename)
		{
			((ScenePrivate*)this)->load(filename);
		}

		void Scene::save(const wchar_t *filename)
		{

			((ScenePrivate*)this)->save(filename);
		}
		*/
	}
}

