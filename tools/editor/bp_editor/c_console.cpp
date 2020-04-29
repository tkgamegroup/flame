#include "bp_editor.h"

cConsole::cConsole() :
	Component("cConsole")
{
	utils::next_element_padding = 8.f;
	auto e_page = utils::e_begin_docker_page(L"Console").second;
	{
		auto c_layout = utils::c_layout(LayoutVertical);
		c_layout->item_padding = 4.f;
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;

		e_page->add_component(this);
	}

		utils::e_begin_scrollbar(ScrollbarVertical, true);
			utils::e_begin_layout(LayoutVertical)->get_component(cElement)->clip_flags = ClipChildren;
			utils::c_aligner(AlignMinMax, AlignMinMax);
			c_text_log = utils::e_text(bp_editor.filepath.c_str())->get_component(cText);
			utils::e_end_layout();
		utils::e_end_scrollbar(utils::style(FontSize).u.x());

		utils::e_button(L"Clear", [](Capture& c) {
			bp_editor.console->c_text_log->set_text(L"");
		}, Capture());

		utils::e_begin_layout(LayoutHorizontal, 4.f);
		utils::c_aligner(AlignMinMax, 0);
			c_edit_input = utils::e_edit(0.f)->get_component(cEdit);
			utils::e_button(L"Exec", [](Capture& c) {
				auto log_text = bp_editor.console->c_text_log;
				auto log = log_text->text.str();
				auto input_text = bp_editor.console->c_edit_input->text;
				auto cmd = input_text->text.str();
				log += cmd + L"\n";
				input_text->set_text(L"");
				bp_editor.console->c_edit_input->set_select(0);

				auto tokens = SUW::split(cmd);

				if (tokens[0] == L"help")
				{
					log +=
						L"  help - show this help\n"
						"  show udts - show all available udts (see blueprint.h for more details)\n"
						"  show udt [udt_name] - show an udt\n"
						"  show nodes - show all nodes\n"
						"  show node [id] - show a node\n"
						"  show graph - use Graphviz to show graph\n"
						"  add node [type_name] [id] - add a node (id of '-' means don't care)\n"
						"  add link [out_adress] [in_adress] - add a link\n"
						"  remove node [id] - remove a node\n"
						"  remove link [in_adress] - remove a link\n"
						"  set [in_adress] [value] - set value for input\n"
						"  update - update this blueprint\n"
						"  save - save this blueprint\n"
						"  auto-set-layout - set nodes' positions using 'bp.png' and 'bp.graph.txt', need do show graph first\n";;
				}
				else if (tokens[0] == L"show")
				{
					if (tokens[1] == L"udts")
					{
						std::vector<UdtInfo*> all_udts;
						for (auto i = 0; i < global_db_count(); i++)
						{
							auto udts = global_db(i)->get_udts();
							for (auto i = 0; i < udts.s; i++)
								all_udts.push_back(udts.v[i]);
						}
						std::sort(all_udts.begin(), all_udts.end(), [](UdtInfo* a, UdtInfo* b) {
							return std::string(a->name()) < std::string(b->name());
						});
						for (auto udt : all_udts)
							log += s2w(udt->name()) + L"\n";
					}
					else if (tokens[1] == L"udt")
					{
						auto udt = find_udt(FLAME_HASH(w2s(tokens[2]).c_str()));
						if (udt)
						{
							log += s2w(udt->name()) + L"\n";
							std::vector<VariableInfo*> inputs;
							std::vector<VariableInfo*> outputs;
							for (auto i_i = 0; i_i < udt->variable_count(); i_i++)
							{
								auto vari = udt->variable(i_i);
								auto flags = vari->flags();
								if (flags & VariableFlagInput)
									inputs.push_back(vari);
								else if (flags & VariableFlagOutput)
									outputs.push_back(vari);
							}
							log += L"[In]\n";
							for (auto& i : inputs)
								log += wfmt(L"name:%s flags:%d type:%s", s2w(i->name()).c_str(), i->flags(), s2w(i->type()->name()).c_str()) + L"\n";
							log += L"[Out]\n";
							for (auto& o : outputs)
								log += wfmt(L"name:%s flags:%d type:%s", s2w(o->name()).c_str(), o->flags(), s2w(o->type()->name()).c_str()) + L"\n";
						}
						else
							log += L"udt not found\n";
					}
					else if (tokens[1] == L"nodes")
					{
						for (auto i = 0; i < bp_editor.bp->node_count(); i++)
						{
							auto n = bp_editor.bp->node(i);
							log += wfmt(L"id:%s type:%s", s2w(n->id()).c_str(), s2w(n->udt()->name()).c_str()) + L"\n";
						}
					}
					else if (tokens[1] == L"node")
					{
						auto n = bp_editor.bp->find_node(w2s(tokens[2]).c_str());
						if (n)
						{
							log += L"[In]\n";
							for (auto i = 0; i < n->input_count(); i++)
							{
								auto input = n->input(i);
								auto type = input->type();
								log += s2w(input->name()) + L"\n";
								std::string link_address;
								if (input->link())
									link_address = input->link()->get_address().str();
								log += wfmt(L"[%s]", s2w(link_address).c_str()) + L"\n";
								auto str = s2w(type->serialize(input->data()));
								if (str.empty())
									str = L"-";
								log += wfmt(L"   %s", str.c_str()) + L"\n";
							}
							log += L"[Out]\n";
							for (auto i = 0; i < n->output_count(); i++)
							{
								auto output = n->output(i);
								auto type = output->type();
								log += s2w(output->name()) + L"\n";
								auto str = s2w(type->serialize(output->data()).c_str());
								if (str.empty())
									str = L"-";
								log += wfmt(L"   %s", str.c_str()) + L"\n";
							}
						}
						else
							log += L"node not found\n";
					}
					else if (tokens[1] == L"graph")
					{
						if (!bp_editor.generate_graph_image())
						{
							exec(L"bp_editor.bp.png", L"", false);
							log += L"ok\n";
						}
						else
							log += L"bp_editor.bp.png not found, perhaps Graphviz is not available\n";
					}
					else
						log += L"unknow object to show\n";
				}
				else if (tokens[0] == L"add")
				{
					if (tokens[1] == L"node")
					{
						NodeDesc d;
						d.id = tokens[3] == L"-" ? "" : w2s(tokens[3]);
						d.type = w2s(tokens[2]);
						d.object_type = BP::ObjectReal;
						d.pos = 0.f;
						auto n = bp_editor.add_node(d);
						if (n)
						{
							log += wfmt(L"node added: %s", s2w(n->id()).c_str()) + L"\n";
						}
						else
							log += L"bad udt name or id already exist\n";
					}
					else if (tokens[1] == L"link")
					{
						auto out = bp_editor.bp->find_output(w2s(tokens[2]).c_str());
						auto in = bp_editor.bp->find_input(w2s(tokens[3]).c_str());
						if (out && in)
						{
							bp_editor.set_links({ {in, out} });
							log += wfmt(L"link added: %s -> %s", s2w(out->get_address().str()).c_str(), s2w(in->get_address().str()).c_str()) + L"\n";
						}
						else
							log += L"wrong address\n";
					}
					else
						log += L"unknow object to add\n";
				}
				else if (tokens[0] == L"remove")
				{
					if (tokens[1] == L"node")
					{
						auto n = bp_editor.bp->find_node(w2s(tokens[2]).c_str());
						if (n)
						{
							bp_editor.remove_nodes({ n });
							log += wfmt(L"node removed: %s", tokens[2].c_str()) + L"\n";
						}
						else
							log += L"node not found\n";
					}
					else if (tokens[1] == L"link")
					{
						auto in = bp_editor.bp->find_input(w2s(tokens[2]).c_str());
						if (in)
						{
							bp_editor.set_links({ {in, nullptr} });
							log += wfmt(L"link removed: %s", tokens[2].c_str()) + L"\n";
						}
						else
							log += L"input not found\n";
					}
					else
						log += L"unknow object to remove\n";
				}
				else if (tokens[0] == L"set")
				{
					auto i = bp_editor.bp->find_input(w2s(tokens[1]).c_str());
					if (i)
					{
						auto type = i->type();
						if (type->tag() != TypePointer)
						{
							auto value_before = type->serialize(i->data());
							auto data = new char[i->size()];
							type->unserialize(w2s(tokens[2]), data);
							bp_editor.set_data(i, data, false);
							delete[] data;
							auto value_after = type->serialize(i->data());
							log += L"set value: " + tokens[1] + L", " + s2w(value_before) + L" -> " + s2w(value_after) + L"\n";
						}
						else
							log += L"cannot set pointer type\n";
					}
					else
						log += L"input not found\n";
				}
				else if (tokens[0] == L"update")
				{
					bp_editor.bp->update();
					log += L"BP updated\n";
				}
				else if (tokens[0] == L"save")
				{
					bp_editor.save();
					log += L"file saved\n";
				}
				else if (tokens[0] == L"auto-set-layout")
				{
					if (bp_editor.auto_set_layout())
						log += L"ok\n";
					else
						log += L"bp_editor.bp.graph.txt not found\n";
				}
				else
					log += L"unknow command\n";

				log_text->set_text(log.c_str());

			}, Capture());
		utils::e_end_layout();

	utils::e_end_docker_page();
}

cConsole::~cConsole()
{
	bp_editor.console = nullptr;
}
