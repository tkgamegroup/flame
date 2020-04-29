#include <flame/universe/utils/ui_reflector.h>

#include "bp_editor.h"

struct Action
{
	wchar_t* name;
	virtual void undo() = 0;
	virtual void redo() = 0;
};

struct Action_ChangeNodeID : Action
{
	std::string prev_id;
	std::string after_id;

	Action_ChangeNodeID()
	{
		name = L"Change Node ID";
	}

	void undo() override
	{
		auto n = bp_editor.bp->find_node(after_id.c_str());
		if (n)
		{
			n->set_id(prev_id.c_str());

			if (bp_editor.detail)
				bp_editor.detail->on_after_select();
		}
	}

	void redo() override
	{
		auto n = bp_editor.bp->find_node(prev_id.c_str());
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
		std::string id;
		Vec2f prev_pos;
		Vec2f after_pos;
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
			auto n = bp_editor.bp->find_node(t.id.c_str());
			if (n)
			{
				n->pos = t.prev_pos;
				if (bp_editor.editor)
					bp_editor.editor->on_pos_changed(n);
			}
		}
	}

	void redo() override
	{
		for (auto& t : targets)
		{
			auto n = bp_editor.bp->find_node(t.id.c_str());
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

	Action_AddNode()
	{
		name = L"Add Node";
	}

	void undo() override
	{
		auto n = bp_editor.bp->find_node(desc.id.c_str());
		if (n)
			bp_editor._remove_node(n);
	}

	void redo() override
	{
		bp_editor._add_node(desc.object_type, desc.id.c_str(), desc.type.c_str(), desc.pos);
	}
};

struct Action_DuplicateNodes : Action
{
	std::vector<std::string> models;
	std::vector<std::string> duplications;

	Action_DuplicateNodes()
	{
		name = L"Duplicate Nodes";
	}

	void undo() override
	{
		for (auto& d : duplications)
		{
			auto n = bp_editor.bp->find_node(d.c_str());
			if (n)
				bp_editor._remove_node(n);
		}
	}

	void redo() override
	{
		std::vector<BP::Node*> nodes(models.size());
		for (auto i = 0; i < models.size(); i++)
			nodes[i] = bp_editor.bp->find_node(models[i].c_str());
		auto new_nodes = bp_editor._duplicate_nodes(nodes);
		for (auto i = 0; i < models.size(); i++)
			duplications[i] = new_nodes[i]->id();
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
		for (auto& s : savings)
		{
			auto n = bp_editor._add_node(s.desc.object_type, s.desc.id.c_str(), s.desc.type.c_str(), s.desc.pos);
			if (n)
			{
				for (auto i = 0; i < s.inputs.size(); i++)
				{
					auto& src = s.inputs[i];
					auto dst = n->input(i);
					auto type = dst->type();
					if (type->tag() != TypePointer)
					{
						auto data = new char[dst->size()];
						type->unserialize(src.data, data);
						dst->set_data((char*)data);
						delete[] data;
					}

					if (!src.link.empty())
					{
						auto slot = bp_editor.bp->find_output(src.link.c_str());
						if (slot)
							dst->link_to(slot);
					}
				}
				for (auto i = 0; i < s.outputs.size(); i++)
				{
					auto& src = s.outputs[i];
					auto dst = n->output(i);

					for (auto& a : src.links)
					{
						auto slot = bp_editor.bp->find_input(a.c_str());
						if (slot)
							slot->link_to(dst);
					}
				}
			}
		}
	}

	void redo() override
	{
		for (auto& s : savings)
		{
			auto n = bp_editor.bp->find_node(s.desc.id.c_str());
			if (n)
				bp_editor._remove_node(n);
		}
	}
};

struct Action_SetLinks : Action
{
	struct LinkDesc
	{
		std::string input_addr;
		std::string prev_output_addr;
		std::string after_output_addr;
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
			auto i = bp_editor.bp->find_input(l.input_addr.c_str());
			if (i)
				bp_editor._set_link(i, l.prev_output_addr.empty() ? nullptr : bp_editor.bp->find_output(l.prev_output_addr.c_str()));
		}

		bp_editor.window->s_2d_renderer->pending_update = true;
	}

	void redo() override
	{
		for (auto& l : link_descs)
		{
			auto i = bp_editor.bp->find_input(l.input_addr.c_str());
			if (i)
				bp_editor._set_link(i, l.after_output_addr.empty() ? nullptr : bp_editor.bp->find_output(l.after_output_addr.c_str()));
		}

		bp_editor.window->s_2d_renderer->pending_update = true;
	}
};

struct Action_SetData : Action
{
	std::string input_addr;
	std::string prev_data;
	std::string after_data;

	Action_SetData()
	{
		name = L"Set Data";
	}

	void undo() override
	{
		auto s = bp_editor.bp->find_input(input_addr.c_str());
		if (s)
		{
			auto type = s->type();
			auto data = new char[s->size()];
			type->unserialize(prev_data, data);
			s->set_data((char*)data);
			delete[] data;

			if (bp_editor.editor)
				bp_editor.editor->on_data_changed(s);
		}
	}

	void redo() override
	{
		auto s = bp_editor.bp->find_input(input_addr.c_str());
		if (s)
		{
			auto type = s->type();
			auto data = new char[s->size()];
			type->unserialize(after_data, data);
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
		action_idx--;
		auto a = actions[action_idx].get();
		a->undo();

		if (action_idx == 0)
			bp_editor.set_changed(false);

		bp_editor.e_notification->set_visible(true);
		bp_editor.e_notification->get_component(cTimer)->start();
		bp_editor.e_notification->get_component(cText)->set_text((std::wstring(L"Undo ") + a->name).c_str());
	}
}

static void redo()
{
	if (action_idx < actions.size())
	{
		auto a = actions[action_idx].get();
		a->redo();
		action_idx++;

		bp_editor.set_changed(true);

		bp_editor.e_notification->set_visible(true);
		bp_editor.e_notification->get_component(cTimer)->start();
		bp_editor.e_notification->get_component(cText)->set_text((std::wstring(L"Redo ") + a->name).c_str());
	}
}

static void duplicate_selected()
{
	if (bp_editor.selected_nodes.empty())
		return;

	auto a = new Action_DuplicateNodes;
	a->models.resize(bp_editor.selected_nodes.size());
	for (auto i = 0; i < bp_editor.selected_nodes.size(); i++)
		a->models[i] = bp_editor.selected_nodes[i]->id();

	auto new_nodes = bp_editor._duplicate_nodes(bp_editor.selected_nodes);
	bp_editor.select(new_nodes);

	a->duplications.resize(new_nodes.size());
	for (auto i = 0; i < new_nodes.size(); i++)
		a->duplications[i] = new_nodes[i]->id();

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
		std::vector<std::pair<BP::Slot*, BP::Slot*>> links(bp_editor.selected_links.size());
		for (auto i = 0; i < bp_editor.selected_links.size(); i++)
			links[i] = { bp_editor.selected_links[i], nullptr };
		bp_editor.set_links(links);
		bp_editor.selected_links.clear();

		bp_editor.set_changed(true);

		bp_editor.window->s_2d_renderer->pending_update = true;
	}
}

static void add_window(pugi::xml_node n)
{
	auto parent_layout = utils::current_parent()->get_component(cLayout)->type == LayoutHorizontal;
	auto r = n.attribute("r").as_int(1);
	std::string name(n.name());
	if (name == "layout")
	{
		auto t = n.attribute("type").value() == std::string("h") ? LayoutHorizontal : LayoutVertical;
		auto ca = utils::e_begin_docker_layout(t)->get_component(cAligner);
		if (parent_layout)
			ca->width_factor = r;
		else
			ca->height_factor = r;
		for (auto c : n.children())
			add_window(c);
		utils::e_end_docker_layout();
	}
	else if (name == "docker")
	{
		auto ca = utils::e_begin_docker()->get_component(cAligner);
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
		utils::e_end_docker();
	}
}

BPEditorWindow::BPEditorWindow(const std::filesystem::path& filename) :
	App::Window(&app, true, true, "BP Editor", Vec2u(300, 200), WindowFrame | WindowResizable, nullptr, true)
{
	bp_editor.window = this;

	utils::set_current_root(root);
	utils::set_current_entity(root);

	canvas->clear_color = Vec4f(100, 100, 100, 255) / 255.f;
	utils::style_set_to_light();

	bp_editor.e_test = Entity::create();
	{
		auto ce = cElement::create();
		ce->pos = 150.f;
		ce->size = 100.f;
		ce->pivot = 0.5f;
		ce->color = Vec4c(0, 0, 0, 255);
		cElement::set_linked_object(ce);
		bp_editor.e_test->add_component(ce);
	}
	{
		auto cer = cEventReceiver::create();
		cEventReceiver::set_linked_object(cer);
		bp_editor.e_test->add_component(cer);
	}

	if (!filename.empty())
	{
		bp_editor.filepath = filename;
		bp_editor.fileppath = filename.parent_path();
		bp_editor.bp = BP::create_from_file(filename.c_str());

		sys_window->set_title(filename.string().c_str());
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
		auto c_event_receiver = root->get_component(cEventReceiver);
		c_event_receiver->key_listeners.add([](Capture& c, KeyStateFlags action, int value) {
			if (is_key_down(action))
			{
				auto ed = c.current<cEventReceiver>()->dispatcher;
				switch (value)
				{
				case Key_S:
					if (ed->key_states[Key_Ctrl] & KeyStateDown)
						bp_editor.save();
					break;
				case Key_Z:
					if (ed->key_states[Key_Ctrl] & KeyStateDown)
						undo();
					break;
				case Key_Y:
					if (ed->key_states[Key_Ctrl] & KeyStateDown)
						redo();
					break;
				case Key_D:
					if (ed->key_states[Key_Ctrl] & KeyStateDown)
						duplicate_selected();
					break;
				case Key_Del:
					delete_selected();
					break;
				case Key_F2:
					bp_editor.update();
					break;
				case Key_F3:
					bp_editor.c_auto_update->set_checked(true);
					break;
				}
			}
			return true;
		}, Capture());
		s_event_dispatcher->next_focusing = c_event_receiver;
	}

	utils::push_parent(root);

	utils::e_begin_layout(LayoutVertical, 0.f, false, false);
	utils::c_aligner(AlignMinMax, AlignMinMax);

	utils::e_begin_menu_bar();
	utils::e_begin_menubar_menu(L"Blueprint");
	utils::e_menu_item((std::wstring(Icon_FLOPPY_O) + L"    Save").c_str(), [](Capture& c) {
		bp_editor.save();
	}, Capture());
	utils::e_end_menubar_menu();
	utils::e_begin_menubar_menu(L"Edit");
	utils::e_menu_item((std::wstring(Icon_UNDO) + L"    Undo").c_str(), [](Capture&) {
		undo();
	}, Capture());
	utils::e_menu_item((std::wstring(Icon_REPEAT) + L"    Redo").c_str(), [](Capture&) {
		redo();
	}, Capture());
	utils::e_menu_item((std::wstring(Icon_CLONE) + L"   Duplicate").c_str(), [](Capture&) {
		duplicate_selected();
	}, Capture());
	utils::e_menu_item((std::wstring(Icon_TIMES) + L"    Delete").c_str(), [](Capture&) {
		delete_selected();
	}, Capture());
	utils::e_end_menubar_menu();
	utils::e_begin_menubar_menu(L"View");
	utils::e_menu_item(L"Editor", [](Capture&) {
		if (!bp_editor.editor)
		{
			utils::push_parent(bp_editor.window->root);
			utils::next_element_pos = Vec2f(100.f);
			utils::next_element_size = Vec2f(400.f, 300.f);
			utils::e_begin_docker_floating_container();
			utils::e_begin_docker();
			bp_editor.editor = new cBPEditor;
			utils::e_end_docker();
			utils::e_end_docker_floating_container();
			utils::pop_parent();
		}
	}, Capture());
	utils::e_menu_item(L"Detail", [](Capture&) {
		if (!bp_editor.detail)
		{
			utils::push_parent(bp_editor.window->root);
			utils::next_element_pos = Vec2f(100.f);
			utils::next_element_size = Vec2f(400.f, 300.f);
			utils::e_begin_docker_floating_container();
			utils::e_begin_docker();
			bp_editor.detail = new cDetail;
			utils::e_end_docker();
			utils::e_end_docker_floating_container();
			utils::pop_parent();
		}
	}, Capture());
	utils::e_menu_item(L"Preview", [](Capture&) {
		if (!bp_editor.preview)
		{
			utils::push_parent(bp_editor.window->root);
			utils::next_element_pos = Vec2f(100.f);
			utils::next_element_size = Vec2f(400.f, 300.f);
			utils::e_begin_docker_floating_container();
			utils::e_begin_docker();
			bp_editor.preview = new cPreview;
			utils::e_end_docker();
			utils::e_end_docker_floating_container();
			utils::pop_parent();
		}
	}, Capture());
	utils::e_menu_item(L"Console", [](Capture&) {
		if (!bp_editor.console)
		{
			utils::push_parent(bp_editor.window->root);
			utils::next_element_pos = Vec2f(100.f);
			utils::next_element_size = Vec2f(400.f, 300.f);
			utils::e_begin_docker_floating_container();
			utils::e_begin_docker();
			bp_editor.console = new cConsole;
			utils::e_end_docker();
			utils::e_end_docker_floating_container();
			utils::pop_parent();
		}
	}, Capture());
	utils::e_end_menubar_menu();
	utils::e_begin_menubar_menu(L"Tools");
	utils::e_menu_item(L"Generate Graph Image", [](Capture& c) {
		bp_editor.generate_graph_image();
	}, Capture());
	utils::e_menu_item(L"Auto Set Layout", [](Capture& c) {
		bp_editor.auto_set_layout();
	}, Capture());
	utils::e_menu_item(L"Reflector", [](Capture& c) {
		utils::e_ui_reflector_window();
	}, Capture());
	utils::e_end_menubar_menu();
	utils::e_end_menu_bar();

	{
		utils::next_element_padding = 4.f;
		utils::next_element_color = utils::style(FrameColorNormal).c;
		utils::e_begin_layout(LayoutHorizontal, 8.f);
		utils::c_aligner(AlignMinMax, 0);
		bp_editor.c_auto_update = utils::e_checkbox(L"Auto (F3)")->get_component(cCheckbox);
		bp_editor.c_auto_update->data_changed_listeners.add([](Capture&, uint hash, void*) {
			if (hash == FLAME_CHASH("checked"))
				bp_editor.auto_update = bp_editor.c_auto_update->checked;
			return true;
		}, Capture());
		utils::e_button(L"Update (F2)", [](Capture&) {
			bp_editor.update();
		}, Capture());
		utils::e_button(L"Reset Time", [](Capture&) {
			bp_editor.bp->time = 0.f;
		}, Capture());
		utils::e_end_layout();
	}

	utils::e_begin_docker_static_container();
	if (window_layout_root)
		add_window(window_layout_root.child("static").first_child());
	utils::e_end_docker_static_container();

	utils::e_end_layout();

	utils::push_style(FontSize, common(Vec1u(30)));
	utils::next_element_padding = Vec4f(20.f, 10.f, 20.f, 10.f);
	utils::next_element_color = Vec4c(0, 0, 0, 255);
	bp_editor.e_notification = utils::e_text(L"");
	bp_editor.e_notification->get_component(cText)->color = Vec4c(255);
	utils::c_aligner(AlignMax, AlignMax);
	bp_editor.e_notification->set_visible(false);
	{
		auto c_timer = utils::c_timer();
		c_timer->interval = 1.f;
		c_timer->max_times = 1;
		c_timer->set_callback([](Capture&) {
			bp_editor.e_notification->set_visible(false);
		}, Capture(), false);
	}
	utils::pop_style(FontSize);

	utils::pop_parent();
}

BPEditorWindow::~BPEditorWindow()
{
	bp_editor.window = nullptr;

	{
		auto p = bp_editor.e_test->parent();
		if (p)
			p->remove_child(bp_editor.e_test);
		else
			Entity::destroy(bp_editor.e_test);
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

void BPEditor::select(const std::vector<BP::Node*>& nodes)
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

void BPEditor::select(const std::vector<BP::Slot*>& links)
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
	BP::save_to_file(bp_editor.bp, bp_editor.filepath.c_str());

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
		bp_editor.window->sys_window->set_title(title.c_str());
	}
}

BP::Node* BPEditor::add_node(const NodeDesc& desc)
{
	auto n = _add_node(desc.object_type, desc.id, desc.type, desc.pos);
	if (!n)
		return nullptr;

	auto a = new Action_AddNode;
	a->desc.object_type = desc.object_type;
	a->desc.type = desc.type;
	a->desc.id = n->id();
	a->desc.pos = desc.pos;
	add_action(a);

	set_changed(true);

	return n;
}

void BPEditor::remove_nodes(const std::vector<BP::Node*> nodes)
{
	auto a = new Action_RemoveNodes;
	a->savings.resize(nodes.size());
	for (auto i = 0; i < nodes.size(); i++)
	{
		auto n = nodes[i];
		auto& s = a->savings[i];
		s.desc.object_type = n->object_type();
		s.desc.type = n->type();
		s.desc.id = n->id();
		s.desc.pos = n->pos;
		s.inputs.resize(n->input_count());
		for (auto j = 0; j < s.inputs.size(); j++)
		{
			auto src = n->input(j);
			auto& dst = s.inputs[j];
			auto type = src->type();
			if (type->tag() != TypePointer)
				dst.data = type->serialize(src->data());
			auto slot = src->link(0);
			if (slot)
				dst.link = slot->get_address().v;
		}
		s.outputs.resize(n->output_count());
		for (auto j = 0; j < s.outputs.size(); j++)
		{
			auto src = n->output(j);
			auto& dst = s.outputs[j];
			for (auto k = 0; k < src->link_count(); k++)
			{
				auto slot = src->link(k);
				auto _n = slot->node();
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
				dst.links.push_back(slot->get_address().v);
			}
		}
	}
	add_action(a);

	for (auto& n : nodes)
		_remove_node(n);

	set_changed(true);
}

void BPEditor::set_node_id(BP::Node* n, const std::string& id)
{
	std::string prev_id = n->id();

	if (!n->set_id(id.c_str()))
		return;

	auto a = new Action_ChangeNodeID;
	a->prev_id = prev_id;
	a->after_id = id;
	add_action(a);

	set_changed(true);
}

void BPEditor::set_nodes_pos(const std::vector<BP::Node*>& nodes, const std::vector<Vec2f>& poses)
{
	auto a = new Action_MoveNodes;
	a->targets.resize(nodes.size());
	for (auto i = 0; i < nodes.size(); i++)
	{
		auto& src = nodes[i];
		auto& dst = a->targets[i];
		dst.id = src->id();
		dst.prev_pos = src->pos;
		dst.after_pos = poses[i];
	}
	add_action(a);

	for (auto i = 0; i < nodes.size(); i++)
		nodes[i]->pos = poses[i];

	set_changed(true);
}

void BPEditor::set_links(const std::vector<std::pair<BP::Slot*, BP::Slot*>>& links)
{
	auto a = new Action_SetLinks;
	a->link_descs.resize(links.size());
	for (auto i = 0; i < links.size(); i++)
	{
		auto& src = links[i];
		auto& dst = a->link_descs[i];
		dst.input_addr = src.first->get_address().v;
		auto prev = src.first->link(0);
		if (prev)
			dst.prev_output_addr = prev->get_address().v;
		if (src.second)
			dst.after_output_addr = src.second->get_address().v;
	}
	add_action(a);

	for (auto& l : links)
		_set_link(l.first, l.second);

	set_changed(true);
}

void BPEditor::set_data(BP::Slot* input, void* data, bool from_editor)
{
	auto type = input->type();

	auto a = new Action_SetData;
	a->input_addr = input->get_address().v;
	a->prev_data = type->serialize(input->data());
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
	auto gv_filename = fileppath / L"bp.gv";
	if (!std::filesystem::exists(gv_filename) || std::filesystem::last_write_time(gv_filename) < std::filesystem::last_write_time(filepath))
	{
		if (!GRAPHVIZ_PATH[0])
			assert(0);

		std::string gv = "digraph bp {\nrankdir=LR\nnode [shape = Mrecord];\n";
		for (auto i = 0; i < bp->node_count(); i++)
		{
			auto n = bp->node(i);
			auto name = n->id();
			auto str = sfmt("\t%s[label = \"%s|%s|{{", name, name, n->type());
			for (auto j = 0; j < n->input_count(); j++)
			{
				auto input = n->input(j);
				auto name = input->name();
				str += sfmt("<%s>%s", name, name);
				if (j != n->input_count() - 1)
					str += "|";
			}
			str += "}|{";
			for (auto j = 0; j < n->output_count(); j++)
			{
				auto output = n->output(j);
				auto name = output->name();
				str += sfmt("<%s>%s", name, name);
				if (j != n->output_count() - 1)
					str += "|";
			}
			str += "}}\"];\n";

			gv += str;
		}
		for (auto i = 0; i < bp->node_count(); i++)
		{
			auto n = bp->node(i);

			for (auto j = 0; j < n->input_count(); j++)
			{
				auto input = n->input(j);
				if (input->link())
				{
					auto in_sp = SUS::split(input->get_address().str(), '.');
					auto out_sp = SUS::split(input->link()->get_address().str(), '.');

					gv += "\t" + out_sp[0] + ":" + out_sp[1] + " -> " + in_sp[0] + ":" + in_sp[1] + ";\n";
				}
			}
		}
		gv += "}\n";

		std::ofstream file(gv_filename);
		file << gv;
		file.close();
	}
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
	update_gv();
	auto graph_filename = fileppath / L"bp.graph";
	if (!std::filesystem::exists(graph_filename) || std::filesystem::last_write_time(graph_filename) < std::filesystem::last_write_time(filepath))
		exec(dot_path.c_str(), (wchar_t*)(L"-Tplain" + fileppath.wstring() + L"/bp.gv -y -o " + graph_filename.wstring()).c_str(), true);
	if (!std::filesystem::exists(graph_filename))
		return false;

	auto str = get_file_content(L"bp.graph.txt");
	for (auto it = str.begin(); it != str.end(); )
	{
		if (*it == '\\')
		{
			it = str.erase(it);
			if (it != str.end())
			{
				if (*it == '\r')
				{
					it = str.erase(it);
					if (it != str.end() && *it == '\n')
						it = str.erase(it);
				}
				else if (*it == '\n')
					it = str.erase(it);
			}
		}
		else
			it++;
	}

	std::regex reg_node(R"(node ([\w]+) ([\d\.]+) ([\d\.]+))");
	std::smatch res;
	while (std::regex_search(str, res, reg_node))
	{
		auto n = bp->find_node(res[1].str().c_str());
		if (n)
		{
			n->pos = Vec2f(std::stof(res[2].str().c_str()), std::stof(res[3].str().c_str())) * 100.f;
			((Entity*)n->user_data)->get_component(cElement)->set_pos(n->pos);
		}

		str = res.suffix();
	}

	return true;
}

BP::Node* BPEditor::_add_node(BP::ObjectType object_type, const std::string& id, const std::string& type, const Vec2f& pos)
{
	auto n = bp_editor.bp->add_node(id.c_str(), type.c_str(), object_type);
	if (!n)
		return nullptr;
	n->pos = pos;

	if (bp_editor.editor)
		bp_editor.editor->on_add_node(n);

	return n;
}

void BPEditor::_remove_node(BP::Node* n)
{
	if (bp_editor.editor)
	{
		looper().add_event([](Capture& c) {
			auto n = c.thiz<BP::Node>();
			std::vector<BP::Node*> ref_ns;
			for (auto i = 0; i < n->output_count(); i++)
			{
				auto o = n->output(i);
				for (auto j = 0; j < o->link_count(); j++)
					ref_ns.push_back(o->link(j)->node());
			}
			bp_editor.editor->on_remove_node(n);
			bp_editor.bp->remove_node(n);
			for (auto n : ref_ns)
			{
				bp_editor.editor->on_remove_node(n);
				bp_editor.editor->on_add_node(n);
			}
		}, Capture().set_thiz(n));
	}
}

std::vector<BP::Node*> BPEditor::_duplicate_nodes(const std::vector<BP::Node*>& models)
{
	std::vector<BP::Node*> ret(models.size());
	for (auto i = 0; i < models.size(); i++)
	{
		auto n = models[i];
		ret[i] = _add_node(n->object_type(), "", n->type(), n->pos + Vec2f(20.f));
	}
	for (auto i = 0; i < models.size(); i++)
	{
		auto n = ret[i];
		for (auto j = 0; j < n->input_count(); j++)
		{
			n->input(j)->set_data(models[i]->input(j)->data());
			auto link = models[i]->input(j)->link(0);
			if (link)
			{
				auto nn = link->node();
				for (auto k = 0; k < models.size(); k++)
				{
					if (nn == models[k])
					{
						link = ret[k]->output(link->index());
						break;
					}
				}
			}
			n->input(j)->link_to(link);
		}
	}
	return ret;
}

void BPEditor::_set_link(BP::Slot* in, BP::Slot* out)
{
	looper().add_event([](Capture& c) {
		auto n = c.thiz<BP::Node>();
		bp_editor.editor->on_remove_node(n);
		bp_editor.editor->on_add_node(n);
	}, Capture().set_thiz(in->node()));
	in->link_to(out);
}

BPEditor bp_editor;
