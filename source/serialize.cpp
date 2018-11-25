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

#include <flame/file.h>
#include <flame/array.h>
#include <flame/function.h>
#include <flame/typeinfo.h>
#include <flame/serialize.h>

#include <assert.h>

namespace flame
{
	static int find_obj(const std::vector<void*> &table, void *obj)
	{
		for (auto i = 0; i < table.size(); i++)
		{
			if (table[i] == obj)
				return i;
		}
		return -1;
	}

	void _serialize(const std::vector<void*> &obj_table, XmlNode *n, typeinfo::cpp::UDT *u, void *obj, int precision, void *default_obj)
	{
		for (auto i = 0; i < u->item_count(); i++)
		{
			auto item = u->item(i);

			switch (item->tag())
			{
			case typeinfo::cpp::VariableTagArrayOfPointer:
			{
				const auto &arr = *(Array<void*>*)((char*)obj + item->offset());

				if (arr.size == 0)
					break;

				auto n_item = n->new_node("attribute");
				n_item->new_attr("name", item->name());

				if (item->type_hash() == cH("Function"))
				{
					for (auto i_i = 0; i_i < arr.size; i_i++)
					{
						auto f = (Function*)arr[i_i];
						auto id = find_registered_PF(f->pf);

						auto n_fn = n_item->new_node("function");
						n_fn->new_attr("id", to_stdstring(id));

						auto d = f->datas + f->para_cnt;
						for (auto i = 0; i < f->capt_cnt; i++)
						{
							auto n_cpt = n_fn->new_node("capture");
							auto a_ty = n_cpt->new_attr("type", "");
							auto a_vl = n_cpt->new_attr("value", "");
							std::string ty_str, vl_str;

							if (d->cmp_fmt("f"))
							{
								ty_str = "f";
								vl_str = to_stdstring(d->f1());
							}
							else if (d->cmp_fmt("f2"))
							{
								ty_str = "f2";
								vl_str = to_stdstring(d->f2());
							}
							else if (d->cmp_fmt("f3"))
							{
								ty_str = "f3";
								vl_str = to_stdstring(d->f3());
							}
							else if (d->cmp_fmt("f4"))
							{
								ty_str = "f4";
								vl_str = to_stdstring(d->f4());
							}
							else if (d->cmp_fmt("i"))
							{
								ty_str = "i";
								vl_str = to_stdstring(d->i1());
							}
							else if (d->cmp_fmt("i2"))
							{
								ty_str = "i2";
								vl_str = to_stdstring(d->i2());
							}
							else if (d->cmp_fmt("i3"))
							{
								ty_str = "i3";
								vl_str = to_stdstring(d->i3());
							}
							else if (d->cmp_fmt("i4"))
							{
								ty_str = "i4";
								vl_str = to_stdstring(d->i4());
							}
							else if (d->cmp_fmt("b"))
							{
								ty_str = "b";
								vl_str = to_stdstring((int)d->b1());
							}
							else if (d->cmp_fmt("b2"))
							{
								ty_str = "b2";
								vl_str = to_stdstring(d->b2());
							}
							else if (d->cmp_fmt("b3"))
							{
								ty_str = "b3";
								vl_str = to_stdstring(d->b3());
							}
							else if (d->cmp_fmt("b4"))
							{
								ty_str = "b4";
								vl_str = to_stdstring(d->b4());
							}
							else if (d->cmp_fmt("p"))
							{
								ty_str = "p";
								vl_str = to_stdstring(find_obj(obj_table, d->p()));
							}
							else
								assert(0);

							a_vl->set_value(vl_str);
							d++;
						}
					}
				}
				else
				{
					auto u_sub = typeinfo::cpp::find_udt(item->type_hash());

					if (u_sub)
					{
						for (auto i_i = 0; i_i < arr.size; i_i++)
						{
							auto n_sub = n_item->new_node(u_sub->name());
							_serialize(obj_table, n_sub, u_sub, arr[i_i], precision, default_obj);
						}
					}
				}
			}
				break;
			default:
				if (!default_obj || !item->equal(obj, default_obj))
				{
					auto n_item = n->new_node("attribute");
					n_item->new_attr("name", item->name());

					n_item->new_attr("value", item->serialize_value(obj, true, precision).v);
				}
			}
		}
	}

	void serialize(XmlNode *n, typeinfo::cpp::UDT *u, void *obj, int precision, void *default_obj)
	{
		std::vector<void*> obj_table;
		obj_table.push_back(obj);

		_serialize(obj_table, n, u, obj, precision, default_obj);
	}

	void unserialize(XmlNode *n, typeinfo::cpp::UDT *u, void *obj)
	{
		std::vector<void*> obj_table;
		obj_table.push_back(obj);

		for (auto i = 0; i < n->node_count(); i++)
		{
			auto n_item = n->node(i);

			auto item = u->item(u->find_item_i(n_item->find_attr("name")->value().c_str()));

			switch (item->tag())
			{
			case typeinfo::cpp::VariableTagArrayOfPointer:
			{
				for (auto i_i = 0; i_i < n_item->node_count(); i_i++)
				{
					auto n_i = n_item->node(i_i);

					auto i_name = n_i->name();
					if (i_name == "function")
					{
						auto cpt_cnt = n_i->node_count();
						auto id = stoi(n_i->find_attr("id")->value());
						auto f = Function::create(id, cpt_cnt);

						auto d = f->datas + f->para_cnt;
						for (auto i_c = 0; i_c < cpt_cnt; i_c++)
						{
							auto n_c = n_i->node(i_c);
							auto t = n_c->find_attr("type")->value();
							auto str = n_c->find_attr("value")->value();

							if (t == "f")
							{
								d->set_fmt("f");
								d->f1() = stof1(str);
							}
							else if (t == "f2")
							{
								d->set_fmt("f2");
								d->f2() = stof2(str);
							}
							else if (t == "f3")
							{
								d->set_fmt("f3");
								d->f3() = stof3(str);
							}
							else if (t == "f4")
							{
								d->set_fmt("f4");
								d->f4() = stof4(str);
							}
							else if (t == "i")
							{
								d->set_fmt("i");
								d->i1() = stoi1(str);
							}
							else if (t == "i2")
							{
								d->set_fmt("i2");
								d->i2() = stoi2(str);
							}
							else if (t == "i3")
							{
								d->set_fmt("i3");
								d->i3() = stoi3(str);
							}
							else if (t == "i4")
							{
								d->set_fmt("i4");
								d->i4() = stoi4(str);
							}
							else if (t == "b")
							{
								d->set_fmt("b");
								d->b1() = stob1(str);
							}
							else if (t == "b2")
							{
								d->set_fmt("b2");
								d->b2() = stob2(str);
							}
							else if (t == "b3")
							{
								d->set_fmt("b3");
								d->b3() = stob3(str);
							}
							else if (t == "b4")
							{
								d->set_fmt("b4");
								d->b4() = stob4(str);
							}
							else if (t == "p")
							{
								d->set_fmt("p");
								d->p() = obj_table[stoi(str)];
							}
							else
								assert(0);
							d++;
						}
					}
				}
			}
				break;
			default:
				item->unserialize_value(n_item->find_attr("value")->value(), obj, true);
			}
		}

	}
}
