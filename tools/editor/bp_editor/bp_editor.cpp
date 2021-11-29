#include <flame/universe/ui/reflector.h>

#include "bp_editor.h"

struct Action
{
	wchar_t* name;
	virtual void undo() = 0;
	virtual void redo() = 0;
};

struct Action_ChangeNodeID : Action
{
	Guid guid;
	std::string before_id;
	std::string after_id;

	Action_ChangeNodeID()
	{
		name = L"Change Node ID";
	}

	void undo() override
	{
		auto n = bp_editor.bp->root->find_node(guid);
		if (n)
		{
			n->set_id(before_id.c_str());

			if (bp_editor.detail)
				bp_editor.detail->on_after_select();
		}
	}

	void redo() override
	{
		auto n = bp_editor.bp->root->find_node(guid);
		if (n)
		{
			n->set_id(after_id.c_str());

			if (bp_editor.detail)
				bp_editor.detail->on_after_select();
		}
	}
};

struct Action_MoveNodes : Action
{
	struct Target
	{
		Guid guid;
		vec2 before_pos;
		vec2 after_pos;
	};
	std::vector<Target> targets;

	Action_MoveNodes()
	{
		name = L"Move Nodes";
	}

	void undo() override
	{
		for (auto& t : targets)
		{
			auto n = bp_editor.bp->root->find_node(t.guid);
			if (n)
			{
				n->pos = t.before_pos;
				if (bp_editor.editor)
					bp_editor.editor->on_pos_changed(n);
			}
		}
	}

	void redo() override
	{
		for (auto& t : targets)
		{
			auto n = bp_editor.bp->root->find_node(t.guid);
			if (n)
			{
				n->pos = t.after_pos;
				if (bp_editor.editor)
					bp_editor.editor->on_pos_changed(n);
			}
		}
	}
};

struct Action_AddNode : Action
{
	NodeDesc desc;
	bpSlotIO init_link_io;
	LinkSaving init_link_src;
	LinkSaving init_link_dst;

	Action_AddNode()
	{
		name = L"Add Node";
	}

	void undo() override
	{
		auto n = bp_editor.bp->root->find_node(desc.guid);
		if (n)
			bp_editor._remove_nodes({ n });
	}

	void redo() override
	{
		auto n = bp_editor._add_node(desc.node_type, desc.id.c_str(), desc.type.c_str(), desc.pos);
		n->guid = desc.guid;
	}
};

struct Action_DuplicateNodes : Action
{
	std::vector<Guid> models;
	std::vector<Guid> duplications;

	Action_DuplicateNodes()
	{
		name = L"Duplicate Nodes";
	}

	void undo() override
	{
		std::vector<bpNode*> nodes;
		for (auto& d : duplications)
			nodes.push_back(bp_editor.bp->root->find_node(d));
		bp_editor._remove_nodes(nodes);
	}

	void redo() override
	{
		std::vector<bpNode*> nodes(models.size());
		for (auto i = 0; i < models.size(); i++)
			nodes[i] = bp_editor.bp->root->find_node(models[i]);
		auto new_nodes = bp_editor._duplicate_nodes(nodes);
		for (auto i = 0; i < new_nodes.size(); i++)
			new_nodes[i]->guid = duplications[i];
	}
};

struct Action_RemoveNodes : Action
{
	std::vector<NodeSaving> savings;

	Action_RemoveNodes()
	{
		name = L"Remove Nodes";
	}

	void undo() override
	{
		std::vector<bpNode*> nodes;
		for (auto& s : savings)
		{
			auto n = bp_editor._add_node(s.desc.node_type, s.desc.id.c_str(), s.desc.type.c_str(), s.desc.pos);
			n->guid = s.desc.guid;
			nodes.push_back(n);
		}
		for (auto i = 0; i < savings.size(); i++)
		{
			auto n = nodes[i];
			auto& s = savings[i];

			for (auto i = 0; i < s.inputs.size(); i++)
			{
				auto& src = s.inputs[i];
				auto dst = n->inputs[i];
				auto type = dst->type;
				if (type->tag != TagPointer)
				{
					auto data = new char[dst->size];
					type->unserialize(src.data, data);
					dst->set_data((char*)data);
					delete[] data;
				}

				if (!src.link.slot_name.empty())
					dst->link_to(bp_editor.bp->root->find_node(src.link.node_guid)->find_output(src.link.slot_name.c_str()));
			}
			for (auto i = 0; i < s.outputs.size(); i++)
			{
				auto& src = s.outputs[i];
				auto dst = n->outputs[i];

				for (auto& a : src.links)
				{
					auto n = bp_editor.bp->root->find_node(a.node_guid);
					if (bp_editor.editor)
					{
						looper().add_event([](Capture& c) {
							auto n = c.thiz<bpNode>();
							bp_editor.editor->on_remove_node(n);
							bp_editor.editor->on_add_node(n);
						}, Capture().set_thiz(n));
					}
					n->find_input(a.slot_name.c_str())->link_to(dst);
				}
			}
		}
	}

	void redo() override
	{
		std::vector<bpNode*> nodes;
		for (auto& s : savings)
			nodes.push_back(bp_editor.bp->root->find_node(s.desc.guid));
		bp_editor._remove_nodes(nodes);
	}
};

struct Action_SetLinks : Action
{
	struct LinkDesc
	{
		LinkSaving input;
		LinkSaving before_output_addr;
		LinkSaving after_output_addr;
	};
	std::vector<LinkDesc> link_descs;

	Action_SetLinks()
	{
		name = L"Set Links";
	}

	void undo() override
	{
		for (auto& l : link_descs)
		{
			auto i = bp_editor.bp->root->find_node(l.input.node_guid)->find_input(l.input.slot_name.c_str());
			if (i)
				bp_editor._set_link(i, l.before_output_addr.slot_name.empty() ? nullptr : bp_editor.bp->root->find_node(l.before_output_addr.node_guid)->find_output(l.before_output_addr.slot_name.c_str()));
		}

		bp_editor.window->s_renderer->pending_update = true;
	}

	void redo() override
	{
		for (auto& l : link_descs)
		{
			auto i = bp_editor.bp->root->find_node(l.input.node_guid)->find_input(l.input.slot_name.c_str());
			if (i)
				bp_editor._set_link(i, l.after_output_addr.slot_name.empty() ? nullptr : bp_editor.bp->root->find_node(l.after_output_addr.node_guid)->find_output(l.after_output_addr.slot_name.c_str()));
		}

		bp_editor.window->s_renderer->pending_update = true;
	}
};

struct Action_SetData : Action
{
	Guid node_guid;
	std::string input_name;
	std::string before_data;
	std::string after_data;

	Action_SetData()
	{
		name = L"Set Data";
	}

	void undo() override
	{
		auto s = bp_editor.bp->root->find_node(node_guid)->find_input(input_name.c_str());
		if (s)
		{
			auto data = new char[s->size];
			s->type->unserialize(before_data, data);
			s->set_data((char*)data);
			delete[] data;

			if (bp_editor.editor)
				bp_editor.editor->on_data_changed(s);
		}
	}

	void redo() override
	{
		auto s = bp_editor.bp->root->find_node(node_guid)->find_input(input_name.c_str());
		if (s)
		{
			auto data = new char[s->size];
			s->type->unserialize(after_data, data);
			s->set_data((char*)data);
			delete[] data;

			if (bp_editor.editor)
				bp_editor.editor->on_data_changed(s);
		}
	}
};

static std::vector<std::unique_ptr<Action>> actions;
static auto action_idx = 0;

static void add_action(Action* a)
{
	if (actions.size() > action_idx)
		actions.erase(actions.begin() + action_idx, actions.end());
	actions.emplace_back(a);
	action_idx++;
}

static void undo()
{
	if (action_idx > 0)
	{
		bp_editor.select();

		action_idx--;
		auto a = actions[action_idx].get();
		a->undo();

		if (action_idx == 0)
			bp_editor.set_changed(false);

		bp_editor.e_notification->set_visible(true);
		bp_editor.e_notification->get_component(cText)->set_text((std::wstring(L"Undo ") + a->name).c_str());
		looper().remove_all_events(FLAME_CHASH("hide_notification"));
		looper().add_event([](Capture& c) {
			bp_editor.e_notification->set_visible(false);
		}, Capture(), 1.f, FLAME_CHASH("hide_notification"));
	}
}

static void redo()
{
	if (action_idx < actions.size())
	{
		bp_editor.select();

		auto a = actions[action_idx].get();
		a->redo();
		action_idx++;

		bp_editor.set_changed(true);

		bp_editor.e_notification->set_visible(true);
		bp_editor.e_notification->get_component(cText)->set_text((std::wstring(L"Redo ") + a->name).c_str());
		looper().remove_all_events(FLAME_CHASH("hide_notification"));
		looper().add_event([](Capture& c) {
			bp_editor.e_notification->set_visible(false);
		}, Capture(), 1.f, FLAME_CHASH("hide_notification"));
	}
}

static void duplicate_selected()
{
	if (bp_editor.selected_nodes.empty())
		return;

	auto a = new Action_DuplicateNodes;
	a->models.resize(bp_editor.selected_nodes.size());
	for (auto i = 0; i < bp_editor.selected_nodes.size(); i++)
		a->models[i] = bp_editor.selected_nodes[i]->guid;

	auto new_nodes = bp_editor._duplicate_nodes(bp_editor.selected_nodes);
	bp_editor.select(new_nodes);

	a->duplications.resize(new_nodes.size());
	for (auto i = 0; i < new_nodes.size(); i++)
		a->duplications[i] = new_nodes[i]->guid;

	add_action(a);

	bp_editor.set_changed(true);
}

static void delete_selected()
{
	if (!bp_editor.selected_nodes.empty())
	{
		bp_editor.remove_nodes(bp_editor.selected_nodes);
		bp_editor.selected_nodes.clear();

		if (bp_editor.detail)
			bp_editor.detail->on_after_select();
	}
	if (!bp_editor.selected_links.empty())
	{
		std::vector<std::pair<bpSlot*, bpSlot*>> links(bp_editor.selected_links.size());
		for (auto i = 0; i < bp_editor.selected_links.size(); i++)
			links[i] = { bp_editor.selected_links[i], nullptr };
		bp_editor.set_links(links);
		bp_editor.selected_links.clear();

		bp_editor.set_changed(true);

		bp_editor.window->s_renderer->pending_update = true;
	}
}

static void add_window(pugi::xml_node n)
{
	auto& ui = bp_editor.window->ui;

	auto parent_layout = ui.parents.top()->get_component(cLayout)->type == LayoutHorizontal;
	auto r = n.attribute("r").as_int(1);
	std::string name(n.name());
	if (name == "layout")
	{
		auto t = n.attribute("type").value() == std::string("h") ? LayoutHorizontal : LayoutVertical;
		auto ca = ui.e_begin_docker_layout(t)->get_component(cAligner);
		if (parent_layout)
			ca->width_factor = r;
		else
			ca->height_factor = r;
		for (auto c : n.children())
			add_window(c);
		ui.e_end_docker_layout();
	}
	else if (name == "docker")
	{
		auto ca = ui.e_begin_docker()->get_component(cAligner);
		if (parent_layout)
			ca->width_factor = r;
		else
			ca->height_factor = r;
		auto name = std::string(n.child("page").attribute("name").value());
		if (name == "editor")
			bp_editor.editor = new cBPEditor;
		else if (name == "detail")
			bp_editor.detail = new cDetail;
		else if (name == "preview")
			bp_editor.preview = new cPreview;
		else if (name == "console")
			bp_editor.console = new cConsole;
		ui.e_end_docker();
	}
}

BPEditorWindow::BPEditorWindow(const std::filesystem::path& filename) :
	GraphicsWindow(&app, true, true, "BP Editor", uvec2(300, 200), WindowFrame | WindowResizable, nullptr, true)
{
	bp_editor.window = this;

	canvas->clear_color = vec4(100, 100, 100, 255) / 255.f;

	ui.init(world);
	ui.style_set_to_light();

	bp_editor.e_test = new<Entity>();
	{
		auto ce = cElement::create();
		ce->pos = 150.f;
		ce->size = 100.f;
		ce->pivot = 0.5f;
		ce->color = cvec4(0, 0, 0, 255);
		//cElement::set_linked_object(ce);
		bp_editor.e_test->add_component(ce);
	}
	{
		auto cer = cReceiver::create();
		//cReceiver::set_linked_object(cer);
		bp_editor.e_test->add_component(cer);
	}

	if (!filename.empty())
	{
		bp_editor.filepath = filename;
		bp_editor.fileppath = filename.parent_path();
		bp_editor.bp = bpScene::create_from_file(filename.c_str());

		window->set_title(filename.string().c_str());
	}

	update_event = looper().add_event([](Capture& c) {
		if (bp_editor.auto_update)
			bp_editor.update();
		c._current = INVALID_POINTER;
	}, Capture(), 0.f);

	pugi::xml_document window_layout;
	pugi::xml_node window_layout_root;
	if (window_layout.load_file(L"bp_editor_layout.xml"))
		window_layout_root = window_layout.first_child();

	{
		auto c_receiver = root->get_component(cReceiver);
		c_receiver->key_listeners.add([](Capture& c, KeyStateFlags action, int value) {
			if (is_key_down(action))
			{
				auto ed = c.current<cReceiver>()->dispatcher;
				switch (value)
				{
				case Keyboard_S:
					if (ed->key_states[Keyboard_Ctrl] & KeyStateDown)
						bp_editor.save();
					break;
				case Keyboard_Z:
					if (ed->key_states[Keyboard_Ctrl] & KeyStateDown)
						undo();
					break;
				case Keyboard_Y:
					if (ed->key_states[Keyboard_Ctrl] & KeyStateDown)
						redo();
					break;
				case Keyboard_D:
					if (ed->key_states[Keyboard_Ctrl] & KeyStateDown)
						duplicate_selected();
					break;
				case Keyboard_Del:
					delete_selected();
					break;
				case Keyboard_F2:
					bp_editor.update();
					break;
				case Keyboard_F3:
					bp_editor.c_auto_update->set_checked(true);
					break;
				}
			}
			return true;
		}, Capture());
		s_dispatcher->next_focusing = c_receiver;
	}

	ui.parents.push(root);

	ui.e_begin_layout(LayoutVertical, 0.f, false, false);
	ui.c_aligner(AlignMinMax, AlignMinMax);

	ui.e_begin_menu_bar();
	ui.e_begin_menubar_menu(L"Blueprint");
	ui.e_menu_item((std::wstring(Icon_FLOPPY_O) + L"    Save").c_str(), [](Capture& c) {
		bp_editor.save();
	}, Capture());
	ui.e_end_menubar_menu();
	ui.e_begin_menubar_menu(L"Edit");
	ui.e_menu_item((std::wstring(Icon_UNDO) + L"    Undo").c_str(), [](Capture&) {
		undo();
	}, Capture());
	ui.e_menu_item((std::wstring(Icon_REPEAT) + L"    Redo").c_str(), [](Capture&) {
		redo();
	}, Capture());
	ui.e_menu_item((std::wstring(Icon_CLONE) + L"   Duplicate").c_str(), [](Capture&) {
		duplicate_selected();
	}, Capture());
	ui.e_menu_item((std::wstring(Icon_TIMES) + L"    Delete").c_str(), [](Capture&) {
		delete_selected();
	}, Capture());
	ui.e_end_menubar_menu();
	ui.e_begin_menubar_menu(L"View");
	ui.e_menu_item(L"Editor", [](Capture&) {
		if (!bp_editor.editor)
		{
			auto& ui = bp_editor.window->ui;
			ui.parents.push(bp_editor.window->root);
			ui.next_element_pos = vec2(100.f);
			ui.next_element_size = vec2(400.f, 300.f);
			ui.e_begin_docker_floating_container();
			ui.e_begin_docker();
			bp_editor.editor = new cBPEditor;
			ui.e_end_docker();
			ui.e_end_docker_floating_container();
			ui.parents.pop();
		}
	}, Capture());
	ui.e_menu_item(L"Detail", [](Capture&) {
		if (!bp_editor.detail)
		{
			auto& ui = bp_editor.window->ui;
			ui.parents.push(bp_editor.window->root);
			ui.next_element_pos = vec2(100.f);
			ui.next_element_size = vec2(400.f, 300.f);
			ui.e_begin_docker_floating_container();
			ui.e_begin_docker();
			bp_editor.detail = new cDetail;
			ui.e_end_docker();
			ui.e_end_docker_floating_container();
			ui.parents.pop();
		}
	}, Capture());
	ui.e_menu_item(L"Preview", [](Capture&) {
		if (!bp_editor.preview)
		{
			auto& ui = bp_editor.window->ui;
			ui.parents.push(bp_editor.window->root);
			ui.next_element_pos = vec2(100.f);
			ui.next_element_size = vec2(400.f, 300.f);
			ui.e_begin_docker_floating_container();
			ui.e_begin_docker();
			bp_editor.preview = new cPreview;
			ui.e_end_docker();
			ui.e_end_docker_floating_container();
			ui.parents.pop();
		}
	}, Capture());
	ui.e_menu_item(L"Console", [](Capture&) {
		if (!bp_editor.console)
		{
			auto& ui = bp_editor.window->ui;
			ui.parents.push(bp_editor.window->root);
			ui.next_element_pos = vec2(100.f);
			ui.next_element_size = vec2(400.f, 300.f);
			ui.e_begin_docker_floating_container();
			ui.e_begin_docker();
			bp_editor.console = new cConsole;
			ui.e_end_docker();
			ui.e_end_docker_floating_container();
			ui.parents.pop();
		}
	}, Capture());
	ui.e_end_menubar_menu();
	ui.e_begin_menubar_menu(L"Tools");
	ui.e_menu_item(L"Generate Graph Image", [](Capture& c) {
		bp_editor.generate_graph_image();
	}, Capture());
	ui.e_menu_item(L"Auto Set Layout", [](Capture& c) {
		bp_editor.auto_set_layout();
	}, Capture());
	ui.e_menu_item(L"Reflector", [](Capture& c) {
		auto& ui = bp_editor.window->ui;
		create_ui_reflector(ui);
	}, Capture());
	ui.e_end_menubar_menu();
	ui.e_end_menu_bar();

	{
		ui.next_element_padding = 4.f;
		ui.next_element_color = ui.style(FrameColorNormal).c;
		ui.e_begin_layout(LayoutHorizontal, 8.f);
		ui.c_aligner(AlignMinMax, 0);
		ui.e_begin_layout(LayoutHorizontal, 4.f);
		ui.e_text(L"Auto (F3)");
		bp_editor.c_auto_update = ui.e_checkbox()->get_component(cCheckbox);
		ui.e_end_layout();
		bp_editor.c_auto_update->data_changed_listeners.add([](Capture&, uint hash, void*) {
			if (hash == FLAME_CHASH("toggled"))
				bp_editor.auto_update = bp_editor.c_auto_update->toggled;
			return true;
		}, Capture());
		ui.e_button(L"Update (F2)", [](Capture&) {
			bp_editor.update();
		}, Capture());
		ui.e_button(L"Reset Time", [](Capture&) {
			bp_editor.bp->time = 0.f;
		}, Capture());
		ui.e_end_layout();
	}

	ui.e_begin_docker_static_container();
	if (window_layout_root)
		add_window(window_layout_root.child("static").first_child());
	ui.e_end_docker_static_container();

	ui.e_end_layout();

	ui.push_style(FontSize, common(Vec1u(30)));
	ui.next_element_padding = vec4(20.f, 10.f, 20.f, 10.f);
	ui.next_element_color = cvec4(0, 0, 0, 255);
	bp_editor.e_notification = ui.e_text(L"");
	bp_editor.e_notification->get_component(cText)->color = cvec4(255);
	ui.c_aligner(AlignMax, AlignMax);
	bp_editor.e_notification->set_visible(false);
	ui.pop_style(FontSize);

	ui.parents.pop();
}

BPEditorWindow::~BPEditorWindow()
{
	bp_editor.window = nullptr;

	{
		auto p = bp_editor.e_test->parent;
		if (p)
			p->remove_child(bp_editor.e_test);
		else
			delete(bp_editor.e_test);
		bp_editor.e_test = nullptr;
	}

	looper().remove_event(update_event);
}

void BPEditor::select()
{
	if (editor)
		editor->on_before_select();

	selected_nodes.clear();
	selected_links.clear();

	if (detail)
		detail->on_after_select();
	if (editor)
		editor->on_after_select();
}

void BPEditor::select(const std::vector<bpNode*>& nodes)
{
	if (editor)
		editor->on_before_select();

	selected_nodes = nodes;
	selected_links.clear();

	if (detail)
		detail->on_after_select();
	if (editor)
		editor->on_after_select();
}

void BPEditor::select(const std::vector<bpSlot*>& links)
{
	if (editor)
		editor->on_before_select();

	selected_links = links;
	selected_nodes.clear();

	if (detail)
		detail->on_after_select();
	if (editor)
		editor->on_after_select();
}

void BPEditor::save()
{
	bpScene::save_to_file(bp_editor.bp, bp_editor.filepath.c_str());

	actions.clear();
	action_idx = 0;

	bp_editor.set_changed(false);
}

void BPEditor::set_changed(bool v)
{
	if (changed != v)
	{
		changed = v;

		std::string title = filepath.string();
		if (changed)
			title += "*";
		bp_editor.window->window->set_title(title.c_str());
	}
}

bpNode* BPEditor::add_node(const NodeDesc& desc)
{
	auto n = _add_node(desc.node_type, desc.id, desc.type, desc.pos);
	if (!n)
		return nullptr;

	auto a = new Action_AddNode;
	a->desc.node_type = desc.node_type;
	a->desc.type = desc.type;
	auto unit = n;
	a->desc.guid = unit->guid;
	a->desc.id = unit->id.str();
	a->desc.pos = desc.pos;
	add_action(a);

	set_changed(true);

	return n;
}

void BPEditor::remove_nodes(const std::vector<bpNode*> nodes)
{
	auto a = new Action_RemoveNodes;
	a->savings.resize(nodes.size());
	for (auto i = 0; i < nodes.size(); i++)
	{
		auto n = nodes[i];
		auto& s = a->savings[i];
		s.desc.node_type = n->node_type;
		s.desc.type = n->type.str();
		auto unit = n;
		s.desc.guid = unit->guid;
		s.desc.id = unit->id.str();
		s.desc.pos = unit->pos;
		s.inputs.resize(n->inputs.s);
		for (auto j = 0; j < s.inputs.size(); j++)
		{
			auto src = n->inputs[j];
			auto& dst = s.inputs[j];
			auto type = src->type;
			if (type->tag != TagPointer)
				dst.data = type->serialize(src->data);
			auto slot = src->links[0];
			if (slot)
			{
				dst.link.node_guid = slot->node->guid;
				dst.link.slot_name = slot->name.str();
			}
		}
		s.outputs.resize(n->outputs.s);
		for (auto j = 0; j < s.outputs.size(); j++)
		{
			auto src = n->outputs[j];
			auto& dst = s.outputs[j];
			for (auto k = 0; k < src->links.s; k++)
			{
				auto slot = src->links[k];
				auto _n = (bpNode*)slot->node;
				auto existed = false;
				for (auto& n : nodes)
				{
					if (_n == n)
					{
						existed = true;
						break;
					}
				}
				if (existed)
					continue;

				LinkSaving l;
				l.node_guid = _n->guid;
				l.slot_name = slot->name.str();
				dst.links.push_back(l);
			}
		}
	}
	add_action(a);

	_remove_nodes(nodes);

	set_changed(true);
}

void BPEditor::set_node_id(bpNode* n, std::string_view id)
{
	auto before_id = n->id.str();

	if (!n->set_id(id.c_str()))
		return;

	auto a = new Action_ChangeNodeID;
	a->before_id = before_id;
	a->after_id = id;
	add_action(a);

	set_changed(true);
}

void BPEditor::set_nodes_pos(const std::vector<bpNode*>& nodes, const std::vector<vec2>& poses)
{
	auto a = new Action_MoveNodes;
	a->targets.resize(nodes.size());
	for (auto i = 0; i < nodes.size(); i++)
	{
		auto& src = nodes[i];
		auto& dst = a->targets[i];
		dst.guid = src->guid;
		dst.before_pos = src->pos;
		dst.after_pos = poses[i];
	}
	add_action(a);

	for (auto i = 0; i < nodes.size(); i++)
		nodes[i]->pos = poses[i];

	set_changed(true);
}

void BPEditor::set_links(const std::vector<std::pair<bpSlot*, bpSlot*>>& links)
{
	auto a = new Action_SetLinks;
	a->link_descs.resize(links.size());
	for (auto i = 0; i < links.size(); i++)
	{
		auto& src = links[i];
		auto& dst = a->link_descs[i];
		dst.input.node_guid = src.first->node->guid;
		dst.input.slot_name = src.first->name.str();
		auto prev = src.first->links[0];
		if (prev)
		{
			dst.before_output_addr.node_guid = prev->node->guid;
			dst.before_output_addr.slot_name = prev->name.str();
		}
		if (src.second)
		{
			dst.after_output_addr.node_guid = src.second->node->guid;
			dst.after_output_addr.slot_name = src.second->name.str();
		}
	}
	add_action(a);

	for (auto& l : links)
		_set_link(l.first, l.second);

	set_changed(true);
}

void BPEditor::set_data(bpSlot* input, void* data, bool from_editor)
{
	auto type = input->type;

	auto a = new Action_SetData;
	a->node_guid = input->node->guid;
	a->input_name = input->name.str();
	a->before_data = type->serialize(input->data);
	a->after_data = type->serialize(data);
	add_action(a);

	input->set_data(data);
	set_changed(true);

	if (!from_editor && editor)
		editor->on_data_changed(input);
}

void BPEditor::update()
{
	bp->update();
}

void BPEditor::update_gv()
{
	//auto gv_filename = fileppath / L"bp.gv";
	//if (!std::filesystem::exists(gv_filename) || std::filesystem::last_write_time(gv_filename) < std::filesystem::last_write_time(filepath))
	//{
	//	if (!GRAPHVIZ_PATH[0])
	//		assert(0);

	//	std::string gv = "digraph bp {\nrankdir=LR\nnode [shape = Mrecord];\n";
	//	for (auto i = 0; i < bp->node_count(); i++)
	//	{
	//		auto n = bp->nodes(i);
	//		auto name = n->id();
	//		auto str = sfmt("\t%s[label = \"%s|%s|{{", name, name, n->type());
	//		for (auto j = 0; j < n->input_count(); j++)
	//		{
	//			auto input = n->input(j);
	//			auto name = input->name();
	//			str += sfmt("<%s>%s", name, name);
	//			if (j != n->input_count() - 1)
	//				str += "|";
	//		}
	//		str += "}|{";
	//		for (auto j = 0; j < n->output_count(); j++)
	//		{
	//			auto output = n->output(j);
	//			auto name = output->name();
	//			str += sfmt("<%s>%s", name, name);
	//			if (j != n->output_count() - 1)
	//				str += "|";
	//		}
	//		str += "}}\"];\n";

	//		gv += str;
	//	}
	//	for (auto i = 0; i < bp->node_count(); i++)
	//	{
	//		auto n = bp->node(i);

	//		for (auto j = 0; j < n->input_count(); j++)
	//		{
	//			auto input = n->input(j);
	//			if (input->link())
	//			{
	//				auto in_sp = SUS::split(input->get_address().str(), '.');
	//				auto out_sp = SUS::split(input->link()->get_address().str(), '.');

	//				gv += "\t" + out_sp[0] + ":" + out_sp[1] + " -> " + in_sp[0] + ":" + in_sp[1] + ";\n";
	//			}
	//		}
	//	}
	//	gv += "}\n";

	//	std::ofstream file(gv_filename);
	//	file << gv;
	//	file.close();
	//}
}

const auto dot_path = s2w(GRAPHVIZ_PATH) + L"/bin/dot.exe";

bool BPEditor::generate_graph_image()
{
	update_gv();
	auto png_filename = fileppath / L"bp.png";
	if (!std::filesystem::exists(png_filename) || std::filesystem::last_write_time(png_filename) < std::filesystem::last_write_time(filepath))
		exec(dot_path.c_str(), (wchar_t*)(L"-Tpng " + fileppath.wstring() + L"/bp.gv -y -o " + png_filename.wstring()).c_str(), true);

	return std::filesystem::exists(png_filename);
}

bool BPEditor::auto_set_layout()
{
	//update_gv();
	//auto graph_filename = fileppath / L"bp.graph";
	//if (!std::filesystem::exists(graph_filename) || std::filesystem::last_write_time(graph_filename) < std::filesystem::last_write_time(filepath))
	//	exec(dot_path.c_str(), (wchar_t*)(L"-Tplain" + fileppath.wstring() + L"/bp.gv -y -o " + graph_filename.wstring()).c_str(), true);
	//if (!std::filesystem::exists(graph_filename))
	//	return false;

	//auto str = get_file_content(L"bp.graph.txt");
	//for (auto it = str.begin(); it != str.end(); )
	//{
	//	if (*it == '\\')
	//	{
	//		it = str.erase(it);
	//		if (it != str.end())
	//		{
	//			if (*it == '\r')
	//			{
	//				it = str.erase(it);
	//				if (it != str.end() && *it == '\n')
	//					it = str.erase(it);
	//			}
	//			else if (*it == '\n')
	//				it = str.erase(it);
	//		}
	//	}
	//	else
	//		it++;
	//}

	//std::regex reg_node(R"(node ([\w]+) ([\d\.]+) ([\d\.]+))");
	//std::smatch res;
	//while (std::regex_search(str, res, reg_node))
	//{
	//	auto n = bp->root->find_node(res[1].str().c_str());
	//	if (n)
	//	{
	//		n->pos = vec2(std::stof(res[2].str().c_str()), std::stof(res[3].str().c_str())) * 100.f;
	//		((Entity*)n->user_data)->get_component(cElement)->set_pos(n->pos);
	//	}

	//	str = res.suffix();
	//}

	return true;
}

bpNode* BPEditor::_add_node(bpNodeType node_type, std::string_view id, std::string_view type, const vec2& pos)
{
	auto n = bp_editor.bp->root->add_node(id.c_str(), type.c_str(), node_type);
	if (!n)
		return nullptr;
	n->pos = pos;

	if (bp_editor.editor)
		bp_editor.editor->on_add_node(n);

	return n;
}

void BPEditor::_remove_nodes(const std::vector<bpNode*>& nodes)
{
	if (bp_editor.editor)
	{
		std::vector<bpNode*> refresh_ns;
		for (auto n : nodes)
		{
			for (auto o : n->outputs)
			{
				for (auto l : o->links)
				{
					auto nn = l->node;
					auto found = false;
					for (auto _n : nodes)
					{
						if (_n == nn)
						{
							found = true;
							break;
						}
					}
					if (!found)
						refresh_ns.push_back(nn);
				}
			}
		}
		for (auto n : nodes)
		{
			looper().add_event([](Capture& c) {
				auto n = c.thiz<bpNode>();
				bp_editor.editor->on_remove_node(n);
				n->parent->remove_node(n);
			}, Capture().set_thiz(n));
		}
		for (auto n : refresh_ns)
		{
			looper().add_event([](Capture& c) {
				auto n = c.thiz<bpNode>();
				bp_editor.editor->on_remove_node(n);
				bp_editor.editor->on_add_node(n);
			}, Capture().set_thiz(n));
		}
	}
}

std::vector<bpNode*> BPEditor::_duplicate_nodes(const std::vector<bpNode*>& models)
{
	std::vector<bpNode*> ret(models.size());
	for (auto i = 0; i < models.size(); i++)
	{
		auto n = models[i];
		ret[i] = _add_node(n->node_type, "", n->type.v, n->pos + vec2(20.f));
	}
	for (auto i = 0; i < models.size(); i++)
	{
		auto n = ret[i];
		for (auto j = 0; j < n->inputs.s; j++)
		{
			n->inputs[j]->set_data(models[i]->inputs[j]->data);
			auto link = models[i]->inputs[j]->links[0];
			if (link)
			{
				auto nn = link->node;
				for (auto k = 0; k < models.size(); k++)
				{
					if (nn == models[k])
					{
						link = ret[k]->outputs[link->index];
						break;
					}
				}
			}
			n->inputs[j]->link_to(link);
		}
	}
	return ret;
}

void BPEditor::_set_link(bpSlot* in, bpSlot* out)
{
	if (auto node = in->node; bp_editor.editor)
	{
		looper().add_event([](Capture& c) {
			auto n = c.thiz<bpNode>();
			bp_editor.editor->on_remove_node(n);
			bp_editor.editor->on_add_node(n);
		}, Capture().set_thiz(node));
	}
	in->link_to(out);
}

BPEditor bp_editor;
