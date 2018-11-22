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

						auto sp = string_split(std::string(f->capt_fmt));
						auto d = f->datas + f->para_cnt;
						for (auto &t : sp)
						{
							auto n_cpt = n_fn->new_node("capture");
							n_cpt->new_attr("type", t);
							auto a_vl = n_cpt->new_attr("value", "");
							std::string str;

							if (t == "i")
								str = to_stdstring(d->i[0]);
							else if (t == "i2")
								str = to_stdstring(Ivec2(d->i));
							else if (t == "i3")
								str = to_stdstring(Ivec3(d->i));
							else if (t == "i4")
								str = to_stdstring(Ivec4(d->i));
							else if (t == "f")
								str = to_stdstring(d->f[0]);
							else if (t == "f2")
								str = to_stdstring(Vec2(d->f));
							else if (t == "f3")
								str = to_stdstring(Vec3(d->f));
							else if (t == "f4")
								str = to_stdstring(Vec4(d->f));
							else if (t == "b")
								str = to_stdstring((int)d->b[0]);
							else if (t == "b2")
								str = to_stdstring(Bvec2(d->b));
							else if (t == "b3")
								str = to_stdstring(Bvec3(d->b));
							else if (t == "b4")
								str = to_stdstring(Bvec4(d->b));
							else if (t == "p")
								str = to_stdstring(find_obj(obj_table, d->p));
							else
								assert(0);

							a_vl->set_value(str);
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

							if (t == "i")
								d->i[0] = stoi1(str);
							else if (t == "i2")
								*(Ivec2*)d->i = stoi2(str);
							else if (t == "i3")
								*(Ivec3*)d->i = stoi3(str);
							else if (t == "i4")
								*(Ivec4*)d->i = stoi4(str);
							else if (t == "f")
								d->f[0] = stof1(str);
							else if (t == "f2")
								*(Vec2*)d->f = stof2(str);
							else if (t == "f3")
								*(Vec3*)d->f = stof3(str);
							else if (t == "f4")
								*(Vec4*)d->f = stof4(str);
							else if (t == "b")
								d->b[0] = stob1(str);
							else if (t == "b2")
								*(Bvec2*)d->b = stob2(str);
							else if (t == "b3")
								*(Bvec3*)d->b = stob3(str);
							else if (t == "b4")
								*(Bvec4*)d->b = stob4(str);
							else if (t == "p")
								d->p = obj_table[stoi(str)];
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
