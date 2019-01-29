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

using namespace flame;

int main(int argc, char **args)
{
	std::wstring filename;

	typeinfo_load(L"typeinfo.xml");

	BP *bp = nullptr;
	if (argc > 1)
	{
		filename = s2w(args[1]);
		bp = BP::create_from_file(filename.c_str());
		if (!bp)
			filename = L"";
	}

	if (!bp)
		bp = BP::create();
	if (filename != L"")
		printf("\"%s\":\n", w2s(filename).c_str());
	else
		printf("\"unnamed\":\n");

	while (true)
	{
		char command_line[260];
		scanf("%s", command_line);
		auto s_command_line = std::string(command_line);
		if (s_command_line == "help")
		{
			printf(
				"	help - show this help\n"
				"	show udts - show all available udts (see blueprint.h for more details)\n"
				"	show udt [udt_name] - show an udt\n"
				"	show nodes - show all nodes\n"
				"	show node [id] - show a node\n"
				"	add node [id1,id2...] [udt_name] - add a node (id of '-' means don't care)\n"
				"	add link [out_adress] [in_adress] - add a link (e.g. add link a.b c.d.0)\n"
				"	remove node [id] - remove a node\n"
				"	remove link [in_adress] - remove a link (e.g. remove link c.d.0)\n"
				"	set [in_adress] [value] - set value for item (e.g. set a.b.0 46)\n"
				"	save [filename] - save this blueprint (you don't need filename while this blueprint already having a filename)\n"
			);
		}
		else if (s_command_line == "show")
		{
			scanf("%s", command_line);
			auto s_what = std::string(command_line);

			if (s_what == "udts")
			{
				auto udts = get_udts();
				for (auto i_u = 0; i_u < udts.size; i_u++)
				{
					auto udt = udts[i_u];
					auto input_count = 0;
					auto output_count = 0;
					for (auto i_i = 0; i_i < udt->item_count(); i_i++)
					{
						auto item = udt->item(i_i);
						auto attribute = std::string(item->attribute());
						if (attribute.find('i') != std::string::npos)
							input_count++;
						if (attribute.find('o') != std::string::npos)
							output_count++;
					}
					if (input_count > 0 && output_count > 0)
						printf("%s\n", udt->name());
				}
			}
			else if (s_what == "udt")
			{
				scanf("%s", command_line);
				auto s_name = std::string(command_line);

				auto udt = find_udt(H(s_name.c_str()));
				if (udt)
				{
					printf("%s:\n", udt->name());
					std::vector<VaribleInfo*> inputs;
					std::vector<VaribleInfo*> outputs;
					for (auto i_i = 0; i_i < udt->item_count(); i_i++)
					{
						auto item = udt->item(i_i);
						auto attribute = std::string(item->attribute());
						if (attribute.find('i') != std::string::npos)
							inputs.push_back(item);
						if (attribute.find('o') != std::string::npos)
							outputs.push_back(item);
					}
					printf("[In]\n");
					for (auto &i : inputs)
						printf("name:%s attribute:%s tag:%s type:%s\n", i->name(), i->attribute(), get_variable_tag_name(i->tag()), i->type_name());
					printf("[Out]\n");
					for (auto &i : outputs)
						printf("name:%s attribute:%s tag:%s type:%s\n", i->name(), i->attribute(), get_variable_tag_name(i->tag()), i->type_name());
				}
			}
			else if (s_what == "nodes")
			{
				for (auto i = 0; i < bp->node_count(); i++)
				{
					auto n = bp->node(i);
					printf("id:%s type:%s\n", n->id(), n->udt()->name());
				}
			}
			else if (s_what == "node")
			{
				scanf("%s", command_line);
				auto s_id = std::string(command_line);

				auto n = bp->find_node(s_id.c_str());
				if (n)
				{
					printf("[In]\n");
					for (auto i = 0; i < n->input_count(); i++)
					{
						auto input = n->input(i);
						printf("%s\n", input->varible_info()->name());
						for (auto i_v = 0; i_v < input->array_item_count(); i_v++)
						{
							auto v = input->array_item(i_v);
							std::string link_address;
							if (v->link())
								link_address = v->link()->get_address().v;
							printf("  [%s]->\n", link_address.c_str());
							printf("  %s\n", input->varible_info()->serialize_value(&v->data().v, false, 2).v);
						}
					}
					printf("[Out]\n");
					for (auto i = 0; i < n->output_count(); i++)
					{
						auto output = n->output(i);
						printf("%s\n", output->varible_info()->name());
						/* output has only one item */
						{
							auto v = output->item();
							std::string link_address;
							if (v->link())
								link_address = v->link()->get_address().v;
							printf("  %s\n", output->varible_info()->serialize_value(&v->data().v, false, 2).v);
							printf("  ->[%s]\n", link_address.c_str());
						}
					}
				}
				else
					printf("node not found\n");
			}
			else
				printf("unknow object to show\n");
		}
		else if (s_command_line == "add")
		{
			scanf("%s", command_line);
			auto s_what = std::string(command_line);

			if (s_what == "node")
			{
				scanf("%s", command_line);
				auto s_ids = std::string(command_line);

				scanf("%s", command_line);
				auto s_udt = "BP_" + std::string(command_line);

				auto udt = find_udt(H(s_udt.c_str()));
				if (udt)
				{
					auto ids = string_split(s_ids, ',');
					for (auto &id : ids)
					{
						auto n = bp->add_node(id == "-" ? nullptr : id.c_str(), udt);
						if (!n)
							printf("%s already exist\n", id.c_str());
						else
							printf("node added: %s\n", n->id());
					}
				}
				else
					printf("bad udt name\n");
			}
			else if (s_what == "link")
			{
				scanf("%s", command_line);
				auto s_out_address = std::string(command_line);

				scanf("%s", command_line);
				auto s_in_address = std::string(command_line);

				auto out_item = bp->find_item(s_out_address.c_str());
				auto in_item = bp->find_item(s_in_address.c_str());
				if (out_item && in_item)
				{
					in_item->set_link(out_item);
					printf("link added: %s - %s\n", in_item->link()->get_address().v, in_item->get_address().v);
				}
				else
					printf("address wrong\n");
			}
			else
				printf("unknow object to add\n");
		}
		else if (s_command_line == "remove")
		{
			scanf("%s", command_line);
			auto s_what = std::string(command_line);

			if (s_what == "node")
			{
				scanf("%s", command_line);
				auto s_id = std::string(command_line);

				auto n = bp->find_node(s_id.c_str());
				if (n)
				{
					bp->remove_node(n);
					printf("node removed: %s\n", s_id.c_str());
				}
				else
					printf("node not found\n");
			}
			else if (s_what == "link")
			{
				scanf("%s", command_line);
				auto s_in_address = std::string(command_line);

				auto i = bp->find_item(s_in_address.c_str());
				if (i)
				{
					i->set_link(nullptr);
					printf("link removed: %s\n", s_in_address.c_str());
				}
				else
					printf("item not found\n");
			}
			else
				printf("unknow object to remove\n");
		}
		else if (s_command_line == "set")
		{
			scanf("%s", command_line);
			auto s_address = std::string(command_line);

			scanf("%s", command_line);
			auto s_value = std::string(command_line);

			auto i = bp->find_item(s_address.c_str());
			if (i)
			{
				VaribleInfo* v;
				if (i->parent_i())
					v = i->parent_i()->varible_info();
				else if (i->parent_o())
					v = i->parent_o()->varible_info();
				auto value_before = v->serialize_value(&i->data().v, false, 2);
				v->unserialize_value(s_value, &i->data().v, false);
				auto value_after = v->serialize_value(&i->data().v, false, 2);
				printf("set value: %s, %s -> %s\n", s_address.c_str(), value_before.v, value_after.v);
			}
			else
				printf("item not found\n");
		}
		else if (s_command_line == "update")
		{
			bp->install_dummys();
			bp->update();
			bp->uninstall_dummys();
			printf("BP updated\n");
		}
		else if (s_command_line == "save")
		{
			if (filename != L"")
			{
				bp->save(filename.c_str());
				printf("file saved\n");
			}
			else
			{
				scanf("%s", command_line);
				auto s_filename = std::string(command_line);

				if (!std::filesystem::exists(s_filename))
				{
					bp->save(filename.c_str());
					filename = s2w(s_filename);
					printf("file saved\n");
					printf("%s:\n", s_filename.c_str());
				}
				else
					printf("filename taken\n");
			}
		}
		else
			printf("unknow command\n");
	}

	return 0;
}
