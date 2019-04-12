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
#include <flame/network/network.h>

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

	network_init();

	while (true)
	{
		char command_line[260];
		scanf("%s", command_line);
		auto s_command_line = std::string(command_line);
		if (s_command_line == "help")
		{
			printf(
				"  help - show this help\n"
				"  show udts - show all available udts (see blueprint.h for more details)\n"
				"  show udt [udt_name] - show an udt\n"
				"  show nodes - show all nodes\n"
				"  show node [id] - show a node\n"
				"  add node [id1,id2...] [udt_name] - add a node (id of '-' means don't care)\n"
				"  add link [out_adress] [in_adress] - add a link\n"
				"  add item [in_adress] - add an item to input\n"
				"  remove node [id] - remove a node\n"
				"  remove link [in_adress] - remove a link\n"
				"  remove item [in_adress] - remove an item from input\n"
				"  set [in_adress] [value] - set value for item\n"
				"  update - update this blueprint\n"
				"  save [filename] - save this blueprint (you don't need filename while this blueprint already having a filename)\n"
				"  tobin - generate code to a dll\n"
				"  gui-browser - use the power of browser to show and edit\n"
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
					auto name = std::string(udt->name());
					if (name.find("BP_") == 0)
						printf("%s\n", udt->name());
				}
			}
			else if (s_what == "udt")
			{
				scanf("%s", command_line);
				auto s_name = "BP_" + std::string(command_line);

				auto udt = find_udt(H(s_name.c_str()));
				if (udt)
				{
					printf("%s:\n", udt->name());
					std::vector<VariableInfo*> inputs;
					std::vector<VariableInfo*> outputs;
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
						printf(" name:%s attribute:%s tag:%s type:%s\n", i->name(), i->attribute(), get_type_tag_name(i->type()->tag()), i->type()->name());
					printf("[Out]\n");
					for (auto &i : outputs)
						printf(" name:%s attribute:%s tag:%s type:%s\n", i->name(), i->attribute(), get_type_tag_name(i->type()->tag()), i->type()->name());
				}
				else
					printf("udt not found\n");
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
						printf(" %s\n", input->variable_info()->name());
						if (input->array_item_count() > 0)
						{
							for (auto i_v = 0; i_v < input->array_item_count(); i_v++)
							{
								auto v = input->array_item(i_v);
								std::string link_address;
								if (v->link())
									link_address = v->link()->get_address().v;
								printf("  %d:\n", i_v);
								printf("   [%s]->\n", link_address.c_str());
								auto str = input->variable_info()->serialize_value(&v->data().v, false, -1, 2);
								if (str.size == 0)
									str = "-";
								printf("   %s\n", str.v);
							}
						}
						else
							printf("  -\n");
					}
					printf("[Out]\n");
					for (auto i = 0; i < n->output_count(); i++)
					{
						auto output = n->output(i);
						printf(" %s\n", output->variable_info()->name());
						/* output has only one item */
						{
							auto v = output->item();
							auto str = output->variable_info()->serialize_value(&v->data().v, false, -1, 2);
							if (str.size == 0)
								str = "-";
							printf("   %s\n", str.v);
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
					printf("wrong address\n");
			}
			else if (s_what == "item")
			{
				scanf("%s", command_line);
				auto s_in_address = std::string(command_line);

				auto sp = string_split(s_in_address, '.');
				if (sp.size() >= 2)
				{
					uint index = 0;
					if (sp.size() >= 3)
						index = std::stoi(sp[2]);

					auto input = bp->find_input((sp[0] + "." + sp[1]).c_str());
					if (input)
					{
						auto item = input->array_insert_item(index);
						printf("item added: %s\n", item->get_address().v);
					}
					else
						printf("input not found");
				}
				else
					printf("wrong address");
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
				VariableInfo* v;
				if (i->parent_i())
					v = i->parent_i()->variable_info();
				else if (i->parent_o())
					v = i->parent_o()->variable_info();
				auto value_before = v->serialize_value(&i->data().v, false, -1, 2);
				v->unserialize_value(s_value, &i->data().v, false, -1);
				auto value_after = v->serialize_value(&i->data().v, false, -1, 2);
				printf("set value: %s, %s -> %s\n", s_address.c_str(), value_before.v, value_after.v);
			}
			else
				printf("item not found\n");
		}
		else if (s_command_line == "update")
		{
			bp->prepare();
			bp->update();
			bp->unprepare();
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
					filename = s2w(s_filename);
					bp->save(filename.c_str());
					printf("file saved\n");
					printf("%s:\n", s_filename.c_str());
				}
				else
					printf("filename taken\n");
			}
		}
		else if (s_command_line == "tobin")
		{
			bp->prepare();
			bp->tobin();
			bp->unprepare();

			printf("code generated\n");
		}
		else if (s_command_line == "gui-browser")
		{
			//exec((std::wstring(L"file:///") + get_curr_path() + L"/bp.html").c_str(), "", false);
			printf("waiting for browser on port 5566 ...");
			auto s = OneClientServerWebSocket::create(5566, 100, Function<void(void*, int, void*)>(
				[](void* c, int len, void* data) {
					;
				}, 0, nullptr));
			if (!s)
				printf("  timeout\n");
			else
			{
				printf("  ok\nbrowser working\n");

				auto json = SerializableNode::create("");
				json->set_array(true);
				for (auto i = 0; i < bp->node_count(); i++)
				{
					auto src = bp->node(i);

					auto n = json->new_node("");
					n->new_attr("name", src->id());
					auto n_inputs = n->new_node("inputs");
					n_inputs->set_array(true);
					for (auto j = 0; j < src->input_count(); j++)
					{
						auto input = src->input(j);
						auto n_input = n_inputs->new_node("");
						n_input->set_object(false);
						n_input->new_attr("", input->variable_info()->name());
					}
					auto n_outputs = n->new_node("outputs");
					for (auto j = 0; j < src->output_count(); j++)
					{
						auto output = src->output(j);
						auto n_output = n_outputs->new_node("");
						n_output->set_object(false);
						n_output->new_attr("", output->variable_info()->name());
					}
					n_outputs->set_array(true);
				}

				auto str = json->to_string_json();
				//auto str = String("123");
				s->send(str.size, str.v);
			}
		}
		else
			printf("unknow command\n");
	}

	return 0;
}
