#include <flame/universe/ui/data_tracker.h>

#include "bp_editor.h"

static const char* basic_types[] = {
	"bool",
	"int",
	"uint",
	"flame::Vec<2,int>",
	"flame::Vec<3,int>",
	"flame::Vec<4,int>",
	"flame::Vec<2,uint>",
	"flame::Vec<3,uint>",
	"flame::Vec<4,uint>",
	"int64",
	"uint64",
	"float",
	"flame::Vec<2,float>",
	"flame::Vec<3,float>",
	"flame::Vec<4,float>",
	"uchar",
	"flame::Vec<2,uchar>",
	"flame::Vec<3,uchar>",
	"flame::Vec<4,uchar>",
	"flame::StringA",
	"flame::StringW"
};

static cvec4 unselected_col = cvec4(0, 0, 0, 255);
static cvec4 selected_col = cvec4(0, 0, 145, 255);

const auto slot_bezier_extent = 50.f;

struct cNode;

struct cSlot : Component
{
	cElement* element;
	cReceiver* receiver;
	cDataTracker* tracker;

	bpSlot* s;

	bool dragging;

	Entity* tip_info;
	Entity* tip_link;

	cNode* group;

	cSlot();
	~cSlot() override;
	void clear_tips();
	void on_event(EntityEvent e, void* t);
};

struct cNode : Component
{
	cElement* element;
	cReceiver* receiver;

	bpNode* n;

	bool moved;

	_2DEditor edt;

	bpSlot* dragging_slot;
	vec2 dragging_slot_pos;

	cNode();
	void on_event(EntityEvent e, void* t);
};

cSlot::cSlot() :
	Component("cSlot")
{
	element = nullptr;
	receiver = nullptr;
	tracker = nullptr;

	dragging = false;

	tip_info = nullptr;
	tip_link = nullptr;
}

cSlot::~cSlot()
{
	clear_tips();
}

void cSlot::clear_tips()
{
	if (tip_info)
	{
		auto e = tip_info;
		tip_info = nullptr;
		looper().add_event([](Capture& c) {
			auto e = c.thiz<Entity>();
			e->parent->remove_child(e);
		}, Capture().set_thiz(e));
	}
	if (tip_link)
	{
		auto e = tip_link;
		tip_link = nullptr;
		looper().add_event([](Capture& c) {
			auto e = c.thiz<Entity>();
			e->parent->remove_child(e);
		}, Capture().set_thiz(e));
	}
}

void cSlot::on_event(EntityEvent e, void* t)
{
	switch (e)
	{
	case EntityComponentAdded:
		if (t == this)
		{
			element = entity->get_component(cElement);
			receiver = entity->get_component(cReceiver);

			if (s->io == bpSlotIn)
			{
				receiver->drag_hash = FLAME_CHASH("input_slot");
				receiver->set_acceptable_drops(1, &FLAME_CHASH("output_slot"));
			}
			else
			{
				receiver->drag_hash = FLAME_CHASH("output_slot");
				receiver->set_acceptable_drops(1, &FLAME_CHASH("input_slot"));
			}
			receiver->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const ivec2& pos) {
				auto thiz = c.thiz<cSlot>();
				if (thiz->dragging)
				{
					if (is_mouse_scroll(action, key) || is_mouse_move(action, key))
						thiz->group->edt.receiver->send_mouse_event(action, key, pos);
				}
				return true;
			}, Capture().set_thiz(this));
			receiver->drag_and_drop_listeners.add([](Capture& c, DragAndDrop action, cReceiver* er, const ivec2& pos) {
				auto& ui = bp_editor.window->ui;
				auto thiz = c.thiz<cSlot>();
				auto element = thiz->element;
				auto s = thiz->s;
				auto is_in = s->io == bpSlotIn;

				if (action == DragStart)
				{
					thiz->dragging = true;
					thiz->group->dragging_slot = s;
					bp_editor.select();
				}
				else if (action == DragOvering)
				{
					thiz->group->dragging_slot_pos = vec2(pos);
					bp_editor.window->s_renderer->pending_update = true;
				}
				else if (action == DragEnd)
				{
					thiz->dragging = false;
					if (!er)
						bp_editor.editor->show_add_node_menu(vec2(pos));
					else
						thiz->group->dragging_slot = nullptr;
					bp_editor.window->s_renderer->pending_update = true;
				}
				else if (action == BeingOverStart)
				{
					auto oth = er->entity->get_component(cSlot)->s;
					auto ok = (is_in ? bpSlot::can_link(s->type, oth->type) : bpSlot::can_link(oth->type, s->type));

					element->set_frame_color(ok ? cvec4(0, 255, 0, 255) : cvec4(255, 0, 0, 255));
					element->set_frame_thickness(2.f);

					if (!thiz->tip_link)
					{
						thiz->tip_link = ui.e_begin_layout(LayoutVertical, 4.f);
						auto c_element = thiz->tip_link->get_component(cElement);
						c_element->pos = thiz->element->global_pos + vec2(thiz->element->global_size.x, -8.f);
						c_element->pivot = vec2(0.5f, 1.f);
						c_element->padding = 4.f;
						c_element->frame_thickness = 2.f;
						c_element->color = cvec4(200, 200, 200, 255);
						c_element->frame_color = cvec4(0, 0, 0, 255);
						{
							auto s_type = s->type;
							auto o_type = oth->type;
							std::wstring str;
							if (is_in)
								str = s2w(o_type->name.str()) + (ok ? L"  =>  " : L"  ¡Ù>  ") + s2w(s_type->name.str());
							else
								str = s2w(s_type->name.str()) + (ok ? L"  =>  " : L"  ¡Ù>  ") + s2w(o_type->name.str());
							ui.e_text(str.c_str())->get_component(cText)->color = ok ? cvec4(0, 128, 0, 255) : cvec4(255, 0, 0, 255);
						}
						ui.e_end_layout();
						looper().add_event([](Capture& c) {
							bp_editor.window->root->add_child(c.thiz<Entity>());
						}, Capture().set_thiz(thiz->tip_link));
					}
				}
				else if (action == BeingOverEnd)
				{
					element->set_frame_thickness(0.f);

					thiz->clear_tips();
				}
				else if (action == BeenDropped)
				{
					auto oth = er->entity->get_component(cSlot)->s;
					if (s->io == bpSlotIn)
						bp_editor.set_links({ {s, oth} });
					else
						bp_editor.set_links({ {oth, s} });
				}

				return true;
			}, Capture().set_thiz(this));
			receiver->hover_listeners.add([](Capture& c, bool hovering) {
				auto& ui = bp_editor.window->ui;
				auto thiz = c.thiz<cSlot>();
				auto s = thiz->s;
				auto is_in = s->io == bpSlotIn;

				if (!hovering)
					thiz->clear_tips();
				else
				{
					if (!thiz->tip_info)
					{
						thiz->tip_info = ui.e_begin_layout(LayoutVertical, 8.f);
						auto c_element = thiz->tip_info->get_component(cElement);
						c_element->pos = thiz->element->global_pos + vec2(is_in ? -8.f : thiz->element->global_size.x + 8.f, 0.f);
						c_element->pivot = vec2(is_in ? 1.f : 0.f, 0.f);
						c_element->padding = 4.f;
						c_element->frame_thickness = 2.f;
						c_element->color = cvec4(200, 200, 200, 255);
						c_element->frame_color = cvec4(0, 0, 0, 255);
						auto type = s->type;
						auto tag = type->tag;
						ui.e_text((type_prefix(tag, type->is_array) + s2w(type->base_name.str())).c_str())->get_component(cText)->color = type_color(tag);
						auto text_value = ui.e_text(L"-")->get_component(cText);
						ui.current_entity = thiz->tip_info;
						ui.e_end_layout();
						thiz->tip_info->event_listeners.add([](Capture& c, EntityEvent e, void*) {
							if (e == EntityDestroyed)
								looper().remove_all_events(FLAME_CHASH("update_tip"));
							return true;
						}, Capture());
						looper().add_event([](Capture& c) {
							auto tip_info = c.thiz<Entity>();
							bp_editor.window->root->add_child(tip_info);
						}, Capture().set_thiz(thiz->tip_info), 0.f, FLAME_CHASH("update_tip"));
						struct Capturing
						{
							const TypeInfo* t;
							void* d;
							cText* text;
						}capture;
						capture.t = s->type;
						capture.d = s->data;
						capture.text = text_value;
						looper().add_event([](Capture& c) {
							auto& capture = c.data<Capturing>();
							capture.text->set_text(s2w(capture.t->serialize(capture.d)).c_str());
							c._current = INVALID_POINTER;
						}, Capture().set_data(&capture), 0.f);
					}
				}

				return true;
			}, Capture().set_thiz(this));
		}
		break;
	}
}

cNode::cNode() :
	Component("cNode")
{
	moved = false;

	dragging_slot = nullptr;
}

void cNode::on_event(EntityEvent e, void* t)
{
	switch (e)
	{
	case EntityComponentAdded:
		if (t == this)
		{
			element = entity->get_component(cElement);
			receiver = entity->get_component(cReceiver);

			receiver->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const ivec2& pos) {
				auto thiz = c.thiz<cNode>();
				if (is_mouse_down(action, key, true) && key == Mouse_Left)
				{
					auto n = thiz->n;
					for (auto& s : bp_editor.selected_nodes)
					{
						if (n == s)
						{
							n = nullptr;
							break;
						}
					}
					if (n)
						bp_editor.select({ n });
				}
				else if (is_mouse_move(action, key) && thiz->receiver->is_active())
				{
					for (auto& s : bp_editor.selected_nodes)
					{
						auto e = ((Entity*)s->user_data)->get_component(cElement);
						e->add_pos((vec2)pos / e->global_scale);
					}
					thiz->moved = true;
				}
				return true;
			}, Capture().set_thiz(this));
			receiver->state_listeners.add([](Capture& c, EventReceiverState) {
				auto thiz = c.thiz<cNode>();
				if (thiz->moved && !thiz->receiver->is_active())
				{
					std::vector<vec2> poses;
					for (auto& s : bp_editor.selected_nodes)
						poses.push_back(((Entity*)s->user_data)->get_component(cElement)->pos);
					bp_editor.set_nodes_pos(bp_editor.selected_nodes, poses);
					thiz->moved = false;
				}
				return true;
			}, Capture().set_thiz(this));
		}
		break;
	}
}

cBPEditor::cBPEditor() :
	Component("cBPEditor")
{
	auto& ui = bp_editor.window->ui;

	auto e_page = ui.e_begin_docker_page(L"Editor").second;
	{
		auto c_layout = ui.c_layout(LayoutVertical);
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;
		c_layout->fence = 2;
	}
	e_page->add_component(this);

	on_add_node(bp_editor.bp->root);

	ui.e_end_docker_page();
}

cBPEditor::~cBPEditor()
{
	bp_editor.editor = nullptr;
}

void cBPEditor::on_before_select()
{
	for (auto& s : bp_editor.selected_nodes)
	{
		auto e = (Entity*)s->user_data;
		if (e)
			e->get_component(cElement)->set_frame_color(unselected_col);
	}
}

void cBPEditor::on_after_select()
{
	for (auto& s : bp_editor.selected_nodes)
	{
		auto e = (Entity*)s->user_data;
		if (e)
			e->get_component(cElement)->set_frame_color(selected_col);
	}
}

void cBPEditor::on_pos_changed(bpNode* n)
{
	((Entity*)n->user_data)->get_component(cElement)->set_pos(n->pos);
}

void cBPEditor::on_add_node(bpNode* n)
{
	/* TODO
	ui.next_element_size = 300.f;
	{
		auto cl = ui.c_layout(LayoutVertical, 4.f);
		cl->width_fit_children = false;
		cl->height_fit_children = false;
		cl->fence = -2;
	}

	ui.parents.push(e_group);

		ui.e_begin_layout(LayoutHorizontal, 4.f);
			auto input = g->signal;

			ui.next_element_size = ui.style(FontSize).u[0];
			ui.next_element_roundness = ui.next_element_size.x * 0.4f;
			ui.next_element_roundness_lod = 2;
			ui.next_element_color = cvec4(200, 200, 200, 255);
			ui.e_element();
			ui.c_receiver();
			auto c_slot = new<cSlot>();
			c_slot->s = input;
			input->user_data = c_slot;
			ui.current_entity->add_component(c_slot);
			ui.e_begin_popup_menu(false);
			ui.e_menu_item(L"Break Link(s)", [](Capture& c) {
			}, Capture().set_thiz(input));
			ui.e_end_popup_menu();
			ui.push_style(TextColorNormal, common(type_color(input->type->tag)));
			auto e_name = ui.e_text(s2w(input->name.str()).c_str());
			ui.pop_style(TextColorNormal);
		ui.e_end_layout();

		edt.create(ui, [](Capture&, const vec4& r) {
			if (r.x == r.z && r.y == r.z)
				bp_editor.select();
			else
			{
				std::vector<bpNode*> nodes;
				for (auto n : bp_editor.bp->root->children)
				{
					auto e = ((Entity*)n->user_data)->get_component(cElement);
					if (rect_overlapping(r, rect(e->global_pos, e->global_pos + e->global_size)))
						nodes.push_back(n);
				}
				if (!nodes.empty())
				{
					bp_editor.select(nodes);
					return;
				}

				std::vector<bpSlot*> links;

				vec2 lines[8];
				lines[0] = vec2(r.x, r.y);
				lines[1] = vec2(r.z, r.y);
				lines[2] = vec2(r.x, r.y);
				lines[3] = vec2(r.x, r.w);
				lines[4] = vec2(r.z, r.w);
				lines[5] = vec2(r.x, r.w);
				lines[6] = vec2(r.z, r.w);
				lines[7] = vec2(r.z, r.y);

				auto& edt = bp_editor.editor->edt;

				auto range = rect(edt.element->global_pos, edt.element->global_pos + edt.element->global_size);
				auto scale = edt.base->global_scale;
				auto extent = slot_bezier_extent * scale;
				for (auto n : bp_editor.bp->root->children)
				{
					for (auto input : n->inputs)
					{
						auto output = input->links[0];
						if (!output)
							continue;
						auto p1 = ((cSlot*)output->user_data)->element->center();
						auto p4 = ((cSlot*)input->user_data)->element->center();
						auto p2 = p1 + vec2(extent, 0.f);
						auto p3 = p4 - vec2(extent, 0.f);
						auto bb = rect_from_points(p1, p2, p3, p4);
						if (rect_overlapping(bb, range))
						{
							std::vector<vec2> points;
							path_bezier(points, p1, p2, p3, p4);
							auto ok = false;
							for (auto k = 0; k < points.size() - 1; k++)
							{
								for (auto m = 0; m < 8; m += 2)
								{
									if (segment_intersect(lines[m], lines[m + 1], points[k], points[k + 1]))
									{
										links.push_back(input);
										ok = true;
										break;
									}
									if (ok)
										break;
								}
								if (ok)
									break;
							}
						}
					}
				}

				if (!links.empty())
					bp_editor.select(links);
			}
		}, Capture());
		edt.element->cmds.add([](Capture&, graphics::Canvas* canvas) {
			auto& edt = bp_editor.editor->edt;

			auto scale = edt.base->global_scale;
			auto extent = slot_bezier_extent * scale;
			auto range = rect(edt.element->global_pos, edt.element->global_pos + edt.element->global_size);
			auto line_width = 3.f * scale;
			for (auto n : bp_editor.bp->root->children)
			{
				for (auto output : n->outputs)
				{
					for (auto input : output->links)
					{
						auto e1 = ((cSlot*)output->user_data)->element;
						auto e2 = ((cSlot*)input->user_data)->element;
						if (e1->entity->global_visibility && e2->entity->global_visibility)
						{
							auto p1 = e1->center();
							auto p4 = e2->center();
							auto p2 = p1 + vec2(extent, 0.f);
							auto p3 = p4 - vec2(extent, 0.f);
							auto bb = rect_from_points(p1, p2, p3, p4);
							if (rect_overlapping(bb, range))
							{
								std::vector<vec2> points;
								path_bezier(points, p1, p2, p3, p4);
								auto selected = false;
								for (auto& s : bp_editor.selected_links)
								{
									if (s == input)
									{
										selected = true;
										break;
									}
								}
								canvas->stroke(points.size(), points.data(), selected ? selected_col : cvec4(100, 100, 120, 255), line_width);
							}
						}
					}
				}
			}
			auto ds = bp_editor.editor->dragging_slot;
			if (ds)
			{
				auto e1 = ((cSlot*)ds->user_data)->element;
				if (e1->entity->global_visibility)
				{
					auto p1 = e1->center();
					auto p4 = bp_editor.editor->dragging_slot_pos;
					auto is_in = ds->io == bpSlotIn;
					auto p2 = p1 + vec2(is_in ? -extent : extent, 0.f);
					auto p3 = p4 + vec2(is_in ? extent : -extent, 0.f);
					auto bb = rect_from_points(p1, p2, p3, p4);
					if (rect_overlapping(bb, range))
					{
						std::vector<vec2> points;
						path_bezier(points, p1, p2, p3, p4);
						canvas->stroke(points.size(), points.data(), selected_col, line_width);
					}
				}
			}
			return true;
		}, Capture());
		edt.receiver->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const ivec2& pos) {
			if (is_mouse_up(action, key, true) && key == Mouse_Right)
			{
				if (!bp_editor.editor->edt.moved)
					bp_editor.editor->show_add_node_menu(vec2(pos));
			}
			return true;
		}, Capture());

		ui.e_size_dragger();

		ui.e_bring_to_front();

	ui.parents.pop();

	*/

	auto& ui = bp_editor.window->ui;

	auto e_node = new<Entity>();
	ui.current_entity = e_node;
	n->user_data = e_node;
	ui.next_element_pos = n->pos;
	ui.next_element_padding = 8.f;
	ui.next_element_roundness = 8.f;
	ui.next_element_roundness_lod = 2;
	ui.next_element_frame_thickness = 4.f;
	ui.next_element_color = cvec4(255, 255, 255, 200);
	ui.next_element_frame_color = unselected_col;
	ui.c_element();
	ui.c_receiver();
	ui.c_layout(LayoutVertical, 4.f)->fence = -1;

	auto c_node = new<cNode>();
	c_node->n = n;
	auto n_type = bp_break_node_type(n->type.str());
	e_node->add_component(c_node);
	ui.e_begin_popup_menu(false);
		ui.e_menu_item(L"Duplicate", [](Capture& c) {
		}, Capture());
		ui.e_menu_item(L"Delete", [](Capture& c) {
		}, Capture());
	ui.e_end_popup_menu();

	ui.parents.push(e_node);

		if (n_type == 0)
		{
			ui.push_style(FontSize, common(Vec1u(20)));
			auto str = s2w(n->type.str());
			auto last_colon = str.find_last_of(L':');
			if (last_colon != std::wstring::npos)
				str = std::wstring(str.begin() + last_colon + 1, str.end());
			ui.next_element_padding = vec4(4.f, 2.f, 4.f, 2.f);
			ui.e_text(str.c_str())->get_component(cText)->color = node_type_color(n_type);
			ui.pop_style(FontSize);
		}

		auto type = n->type.str();

		ui.e_begin_layout(LayoutHorizontal, 16.f);
		ui.c_aligner(AlignMinMax | AlignGreedy, 0);

			ui.e_begin_layout(LayoutVertical, 4.f);
			ui.c_aligner(AlignMinMax | AlignGreedy, 0);
				for (auto input : n->inputs)
				{
					ui.e_begin_layout(LayoutHorizontal);
						ui.next_element_size = ui.style(FontSize).u[0];
						ui.next_element_roundness = ui.next_element_size.x * 0.4f;
						ui.next_element_roundness_lod = 2;
						ui.next_element_color = cvec4(200, 200, 200, 255);
						ui.e_element();
						ui.c_receiver();
						auto c_slot = new<cSlot>();
						c_slot->s = input;
						c_slot->group = c_node;
						input->user_data = c_slot;
						ui.current_entity->add_component(c_slot);
						ui.e_begin_popup_menu(false);
							ui.e_menu_item(L"Break Link(s)", [](Capture& c) {
							}, Capture().set_thiz(input));
							ui.e_menu_item(L"Reset Value", [](Capture& c) {
							}, Capture().set_thiz(input));
							if (n_type == 'A')
							{
								ui.e_menu_item(L"Remove Slot", [](Capture& c) {
									auto input = c.thiz<bpSlot>();
									auto n = (bpNode*)input->node;
									if (n->inputs.s == 1)
										return;
									bp_editor.select();
									auto idx = input->index;
									auto type = n->type.str();
									auto id = n->id.str();
									auto left_pos = type.find('(');
									auto plus_pos = type.find('+');
									auto size = std::stoi(std::string(type.begin() + left_pos + 1, type.begin() + plus_pos));
									type = std::string(type.begin(), type.begin() + left_pos + 1) + std::to_string(size - 1) + std::string(type.begin() + plus_pos, type.end());
									NodeDesc d;
									d.id = "";
									d.type = type;
									d.node_type = bpNodeReal;
									d.pos = n->pos;
									auto nn = bp_editor.add_node(d);
									for (auto i = 0; i < n->inputs.s; i++)
									{
										if (i == idx)
											continue;
										auto src = n->inputs[i];
										auto dst = nn->inputs[i > idx ? i - 1 : i];
										dst->set_data(src->data);
										dst->link_to(src->links[0]);
									}
									for (auto i = 0; i < n->outputs.s; i++)
									{
										auto src = n->outputs[i];
										auto dst = nn->outputs[i];
										for (auto l : src->links)
											l->link_to(dst);
									}
									bp_editor.remove_nodes({ n });
									nn->set_id(id.c_str());
								}, Capture().set_thiz(input));
							}
						ui.e_end_popup_menu();

						ui.push_style(TextColorNormal, common(type_color(input->type->tag)));
						auto e_name = ui.e_text(s2w(input->name.str()).c_str());
						ui.pop_style(TextColorNormal);
					ui.e_end_layout();

					auto type = input->type;
					auto tag = type->tag;
					if (!input->links[0] && tag != TagPointer)
					{
						auto base_name = type->base_name.str();
						auto base_hash = type->base_hash;

						switch (tag)
						{
						case TagEnumSingle:
						{
							cCombobox* combobox;

							auto info = find_enum(base_hash);
							combobox = ui.e_begin_combobox()->get_component(cCombobox);
							ui.current_entity = combobox->entity;
							ui.c_aligner(AlignMin, 0)->margin.x = ui.style(FontSize).u.x;
							for (auto item : info->items)
								ui.e_combobox_item(s2w(item->name.str()).c_str());
							ui.e_end_combobox();

							e_name->add_component(new<cEnumSingleDataTracker>(input->data, info, [](Capture& c, int v) {
								bp_editor.set_data(c.thiz<bpSlot>(), &v, true);
							}, Capture().set_thiz(input), combobox));
						}
							break;
						case TagEnumMulti:
						{
							std::vector<cCheckbox*> checkboxes;

							ui.e_begin_layout(LayoutVertical, 4.f);
							ui.c_aligner(AlignMin, 0)->margin.x = ui.style(FontSize).u.x;
							auto info = find_enum(base_hash);
							for (auto item : info->items)
							{
								ui.e_begin_layout(LayoutHorizontal, 4.f);
								checkboxes.push_back(ui.e_checkbox()->get_component(cCheckbox));
								ui.e_text(s2w(item->name.str()).c_str());
								ui.e_end_layout();
							}
							ui.e_end_layout();

							e_name->add_component(new<cEnumMultiDataTracker>(input->data, info, [](Capture& c, int v) {
								bp_editor.set_data(c.thiz<bpSlot>(), &v, true);
							}, Capture().set_thiz(input), checkboxes));
						}
							break;
						case TagD:
							switch (base_hash)
							{
							case FLAME_CHASH("bool"):
							{
								cCheckbox* checkbox;

								checkbox = ui.e_checkbox()->get_component(cCheckbox);
								ui.c_aligner(AlignMin, 0)->margin.x = ui.style(FontSize).u.x;

								e_name->add_component(new<cBoolDataTracker>(input->data, [](Capture& c, bool v) {
									bp_editor.set_data(c.thiz<bpSlot>(), &v, true);
								}, Capture().set_thiz(input), checkbox));
							}
								break;
							case FLAME_CHASH("int"):
							{
								cText* edit_text;
								cText* drag_text;

								auto e = ui.e_drag_edit();
								ui.c_aligner(AlignMin, 0)->margin.x = ui.style(FontSize).u.x;
								edit_text = e->children[0]->get_component(cText);
								drag_text = e->children[1]->get_component(cText);

								e_name->add_component(new<cDigitalDataTracker<int>>(input->data, [](Capture& c, int v, bool exit_editing) {
									if (exit_editing)
										bp_editor.set_data(c.thiz<bpSlot>(), &v, true);
									else
										c.thiz<bpSlot>()->set_data(&v);
								}, Capture().set_thiz(input), edit_text, drag_text));
							}
								break;
							case FLAME_CHASH("flame::Vec<2,int>"):
							{
								std::array<cText*, 2> edit_texts;
								std::array<cText*, 2> drag_texts;

								ui.e_begin_layout(LayoutHorizontal, 4.f);
								ui.c_aligner(AlignMin, 0)->margin.x = ui.style(FontSize).u.x;
								for (auto i = 0; i < 2; i++)
								{
									auto e = ui.e_drag_edit();
									edit_texts[i] = e->children[0]->get_component(cText);
									drag_texts[i] = e->children[1]->get_component(cText);
								}
								ui.e_end_layout();

								auto p = e_name;
								p->get_component(cLayout)->type = LayoutHorizontal;
								p->add_component(new<cDigitalVecDataTracker<2, int>>(input->data, [](Capture& c, const Vec<2, int>& v, bool exit_editing) {
									if (exit_editing)
										bp_editor.set_data(c.thiz<bpSlot>(), (void*)&v, true);
									else
										c.thiz<bpSlot>()->set_data((void*)&v);
								}, Capture().set_thiz(input), edit_texts, drag_texts));
							}
								break;
							case FLAME_CHASH("flame::Vec<3,int>"):
							{
								std::array<cText*, 3> edit_texts;
								std::array<cText*, 3> drag_texts;

								ui.e_begin_layout(LayoutHorizontal, 4.f);
								ui.c_aligner(AlignMin, 0)->margin.x = ui.style(FontSize).u.x;
								for (auto i = 0; i < 3; i++)
								{
									auto e = ui.e_drag_edit();
									edit_texts[i] = e->children[0]->get_component(cText);
									drag_texts[i] = e->children[1]->get_component(cText);
								}
								ui.e_end_layout();

								e_name->add_component(new<cDigitalVecDataTracker<3, int>>(input->data, [](Capture& c, const Vec<3, int>& v, bool exit_editing) {
									if (exit_editing)
										bp_editor.set_data(c.thiz<bpSlot>(), (void*)&v, true);
									else
										c.thiz<bpSlot>()->set_data((void*)&v);
								}, Capture().set_thiz(input), edit_texts, drag_texts));
							}
								break;
							case FLAME_CHASH("flame::Vec<4,int>"):
							{
								std::array<cText*, 4> edit_texts;
								std::array<cText*, 4> drag_texts;

								ui.e_begin_layout(LayoutHorizontal, 4.f);
								ui.c_aligner(AlignMin, 0)->margin.x = ui.style(FontSize).u.x;
								for (auto i = 0; i < 4; i++)
								{
									auto e = ui.e_drag_edit();
									edit_texts[i] = e->children[0]->get_component(cText);
									drag_texts[i] = e->children[1]->get_component(cText);
								}
								ui.e_end_layout();

								e_name->add_component(new<cDigitalVecDataTracker<4, int>>(input->data, [](Capture& c, const Vec<4, int>& v, bool exit_editing) {
									if (exit_editing)
										bp_editor.set_data(c.thiz<bpSlot>(), (void*)&v, true);
									else
										c.thiz<bpSlot>()->set_data((void*)&v);
								}, Capture().set_thiz(input), edit_texts, drag_texts));
							}
								break;
							case FLAME_CHASH("uint"):
							{
								cText* edit_text;
								cText* drag_text;

								auto e = ui.e_drag_edit();
								ui.c_aligner(AlignMin, 0)->margin.x = ui.style(FontSize).u.x;
								edit_text = e->children[0]->get_component(cText);
								drag_text = e->children[1]->get_component(cText);

								e_name->add_component(new<cDigitalDataTracker<uint>>(input->data, [](Capture& c, uint v, bool exit_editing) {
									if (exit_editing)
										bp_editor.set_data(c.thiz<bpSlot>(), &v, true);
									else
										c.thiz<bpSlot>()->set_data(&v);
								}, Capture().set_thiz(input), edit_text, drag_text));
							}
								break;
							case FLAME_CHASH("flame::Vec<2,uint>"):
							{
								std::array<cText*, 2> edit_texts;
								std::array<cText*, 2> drag_texts;

								ui.e_begin_layout(LayoutHorizontal, 4.f);
								ui.c_aligner(AlignMin, 0)->margin.x = ui.style(FontSize).u.x;
								for (auto i = 0; i < 2; i++)
								{
									auto e = ui.e_drag_edit();
									edit_texts[i] = e->children[0]->get_component(cText);
									drag_texts[i] = e->children[1]->get_component(cText);
								}
								ui.e_end_layout();

								e_name->add_component(new<cDigitalVecDataTracker<2, uint>>(input->data, [](Capture& c, const Vec<2, uint>& v, bool exit_editing) {
									if (exit_editing)
										bp_editor.set_data(c.thiz<bpSlot>(), (void*)&v, true);
									else
										c.thiz<bpSlot>()->set_data((void*)&v);
								}, Capture().set_thiz(input), edit_texts, drag_texts));
							}
								break;
							case FLAME_CHASH("flame::Vec<3,uint>"):
							{
								std::array<cText*, 3> edit_texts;
								std::array<cText*, 3> drag_texts;

								ui.e_begin_layout(LayoutHorizontal, 4.f);
								ui.c_aligner(AlignMin, 0)->margin.x = ui.style(FontSize).u.x;
								for (auto i = 0; i < 3; i++)
								{
									auto e = ui.e_drag_edit();
									edit_texts[i] = e->children[0]->get_component(cText);
									drag_texts[i] = e->children[1]->get_component(cText);
								}
								ui.e_end_layout();

								e_name->add_component(new<cDigitalVecDataTracker<3, uint>>(input->data, [](Capture& c, const Vec<3, uint>& v, bool exit_editing) {
									if (exit_editing)
										bp_editor.set_data(c.thiz<bpSlot>(), (void*)&v, true);
									else
										c.thiz<bpSlot>()->set_data((void*)&v);
								}, Capture().set_thiz(input), edit_texts, drag_texts));
							}
								break;
							case FLAME_CHASH("flame::Vec<4,uint>"):
							{
								std::array<cText*, 4> edit_texts;
								std::array<cText*, 4> drag_texts;

								ui.e_begin_layout(LayoutHorizontal, 4.f);
								ui.c_aligner(AlignMin, 0)->margin.x = ui.style(FontSize).u.x;
								for (auto i = 0; i < 4; i++)
								{
									auto e = ui.e_drag_edit();
									edit_texts[i] = e->children[0]->get_component(cText);
									drag_texts[i] = e->children[1]->get_component(cText);
								}
								ui.e_end_layout();

								e_name->add_component(new<cDigitalVecDataTracker<4, uint>>(input->data, [](Capture& c, const Vec<4, uint>& v, bool exit_editing) {
									if (exit_editing)
										bp_editor.set_data(c.thiz<bpSlot>(), (void*)&v, true);
									else
										c.thiz<bpSlot>()->set_data((void*)&v);
								}, Capture().set_thiz(input), edit_texts, drag_texts));
							}
								break;
							case FLAME_CHASH("float"):
							{
								cText* edit_text;
								cText* drag_text;

								auto e = ui.e_drag_edit();
								ui.c_aligner(AlignMin, 0)->margin.x = ui.style(FontSize).u.x;
								edit_text = e->children[0]->get_component(cText);
								drag_text = e->children[1]->get_component(cText);

								e_name->add_component(new<cDigitalDataTracker<float>>(input->data, [](Capture& c, float v, bool exit_editing) {
									if (exit_editing)
										bp_editor.set_data(c.thiz<bpSlot>(), &v, true);
									else
										c.thiz<bpSlot>()->set_data(&v);
								}, Capture().set_thiz(input), edit_text, drag_text));
							}
								break;
							case FLAME_CHASH("flame::Vec<2,float>"):
							{
								std::array<cText*, 2> edit_texts;
								std::array<cText*, 2> drag_texts;

								ui.e_begin_layout(LayoutHorizontal, 4.f);
								ui.c_aligner(AlignMin, 0)->margin.x = ui.style(FontSize).u.x;
								for (auto i = 0; i < 2; i++)
								{
									auto e = ui.e_drag_edit();
									edit_texts[i] = e->children[0]->get_component(cText);
									drag_texts[i] = e->children[1]->get_component(cText);
								}
								ui.e_end_layout();

								e_name->add_component(new<cDigitalVecDataTracker<2, float>>(input->data, [](Capture& c, const Vec<2, float>& v, bool exit_editing) {
									if (exit_editing)
										bp_editor.set_data(c.thiz<bpSlot>(), (void*)&v, true);
									else
										c.thiz<bpSlot>()->set_data((void*)&v);
								}, Capture().set_thiz(input), edit_texts, drag_texts));
							}
								break;
							case FLAME_CHASH("flame::Vec<3,float>"):
							{
								std::array<cText*, 3> edit_texts;
								std::array<cText*, 3> drag_texts;

								ui.e_begin_layout(LayoutHorizontal, 4.f);
								ui.c_aligner(AlignMin, 0)->margin.x = ui.style(FontSize).u.x;
								for (auto i = 0; i < 3; i++)
								{
									auto e = ui.e_drag_edit();
									edit_texts[i] = e->children[0]->get_component(cText);
									drag_texts[i] = e->children[1]->get_component(cText);
								}
								ui.e_end_layout();

								e_name->add_component(new<cDigitalVecDataTracker<3, float>>(input->data, [](Capture& c, const Vec<3, float>& v, bool exit_editing) {
									if (exit_editing)
										bp_editor.set_data(c.thiz<bpSlot>(), (void*)&v, true);
									else
										c.thiz<bpSlot>()->set_data((void*)&v);
								}, Capture().set_thiz(input), edit_texts, drag_texts));
							}
								break;
							case FLAME_CHASH("flame::Vec<4,float>"):
							{
								std::array<cText*, 4> edit_texts;
								std::array<cText*, 4> drag_texts;

								ui.e_begin_layout(LayoutHorizontal, 4.f);
								ui.c_aligner(AlignMin, 0)->margin.x = ui.style(FontSize).u.x;
								for (auto i = 0; i < 4; i++)
								{
									auto e = ui.e_drag_edit();
									edit_texts[i] = e->children[0]->get_component(cText);
									drag_texts[i] = e->children[1]->get_component(cText);
								}
								ui.e_end_layout();

								e_name->add_component(new<cDigitalVecDataTracker<4, float>>(input->data, [](Capture& c, const Vec<4, float>& v, bool exit_editing) {
									if (exit_editing)
										bp_editor.set_data(c.thiz<bpSlot>(), (void*)&v, true);
									else
										c.thiz<bpSlot>()->set_data((void*)&v);
								}, Capture().set_thiz(input), edit_texts, drag_texts));
							}
								break;
							case FLAME_CHASH("uchar"):
							{
								cText* edit_text;
								cText* drag_text;

								auto e = ui.e_drag_edit();
								ui.c_aligner(AlignMin, 0)->margin.x = ui.style(FontSize).u.x;
								edit_text = e->children[0]->get_component(cText);
								drag_text = e->children[1]->get_component(cText);

								e_name->add_component(new<cDigitalDataTracker<uchar>>(input->data, [](Capture& c, uchar v, bool exit_editing) {
									if (exit_editing)
										bp_editor.set_data(c.thiz<bpSlot>(), &v, true);
									else
										c.thiz<bpSlot>()->set_data(&v);
								}, Capture().set_thiz(input), edit_text, drag_text));
							}
								break;
							case FLAME_CHASH("flame::Vec<2,uchar>"):
							{
								std::array<cText*, 2> edit_texts;
								std::array<cText*, 2> drag_texts;

								ui.e_begin_layout(LayoutHorizontal, 4.f);
								ui.c_aligner(AlignMin, 0)->margin.x = ui.style(FontSize).u.x;
								for (auto i = 0; i < 2; i++)
								{
									auto e = ui.e_drag_edit();
									edit_texts[i] = e->children[0]->get_component(cText);
									drag_texts[i] = e->children[1]->get_component(cText);
								}
								ui.e_end_layout();

								e_name->add_component(new<cDigitalVecDataTracker<2, uchar>>(input->data, [](Capture& c, const Vec<2, uchar>& v, bool exit_editing) {
									if (exit_editing)
										bp_editor.set_data(c.thiz<bpSlot>(), (void*)&v, true);
									else
										c.thiz<bpSlot>()->set_data((void*)&v);
								}, Capture().set_thiz(input), edit_texts, drag_texts));
							}
								break;
							case FLAME_CHASH("flame::Vec<3,uchar>"):
							{
								std::array<cText*, 3> edit_texts;
								std::array<cText*, 3> drag_texts;

								ui.e_begin_layout(LayoutHorizontal, 4.f);
								ui.c_aligner(AlignMin, 0)->margin.x = ui.style(FontSize).u.x;
								for (auto i = 0; i < 3; i++)
								{
									auto e = ui.e_drag_edit();
									edit_texts[i] = e->children[0]->get_component(cText);
									drag_texts[i] = e->children[1]->get_component(cText);
								}
								ui.e_end_layout();

								e_name->add_component(new<cDigitalVecDataTracker<3, uchar>>(input->data, [](Capture& c, const Vec<3, uchar>& v, bool exit_editing) {
									if (exit_editing)
										bp_editor.set_data(c.thiz<bpSlot>(), (void*)&v, true);
									else
										c.thiz<bpSlot>()->set_data((void*)&v);
								}, Capture().set_thiz(input), edit_texts, drag_texts));
							}
								break;
							case FLAME_CHASH("flame::Vec<4,uchar>"):
							{
								std::array<cText*, 4> edit_texts;
								std::array<cText*, 4> drag_texts;

								ui.e_begin_layout(LayoutHorizontal, 4.f);
								ui.c_aligner(AlignMin, 0)->margin.x = ui.style(FontSize).u.x;
								for (auto i = 0; i < 4; i++)
								{
									auto e = ui.e_drag_edit();
									edit_texts[i] = e->children[0]->get_component(cText);
									drag_texts[i] = e->children[1]->get_component(cText);
								}
								ui.e_end_layout();

								e_name->add_component(new<cDigitalVecDataTracker<4, uchar>>(input->data, [](Capture& c, const Vec<4, uchar>& v, bool exit_editing) {
									if (exit_editing)
										bp_editor.set_data(c.thiz<bpSlot>(), (void*)&v, true);
									else
										c.thiz<bpSlot>()->set_data((void*)&v);
								}, Capture().set_thiz(input), edit_texts, drag_texts));
							}
								break;
							case FLAME_CHASH("flame::StringA"):
							{
								cText* text;

								text = ui.e_edit(50.f)->get_component(cText);
								ui.c_aligner(AlignMin, 0)->margin.x = ui.style(FontSize).u.x;

								e_name->add_component(new<cStringADataTracker>(input->data, [](Capture& c, const char* v) {
									StringA d = v;
									bp_editor.set_data(c.thiz<bpSlot>(), &d, true);
								}, Capture().set_thiz(input), text));
							}
								break;
							case FLAME_CHASH("flame::StringW"):
							{
								cText* text;

								text = ui.e_edit(50.f)->get_component(cText);
								ui.c_aligner(AlignMin, 0)->margin.x = ui.style(FontSize).u.x;

								e_name->add_component(new<cStringWDataTracker>(input->data, [](Capture& c, const wchar_t* v) {
									StringW d = v;
									bp_editor.set_data(c.thiz<bpSlot>(), &d, true);
								}, Capture().set_thiz(input), text));
							}
								break;
							}
							break;
						}

						c_slot->tracker = e_name->get_component(cDataTracker);
					}
				}
			ui.e_end_layout();

			ui.e_begin_layout(LayoutVertical);
			ui.c_aligner(AlignMinMax | AlignGreedy, 0);
				for (auto output : n->outputs)
				{
					ui.e_begin_layout(LayoutHorizontal);
					ui.c_aligner(AlignMax, 0);
					ui.e_text(s2w(output->name.str()).c_str())->get_component(cText)->color = type_color(output->type->tag);

					ui.next_element_size = ui.style(FontSize).u[0];
					ui.next_element_roundness = ui.next_element_size.x * 0.4f;
					ui.next_element_roundness_lod = 2;
					ui.next_element_color = cvec4(200, 200, 200, 255);
					ui.e_element();
					ui.c_receiver();
					auto c_slot = new<cSlot>();
					c_slot->s = output;
					c_slot->group = c_node;
					ui.current_entity->add_component(c_slot);
					output->user_data = c_slot;
					ui.e_begin_popup_menu(false);
						ui.e_menu_item(L"Break Link(s)", [](Capture& c) {
						}, Capture());
					ui.e_end_popup_menu();
					ui.e_end_layout();
				}
			ui.e_end_layout();
		ui.e_end_layout();

		if (n_type == 'A')
		{
			ui.next_element_padding = vec4(5.f, 2.f, 5.f, 2.f);
			ui.next_element_roundness = 8.f;
			ui.next_element_roundness_lod = 2;
			ui.e_button(L"+", [](Capture& c) {
				auto n = c.thiz<bpNode>();
				bp_editor.select();
				std::string type = n->type.str();
				std::string id = n->id.str();
				{
					auto left_pos = type.find('(');
					auto plus_pos = type.find('+');
					auto size = std::stoi(std::string(type.begin() + left_pos + 1, type.begin() + plus_pos));
					type = std::string(type.begin(), type.begin() + left_pos + 1) + std::to_string(size + 1) + std::string(type.begin() + plus_pos, type.end());
					NodeDesc d;
					d.id = "";
					d.type = type;
					d.node_type = bpNodeReal;
					d.pos = n->pos;
					auto nn = bp_editor.add_node(d);
					for (auto i = 0; i < n->inputs.s; i++)
					{
						auto src = n->inputs[i];
						auto dst = nn->inputs[i];
						dst->set_data(src->data);
						dst->link_to(src->links[0]);
					}
					for (auto i = 0; i < n->outputs.s; i++)
					{
						auto src = n->outputs[i];
						auto dst = nn->outputs[i];
						for (auto l : src->links)
							l->link_to(dst);
					}
					bp_editor.remove_nodes({ n });
					nn->set_id(id.c_str());
				}
			}, Capture().set_thiz(n));
			ui.c_aligner(AlignMiddle, 0);
		}

		ui.e_bring_to_front();
	ui.parents.pop();

	looper().add_event([](Capture& c) {
		auto e_node = c.thiz<Entity>();
		auto n = e_node->get_component(cNode)->n;
		((Entity*)n->parent->user_data)->get_component(cNode)->edt.base->entity->add_child(e_node);
	}, Capture().set_thiz(e_node));

	for (auto g : bp_editor.bp->groups)
	{
		if (g->id != "")
			on_add_group(g);
		for (auto n : g->children)
			on_add_node(n);
	}
}

void cBPEditor::on_remove_node(bpNode* n)
{
	auto e = (Entity*)n->user_data;
	e->parent->remove_child(e);
}

void cBPEditor::on_data_changed(bpSlot* s)
{
	((cSlot*)s->user_data)->tracker->update_view();
}

void cBPEditor::show_add_node_menu(const vec2& pos)
{
	const TypeInfo* type;
	TypeTag tag;
	const char* base_name;
	uint base_hash;
	bool is_array;
	if (dragging_slot)
	{
		type = dragging_slot->type;
		tag = type->tag;
		base_name = type->base_name.v;
		base_hash = type->base_hash;
		is_array = type->is_array;
	}

	struct NodeType
	{
		UdtInfo* u;
		VariableInfo* v;
		bpNodeType t;
	};
	std::vector<NodeType> node_types;
	for (auto db : global_typeinfo_databases)
	{
		for (auto u : db->udts.get_all())
		{
			auto f_update = u->find_function("bp_update");
			if (f_update && check_function(f_update, "D#void", {}))
			{
				if (!dragging_slot)
					node_types.push_back({ u, nullptr, bpNodeReal });
				else
				{
					auto ds_io = dragging_slot->io;
					auto ds_t = dragging_slot->type;
					auto found = false;
					for (auto v : u->variables)
					{
						auto flags = v->flags;
						if (ds_io == bpSlotOut && (flags & VariableFlagInput) && bpSlot::can_link(v->type, ds_t))
						{
							node_types.push_back({ u, v, bpNodeReal });
							found = true;
							break;
						}
						if (ds_io == bpSlotIn && (flags & VariableFlagOutput) && bpSlot::can_link(ds_t, v->type))
						{
							node_types.push_back({ u, v, bpNodeReal });
							found = true;
							break;
						}
						if (found)
							break;
					}
				}
			}
			else
			{
				if (!dragging_slot)
				{
					node_types.push_back({ u, nullptr, bpNodeRefRead });
					node_types.push_back({ u, nullptr, bpNodeRefWrite });
				}
				else
				{
					auto ds_io = dragging_slot->io;
					auto ds_t = dragging_slot->type;
					auto found = false;
					for (auto v : u->variables)
					{
						if (ds_io == bpSlotOut && bpSlot::can_link(v->type, ds_t))
						{
							node_types.push_back({ u, v, bpNodeRefWrite });
							found = true;
							break;
						}
						if (ds_io == bpSlotIn && bpSlot::can_link(ds_t, v->type))
						{
							node_types.push_back({ u, v, bpNodeRefRead });
							found = true;
							break;
						}
						if (found)
							break;
					}
				}
			}
		}
	}
	std::sort(node_types.begin(), node_types.end(), [](const auto& a, const auto& b) {
		return a.u->name.str() < b.u->name.str();
	});

	auto& ui = bp_editor.window->ui;

	ui.parents.push(add_layer(bp_editor.window->root, ""));
		ui.next_element_pos = pos;
		ui.next_element_padding = 4.f;
		ui.next_element_frame_thickness = 2.f;
		ui.next_element_color = ui.style(BackgroundColor).c;
		ui.next_element_frame_color = ui.style(ForegroundColor).c;
		ui.e_element()->event_listeners.add([](Capture&, EntityEvent e, void* t) {
			if (e == EntityRemoved)
				bp_editor.editor->dragging_slot = nullptr;
			return true;
		}, Capture());
		ui.c_layout(LayoutVertical)->item_padding = 4.f;
		ui.parents.push(ui.current_entity);
			if (dragging_slot)
				ui.e_text((L"Filtered For: " + s2w(type->name.str())).c_str())->get_component(cText)->color = type_color(tag);
			ui.e_begin_layout(LayoutHorizontal, 4.f);
				ui.e_text(Icon_SEARCH);
				auto c_text_search = ui.e_edit(300.f)->get_component(cText);
			ui.e_end_layout();
			ui.next_element_size = vec2(0.f, 300.f);
			ui.next_element_padding = 4.f;
			ui.e_begin_scrollbar(ScrollbarVertical, false);
			ui.c_aligner(AlignMinMax | AlignGreedy, 0);
				auto e_list = ui.e_begin_list(true);
					struct Capturing
					{
						Entity* l;
						vec2 p;

						void show_enums(TypeTag tag)
						{
							auto& ui = bp_editor.window->ui;

							std::vector<EnumInfo*> enum_infos;
							for (auto db : global_typeinfo_databases)
							{
								for (auto e : db->enums.get_all())
									enum_infos.push_back(e);
							}
							std::sort(enum_infos.begin(), enum_infos.end(), [](EnumInfo* a, EnumInfo* b) {
								return a->name.str() < b->name.str();
							});

							struct _Capturing
							{
								TypeTag t;
								const char* s;
								vec2 p;
							}capture;
							capture.t = tag;
							capture.p = p;
							l->remove_children(0, -1);
							for (auto ei : enum_infos)
							{
								capture.s = ei->name.v;
								ui.parents.push(l);
								ui.e_menu_item(s2w(ei->name.v).c_str(), [](Capture& c) {
									auto& capture = c.data<_Capturing>();
									NodeDesc d;
									d.id = "";
									d.type = capture.t == TagEnumSingle ? "EnumSingle(" : "EnumMulti(";
									d.type += capture.s;
									d.type += ")";
									d.node_type = bpNodeReal;
									d.pos = capture.p;
									bp_editor.add_node(d);
								}, Capture().set_data(&capture));
								ui.parents.pop();
							}
						}
					}capture;
					capture.l = e_list;
					capture.p = (vec2(pos) - edt.base->global_pos) / (edt.scale_level * 0.1f);
					if (!dragging_slot)
					{
						ui.e_menu_item(L"Enum Single", [](Capture& c) {
							auto& capture = c.data<Capturing>();
							looper().add_event([](Capture& c) {
								auto& capture = c.data<Capturing>();
								capture.show_enums(TagEnumSingle);
							}, Capture().set_data(&capture));
						}, Capture().set_data(&capture), false);
						ui.e_menu_item(L"Enum Multi", [](Capture& c) {
							auto& capture = c.data<Capturing>();
							looper().add_event([](Capture& c) {
								auto& capture = c.data<Capturing>();
								capture.show_enums(TagEnumMulti);
							}, Capture().set_data(&capture));
						}, Capture().set_data(&capture), false);
						ui.e_menu_item(L"Variable", [](Capture& c) {
							auto& capture = c.data<Capturing>();
							looper().add_event([](Capture& c) {
								auto& ui = bp_editor.window->ui;
								auto& capture = c.data<Capturing>();
								struct _Capturing
								{
									const char* s;
									vec2 p;
								}_capture;
								_capture.p = capture.p;
								capture.l->remove_children(0, -1);
								for (auto t : basic_types)
								{
									_capture.s = t;
									ui.parents.push(capture.l);
									ui.e_menu_item(s2w(t).c_str(), [](Capture& c) {
										auto& capture = c.data<_Capturing>();
										NodeDesc d;
										d.id = "";
										d.type = "Variable(";
										d.type += capture.s;
										d.type += ")";
										d.node_type = bpNodeReal;
										d.pos = capture.p;
										bp_editor.add_node(d);
									}, Capture().set_data(&_capture));
									ui.parents.pop();
								}
							}, Capture().set_data(&capture));
						}, Capture().set_data(&capture), false);
						ui.e_menu_item(L"Array", [](Capture& c) {
							auto& capture = c.data<Capturing>();
							looper().add_event([](Capture& c) {
								auto& ui = bp_editor.window->ui;
								auto& capture = c.data<Capturing>();
								struct _Capturing
								{
									const char* s;
									vec2 p;
								}_capture;
								_capture.p = capture.p;
								capture.l->remove_children(0, -1);
								for (auto t : basic_types)
								{
									_capture.s = t;
									ui.parents.push(capture.l);
									ui.e_menu_item(s2w(t).c_str(), [](Capture& c) {
										auto& capture = c.data<_Capturing>();
										NodeDesc d;
										d.id = "";
										d.type = "Array(1+";
										d.type += capture.s;
										d.type += ")";
										d.node_type = bpNodeReal;
										d.pos = capture.p;
										bp_editor.add_node(d);
									}, Capture().set_data(&_capture));
									ui.parents.pop();
								}
							}, Capture().set_data(&capture));
						}, Capture().set_data(&capture), false);
					}
					else
					{
						if (tag == TagEnumSingle || tag == TagEnumMulti)
						{
							struct _Capturing
							{
								TypeTag t;
								const char* s;
								vec2 p;
							}_capture;
							_capture.t = tag;
							_capture.p = capture.p;
							_capture.s = base_name;
							ui.e_menu_item(((tag == TagEnumSingle ? L"Enum Single: " : L"Enum Multi: ") + s2w(base_name)).c_str(), [](Capture& c) {
								auto& capture = c.data<_Capturing>();
								NodeDesc d;
								d.id = "";
								d.type = capture.t == TagEnumSingle ? "EnumSingle(" : "EnumMulti(";
								d.type += capture.s;
								d.type += ")";
								d.node_type = bpNodeReal;
								d.pos = capture.p;
								auto n = bp_editor.add_node(d);
								auto s = bp_editor.editor->dragging_slot;
								if (s)
								{
									if (s->io == bpSlotIn)
										bp_editor._set_link(s, n->find_output("out"));
									else
										bp_editor._set_link(n->find_input("in"), s);
								}
							}, Capture().set_data(&_capture));
						}
						else if (tag == TagD && !is_array)
						{
							if (basic_type_size(base_hash))
							{
								struct _Capturing
								{
									const char* s;
									vec2 p;
								}_capture;
								_capture.p = capture.p;
								_capture.s = base_name;
								ui.e_menu_item((L"Variable: " + s2w(base_name)).c_str(), [](Capture& c) {
									auto& capture = c.data<_Capturing>();
									NodeDesc d;
									d.id = "";
									d.type = "Variable(";
									d.type += capture.s;
									d.type += ")";
									d.node_type = bpNodeReal;
									d.pos = capture.p;
									auto n = bp_editor.add_node(d);
									auto s = bp_editor.editor->dragging_slot;
									if (s)
									{
										if (s->io == bpSlotIn)
											bp_editor._set_link(s, n->find_output("out"));
										else
											bp_editor._set_link(n->find_input("in"), s);
									}
								}, Capture().set_data(&_capture));
							}
						}
						else if (tag == TagPointer && is_array)
						{
							struct _Capturing
							{
								const char* s;
								vec2 p;
							}_capture;
							_capture.p = capture.p;
							_capture.s = base_name;
							ui.e_menu_item((L"Array: " + s2w(base_name)).c_str(), [](Capture& c) {
								auto& capture = c.data<_Capturing>();
								NodeDesc d;
								d.id = "";
								d.type = "Array(1+";
								d.type += capture.s;
								d.type += ")";
								d.node_type = bpNodeReal;
								d.pos = capture.p;
								auto n = bp_editor.add_node(d);
								auto s = bp_editor.editor->dragging_slot;
								if (s)
								{
									if (s->io == bpSlotIn)
										bp_editor._set_link(s, n->find_output("out"));
									else
										bp_editor._set_link(n->find_input("0"), s);
								}
							}, Capture().set_data(&_capture));
						}
					}
					{
						struct _Capturing
						{
							bpNodeType t;
							const char* us;
							const char* vs;
							vec2 p;
						}capture;
						capture.p = (vec2(pos) - edt.base->global_pos) / (edt.scale_level * 0.1f);
						for (auto& t : node_types)
						{
							capture.t = t.t;
							capture.us = t.u->name.v;
							capture.vs = t.v ? t.v->name.v : nullptr;
							auto name = s2w(t.u->name.str());
							if (capture.t == bpNodeRefRead)
								name += L" (RefRead)";
							else if (capture.t == bpNodeRefWrite)
								name += L" (RefWrite)";
							ui.e_menu_item(name.c_str(), [](Capture& c) {
								auto& capture = c.data<_Capturing>();
								NodeDesc d;
								d.node_type = capture.t;
								d.type = capture.us;
								d.id = "";
								d.pos = capture.p;
								auto n = bp_editor.add_node(d);
								auto s = bp_editor.editor->dragging_slot;
								if (s)
								{
									if (s->io == bpSlotIn)
										bp_editor._set_link(s, n->find_output(capture.vs));
									else
										bp_editor._set_link(n->find_input(capture.vs), s);
								}
							}, Capture().set_data(&capture));
						}
					}
				ui.e_end_list();
			ui.e_end_scrollbar();
		ui.parents.pop();
	ui.parents.pop();

	c_text_search->data_changed_listeners.add([](Capture& c, uint hash, void*) {
		if (hash == FLAME_CHASH("text"))
		{
			auto l = c.thiz<Entity>();
			auto str = c.current<cText>()->text.str();
			for (auto i = 0; i < l->children.s; i++)
			{
				auto item = l->children[i];
				item->set_visible(str[0] ? item->get_component(cText)->text.str().find(str) != std::string::npos : true);
			}
		}
		return true;
	}, Capture().set_thiz(e_list));
}
