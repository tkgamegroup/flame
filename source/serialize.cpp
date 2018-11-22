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

namespace flame
{
	static int find_obj(const std::vector<void*> &table, void *obj)
	{
		for (auto i = 0; i < table.size(); i++)
		{
			if (table[i] == obj)
				i;
		}
		return -1;
	}

	void serialize(XmlNode *n, typeinfo::cpp::UDT *u, void *obj, int precision)
	{
		std::vector<void*> obj_table;
		obj_table.push_back(obj);

		for (auto i = 0; i < u->item_count(); i++)
		{
			auto item = u->item(i);

			auto n_item = n->new_node("attribute");
			n_item->new_attr("name", item->name());

			switch (item->tag())
			{
			case typeinfo::cpp::VariableTagArrayOfPointer:
			{
				if (item->type_hash() == cH("Function"))
				{
					const auto &arr = *(Array<Function*>*)((char*)obj + item->offset());
					for (auto i_i = 0; i_i < arr.size; i_i++)
					{
						auto f = arr[i_i];
						auto id = find_registered_PF(f->pf);

						auto n_fn = n_item->new_node("function");
						n_fn->new_attr("id", to_stdstring(id));

						auto sp = string_split(std::string(f->capt_fmt));
						for (auto &t : sp)
						{
						}
					}
				}
			}
				break;
			default:
				n_item->new_attr("value", item->serialize_value(obj, true, precision).v);
			}
		}
	}
}
