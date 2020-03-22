#include <flame/universe/utils/typeinfo.h>
#include "app.h"

#include <flame/reflect_macros.h>

struct R(TestRenderTarget)
{
	BP::Node* n;

	BASE1;
	RV(Image*, img, o);
	RV(TargetType, type, o);
	RV(Imageview*, view, o);
	RV(uint, idx, o);
	RV(Array<Commandbuffer*>, out, o);

	__declspec(dllexport) void RF(active_update)(uint frame)
	{
		if (img_s()->frame() == -1)
		{
			if (idx > 0)
				app.canvas->set_image(idx, nullptr);
			auto d = Device::default_one();
			if (d)
			{
				img = Image::create(d, Format_R8G8B8A8_UNORM, Vec2u(800, 600), 1, 1, SampleCount_1, ImageUsageTransferDst | ImageUsageAttachment | ImageUsageSampled);
				(img)->init(Vec4c(0, 0, 0, 255));
			}
			else
				img = nullptr;
			type = TargetImageview;
			type_s()->set_frame(frame);
			if (img)
			{
				view = Imageview::create(img);
				idx = app.canvas->set_image(-1, view);
			}
			img_s()->set_frame(frame);
			view_s()->set_frame(frame);
			idx_s()->set_frame(frame);
		}
		if (out_s()->frame() == -1)
		{
			auto d = Device::default_one();
			if (d)
			{
				out.resize(1);
				out[0] = Commandbuffer::create(d->gcp);
			}
			else
				out.resize(0);
			out_s()->set_frame(frame);
		}

		app.graphics_cbs.push_back(out[0]);
	}

	__declspec(dllexport) RF(~TestRenderTarget)()
	{
		if (idx > 0)
			app.canvas->set_image(idx, nullptr);
		if (img)
			Image::destroy(img);
		if (view)
			Imageview::destroy(view);
		for (auto i = 0; i < out.s; i++)
			Commandbuffer::destroy(out[i]);
	}
};

static Vec4c unselected_col = Vec4c(0, 0, 0, 255);
static Vec4c selected_col = Vec4c(0, 0, 145, 255);

static const wchar_t* type_prefix(TypeTag t, bool is_array = false)
{
	switch (t)
	{
	case TypeEnumSingle:
		return L"Enum Single\n";
	case TypeEnumMulti:
		return L"Enum Multi\n";
	case TypeData:
		return is_array ? L"Array\n" : L"Data\n";
	case TypePointer:
		return is_array ? L"Array Pointer\n" : L"Pointer\n";
	}
	return L"";
}

static Vec4c type_color(TypeTag t)
{
	switch (t)
	{
	case TypeEnumSingle:
		return Vec4c(128, 128, 0, 255);
	case TypeEnumMulti:
		return Vec4c(160, 128, 0, 255);
	case TypeData:
		return Vec4c(0, 128, 128, 255);
	case TypePointer:
		return Vec4c(255, 0, 0, 255);
	}
	return Vec4c(0);
}

const auto slot_bezier_extent = 50.f;

struct cSlot : Component
{
	cElement* element;
	cEventReceiver* event_receiver;
	cDataTracker* tracker;

	BP::Slot* s;

	Entity* tip_type;
	Entity* tip_link;

	cSlot() :
		Component("cSlot")
	{
		element = nullptr;
		event_receiver = nullptr;
		tracker = nullptr;

		tip_type = nullptr;
		tip_link = nullptr;
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

			event_receiver->drag_and_drop_listeners.add([](void* c, DragAndDrop action, cEventReceiver* er, const Vec2i& pos) {
				auto thiz = *(cSlot**)c;
				auto element = thiz->element;
				auto s = thiz->s;
				auto is_in = s->io() == BP::Slot::In;

				if (action == DragStart)
					app.editor->dragging_slot = s;
				else if (action == DragOvering)
				{
					app.editor->dragging_slot_pos = Vec2f(pos);
					app.s_2d_renderer->pending_update = true;
				}
				else if (action == DragEnd)
				{
					if (!er)
						app.editor->show_add_node_menu(Vec2f(pos));
					else
						app.editor->dragging_slot = nullptr;
					app.s_2d_renderer->pending_update = true;
				}
				else if (action == BeingOverStart)
				{
					auto oth = er->entity->get_component(cSlot)->s;
					auto ok = (is_in ? s->can_link(oth) : oth->can_link(s));

					element->set_frame_color(ok ? Vec4c(0, 255, 0, 255) : Vec4c(255, 0, 0, 255));
					element->set_frame_thickness(2.f);

					if (!thiz->tip_link)
					{
						utils::push_parent(app.root);
						thiz->tip_link = utils::e_begin_layout(LayoutVertical, 4.f);
						auto c_element = thiz->tip_link->get_component(cElement);
						c_element->pos_ = thiz->element->global_pos + Vec2f(thiz->element->global_size.x(), -8.f);
						c_element->pivot_ = Vec2f(0.5f, 1.f);
						c_element->inner_padding_ = 4.f;
						c_element->frame_thickness_ = 2.f;
						c_element->color_ = Vec4c(200, 200, 200, 255);
						c_element->frame_color_ = Vec4c(0, 0, 0, 255);
						{
							auto s_type = s->type();
							auto o_type = oth->type();
							std::wstring str;
							if (is_in)
								str = s2w(o_type->name()) + (ok ? L"  =>  " : L"  ¡Ù>  ") + s2w(s_type->name());
							else
								str = s2w(s_type->name()) + (ok ? L"  =>  " : L"  ¡Ù>  ") + s2w(o_type->name());
							utils::e_text(str.c_str())->get_component(cText)->color_ = ok ? Vec4c(0, 128, 0, 255) : Vec4c(255, 0, 0, 255);
						}
						utils::e_end_layout();
						utils::pop_parent();
					}
				}
				else if (action == BeingOverEnd)
				{
					element->set_frame_thickness(0.f);

					if (thiz->tip_link)
					{
						auto e = thiz->tip_link;
						thiz->tip_link = nullptr;
						looper().add_event([](void* c, bool*) {
							auto e = *(Entity**)c;
							e->parent()->remove_child(e);
						}, new_mail_p(e));
					}
				}
				else if (action == BeenDropped)
				{
					auto oth = er->entity->get_component(cSlot)->s;
					if (s->io() == BP::Slot::In)
					{
						if (s->link_to(oth))
							app.set_changed(true);
					}
					else
					{
						if (oth->link_to(thiz->s))
							app.set_changed(true);
					}
				}

				return true;
			}, new_mail_p(this));
			event_receiver->hover_listeners.add([](void* c, bool hovering) {
				auto thiz = *(cSlot**)c;
				auto s = thiz->s;
				auto is_in = s->io() == BP::Slot::In;

				if (!hovering)
				{
					if (thiz->tip_type)
					{
						auto e = thiz->tip_type;
						thiz->tip_type = nullptr;
						looper().add_event([](void* c, bool*) {
							auto e = *(Entity**)c;
							e->parent()->remove_child(e);
						}, new_mail_p(e));
					}
				}
				else
				{
					if (!thiz->tip_type)
					{
						utils::push_parent(app.root);
							thiz->tip_type = utils::e_begin_layout(LayoutVertical, 4.f);
							auto c_element = thiz->tip_type->get_component(cElement);
							c_element->pos_ = thiz->element->global_pos + Vec2f(is_in ? -8.f : thiz->element->global_size.x() + 8.f, 0.f);
							c_element->pivot_ = Vec2f(is_in ? 1.f : 0.f , 0.f);
							c_element->inner_padding_ = 4.f;
							c_element->frame_thickness_ = 2.f;
							c_element->color_ = Vec4c(200, 200, 200, 255);
							c_element->frame_color_ = Vec4c(0, 0, 0, 255);
								{
									auto type = s->type();
									auto tag = type->tag();
									utils::e_text((type_prefix(tag, type->is_array()) + s2w(type->base_name())).c_str())->get_component(cText)->color_ = type_color(tag);
								}
							utils::e_end_layout();
						utils::pop_parent();
					}
				}

				return true;
			}, new_mail_p(this));
		}
	}
};

static const wchar_t* node_type_prefix(char t)
{
	switch (t)
	{
	case 'S':
		return type_prefix(TypeEnumSingle);
	case 'M':
		return type_prefix(TypeEnumMulti);
	case 'V':
		return L"Variable\n";
	case 'A':
		return L"Array\n";
	}
	return L"";
}

static Vec4c node_type_color(char t)
{
	switch (t)
	{
	case 'S':
		return type_color(TypeEnumSingle);
	case 'M':
		return type_color(TypeEnumMulti);
	case 'V':
		return Vec4c(0, 128, 0, 255);
	case 'A':
		return Vec4c(0, 0, 255, 255);
	}
	return Vec4c(128, 60, 220, 255);
}

struct cNode : Component
{
	cElement* element;
	cEventReceiver* event_receiver;

	BP::Node* n;
	char n_type;
	std::wstring n_name;

	bool moved;

	Entity* tip;

	cNode() :
		Component("cNode")
	{
		moved = false;

		tip = nullptr;
	}

	void on_component_added(Component* c) override
	{
		if (c->name_hash == FLAME_CHASH("cElement"))
		{
			element = (cElement*)c;
			element->data_changed_listeners.add([](void* c, uint hash, void*) {
				if (hash == FLAME_CHASH("pos"))
				{
					auto thiz = *(cNode**)c;
					auto pos = thiz->element->pos_;
					thiz->n->pos = pos;
					thiz->moved = true;
				}
				return true;
			}, new_mail_p(this));
		}
		else if (c->name_hash == FLAME_CHASH("cEventReceiver"))
		{
			event_receiver = (cEventReceiver*)c;
			event_receiver->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
				auto thiz = *(cNode**)c;
				if (is_mouse_down(action, key, true) && key == Mouse_Left)
					app.select(SelNode, thiz->n);
				return true;
			}, new_mail_p(this));
			event_receiver->hover_listeners.add([](void* c, bool hovering) {
				auto thiz = *(cNode**)c;
				if (!hovering)
				{
					if (thiz->tip)
					{
						auto e = thiz->tip;
						thiz->tip = nullptr;
						looper().add_event([](void* c, bool*) {
							auto e = *(Entity**)c;
							e->parent()->remove_child(e);
						}, new_mail_p(e));
					}
				}
				else
				{
					if (!thiz->tip)
					{
						utils::push_parent(app.root);
							thiz->tip = utils::e_begin_layout(LayoutVertical, 4.f);
							auto c_element = thiz->tip->get_component(cElement);
							c_element->pos_ = thiz->element->global_pos - Vec2f(0.f, 8.f);
							c_element->pivot_ = Vec2f(0.f, 1.f);
							c_element->inner_padding_ = 4.f;
							c_element->frame_thickness_ = 2.f;
							c_element->color_ = Vec4c(200, 200, 200, 255);
							c_element->frame_color_ = Vec4c(0, 0, 0, 255);
								auto n = thiz->n;
								std::wstring str;
								auto udt = n->udt();
								if (udt)
									str = L"UDT (" + std::wstring(udt->db()->module_name()) + L")\n" + thiz->n_name;
								else
									str = node_type_prefix(thiz->n_type) + thiz->n_name;
								utils::e_text(str.c_str())->get_component(cText)->color_ = node_type_color(thiz->n_type);
							utils::e_end_layout();
						utils::pop_parent();
					}
				}
				return true;
			}, new_mail_p(this));
			event_receiver->state_listeners.add([](void* c, EventReceiverState state) {
				auto thiz = *(cNode**)c;
				switch (state)
				{
				case EventReceiverActive:
					thiz->moved = false;
					break;
				default:
					if (thiz->moved)
					{
						app.set_changed(true);
						thiz->moved = false;
					}
				}
				return true;
			}, new_mail_p(this));
		}
	}
};

cEditor::cEditor() :
	Component("cEditor")
{
	auto tnp = utils::e_begin_docker_page(app.filepath.c_str());
	{
		auto c_layout = utils::c_layout(LayoutVertical);
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;
		c_layout->fence = 2;
	}
	tnp.second->add_component(this);
	c_tab_text = tnp.first->get_component(cText);

		utils::e_begin_layout()->get_component(cElement)->clip_children = true;
		utils::c_aligner(SizeFitParent, SizeFitParent);
			{
				auto c_element = utils::e_element()->get_component(cElement);
				c_element->cmds.add([](void* c, graphics::Canvas* canvas) {
					auto element = *(cElement**)c;
					auto base_element = app.editor->c_base_element;

					if (element->cliped)
						return true;

					auto range = rect(element->global_pos, element->global_size);
					auto scale = base_element->global_scale;
					auto extent = slot_bezier_extent * scale;
					auto line_width = 3.f * scale;

					{
						const auto grid_size = 50.f * base_element->global_scale;
						auto pos = base_element->global_pos;
						auto size = element->global_size + grid_size * 2.f;
						auto grid_number = Vec2i(size / grid_size) + 2;
						auto grid_offset = element->global_pos + (fract(pos / grid_size) - 1.f) * grid_size;
						for (auto x = 0; x < grid_number.x(); x++)
						{
							std::vector<Vec2f> points;
							points.push_back(grid_offset + Vec2f(x * grid_size, 0.f));
							points.push_back(grid_offset + Vec2f(x * grid_size, size.y()));
							canvas->stroke(points.size(), points.data(), Vec4c(200, 200, 200, 255), 1.f);
						}
						for (auto y = 0; y < grid_number.y(); y++)
						{
							std::vector<Vec2f> points;
							points.push_back(grid_offset + Vec2f(0.f, y * grid_size));
							points.push_back(grid_offset + Vec2f(size.x(), y * grid_size));
							canvas->stroke(points.size(), points.data(), Vec4c(200, 200, 200, 255), 1.f);
						}
					}

					for (auto i = 0; i < app.bp->node_count(); i++)
					{
						auto n = app.bp->node(i);
						for (auto j = 0; j < n->output_count(); j++)
						{
							auto output = n->output(j);
							for (auto k = 0; k < output->link_count(); k++)
							{
								auto input = output->link(k);
								auto p1 = ((cSlot*)output->user_data)->element->center();
								auto p4 = ((cSlot*)input->user_data)->element->center();
								auto p2 = p1 + Vec2f(extent, 0.f);
								auto p3 = p4 - Vec2f(extent, 0.f);
								auto bb = rect_from_points(p1, p2, p3, p4);
								if (rect_overlapping(bb, range))
								{
									std::vector<Vec2f> points;
									path_bezier(points, p1, p2, p3, p4);
									canvas->stroke(points.size(), points.data(), app.selected.link == input ? selected_col : Vec4c(100, 100, 120, 255), line_width);
								}
							}
						}
					}
					if (app.editor->dragging_slot)
					{
						auto p1 = ((cSlot*)app.editor->dragging_slot->user_data)->element->center();
						auto p4 = app.editor->dragging_slot_pos;
						auto is_in = app.editor->dragging_slot->io() == BP::Slot::In;
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

					return true;
				}, new_mail_p(c_element));
				utils::c_event_receiver()->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_scroll(action, key))
					{
						auto v = (pos.x() > 0.f ? 1 : -1);
						app.editor->scale += v;
						if (app.editor->scale < 1 || app.editor->scale > 10)
							app.editor->scale -= v;
						else
						{
							auto p = (Vec2f(app.s_event_dispatcher->mouse_pos) - app.editor->c_base_element->global_pos) / ((app.editor->scale - v) * 0.1f);
							app.editor->c_base_element->set_pos(float(v) * p * -0.1f, true);
							app.editor->c_base_element->set_scale(app.editor->scale * 0.1f);
							app.editor->c_scale_text->set_text((std::to_wstring(app.editor->scale * 10) + L"%").c_str());
						}
					}
					else if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						app.s_2d_renderer->pending_update = true;

						app.deselect();

						auto scale = app.editor->c_base_element->global_scale;
						auto extent = slot_bezier_extent * scale;
						auto line_width = 3.f * scale;
						for (auto i = 0; i < app.bp->node_count(); i++)
						{
							auto n = app.bp->node(i);
							for (auto j = 0; j < n->output_count(); j++)
							{
								auto output = n->output(j);
								for (auto k = 0; k < output->link_count(); k++)
								{
									auto input = output->link(k);
									auto p1 = ((cSlot*)output->user_data)->element->center();
									auto p4 = ((cSlot*)input->user_data)->element->center();
									auto p2 = p1 + Vec2f(extent, 0.f);
									auto p3 = p4 - Vec2f(extent, 0.f);
									auto bb = rect_from_points(p1, p2, p3, p4);
									if (rect_contains(bb, Vec2f(pos)) && distance(bezier_closest_point(Vec2f(pos), p1, p2, p3, p4, 4, 7), Vec2f(pos)) < line_width)
									{
										app.select(SelLink, input);
										return false;
									}
								}
							}
						}
					}
					else if (is_mouse_down(action, key, true) && key == Mouse_Right)
						app.editor->base_moved = false;
					else if (is_mouse_up(action, key, true) && key == Mouse_Right)
					{
						if (!app.editor->base_moved)
							app.editor->show_add_node_menu(Vec2f(pos));
					}
					else if (is_mouse_move(action, key))
					{
						if (app.s_event_dispatcher->mouse_buttons[Mouse_Right] & KeyStateDown)
						{
							app.editor->c_base_element->set_pos(Vec2f(pos), true);
							app.editor->base_moved = true;
						}
					}
					return true;
				}, Mail<>());
				utils::c_aligner(SizeFitParent, SizeFitParent);
				utils::push_parent(utils::current_entity());
					e_base = utils::e_empty();
					c_base_element = utils::c_element();
					base_moved = false;
				utils::pop_parent();
			}
			scale = 10;
			c_scale_text = utils::e_text(L"100%")->get_component(cText);
			utils::c_aligner(AlignxLeft, AlignyBottom);
		utils::e_end_layout();

	utils::e_end_docker_page();

	on_load();
}

cEditor::~cEditor()
{
	app.editor = nullptr;
}

static Entity* selected_entity()
{
	if (app.sel_type == SelNode)
		return (Entity*)app.selected.node->user_data;
	return nullptr;
}

void cEditor::on_deselect()
{
	auto e = selected_entity();
	if (e)
		e->get_component(cElement)->set_frame_color(unselected_col);
}

void cEditor::on_select()
{
	auto e = selected_entity();
	if (e)
		e->get_component(cElement)->set_frame_color(selected_col);
}

void cEditor::on_changed()
{
	std::wstring title = L"editor";
	if (app.changed)
		title += L"*";
	c_tab_text->set_text(title.c_str());
}

void cEditor::on_load()
{
	e_base->remove_child(0, -1);

	for (auto i = 0; i < app.bp->node_count(); i++)
		on_add_node(app.bp->node(i));

	dragging_slot = nullptr;

	on_changed();
}

void cEditor::show_add_node_menu(const Vec2f& pos)
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

	std::vector<std::pair<UdtInfo*, VariableInfo*>> node_types;
	auto add_udt = [&](UdtInfo* u) {
		{
			auto f = find_not_null_and_only(u->find_function("update"), u->find_function("active_update"));
			if (!f.first)
				return;
			if (!check_function(f.first, "D#void", { "D#uint" }))
				return;
		}
		for (auto i = 0; i < u->variable_count(); i++)
		{
			auto v = u->variable(i);
			auto flags = v->flags();
			if (flags & VariableFlagInput)
			{
				if (!dragging_slot)
				{
					node_types.emplace_back(u, nullptr);
					return;
				}
				if (dragging_slot->io() == BP::Slot::Out)
				{

				}
			}
			if (flags & VariableFlagOutput)
			{
				if (!dragging_slot)
				{
					node_types.emplace_back(u, nullptr);
					return;
				}
				if (dragging_slot->io() == BP::Slot::In)
				{

				}
			}
		}
	};
	for (auto i = 0; i < global_db_count(); i++)
	{
		auto udts = global_db(i)->get_udts();
		for (auto j = 0; j < udts.s; j++)
			add_udt(udts.v[j]);
	}
	for (auto i = 0; i < app.bp->library_count(); i++)
	{
		auto udts = app.bp->library(i)->db()->get_udts();
		for (auto j = 0; j < udts.s; j++)
			add_udt(udts.v[j]);
	}
	std::sort(node_types.begin(), node_types.end(), [](const auto& a, const auto& b) {
		return std::string(a.first->type()->name()) < std::string(b.first->type()->name());
	});

	utils::push_parent(utils::add_layer(app.root, ""));
		utils::e_empty()->on_removed_listeners.add([](void*) {
			app.editor->dragging_slot = nullptr;
			return true;
		}, Mail<>());
		utils::next_element_pos = pos;
		auto c_element = utils::c_element();
		c_element->inner_padding_ = 4.f;
		c_element->frame_thickness_ = 2.f;
		c_element->color_ = utils::style_4c(utils::BackgroundColor);
		c_element->frame_color_ = utils::style_4c(utils::ForegroundColor);
		utils::c_layout(LayoutVertical)->item_padding = 4.f;
		utils::push_parent(utils::current_entity());
			if (dragging_slot)
				utils::e_text((L"Filtered For: " + s2w(type->name())).c_str())->get_component(cText)->color_ = type_color(tag);
			utils::e_begin_layout(LayoutHorizontal, 4.f);
				utils::e_text(Icon_SEARCH);
				auto c_text_search = utils::e_edit(300.f)->get_component(cText);
			utils::e_end_layout();
			utils::e_begin_scroll_view1(ScrollbarVertical, Vec2f(0.f, 300.f), 4.f);
			utils::c_aligner(SizeGreedy, SizeFixed);
				auto e_list = utils::e_begin_list(true, 0.f);
					struct Capture
					{
						Entity* l;
						Vec2f p;

						void show_enums(TypeTag tag)
						{
							std::vector<EnumInfo*> enum_infos;
							for (auto i = 0; i < global_db_count(); i++)
							{
								auto enums = global_db(i)->get_enums();
								for (auto j = 0; j < enums.s; j++)
									enum_infos.push_back(enums.v[j]);
							}
							for (auto i = 0; i < app.bp->library_count(); i++)
							{
								auto enums = app.bp->library(i)->db()->get_enums();
								for (auto j = 0; j < enums.s; j++)
									enum_infos.push_back(enums.v[j]);
							}
							std::sort(enum_infos.begin(), enum_infos.end(), [](EnumInfo* a, EnumInfo* b) {
								return std::string(a->name()) < std::string(b->name());
							});

							struct Capture
							{
								TypeTag t;
								EnumInfo* e;
								Vec2f p;
							}capture;
							capture.t = tag;
							capture.p = p;
							l->remove_children(0, -1);
							for (auto ei : enum_infos)
							{
								capture.e = ei;
								utils::push_parent(l);
								utils::e_menu_item(s2w(ei->name()).c_str(), [](void* c) {
									auto& capture = *(Capture*)c;
									app.add_node(((capture.t == TypeEnumSingle ? "EnumSingle(" : "EnumMulti(") + std::string(capture.e->name()) + ")").c_str(), "", capture.p);
								}, new_mail(&capture));
								utils::pop_parent();
							}
						}
					}capture;
					capture.l = e_list;
					capture.p = (Vec2f(pos) - c_base_element->global_pos) / (scale * 0.1f);
					if (!dragging_slot)
					{
						utils::e_menu_item(L"Enum Single", [](void* c) {
							auto& capture = *(Capture*)c;
							looper().add_event([](void* c, bool*) {
								auto& capture = *(Capture*)c;
								capture.show_enums(TypeEnumSingle);
							}, new_mail(&capture));
						}, new_mail(&capture), false);
						utils::e_menu_item(L"Enum Multi", [](void* c) {
							auto& capture = *(Capture*)c;
							looper().add_event([](void* c, bool*) {
								auto& capture = *(Capture*)c;
								capture.show_enums(TypeEnumMulti);
							}, new_mail(&capture));
						}, new_mail(&capture), false);
						utils::e_menu_item(L"Variable", [](void* c) {
							auto& capture = *(Capture*)c;
							looper().add_event([](void* c, bool*) {
								auto& capture = *(Capture*)c;
								struct _Capture
								{
									const char* s;
									Vec2f p;
								}_capture;
								_capture.p = capture.p;
								capture.l->remove_children(0, -1);
								for (auto t : basic_types())
								{
									_capture.s = t;
									utils::push_parent(capture.l);
									utils::e_menu_item(s2w(t).c_str(), [](void* c) {
										auto& capture = *(_Capture*)c;
										app.add_node(("Variable(" + std::string(capture.s) + ")").c_str(), "", capture.p);
									}, new_mail(&_capture));
									utils::pop_parent();
								}
							}, new_mail(&capture));
						}, new_mail(&capture), false);
						utils::e_menu_item(L"Array", [](void* c) {
							auto& capture = *(Capture*)c;
							looper().add_event([](void* c, bool*) {
								auto& capture = *(Capture*)c;
								struct _Capture
								{
									const char* s;
									Vec2f p;
								}_capture;
								_capture.p = capture.p;
								capture.l->remove_children(0, -1);
								for (auto t : basic_types())
								{
									_capture.s = t;
									utils::push_parent(capture.l);
									utils::e_menu_item(s2w(t).c_str(), [](void* c) {
										auto& capture = *(_Capture*)c;
										app.add_node(("Array(1+" + std::string(capture.s) + ")").c_str(), "", capture.p);
									}, new_mail(&_capture));
									utils::pop_parent();
								}
							}, new_mail(&capture));
						}, new_mail(&capture), false);
					}
					else
					{
						if (tag == TypeEnumSingle || tag == TypeEnumMulti)
						{
							struct _Capture
							{
								TypeTag t;
								const char* s;
								Vec2f p;
							}_capture;
							_capture.t = tag;
							_capture.p = capture.p;
							_capture.s = base_name;
							utils::e_menu_item(((tag == TypeEnumSingle ? L"EnumSingle(" : L"EnumMulti(") + s2w(base_name) + L")").c_str(), [](void* c) {
								auto& capture = *(_Capture*)c;
								auto n = app.add_node(((capture.t == TypeEnumSingle ? "EnumSingle(" : "EnumMulti(") + std::string(capture.s) + ")").c_str(), "", capture.p);
								auto s = app.editor->dragging_slot;
								if (s)
								{
									if (s->io() == BP::Slot::In)
										s->link_to(n->find_output("out"));
									else
										n->find_input("in")->link_to(s);
								}
							}, new_mail(&_capture));
						}
						else if (tag == TypeData && !is_array)
						{
							if (basic_type_size(base_hash))
							{
								struct _Capture
								{
									const char* s;
									Vec2f p;
								}_capture;
								_capture.p = capture.p;
								_capture.s = base_name;
								utils::e_menu_item((L"Variable(" + s2w(base_name) + L")").c_str(), [](void* c) {
									auto& capture = *(_Capture*)c;
									auto n = app.add_node(("Variable" + std::string(capture.s) + ")").c_str(), "", capture.p);
									auto s = app.editor->dragging_slot;
									if (s)
									{
										if (s->io() == BP::Slot::In)
											s->link_to(n->find_output("out"));
										else
											n->find_input("in")->link_to(s);
									}
								}, new_mail(&_capture));
							}
						}
						else if (tag == TypePointer && is_array)
						{

						}
					}
					{
						struct Capture
						{
							UdtInfo* u;
							Vec2f p;
						}capture;
						capture.p = (Vec2f(pos) - c_base_element->global_pos) / (scale * 0.1f);
						for (auto udt : node_types)
						{
							capture.u = udt.first;
							utils::e_menu_item(s2w(udt.first->type()->name()).c_str(), [](void* c) {
								auto& capture = *(Capture*)c;
								app.add_node(capture.u->type()->name() + 2, "", capture.p);
							}, new_mail(&capture));
						}
					}
				utils::e_end_list();
			utils::e_end_scroll_view1();
		utils::pop_parent();
	utils::pop_parent();

	{
		struct Capture
		{
			Entity* l;
			cText* t;
		}capture;
		capture.l = e_list;
		capture.t = c_text_search;
		c_text_search->data_changed_listeners.add([](void* c, uint hash, void*) {
			auto& capture = *(Capture*)c;
			std::wstring str = capture.t->text();
			for (auto i = 0; i < capture.l->child_count(); i++)
			{
				auto item = capture.l->child(i);
				item->set_visible(str[0] ? (std::wstring(item->get_component(cText)->text()).find(str) != std::string::npos) : true);
			}
			return true;
		}, new_mail(&capture));
	}
}

template <class T>
void create_edit(cEditor* editor, BP::Slot* input)
{
	auto& data = *(T*)input->data();

	utils::push_style_1u(utils::FontSize, 12);
	auto e_edit = utils::e_drag_edit(std::is_floating_point<T>::value);
	struct Capture
	{
		BP::Slot* input;
		cText* edit_text;
		cText* drag_text;
	}capture;
	capture.input = input;
	capture.edit_text = e_edit->child(0)->get_component(cText);
	capture.drag_text = e_edit->child(1)->get_component(cText);
	capture.edit_text->data_changed_listeners.add([](void* c, uint hash, void*) {
		auto& capture = *(Capture*)c;
		if (hash == FLAME_CHASH("text"))
		{
			auto text = capture.edit_text->text();
			auto data = sto_s<T>(text);
			capture.input->set_data(&data);
			capture.drag_text->set_text(text);
			app.set_changed(true);
		}
		return true;
	}, new_mail(&capture));
	utils::pop_style(utils::FontSize);

	auto c_tracker = new_u_object<cDigitalDataTracker<T>>();
	c_tracker->data = &data;
	utils::current_parent()->add_component(c_tracker);
}

template <uint N, class T>
void create_vec_edit(cEditor* editor, BP::Slot* input)
{
	auto& data = *(Vec<N, T>*)input->data();

	struct Capture
	{
		BP::Slot* input;
		uint i;
		cText* edit_text;
		cText* drag_text;
	}capture;
	capture.input = input;
	utils::push_style_1u(utils::FontSize, 12);
	for (auto i = 0; i < N; i++)
	{
		utils::e_begin_layout(LayoutHorizontal, 4.f);
		auto e_edit = utils::e_drag_edit(std::is_floating_point<T>::value);
		capture.i = i;
		capture.edit_text = e_edit->child(0)->get_component(cText);
		capture.drag_text = e_edit->child(1)->get_component(cText);
		capture.edit_text->data_changed_listeners.add([](void* c, uint hash, void*) {
			auto& capture = *(Capture*)c;
			if (hash == FLAME_CHASH("text"))
			{
				auto text = capture.edit_text->text();
				auto data = *(Vec<N, T>*)capture.input->data();
				data[capture.i] = sto_s<T>(text);
				capture.input->set_data(&data);
				capture.drag_text->set_text(text);
				app.set_changed(true);
			}
			return true;
		}, new_mail(&capture));
		utils::e_text(s2w(Vec<N, T>::coord_name(i)).c_str());
		utils::e_end_layout();
	}
	utils::pop_style(utils::FontSize);

	auto c_tracker = new_u_object<cDigitalVecDataTracker<N, T>>();
	c_tracker->data = &data;
	utils::current_parent()->add_component(c_tracker);
}

void cEditor::on_add_node(BP::Node* n)
{
	utils::push_parent(e_base);
		auto e_node = utils::e_empty();
		n->user_data = e_node;
		{
			auto c_element = utils::c_element();
			c_element->pos_ = n->pos;
			c_element->roundness_ = 8.f;
			c_element->roundness_lod = 2;
			c_element->frame_thickness_ = 4.f;
			c_element->color_ = Vec4c(255, 255, 255, 200);
			c_element->frame_color_ = unselected_col;
			utils::c_event_receiver();
			utils::c_layout(LayoutVertical)->fence = 1;
			utils::c_moveable();
		}
			auto c_node = new_u_object<cNode>();
			c_node->n = n;
			{
				std::string parameters;
				c_node->n_type = BP::type_from_node_name(n->type(), parameters);
				c_node->n_name = c_node->n_type ? s2w(parameters) : s2w(n->type());
			}
			e_node->add_component(c_node);
		utils::push_parent(e_node);
			utils::e_begin_layout(LayoutVertical, 4.f)->get_component(cElement)->inner_padding_ = Vec4f(8.f);
				utils::push_style_1u(utils::FontSize, 21);
				utils::e_begin_layout(LayoutHorizontal, 4.f);
					{
						auto e_text = utils::e_text(s2w(n->id()).c_str());
						e_text->get_component(cElement)->inner_padding_ = Vec4f(4.f, 2.f, 4.f, 2.f);
						e_text->get_component(cText)->color_ = node_type_color(c_node->n_type);
						utils::push_style_4c(utils::ButtonColorNormal, Vec4c(0));
						struct Capture
						{
							BP::Node* n;
							cText* t;
						}capture;
						capture.n = n;
						capture.t = e_text->get_component(cText);
						utils::e_button(Icon_PENCIL_SQUARE_O, [](void* c) {
							auto& capture = *(Capture*)c;
							utils::e_input_dialog(L"ID", [](void* c, bool ok, const wchar_t* text) {
								if (ok && text[0])
								{
									auto& capture = *(Capture*)c;
									capture.n->set_id(w2s(text).c_str());
									capture.t->set_text(text);
								}
							}, new_mail(&capture), s2w(capture.n->id()).c_str());
						}, new_mail(&capture));
					}
					utils::pop_style(utils::ButtonColorNormal);
				utils::e_end_layout();
				utils::pop_style(utils::FontSize);

				std::string type = n->type();
				auto udt = n->udt();

				if (n->id() == "test_dst")
				{
					utils::e_button(L"Show", [](void* c) {
						//open_image_viewer(*(uint*)(*(BP::Node**)c)->find_output("idx")->data(), Vec2f(1495.f, 339.f));
					}, new_mail_p(n));
				}
				else if (type == "D#graphics::Shader")
				{
					/*
					auto e_edit = create_standard_button(app.font_atlas_pixel, 0.9f, L"Edit Shader");
					e_content->add_child(e_edit);

					struct Capture
					{
						BP::Node* n;
					}capture;
					capture.n = n;
					e_edit->get_component(cEventReceiver)->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
						auto& capture = *(Capture*)c;

						if (is_mouse_clicked(action, key))
						{
							capture.e->locked = true;
							auto t = create_topmost(capture.e->entity, false, false, true, Vec4c(255, 255, 255, 235), true);
							{
								t->get_component(cElement)->inner_padding_ = Vec4f(4.f);

								auto c_layout = cLayout::create(LayoutVertical);
								c_layout->width_fit_children = false;
								c_layout->height_fit_children = false;
								t->add_component(c_layout);
							}

							auto e_buttons = Entity::create();
							t->add_child(e_buttons);
							{
								e_buttons->add_component(cElement::create());

								auto c_layout = cLayout::create(LayoutHorizontal);
								c_layout->item_padding = 4.f;
								e_buttons->add_component(c_layout);
							}

							auto e_back = create_standard_button(app.font_atlas_pixel, 1.f, L"Back");
							e_buttons->add_child(e_back);
							{
								e_back->get_component(cEventReceiver)->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
									if (is_mouse_clicked(action, key))
									{
										destroy_topmost(editor->entity, false);
										editor->locked = false;
									}
								}, new_mail_p(capture.e));
							}

							auto e_compile = create_standard_button(app.font_atlas_pixel, 1.f, L"Compile");
							e_buttons->add_child(e_compile);

							auto e_text_tip = Entity::create();
							e_buttons->add_child(e_text_tip);
							{
								e_text_tip->add_component(cElement::create());

								auto c_text = cText::create(app.font_atlas_pixel);
								c_text->set_text(L"(Do update first to get popper result)");
								e_text_tip->add_component(c_text);
							}

							auto filename = ssplit(*(std::wstring*)capture.n->find_input("filename")->data(), L':')[0];

							auto e_text_view = Entity::create();
							{
								auto c_element = cElement::create();
								c_element->clip_children = true;
								e_text_view->add_component(c_element);

								auto c_aligner = cAligner::create();
								c_aligner->width_policy_ = SizeFitParent;
								c_aligner->height_policy_ = SizeFitParent;
								e_text_view->add_component(c_aligner);

								auto c_layout = cLayout::create(LayoutVertical);
								c_layout->width_fit_children = false;
								c_layout->height_fit_children = false;
								e_text_view->add_component(c_layout);
							}

							auto e_text = Entity::create();
							e_text_view->add_child(e_text);
							{
								e_text->add_component(cElement::create());

								auto c_text = cText::create(app.font_atlas_pixel);
								auto file = get_file_string(capture.e->filepath + L"/" + filename);
								c_text->set_text(s2w(file).c_str());
								c_text->auto_width_ = false;
								e_text->add_component(c_text);

								e_text->add_component(cEventReceiver::create());

								e_text->add_component(cEdit::create());

								auto c_aligner = cAligner::create();
								c_aligner->width_policy_ = SizeFitParent;
								e_text->add_component(c_aligner);

								{
									struct _Capture
									{
										BP::Node* n;
										cText* t;
									}_capture;
									_capture.n = capture.n;
									_capture.t = c_text;
									e_compile->get_component(cEventReceiver)->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
										auto& capture = *(_Capture*)c;
										if (is_mouse_clicked(action, key))
										{
											auto i_filename = capture.n->find_input("filename");
											std::ofstream file(capture.e->filepath + L"/" + *(std::wstring*)i_filename->data());
											auto str = w2s(capture.t->text());
											str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
											file.write(str.c_str(), str.size());
											file.close();
											i_filename->set_frame(capture.n->scene()->frame);
										}
									}, new_mail(&_capture));
								}
							}

							auto e_scrollbar_container = wrap_standard_scrollbar(e_text_view, ScrollbarVertical, true, default_style.font_size);
							e_scrollbar_container->get_component(cAligner)->height_factor_ = 2.f / 3.f;
							t->add_child(e_scrollbar_container);
						}
					}, new_mail(&capture));
					*/
				}
				utils::e_begin_layout(LayoutHorizontal, 16.f);
				utils::c_aligner(SizeGreedy, SizeFixed);

					utils::e_begin_layout(LayoutVertical);
					utils::c_aligner(SizeGreedy, SizeFixed);
						for (auto i = 0; i < n->input_count(); i++)
						{
							auto input = n->input(i);

							utils::e_begin_layout(LayoutVertical, 2.f);

							utils::e_begin_layout(LayoutHorizontal);
							utils::e_empty();
							{
								auto c_element = utils::c_element();
								auto r = utils::style_1u(utils::FontSize);
								c_element->size_ = r;
								c_element->roundness_ = r * 0.4f;
								c_element->roundness_lod = 2;
								c_element->color_ = Vec4c(200, 200, 200, 255);
							}
							utils::c_event_receiver();
							auto c_slot = new_u_object<cSlot>();
							c_slot->s = input;
							utils::current_entity()->add_component(c_slot);
							input->user_data = c_slot;
							utils::e_begin_popup_menu(false);
							utils::e_end_popup_menu();

							utils::e_text(s2w(input->name()).c_str())->get_component(cText)->color_ = type_color(input->type()->tag());
							utils::e_end_layout();

							auto e_data = utils::e_begin_layout(LayoutVertical, 2.f);
							e_data->get_component(cElement)->inner_padding_ = Vec4f(utils::style_1u(utils::FontSize), 0.f, 0.f, 0.f);
							std::vector<TypeinfoDatabase*> dbs;
							dbs.resize(app.bp->library_count());
							for (auto i = 0; i < dbs.size(); i++)
								dbs[i] = app.bp->library(i)->db();
							extra_global_db_count = dbs.size();
							extra_global_dbs = dbs.data();
							auto type = input->type();
							auto base_hash = type->base_hash();
							utils::push_style_1u(utils::FontSize, 12);

							switch (type->tag())
							{
							case TypeEnumSingle:
							{
								auto info = find_enum(base_hash);
								utils::create_enum_combobox(info, 120.f);

								struct Capture
								{
									BP::Slot* input;
									EnumInfo* e;
									cCombobox* cb;
								}capture;
								capture.input = input;
								capture.e = info;
								capture.cb = e_data->child(0)->get_component(cCombobox);
								capture.cb->data_changed_listeners.add([](void* c, uint hash, void*) {
									auto& capture = *(Capture*)c;
									if (hash == FLAME_CHASH("index"))
									{
										auto v = capture.e->item(capture.cb->idx)->value();
										capture.input->set_data(&v);
										app.set_changed(true);
									}
									return true;
								}, new_mail(&capture));

								auto c_tracker = new_u_object<cEnumSingleDataTracker>();
								c_tracker->data = input->data();
								c_tracker->info = info;
								e_data->add_component(c_tracker);
							}
							break;
							case TypeEnumMulti:
							{
								auto v = *(int*)input->data();

								auto info = find_enum(base_hash);
								utils::create_enum_checkboxs(info);
								for (auto k = 0; k < info->item_count(); k++)
								{
									auto item = info->item(k);

									struct Capture
									{
										BP::Slot* input;
										int v;
										cCheckbox* cb;
									}capture;
									capture.input = input;
									capture.v = item->value();
									capture.cb = e_data->child(k)->child(0)->get_component(cCheckbox);
									capture.cb->data_changed_listeners.add([](void* c, uint hash, void*) {
										auto& capture = *(Capture*)c;
										if (hash == FLAME_CHASH("checked"))
										{
											auto v = *(int*)capture.input->data();
											if (capture.cb->checked)
												v |= capture.v;
											else
												v &= ~capture.v;
											capture.input->set_data(&v);
											app.set_changed(true);
										}
										return true;
									}, new_mail(&capture));
								}

								auto c_tracker = new_u_object<cEnumMultiDataTracker>();
								c_tracker->data = input->data();
								c_tracker->info = info;
								e_data->add_component(c_tracker);
							}
							break;
							case TypeData:
								switch (base_hash)
								{
								case FLAME_CHASH("bool"):
								{
									auto e_checkbox = utils::e_checkbox(L"");

									struct Capture
									{
										BP::Slot* input;
										cCheckbox* cb;
									}capture;
									capture.input = input;
									capture.cb = e_checkbox->get_component(cCheckbox);
									capture.cb->data_changed_listeners.add([](void* c, uint hash, void*) {
										auto& capture = *(Capture*)c;
										if (hash == FLAME_CHASH("checked"))
										{
											auto v = (capture.cb->checked) ? 1 : 0;
											capture.input->set_data(&v);
											app.set_changed(true);
										}
										return true;
									}, new_mail(&capture));

									auto c_tracker = new_u_object<cBoolDataTracker>();
									c_tracker->data = input->data();
									e_data->add_component(c_tracker);
								}
								break;
								case FLAME_CHASH("int"):
									create_edit<int>(this, input);
									break;
								case FLAME_CHASH("flame::Vec(2+int)"):
									create_vec_edit<2, int>(this, input);
									break;
								case FLAME_CHASH("flame::Vec(3+int)"):
									create_vec_edit<3, int>(this, input);
									break;
								case FLAME_CHASH("flame::Vec(4+int)"):
									create_vec_edit<4, int>(this, input);
									break;
								case FLAME_CHASH("uint"):
									create_edit<uint>(this, input);
									break;
								case FLAME_CHASH("flame::Vec(2+uint)"):
									create_vec_edit<2, uint>(this, input);
									break;
								case FLAME_CHASH("flame::Vec(3+uint)"):
									create_vec_edit<3, uint>(this, input);
									break;
								case FLAME_CHASH("flame::Vec(4+uint)"):
									create_vec_edit<4, uint>(this, input);
									break;
								case FLAME_CHASH("float"):
									create_edit<float>(this, input);
									break;
								case FLAME_CHASH("flame::Vec(2+float)"):
									create_vec_edit<2, float>(this, input);
									break;
								case FLAME_CHASH("flame::Vec(3+float)"):
									create_vec_edit<3, float>(this, input);
									break;
								case FLAME_CHASH("flame::Vec(4+float)"):
									create_vec_edit<4, float>(this, input);
									break;
								case FLAME_CHASH("uchar"):
									create_edit<uchar>(this, input);
									break;
								case FLAME_CHASH("flame::Vec(2+uchar)"):
									create_vec_edit<2, uchar>(this, input);
									break;
								case FLAME_CHASH("flame::Vec(3+uchar)"):
									create_vec_edit<3, uchar>(this, input);
									break;
								case FLAME_CHASH("flame::Vec(4+uchar)"):
									create_vec_edit<4, uchar>(this, input);
									break;
								case FLAME_CHASH("flame::StringA"):
								{
									struct Capture
									{
										BP::Slot* i;
										cText* t;
									}capture;
									capture.i = input;
									capture.t = utils::e_edit(50.f)->get_component(cText);
									capture.t->data_changed_listeners.add([](void* c, uint hash, void*) {
										if (hash == FLAME_CHASH("text"))
										{
											auto& capture = *(Capture*)c;
											capture.i->set_data(&StringA(w2s(capture.t->text())));
											app.set_changed(true);
										}
										return true;
									}, new_mail(&capture));

									auto c_tracker = new_u_object<cStringADataTracker>();
									c_tracker->data = input->data();
									e_data->add_component(c_tracker);
								}
								break;
								case FLAME_CHASH("flame::StringW"):
								{
									struct Capture
									{
										BP::Slot* i;
										cText* t;
									}capture;
									capture.i = input;
									capture.t = utils::e_edit(50.f)->get_component(cText);
									capture.t->data_changed_listeners.add([](void* c, uint hash, void*) {
										if (hash == FLAME_CHASH("text"))
										{
											auto& capture = *(Capture*)c;
											capture.i->set_data(&StringW(capture.t->text()));
											app.set_changed(true);
										}
										return true;
									}, new_mail(&capture));

									auto c_tracker = new_u_object<cStringWDataTracker>();
									c_tracker->data = input->data();
									e_data->add_component(c_tracker);
								}
								break;
								}
								break;
							}
							extra_global_db_count = 0;
							extra_global_dbs = nullptr;
							utils::pop_style(utils::FontSize);
							utils::e_end_layout();

							utils::e_end_layout();

							c_slot->tracker = e_data->get_component(cDataTracker);
						}
					utils::e_end_layout();

					utils::e_begin_layout(LayoutVertical);
					utils::c_aligner(SizeGreedy, SizeFixed);
						for (auto i = 0; i < n->output_count(); i++)
						{
							auto output = n->output(i);

							utils::e_begin_layout(LayoutHorizontal);
							utils::c_aligner(AlignxRight, AlignyFree);
							utils::e_text(s2w(output->name()).c_str())->get_component(cText)->color_ = type_color(output->type()->tag());

							utils::e_empty();
							{
								auto c_element = utils::c_element();
								auto r = utils::style_1u(utils::FontSize);
								c_element->size_ = r;
								c_element->roundness_ = r * 0.4f;
								c_element->roundness_lod = 2;
								c_element->color_ = Vec4c(200, 200, 200, 255);
							}
							utils::c_event_receiver();
							auto c_slot = new_u_object<cSlot>();
							c_slot->s = output;
							utils::current_entity()->add_component(c_slot);
							output->user_data = c_slot;
							utils::e_begin_popup_menu(false);
							utils::e_end_popup_menu();
							utils::e_end_layout();
						}
					utils::e_end_layout();
				utils::e_end_layout();
			utils::e_end_layout();
			utils::e_empty();
			utils::c_element();
			utils::c_event_receiver()->pass_checkers.add([](void*, cEventReceiver*, bool* pass) {
				*pass = true;
				return true;
			}, Mail<>());
			utils::c_aligner(SizeFitParent, SizeFitParent);
			utils::c_bring_to_front();
		utils::pop_parent();
	utils::pop_parent();
}

void cEditor::on_remove_node(BP::Node* n)
{
	auto e = (Entity*)n->user_data;
	e->parent()->remove_child(e);
}

void cEditor::on_data_changed(BP::Slot* s)
{
	((cSlot*)s->user_data)->tracker->update_view();
}

