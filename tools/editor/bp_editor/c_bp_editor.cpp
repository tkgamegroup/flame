#include <flame/universe/utils/typeinfo.h>

#include "bp_editor.h"

static Vec4c unselected_col = Vec4c(0, 0, 0, 255);
static Vec4c selected_col = Vec4c(0, 0, 145, 255);

const auto slot_bezier_extent = 50.f;

struct cSlot : Component
{
	cElement* element;
	cEventReceiver* event_receiver;
	cDataTracker* tracker;

	BP::Slot* s;

	bool dragging;

	Entity* tip_info;
	Entity* tip_link;

	cSlot() :
		Component("cSlot")
	{
		element = nullptr;
		event_receiver = nullptr;
		tracker = nullptr;

		dragging = false;

		tip_info = nullptr;
		tip_link = nullptr;
	}

	~cSlot() override
	{
		clear_tips();
	}

	void clear_tips()
	{
		if (tip_info)
		{
			auto e = tip_info;
			tip_info = nullptr;
			looper().add_event([](Capture& c) {
				auto e = c.thiz<Entity>();
				e->parent()->remove_child(e);
			}, Capture().set_thiz(e));
		}
		if (tip_link)
		{
			auto e = tip_link;
			tip_link = nullptr;
			looper().add_event([](Capture& c) {
				auto e = c.thiz<Entity>();
				e->parent()->remove_child(e);
			}, Capture().set_thiz(e));
		}
	}

	void on_component_added(Component* c) override
	{
		if (c->name_hash == FLAME_CHASH("cElement"))
			element = (cElement*)c;
		else if (c->name_hash == FLAME_CHASH("cEventReceiver"))
		{
			event_receiver = (cEventReceiver*)c;
			if (s->io() == BP::Slot::In)
			{
				event_receiver->drag_hash = FLAME_CHASH("input_slot");
				event_receiver->set_acceptable_drops(1, &FLAME_CHASH("output_slot"));
			}
			else
			{
				event_receiver->drag_hash = FLAME_CHASH("output_slot");
				event_receiver->set_acceptable_drops(1, &FLAME_CHASH("input_slot"));
			}
			event_receiver->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
				auto thiz = c.thiz<cSlot>();
				if (thiz->dragging)
				{
					if (is_mouse_scroll(action, key) || is_mouse_move(action, key))
						bp_editor.editor->edt.event_receiver->on_mouse(action, key, pos);
				}
				return true;
			}, Capture().set_thiz(this));
			event_receiver->drag_and_drop_listeners.add([](Capture& c, DragAndDrop action, cEventReceiver* er, const Vec2i& pos) {
				auto& ui = bp_editor.window->ui;
				auto thiz = c.thiz<cSlot>();
				auto element = thiz->element;
				auto s = thiz->s;
				auto is_in = s->io() == BP::Slot::In;

				if (action == DragStart)
				{
					thiz->dragging = true;
					bp_editor.editor->dragging_slot = s;
					bp_editor.select();
				}
				else if (action == DragOvering)
				{
					bp_editor.editor->dragging_slot_pos = Vec2f(pos);
					bp_editor.window->s_2d_renderer->pending_update = true;
				}
				else if (action == DragEnd)
				{
					thiz->dragging = false;
					if (!er)
						bp_editor.editor->show_add_node_menu(Vec2f(pos));
					else
						bp_editor.editor->dragging_slot = nullptr;
					bp_editor.window->s_2d_renderer->pending_update = true;
				}
				else if (action == BeingOverStart)
				{
					auto oth = er->entity->get_component(cSlot)->s;
					auto ok = (is_in ? BP::Slot::can_link(s->type(), oth->type()) : BP::Slot::can_link(oth->type(), s->type()));

					element->set_frame_color(ok ? Vec4c(0, 255, 0, 255) : Vec4c(255, 0, 0, 255));
					element->set_frame_thickness(2.f);

					if (!thiz->tip_link)
					{
						thiz->tip_link = ui.e_begin_layout(LayoutVertical, 4.f);
						auto c_element = thiz->tip_link->get_component(cElement);
						c_element->pos = thiz->element->global_pos + Vec2f(thiz->element->global_size.x(), -8.f);
						c_element->pivot = Vec2f(0.5f, 1.f);
						c_element->padding = 4.f;
						c_element->frame_thickness = 2.f;
						c_element->color = Vec4c(200, 200, 200, 255);
						c_element->frame_color = Vec4c(0, 0, 0, 255);
						{
							auto s_type = s->type();
							auto o_type = oth->type();
							std::wstring str;
							if (is_in)
								str = s2w(o_type->name()) + (ok ? L"  =>  " : L"  ¡Ù>  ") + s2w(s_type->name());
							else
								str = s2w(s_type->name()) + (ok ? L"  =>  " : L"  ¡Ù>  ") + s2w(o_type->name());
							ui.e_text(str.c_str())->get_component(cText)->color = ok ? Vec4c(0, 128, 0, 255) : Vec4c(255, 0, 0, 255);
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
					if (s->io() == BP::Slot::In)
						bp_editor.set_links({ {s, oth} });
					else
						bp_editor.set_links({ {oth, s} });
				}

				return true;
			}, Capture().set_thiz(this));
			event_receiver->hover_listeners.add([](Capture& c, bool hovering) {
				auto& ui = bp_editor.window->ui;
				auto thiz = c.thiz<cSlot>();
				auto s = thiz->s;
				auto is_in = s->io() == BP::Slot::In;

				if (!hovering)
					thiz->clear_tips();
				else
				{
					if (!thiz->tip_info)
					{
						thiz->tip_info = ui.e_begin_layout(LayoutVertical, 8.f);
						auto c_element = thiz->tip_info->get_component(cElement);
						c_element->pos = thiz->element->global_pos + Vec2f(is_in ? -8.f : thiz->element->global_size.x() + 8.f, 0.f);
						c_element->pivot = Vec2f(is_in ? 1.f : 0.f , 0.f);
						c_element->padding = 4.f;
						c_element->frame_thickness = 2.f;
						c_element->color = Vec4c(200, 200, 200, 255);
						c_element->frame_color = Vec4c(0, 0, 0, 255);
						auto type = s->type();
						auto tag = type->tag();
						ui.e_text((type_prefix(tag, type->is_array()) + s2w(type->base_name())).c_str())->get_component(cText)->color = type_color(tag);
						{
							auto text_value = ui.e_text(L"-")->get_component(cText);
							ui.current_entity = thiz->tip_info;
							auto timer = ui.c_timer();
							timer->interval = 0.f;
							struct Capturing
							{
								const TypeInfo* t;
								void* d;
								cText* text;
							}capture;
							capture.t = s->type();
							capture.d = s->data();
							capture.text = text_value;
							timer->set_callback([](Capture& c) {
								auto& capture = c.data<Capturing>();
								capture.text->set_text(s2w(capture.t->serialize(capture.d)).c_str());
							}, Capture().set_data(&capture), false);
						}
						ui.e_end_layout();
						looper().add_event([](Capture& c) {
							auto tip_info = c.thiz<Entity>();
							bp_editor.window->root->add_child(tip_info);
							tip_info->get_component(cTimer)->start();
						}, Capture().set_thiz(thiz->tip_info));
					}
				}

				return true;
			}, Capture().set_thiz(this));
		}
	}
};

struct cNode : Component
{
	cElement* element;
	cEventReceiver* event_receiver;

	BP::Node* n;

	bool moved;

	cNode() :
		Component("cNode")
	{
		moved = false;
	}

	void on_component_added(Component* c) override
	{
		if (c->name_hash == FLAME_CHASH("cElement"))
			element = (cElement*)c;
		else if (c->name_hash == FLAME_CHASH("cEventReceiver"))
		{
			event_receiver = (cEventReceiver*)c;
			event_receiver->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
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
				else if (is_mouse_move(action, key) && thiz->event_receiver->is_active())
				{
					for (auto& s : bp_editor.selected_nodes)
					{
						auto e = ((Entity*)s->user_data)->get_component(cElement);
						e->add_pos((Vec2f)pos / e->global_scale);
					}
					thiz->moved = true;
				}
				return true;
			}, Capture().set_thiz(this));
			event_receiver->state_listeners.add([](Capture& c, EventReceiverState) {
				auto thiz = c.thiz<cNode>();
				if (thiz->moved && !thiz->event_receiver->is_active())
				{
					std::vector<Vec2f> poses;
					for (auto& s : bp_editor.selected_nodes)
						poses.push_back(((Entity*)s->user_data)->get_component(cElement)->pos);
					bp_editor.set_nodes_pos(bp_editor.selected_nodes, poses);
					thiz->moved = false;
				}
				return true;
			}, Capture().set_thiz(this));
		}
	}
};

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

	edt.create(ui, [](Capture&, const Vec4f& r) {
		if (r.x() == r.z() && r.y() == r.z())
			bp_editor.select();
		else
		{
			std::vector<BP::Node*> nodes;
			for (auto i = 0; i < bp_editor.bp->node_count(); i++)
			{
				auto n = bp_editor.bp->node(i);
				auto e = ((Entity*)n->user_data)->get_component(cElement);
				if (rect_overlapping(r, rect(e->global_pos, e->global_size)))
					nodes.push_back(n);
			}
			if (!nodes.empty())
			{
				bp_editor.select(nodes);
				return;
			}

			std::vector<BP::Slot*> links;

			Vec2f lines[8];
			lines[0] = Vec2f(r.x(), r.y());
			lines[1] = Vec2f(r.z(), r.y());
			lines[2] = Vec2f(r.x(), r.y());
			lines[3] = Vec2f(r.x(), r.w());
			lines[4] = Vec2f(r.z(), r.w());
			lines[5] = Vec2f(r.x(), r.w());
			lines[6] = Vec2f(r.z(), r.w());
			lines[7] = Vec2f(r.z(), r.y());

			auto& edt = bp_editor.editor->edt;

			auto range = rect(edt.element->global_pos, edt.element->global_size);
			auto scale = edt.base->global_scale;
			auto extent = slot_bezier_extent * scale;
			for (auto i = 0; i < bp_editor.bp->node_count(); i++)
			{
				auto n = bp_editor.bp->node(i);
				for (auto j = 0; j < n->input_count(); j++)
				{
					auto input = n->input(j);
					auto output = input->link();
					if (!output)
						continue;
					auto p1 = ((cSlot*)output->user_data)->element->center();
					auto p4 = ((cSlot*)input->user_data)->element->center();
					auto p2 = p1 + Vec2f(extent, 0.f);
					auto p3 = p4 - Vec2f(extent, 0.f);
					auto bb = rect_from_points(p1, p2, p3, p4);
					if (rect_overlapping(bb, range))
					{
						std::vector<Vec2f> points;
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
		auto range = rect(edt.element->global_pos, edt.element->global_size);
		auto line_width = 3.f * scale;
		for (auto i = 0; i < bp_editor.bp->node_count(); i++)
		{
			auto n = bp_editor.bp->node(i);
			for (auto j = 0; j < n->input_count(); j++)
			{
				auto input = n->input(j);
				auto output = input->link();
				if (!output)
					continue;
				auto e1 = ((cSlot*)output->user_data)->element;
				auto e2 = ((cSlot*)input->user_data)->element;
				if (e1->entity->global_visibility && e2->entity->global_visibility)
				{
					auto p1 = e1->center();
					auto p4 = e2->center();
					auto p2 = p1 + Vec2f(extent, 0.f);
					auto p3 = p4 - Vec2f(extent, 0.f);
					auto bb = rect_from_points(p1, p2, p3, p4);
					if (rect_overlapping(bb, range))
					{
						std::vector<Vec2f> points;
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
						canvas->stroke(points.size(), points.data(), selected ? selected_col : Vec4c(100, 100, 120, 255), line_width);
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
				auto is_in = ds->io() == BP::Slot::In;
				auto p2 = p1 + Vec2f(is_in ? -extent : extent, 0.f);
				auto p3 = p4 + Vec2f(is_in ? extent : -extent, 0.f);
				auto bb = rect_from_points(p1, p2, p3, p4);
				if (rect_overlapping(bb, range))
				{
					std::vector<Vec2f> points;
					path_bezier(points, p1, p2, p3, p4);
					canvas->stroke(points.size(), points.data(), selected_col, line_width);
				}
			}
		}
		return true;
	}, Capture());
	edt.event_receiver->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
		if (is_mouse_up(action, key, true) && key == Mouse_Right)
		{
			if (!bp_editor.editor->edt.moved)
				bp_editor.editor->show_add_node_menu(Vec2f(pos));
		}
		return true;
	}, Capture());

	ui.e_end_docker_page();

	dragging_slot = nullptr;

	for (auto i = 0; i < bp_editor.bp->node_count(); i++)
		on_add_node(bp_editor.bp->node(i));
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

void cBPEditor::on_pos_changed(BP::Node* n)
{
	((Entity*)n->user_data)->get_component(cElement)->set_pos(n->pos);
}

void cBPEditor::on_add_node(BP::Node* n)
{
	auto& ui = bp_editor.window->ui;

	auto e_node = Entity::create();
	ui.current_entity = e_node;
	n->user_data = e_node;
	{
		auto c_element = ui.c_element();
		c_element->pos = n->pos;
		c_element->roundness = 8.f;
		c_element->roundness_lod = 2;
		c_element->frame_thickness = 4.f;
		c_element->color = Vec4c(255, 255, 255, 200);
		c_element->frame_color = unselected_col;
		ui.c_event_receiver();
		ui.c_layout(LayoutVertical)->fence = 1;
	}
		auto c_node = new_object<cNode>();
		c_node->n = n;
		auto n_type = BP::break_node_type(n->type());
		e_node->add_component(c_node);
		ui.e_begin_popup_menu(false);
			ui.e_menu_item(L"Duplicate", [](Capture& c) {
			}, Capture());
			ui.e_menu_item(L"Delete", [](Capture& c) {
			}, Capture());
		ui.e_end_popup_menu();
	ui.parents.push(e_node);
		ui.next_element_padding = 8.f;
		ui.e_begin_layout(LayoutVertical, 4.f);
			ui.push_style(FontSize, common(Vec1u(20)));
			ui.e_begin_layout(LayoutHorizontal, 4.f);
				if (n_type == 0)
				{
					auto str = s2w(n->type());
					auto last_colon = str.find_last_of(L':');
					if (last_colon != std::wstring::npos)
						str = std::wstring(str.begin() + last_colon + 1, str.end());
					ui.next_element_padding = Vec4f(4.f, 2.f, 4.f, 2.f);
					ui.e_text(str.c_str())->get_component(cText)->color = node_type_color(n_type);
				}
			ui.e_end_layout();
			ui.pop_style(FontSize);

			std::string type = n->type();

			ui.e_begin_layout(LayoutHorizontal, 16.f);
			ui.c_aligner(AlignMinMax | AlignGreedy, 0);

				ui.e_begin_layout(LayoutVertical, 4.f);
				ui.c_aligner(AlignMinMax | AlignGreedy, 0);
					for (auto i = 0; i < n->input_count(); i++)
					{
						auto input = n->input(i);

						ui.e_begin_layout(LayoutHorizontal);
							ui.e_empty();
							{
								auto c_element = ui.c_element();
								auto r = ui.style(FontSize).u.x();
								c_element->size = r;
								c_element->roundness = r * 0.4f;
								c_element->roundness_lod = 2;
								c_element->color = Vec4c(200, 200, 200, 255);
							}
							ui.c_event_receiver();
							auto c_slot = new_object<cSlot>();
							c_slot->s = input;
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
										auto input = c.thiz<BP::Slot>();
										auto n = input->node();
										if (n->input_count() == 1)
											return;
										bp_editor.select();
										auto idx = input->index();
										std::string type = n->type();
										std::string id = n->id();
										auto left_pos = type.find('(');
										auto plus_pos = type.find('+');
										auto size = std::stoi(std::string(type.begin() + left_pos + 1, type.begin() + plus_pos));
										type = std::string(type.begin(), type.begin() + left_pos + 1) + std::to_string(size - 1) + std::string(type.begin() + plus_pos, type.end());
										NodeDesc d;
										d.id = "";
										d.type = type;
										d.object_type = BP::ObjectReal;
										d.pos = n->pos;
										auto nn = bp_editor.add_node(d);
										for (auto i = 0; i < n->input_count(); i++)
										{
											if (i == idx)
												continue;
											auto src = n->input(i);
											auto dst = nn->input(i > idx ? i - 1 : i);
											dst->set_data(src->data());
											dst->link_to(src->link());
										}
										for (auto i = 0; i < n->output_count(); i++)
										{
											auto src = n->output(i);
											auto dst = nn->output(i);
											for (auto j = 0; j < src->link_count(); j++)
												src->link(j)->link_to(dst);
										}
										bp_editor.remove_nodes({ n });
										nn->set_id(id.c_str());
									}, Capture().set_thiz(input));
								}
							ui.e_end_popup_menu();

							ui.push_style(TextColorNormal, common(type_color(input->type()->tag())));
							auto e_name = ui.e_text(s2w(input->name()).c_str());
							ui.pop_style(TextColorNormal);
						ui.e_end_layout();

						auto type = input->type();
						auto tag = type->tag();
						if (!input->link() && tag != TypePointer)
						{
							auto base_hash = type->base_hash();

							switch (tag)
							{
							case TypeEnumSingle:
							{
								cCombobox* combobox;

								auto info = find_enum(base_hash);
								combobox = ui.e_begin_combobox()->get_component(cCombobox);
								ui.current_entity = combobox->entity;
								ui.c_aligner(AlignMin, 0)->margin.x() = ui.style(FontSize).u.x();
								for (auto i = 0; i < info->item_count(); i++)
									ui.e_combobox_item(s2w(info->item(i)->name()).c_str());
								ui.e_end_combobox();

								e_name->add_component(new_object<cEnumSingleDataTracker>(input->data(), info, [](Capture& c, int v) {
									bp_editor.set_data(c.thiz<BP::Slot>(), &v, true);
								}, Capture().set_thiz(input), combobox));
							}
								break;
							case TypeEnumMulti:
							{
								std::vector<cCheckbox*> checkboxes;

								ui.e_begin_layout(LayoutVertical, 4.f);
								ui.c_aligner(AlignMin, 0)->margin.x() = ui.style(FontSize).u.x();
								auto info = find_enum(base_hash);
								for (auto i = 0; i < info->item_count(); i++)
								{
									ui.e_begin_layout(LayoutHorizontal, 4.f);
									checkboxes.push_back(ui.e_checkbox()->get_component(cCheckbox));
									ui.e_text(s2w(info->item(i)->name()).c_str());
									ui.e_end_layout();
								}
								ui.e_end_layout();

								e_name->add_component(new_object<cEnumMultiDataTracker>(input->data(), info, [](Capture& c, int v) {
									bp_editor.set_data(c.thiz<BP::Slot>(), &v, true);
								}, Capture().set_thiz(input), checkboxes));
							}
								break;
							case TypeData:
								switch (base_hash)
								{
								case FLAME_CHASH("bool"):
								{
									cCheckbox* checkbox;

									checkbox = ui.e_checkbox()->get_component(cCheckbox);
									ui.c_aligner(AlignMin, 0)->margin.x() = ui.style(FontSize).u.x();

									e_name->add_component(new_object<cBoolDataTracker>(input->data(), [](Capture& c, bool v) {
										bp_editor.set_data(c.thiz<BP::Slot>(), &v, true);
									}, Capture().set_thiz(input), checkbox));
								}
									break;
								case FLAME_CHASH("int"):
								{
									cText* edit_text;
									cText* drag_text;

									auto e = ui.e_drag_edit();
									ui.c_aligner(AlignMin, 0)->margin.x() = ui.style(FontSize).u.x();
									edit_text = e->child(0)->get_component(cText);
									drag_text = e->child(1)->get_component(cText);

									e_name->add_component(new_object<cDigitalDataTracker<int>>(input->data(), [](Capture& c, int v, bool exit_editing) {
										if (exit_editing)
											bp_editor.set_data(c.thiz<BP::Slot>(), &v, true);
										else
											c.thiz<BP::Slot>()->set_data(&v);
									}, Capture().set_thiz(input), edit_text, drag_text));
								}
									break;
								case FLAME_CHASH("flame::Vec(2+int)"):
								{
									std::array<cText*, 2> edit_texts;
									std::array<cText*, 2> drag_texts;

									ui.e_begin_layout(LayoutHorizontal, 4.f);
									ui.c_aligner(AlignMin, 0)->margin.x() = ui.style(FontSize).u.x();
									for (auto i = 0; i < 2; i++)
									{
										auto e = ui.e_drag_edit();
										edit_texts[i] = e->child(0)->get_component(cText);
										drag_texts[i] = e->child(1)->get_component(cText);
									}
									ui.e_end_layout();

									auto p = e_name;
									p->get_component(cLayout)->type = LayoutHorizontal;
									p->add_component(new_object<cDigitalVecDataTracker<2, int>>(input->data(), [](Capture& c, const Vec<2, int>& v, bool exit_editing) {
										if (exit_editing)
											bp_editor.set_data(c.thiz<BP::Slot>(), (void*)&v, true);
										else
											c.thiz<BP::Slot>()->set_data((void*)&v);
									}, Capture().set_thiz(input), edit_texts, drag_texts));
								}
									break;
								case FLAME_CHASH("flame::Vec(3+int)"):
								{
									std::array<cText*, 3> edit_texts;
									std::array<cText*, 3> drag_texts;

									ui.e_begin_layout(LayoutHorizontal, 4.f);
									ui.c_aligner(AlignMin, 0)->margin.x() = ui.style(FontSize).u.x();
									for (auto i = 0; i < 3; i++)
									{
										auto e = ui.e_drag_edit();
										edit_texts[i] = e->child(0)->get_component(cText);
										drag_texts[i] = e->child(1)->get_component(cText);
									}
									ui.e_end_layout();

									e_name->add_component(new_object<cDigitalVecDataTracker<3, int>>(input->data(), [](Capture& c, const Vec<3, int>& v, bool exit_editing) {
										if (exit_editing)
											bp_editor.set_data(c.thiz<BP::Slot>(), (void*)&v, true);
										else
											c.thiz<BP::Slot>()->set_data((void*)&v);
									}, Capture().set_thiz(input), edit_texts, drag_texts));
								}
									break;
								case FLAME_CHASH("flame::Vec(4+int)"):
								{
									std::array<cText*, 4> edit_texts;
									std::array<cText*, 4> drag_texts;

									ui.e_begin_layout(LayoutHorizontal, 4.f);
									ui.c_aligner(AlignMin, 0)->margin.x() = ui.style(FontSize).u.x();
									for (auto i = 0; i < 4; i++)
									{
										auto e = ui.e_drag_edit();
										edit_texts[i] = e->child(0)->get_component(cText);
										drag_texts[i] = e->child(1)->get_component(cText);
									}
									ui.e_end_layout();

									e_name->add_component(new_object<cDigitalVecDataTracker<4, int>>(input->data(), [](Capture& c, const Vec<4, int>& v, bool exit_editing) {
										if (exit_editing)
											bp_editor.set_data(c.thiz<BP::Slot>(), (void*)&v, true);
										else
											c.thiz<BP::Slot>()->set_data((void*)&v);
									}, Capture().set_thiz(input), edit_texts, drag_texts));
								}
									break;
								case FLAME_CHASH("uint"):
								{
									cText* edit_text;
									cText* drag_text;

									auto e = ui.e_drag_edit();
									ui.c_aligner(AlignMin, 0)->margin.x() = ui.style(FontSize).u.x();
									edit_text = e->child(0)->get_component(cText);
									drag_text = e->child(1)->get_component(cText);

									e_name->add_component(new_object<cDigitalDataTracker<uint>>(input->data(), [](Capture& c, uint v, bool exit_editing) {
										if (exit_editing)
											bp_editor.set_data(c.thiz<BP::Slot>(), &v, true);
										else
											c.thiz<BP::Slot>()->set_data(&v);
									}, Capture().set_thiz(input), edit_text, drag_text));
								}
									break;
								case FLAME_CHASH("flame::Vec(2+uint)"):
								{
									std::array<cText*, 2> edit_texts;
									std::array<cText*, 2> drag_texts;

									ui.e_begin_layout(LayoutHorizontal, 4.f);
									ui.c_aligner(AlignMin, 0)->margin.x() = ui.style(FontSize).u.x();
									for (auto i = 0; i < 2; i++)
									{
										auto e = ui.e_drag_edit();
										edit_texts[i] = e->child(0)->get_component(cText);
										drag_texts[i] = e->child(1)->get_component(cText);
									}
									ui.e_end_layout();

									e_name->add_component(new_object<cDigitalVecDataTracker<2, uint>>(input->data(), [](Capture& c, const Vec<2, uint>& v, bool exit_editing) {
										if (exit_editing)
											bp_editor.set_data(c.thiz<BP::Slot>(), (void*)&v, true);
										else
											c.thiz<BP::Slot>()->set_data((void*)&v);
									}, Capture().set_thiz(input), edit_texts, drag_texts));
								}
									break;
								case FLAME_CHASH("flame::Vec(3+uint)"):
								{
									std::array<cText*, 3> edit_texts;
									std::array<cText*, 3> drag_texts;

									ui.e_begin_layout(LayoutHorizontal, 4.f);
									ui.c_aligner(AlignMin, 0)->margin.x() = ui.style(FontSize).u.x();
									for (auto i = 0; i < 3; i++)
									{
										auto e = ui.e_drag_edit();
										edit_texts[i] = e->child(0)->get_component(cText);
										drag_texts[i] = e->child(1)->get_component(cText);
									}
									ui.e_end_layout();

									e_name->add_component(new_object<cDigitalVecDataTracker<3, uint>>(input->data(), [](Capture& c, const Vec<3, uint>& v, bool exit_editing) {
										if (exit_editing)
											bp_editor.set_data(c.thiz<BP::Slot>(), (void*)&v, true);
										else
											c.thiz<BP::Slot>()->set_data((void*)&v);
									}, Capture().set_thiz(input), edit_texts, drag_texts));
								}
									break;
								case FLAME_CHASH("flame::Vec(4+uint)"):
								{
									std::array<cText*, 4> edit_texts;
									std::array<cText*, 4> drag_texts;

									ui.e_begin_layout(LayoutHorizontal, 4.f);
									ui.c_aligner(AlignMin, 0)->margin.x() = ui.style(FontSize).u.x();
									for (auto i = 0; i < 4; i++)
									{
										auto e = ui.e_drag_edit();
										edit_texts[i] = e->child(0)->get_component(cText);
										drag_texts[i] = e->child(1)->get_component(cText);
									}
									ui.e_end_layout();

									e_name->add_component(new_object<cDigitalVecDataTracker<4, uint>>(input->data(), [](Capture& c, const Vec<4, uint>& v, bool exit_editing) {
										if (exit_editing)
											bp_editor.set_data(c.thiz<BP::Slot>(), (void*)&v, true);
										else
											c.thiz<BP::Slot>()->set_data((void*)&v);
									}, Capture().set_thiz(input), edit_texts, drag_texts));
								}
									break;
								case FLAME_CHASH("float"):
								{
									cText* edit_text;
									cText* drag_text;

									auto e = ui.e_drag_edit();
									ui.c_aligner(AlignMin, 0)->margin.x() = ui.style(FontSize).u.x();
									edit_text = e->child(0)->get_component(cText);
									drag_text = e->child(1)->get_component(cText);

									e_name->add_component(new_object<cDigitalDataTracker<float>>(input->data(), [](Capture& c, float v, bool exit_editing) {
										if (exit_editing)
											bp_editor.set_data(c.thiz<BP::Slot>(), &v, true);
										else
											c.thiz<BP::Slot>()->set_data(&v);
									}, Capture().set_thiz(input), edit_text, drag_text));
								}
									break;
								case FLAME_CHASH("flame::Vec(2+float)"):
								{
									std::array<cText*, 2> edit_texts;
									std::array<cText*, 2> drag_texts;

									ui.e_begin_layout(LayoutHorizontal, 4.f);
									ui.c_aligner(AlignMin, 0)->margin.x() = ui.style(FontSize).u.x();
									for (auto i = 0; i < 2; i++)
									{
										auto e = ui.e_drag_edit();
										edit_texts[i] = e->child(0)->get_component(cText);
										drag_texts[i] = e->child(1)->get_component(cText);
									}
									ui.e_end_layout();

									e_name->add_component(new_object<cDigitalVecDataTracker<2, float>>(input->data(), [](Capture& c, const Vec<2, float>& v, bool exit_editing) {
										if (exit_editing)
											bp_editor.set_data(c.thiz<BP::Slot>(), (void*)&v, true);
										else
											c.thiz<BP::Slot>()->set_data((void*)&v);
									}, Capture().set_thiz(input), edit_texts, drag_texts));
								}
									break;
								case FLAME_CHASH("flame::Vec(3+float)"):
								{
									std::array<cText*, 3> edit_texts;
									std::array<cText*, 3> drag_texts;

									ui.e_begin_layout(LayoutHorizontal, 4.f);
									ui.c_aligner(AlignMin, 0)->margin.x() = ui.style(FontSize).u.x();
									for (auto i = 0; i < 3; i++)
									{
										auto e = ui.e_drag_edit();
										edit_texts[i] = e->child(0)->get_component(cText);
										drag_texts[i] = e->child(1)->get_component(cText);
									}
									ui.e_end_layout();

									e_name->add_component(new_object<cDigitalVecDataTracker<3, float>>(input->data(), [](Capture& c, const Vec<3, float>& v, bool exit_editing) {
										if (exit_editing)
											bp_editor.set_data(c.thiz<BP::Slot>(), (void*)&v, true);
										else
											c.thiz<BP::Slot>()->set_data((void*)&v);
									}, Capture().set_thiz(input), edit_texts, drag_texts));
								}
									break;
								case FLAME_CHASH("flame::Vec(4+float)"):
								{
									std::array<cText*, 4> edit_texts;
									std::array<cText*, 4> drag_texts;

									ui.e_begin_layout(LayoutHorizontal, 4.f);
									ui.c_aligner(AlignMin, 0)->margin.x() = ui.style(FontSize).u.x();
									for (auto i = 0; i < 4; i++)
									{
										auto e = ui.e_drag_edit();
										edit_texts[i] = e->child(0)->get_component(cText);
										drag_texts[i] = e->child(1)->get_component(cText);
									}
									ui.e_end_layout();

									e_name->add_component(new_object<cDigitalVecDataTracker<4, float>>(input->data(), [](Capture& c, const Vec<4, float>& v, bool exit_editing) {
										if (exit_editing)
											bp_editor.set_data(c.thiz<BP::Slot>(), (void*)&v, true);
										else
											c.thiz<BP::Slot>()->set_data((void*)&v);
									}, Capture().set_thiz(input), edit_texts, drag_texts));
								}
									break;
								case FLAME_CHASH("uchar"):
								{
									cText* edit_text;
									cText* drag_text;

									auto e = ui.e_drag_edit();
									ui.c_aligner(AlignMin, 0)->margin.x() = ui.style(FontSize).u.x();
									edit_text = e->child(0)->get_component(cText);
									drag_text = e->child(1)->get_component(cText);

									e_name->add_component(new_object<cDigitalDataTracker<uchar>>(input->data(), [](Capture& c, uchar v, bool exit_editing) {
										if (exit_editing)
											bp_editor.set_data(c.thiz<BP::Slot>(), &v, true);
										else
											c.thiz<BP::Slot>()->set_data(&v);
									}, Capture().set_thiz(input), edit_text, drag_text));
								}
									break;
								case FLAME_CHASH("flame::Vec(2+uchar)"):
								{
									std::array<cText*, 2> edit_texts;
									std::array<cText*, 2> drag_texts;

									ui.e_begin_layout(LayoutHorizontal, 4.f);
									ui.c_aligner(AlignMin, 0)->margin.x() = ui.style(FontSize).u.x();
									for (auto i = 0; i < 2; i++)
									{
										auto e = ui.e_drag_edit();
										edit_texts[i] = e->child(0)->get_component(cText);
										drag_texts[i] = e->child(1)->get_component(cText);
									}
									ui.e_end_layout();

									e_name->add_component(new_object<cDigitalVecDataTracker<2, uchar>>(input->data(), [](Capture& c, const Vec<2, uchar>& v, bool exit_editing) {
										if (exit_editing)
											bp_editor.set_data(c.thiz<BP::Slot>(), (void*)&v, true);
										else
											c.thiz<BP::Slot>()->set_data((void*)&v);
									}, Capture().set_thiz(input), edit_texts, drag_texts));
								}
									break;
								case FLAME_CHASH("flame::Vec(3+uchar)"):
								{
									std::array<cText*, 3> edit_texts;
									std::array<cText*, 3> drag_texts;

									ui.e_begin_layout(LayoutHorizontal, 4.f);
									ui.c_aligner(AlignMin, 0)->margin.x() = ui.style(FontSize).u.x();
									for (auto i = 0; i < 3; i++)
									{
										auto e = ui.e_drag_edit();
										edit_texts[i] = e->child(0)->get_component(cText);
										drag_texts[i] = e->child(1)->get_component(cText);
									}
									ui.e_end_layout();

									e_name->add_component(new_object<cDigitalVecDataTracker<3, uchar>>(input->data(), [](Capture& c, const Vec<3, uchar>& v, bool exit_editing) {
										if (exit_editing)
											bp_editor.set_data(c.thiz<BP::Slot>(), (void*)&v, true);
										else
											c.thiz<BP::Slot>()->set_data((void*)&v);
									}, Capture().set_thiz(input), edit_texts, drag_texts));
								}
									break;
								case FLAME_CHASH("flame::Vec(4+uchar)"):
								{
									std::array<cText*, 4> edit_texts;
									std::array<cText*, 4> drag_texts;

									ui.e_begin_layout(LayoutHorizontal, 4.f);
									ui.c_aligner(AlignMin, 0)->margin.x() = ui.style(FontSize).u.x();
									for (auto i = 0; i < 4; i++)
									{
										auto e = ui.e_drag_edit();
										edit_texts[i] = e->child(0)->get_component(cText);
										drag_texts[i] = e->child(1)->get_component(cText);
									}
									ui.e_end_layout();

									e_name->add_component(new_object<cDigitalVecDataTracker<4, uchar>>(input->data(), [](Capture& c, const Vec<4, uchar>& v, bool exit_editing) {
										if (exit_editing)
											bp_editor.set_data(c.thiz<BP::Slot>(), (void*)&v, true);
										else
											c.thiz<BP::Slot>()->set_data((void*)&v);
									}, Capture().set_thiz(input), edit_texts, drag_texts));
								}
									break;
								case FLAME_CHASH("flame::StringA"):
								{
									cText* text;

									text = ui.e_edit(50.f)->get_component(cText);
									ui.c_aligner(AlignMin, 0)->margin.x() = ui.style(FontSize).u.x();

									e_name->add_component(new_object<cStringADataTracker>(input->data(), [](Capture& c, const char* v) {
										bp_editor.set_data(c.thiz<BP::Slot>(), (void*)v, true);
									}, Capture().set_thiz(input), text));
								}
									break;
								case FLAME_CHASH("flame::StringW"):
								{
									cText* text;

									text = ui.e_edit(50.f)->get_component(cText);
									ui.c_aligner(AlignMin, 0)->margin.x() = ui.style(FontSize).u.x();

									e_name->add_component(new_object<cStringWDataTracker>(input->data(), [](Capture& c, const wchar_t* v) {
										bp_editor.set_data(c.thiz<BP::Slot>(), (void*)v, true);
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
					for (auto i = 0; i < n->output_count(); i++)
					{
						auto output = n->output(i);

						ui.e_begin_layout(LayoutHorizontal);
						ui.c_aligner(AlignMax, 0);
						ui.e_text(s2w(output->name()).c_str())->get_component(cText)->color = type_color(output->type()->tag());

						ui.e_empty();
						{
							auto c_element = ui.c_element();
							auto r = ui.style(FontSize).u.x();
							c_element->size = r;
							c_element->roundness = r * 0.4f;
							c_element->roundness_lod = 2;
							c_element->color = Vec4c(200, 200, 200, 255);
						}
						ui.c_event_receiver();
						auto c_slot = new_object<cSlot>();
						c_slot->s = output;
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
				ui.next_element_padding = Vec4f(5.f, 2.f, 5.f, 2.f);
				ui.next_element_roundness = 8.f;
				ui.next_element_roundness_lod = 2;
				ui.e_button(L"+", [](Capture& c) {
					auto n = c.thiz<BP::Node>();
					bp_editor.select();
					std::string type = n->type();
					std::string id = n->id();
					{
						auto left_pos = type.find('(');
						auto plus_pos = type.find('+');
						auto size = std::stoi(std::string(type.begin() + left_pos + 1, type.begin() + plus_pos));
						type = std::string(type.begin(), type.begin() + left_pos + 1) + std::to_string(size + 1) + std::string(type.begin() + plus_pos, type.end());
						NodeDesc d;
						d.id = "";
						d.type = type;
						d.object_type = BP::ObjectReal;
						d.pos = n->pos;
						auto nn = bp_editor.add_node(d);
						for (auto i = 0; i < n->input_count(); i++)
						{
							auto src = n->input(i);
							auto dst = nn->input(i);
							dst->set_data(src->data());
							dst->link_to(src->link());
						}
						for (auto i = 0; i < n->output_count(); i++)
						{
							auto src = n->output(i);
							auto dst = nn->output(i);
							for (auto j = 0; j < src->link_count(); j++)
								src->link(j)->link_to(dst);
						}
						bp_editor.remove_nodes({ n });
						nn->set_id(id.c_str());
					}
				}, Capture().set_thiz(n));
				ui.c_aligner(AlignMiddle, 0);
			}

		ui.e_end_layout();
		ui.e_empty();
		ui.c_element();
		ui.c_event_receiver()->pass_checkers.add([](Capture&, cEventReceiver*, bool* pass) {
			*pass = true;
			return true;
		}, Capture());
		ui.c_aligner(AlignMinMax, AlignMinMax);
		ui.c_bring_to_front();
	ui.parents.pop();

	looper().add_event([](Capture& c) {
		bp_editor.editor->edt.base->entity->add_child(c.thiz<Entity>());
	}, Capture().set_thiz(e_node));
}

void cBPEditor::on_remove_node(BP::Node* n)
{
	auto e = (Entity*)n->user_data;
	e->parent()->remove_child(e);
}

void cBPEditor::on_data_changed(BP::Slot* s)
{
	((cSlot*)s->user_data)->tracker->update_view();
}

void cBPEditor::show_add_node_menu(const Vec2f& pos)
{
	const TypeInfo* type;
	TypeTag tag;
	const char* base_name;
	uint base_hash;
	bool is_array;
	if (dragging_slot)
	{
		type = dragging_slot->type();
		tag = type->tag();
		base_name = type->base_name();
		base_hash = type->base_hash();
		is_array = type->is_array();
	}

	struct NodeType
	{
		UdtInfo* u;
		VariableInfo* v;
		BP::ObjectType t;
	};
	std::vector<NodeType> node_types;
	for (auto i = 0; i < global_db_count(); i++)
	{
		auto udts = global_db(i)->get_udts();
		for (auto j = 0; j < udts.s; j++)
		{
			auto u = udts[j];
			auto f_update = u->find_function("bp_update");
			if (f_update && check_function(f_update, "D#void", {}))
			{
				if (!dragging_slot)
					node_types.push_back({ u, nullptr, BP::ObjectReal });
				else
				{
					auto ds_io = dragging_slot->io();
					auto ds_t = dragging_slot->type();
					auto found = false;
					for (auto k = 0; k < u->variable_count(); k++)
					{
						auto v = u->variable(k);
						auto flags = v->flags();
						if (ds_io == BP::Slot::Out && (flags & VariableFlagInput) && BP::Slot::can_link(v->type(), ds_t))
						{
							node_types.push_back({ u, v, BP::ObjectReal });
							found = true;
							break;
						}
						if (ds_io == BP::Slot::In && (flags & VariableFlagOutput) && BP::Slot::can_link(ds_t, v->type()))
						{
							node_types.push_back({ u, v, BP::ObjectReal });
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
					node_types.push_back({ u, nullptr, BP::ObjectRefRead });
					node_types.push_back({ u, nullptr, BP::ObjectRefWrite });
				}
				else
				{
					auto ds_io = dragging_slot->io();
					auto ds_t = dragging_slot->type();
					auto found = false;
					for (auto k = 0; k < u->variable_count(); k++)
					{
						auto v = u->variable(k);
						if (ds_io == BP::Slot::Out && BP::Slot::can_link(v->type(), ds_t))
						{
							node_types.push_back({ u, v, BP::ObjectRefWrite });
							found = true;
							break;
						}
						if (ds_io == BP::Slot::In && BP::Slot::can_link(ds_t, v->type()))
						{
							node_types.push_back({ u, v, BP::ObjectRefRead });
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
		return std::string(a.u->name()) < std::string(b.u->name());
	});

	auto& ui = bp_editor.window->ui;

	ui.parents.push(add_layer(bp_editor.window->root, ""));
		ui.next_element_pos = pos;
		ui.next_element_padding = 4.f;
		ui.next_element_frame_thickness = 2.f;
		ui.next_element_color = ui.style(BackgroundColor).c;
		ui.next_element_frame_color = ui.style(ForegroundColor).c;
		ui.e_element()->on_removed_listeners.add([](Capture&) {
			bp_editor.editor->dragging_slot = nullptr;
			return true;
		}, Capture());
		ui.c_layout(LayoutVertical)->item_padding = 4.f;
		ui.parents.push(ui.current_entity);
			if (dragging_slot)
				ui.e_text((L"Filtered For: " + s2w(type->name())).c_str())->get_component(cText)->color = type_color(tag);
			ui.e_begin_layout(LayoutHorizontal, 4.f);
				ui.e_text(Icon_SEARCH);
				auto c_text_search = ui.e_edit(300.f)->get_component(cText);
			ui.e_end_layout();
			ui.next_element_size = Vec2f(0.f, 300.f);
			ui.next_element_padding = 4.f;
			ui.e_begin_scrollbar(ScrollbarVertical, false);
			ui.c_aligner(AlignMinMax | AlignGreedy, 0);
				auto e_list = ui.e_begin_list(true);
					struct Capturing
					{
						Entity* l;
						Vec2f p;

						void show_enums(TypeTag tag)
						{
							auto& ui = bp_editor.window->ui;

							std::vector<EnumInfo*> enum_infos;
							for (auto i = 0; i < global_db_count(); i++)
							{
								auto enums = global_db(i)->get_enums();
								for (auto j = 0; j < enums.s; j++)
									enum_infos.push_back(enums.v[j]);
							}
							std::sort(enum_infos.begin(), enum_infos.end(), [](EnumInfo* a, EnumInfo* b) {
								return std::string(a->name()) < std::string(b->name());
							});

							struct _Capturing
							{
								TypeTag t;
								const char* s;
								Vec2f p;
							}capture;
							capture.t = tag;
							capture.p = p;
							l->remove_children(0, -1);
							for (auto ei : enum_infos)
							{
								capture.s = ei->name();
								ui.parents.push(l);
								ui.e_menu_item(s2w(ei->name()).c_str(), [](Capture& c) {
									auto& capture = c.data<_Capturing>();
									NodeDesc d;
									d.id = "";
									d.type = capture.t == TypeEnumSingle ? "EnumSingle(" : "EnumMulti(";
									d.type += capture.s;
									d.type += ")";
									d.object_type = BP::ObjectReal;
									d.pos = capture.p;
									bp_editor.add_node(d);
								}, Capture().set_data(&capture));
								ui.parents.pop();
							}
						}
					}capture;
					capture.l = e_list;
					capture.p = (Vec2f(pos) - edt.base->global_pos) / (edt.scale_level * 0.1f);
					if (!dragging_slot)
					{
						ui.e_menu_item(L"Enum Single", [](Capture& c) {
							auto& capture = c.data<Capturing>();
							looper().add_event([](Capture& c) {
								auto& capture = c.data<Capturing>();
								capture.show_enums(TypeEnumSingle);
							}, Capture().set_data(&capture));
						}, Capture().set_data(&capture), false);
						ui.e_menu_item(L"Enum Multi", [](Capture& c) {
							auto& capture = c.data<Capturing>();
							looper().add_event([](Capture& c) {
								auto& capture = c.data<Capturing>();
								capture.show_enums(TypeEnumMulti);
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
									Vec2f p;
								}_capture;
								_capture.p = capture.p;
								capture.l->remove_children(0, -1);
								for (auto t : basic_types())
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
										d.object_type = BP::ObjectReal;
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
									Vec2f p;
								}_capture;
								_capture.p = capture.p;
								capture.l->remove_children(0, -1);
								for (auto t : basic_types())
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
										d.object_type = BP::ObjectReal;
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
						if (tag == TypeEnumSingle || tag == TypeEnumMulti)
						{
							struct _Capturing
							{
								TypeTag t;
								const char* s;
								Vec2f p;
							}_capture;
							_capture.t = tag;
							_capture.p = capture.p;
							_capture.s = base_name;
							ui.e_menu_item(((tag == TypeEnumSingle ? L"Enum Single: " : L"Enum Multi: ") + s2w(base_name)).c_str(), [](Capture& c) {
								auto& capture = c.data<_Capturing>();
								NodeDesc d;
								d.id = "";
								d.type = capture.t == TypeEnumSingle ? "EnumSingle(" : "EnumMulti(";
								d.type += capture.s;
								d.type += ")";
								d.object_type = BP::ObjectReal;
								d.pos = capture.p;
								auto n = bp_editor.add_node(d);
								auto s = bp_editor.editor->dragging_slot;
								if (s)
								{
									if (s->io() == BP::Slot::In)
										bp_editor._set_link(s, n->find_output("out"));
									else
										bp_editor._set_link(n->find_input("in"), s);
								}
							}, Capture().set_data(&_capture));
						}
						else if (tag == TypeData && !is_array)
						{
							if (basic_type_size(base_hash))
							{
								struct _Capturing
								{
									const char* s;
									Vec2f p;
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
									d.object_type = BP::ObjectReal;
									d.pos = capture.p;
									auto n = bp_editor.add_node(d);
									auto s = bp_editor.editor->dragging_slot;
									if (s)
									{
										if (s->io() == BP::Slot::In)
											bp_editor._set_link(s, n->find_output("out"));
										else
											bp_editor._set_link(n->find_input("in"), s);
									}
								}, Capture().set_data(&_capture));
							}
						}
						else if (tag == TypePointer && is_array)
						{
							struct _Capturing
							{
								const char* s;
								Vec2f p;
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
								d.object_type = BP::ObjectReal;
								d.pos = capture.p;
								auto n = bp_editor.add_node(d);
								auto s = bp_editor.editor->dragging_slot;
								if (s)
								{
									if (s->io() == BP::Slot::In)
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
							BP::ObjectType t;
							const char* us;
							const char* vs;
							Vec2f p;
						}capture;
						capture.p = (Vec2f(pos) - edt.base->global_pos) / (edt.scale_level * 0.1f);
						for (auto& t : node_types)
						{
							capture.t = t.t;
							capture.us = t.u->name();
							capture.vs = t.v ? t.v->name() : nullptr;
							auto name = s2w(t.u->name());
							if (capture.t == BP::ObjectRefRead)
								name += L" (RefRead)";
							else if (capture.t == BP::ObjectRefWrite)
								name += L" (RefWrite)";
							ui.e_menu_item(name.c_str(), [](Capture& c) {
								auto& capture = c.data<_Capturing>();
								NodeDesc d;
								d.object_type = capture.t;
								d.type = capture.us;
								d.id = "";
								d.pos = capture.p;
								auto n = bp_editor.add_node(d);
								auto s = bp_editor.editor->dragging_slot;
								if (s)
								{
									if (s->io() == BP::Slot::In)
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
			for (auto i = 0; i < l->child_count(); i++)
			{
				auto item = l->child(i);
				item->set_visible(str[0] ? item->get_component(cText)->text.str().find(str) != std::string::npos : true);
			}
		}
		return true;
	}, Capture().set_thiz(e_list));
}
