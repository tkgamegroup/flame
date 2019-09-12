#include <flame/foundation/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/device.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/image.h>
#include <flame/graphics/font.h>
#include <flame/graphics/canvas.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/edit.h>
#include <flame/universe/components/checkbox.h>
#include <flame/universe/components/combobox.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/event_dispatcher.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/scrollbar.h>
#include <flame/universe/components/menu.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/window.h>

#include "../app.h"
#include "blueprint_editor.h"
#include "console.h"
#include "image_viewer.h"

template<class T>
void create_edit(Entity* parent, BP::Slot* input)
{
	auto& data = *(T*)input->data();

	auto e_edit = create_standard_edit(50.f, app.font_atlas_sdf, 0.5f);
	parent->add_child(e_edit);
	{
		((cText*)e_edit->find_component(cH("Text")))->set_text(std::to_wstring(data));

		((cEdit*)e_edit->find_component(cH("Edit")))->add_changed_listener([](void* c, const wchar_t* text) {
			auto data = text[0] ? sto<T>(text) : 0;
			(*(BP::Slot**)c)->set_data(&data);
		}, new_mail_p(input));
	}
}

template<uint N, class T>
void create_vec_edit(Entity* parent, BP::Slot* input)
{
	static const wchar_t* part_names[] = {
		L"x",
		L"y",
		L"z",
		L"w"
	};

	auto& data = *(Vec<N, T>*)input->data();

	for (auto k = 0; k < 4; k++)
	{
		auto e_item = Entity::create();
		parent->add_child(e_item);
		{
			e_item->add_component(cElement::create());

			auto c_layout = cLayout::create();
			c_layout->type = LayoutHorizontal;
			c_layout->item_padding = 4.f;
			e_item->add_component(c_layout);
		}

		auto e_edit = create_standard_edit(50.f, app.font_atlas_sdf, 0.5f);
		e_item->add_child(e_edit);
		{
			((cText*)e_edit->find_component(cH("Text")))->set_text(std::to_wstring((int)data[k]));

			struct Capture
			{
				BP::Slot* input;
				uint i;
			}capture;
			capture.input = input;
			capture.i = k;
			((cEdit*)e_edit->find_component(cH("Edit")))->add_changed_listener([](void* c, const wchar_t* text) {
				auto& capture = *(Capture*)c;
				auto data = *(Vec4c*)capture.input->data();
				data[capture.i] = text[0] ? std::stoi(text) : 0;
				capture.input->set_data(&data);
			}, new_mail(&capture));
		}

		auto e_name = Entity::create();
		e_item->add_child(e_name);
		{
			e_name->add_component(cElement::create());

			auto c_text = cText::create(app.font_atlas_sdf);
			c_text->sdf_scale = 0.5f;
			c_text->set_text(part_names[k]);
			e_name->add_component(c_text);
		}
	}
}

struct cBPEditor : Component
{
	std::wstring filename;
	BP* bp;
	std::vector<TypeinfoDatabase*> dbs;

	cDockerTab* console_tab;

	graphics::Image* rt;
	graphics::Imageview* rt_v;
	uint rt_id;
	std::vector<void*> rt_cbs;

	enum SelType
	{
		SelAir,
		SelNode,
		SelLink
	}sel_type;
	union
	{
		BP::Node* n;
		BP::Slot* l;
	}selected;
	BP::Slot* dragging_slot;

	bool running;

	cBPEditor() :
		Component("BPEditor")
	{
	}

	~cBPEditor()
	{
		if (console_tab)
			console_tab->take_away(true);

		app.canvas->set_image(rt_id, nullptr);
	}

	void init(const std::wstring& _filename, bool no_compile)
	{
		filename = _filename;
		bp = BP::create_from_file(filename, no_compile);
		for (auto i = 0; i < bp->dependency_count(); i++)
			dbs.push_back(bp->dependency_typeinfodatabase(i));
		dbs.push_back(bp->typeinfodatabase);

		console_tab = nullptr;

		rt = graphics::Image::create(app.d, Format_R8G8B8A8_UNORM, Vec2u(400, 300), 1, 1, SampleCount_1, ImageUsage$(ImageUsageTransferDst | ImageUsageAttachment | ImageUsageSampled));
		rt->init(Vec4c(0, 0, 0, 255));
		rt_v = Imageview::create(rt);
		rt_id = app.canvas->find_free_image();
		rt_cbs.resize(1);
		rt_cbs[0] = Commandbuffer::create(app.d->gcp);

		bp->graphics_device = app.d;
		bp->find_input("rt_dst.type")->set_data_i(TargetImageview);
		bp->find_input("rt_dst.v")->set_data_p(rt_v);
		bp->find_input("make_cmd.cmdbufs")->set_data_p(&rt_cbs);

		app.canvas->set_image(rt_id, rt_v);

		sel_type = SelAir;
		selected.n = nullptr;
		dragging_slot = nullptr;
		running = false;
	}

	void delete_selected()
	{
		switch (sel_type)
		{
		case SelNode:
			bp->remove_node(selected.n);
			break;
		case SelLink:
			selected.l->link_to(nullptr);
			break;
		}
		sel_type = SelAir;
		selected.n = nullptr;
	}

	virtual void update() override
	{
		if (running)
		{
			bp->update();
			app.extra_cbs.push_back((Commandbuffer*)rt_cbs[0]);
		}
	}
};

struct cBP : Component
{
	cElement* element;
	cEventReceiver* event_receiver;
	cElement* base_element;

	cBPEditor* editor;

	cBP() :
		Component("BP")
	{
	}

	virtual void start() override
	{
		element = (cElement*)(entity->find_component(cH("Element")));
		event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
		base_element = (cElement*)(entity->child(0)->find_component(cH("Element")));

		event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
			auto thiz = *(cBP**)c;
			if (is_mouse_down(action, key, true) && key == Mouse_Left)
			{
				thiz->editor->sel_type = cBPEditor::SelAir;
				thiz->editor->selected.n = nullptr;
				auto bp = thiz->editor->bp;

				for (auto i = 0; i < bp->node_count(); i++)
				{
					auto n = bp->node(i);
					for (auto j = 0; j < n->input_count(); j++)
					{
						auto input = n->input(j);
						auto output = input->link(0);
						if (output)
						{
							auto e1 = ((cElement*)output->user_data);
							auto e2 = ((cElement*)input->user_data);
							auto p1 = Vec2f(e1->global_x + e1->global_width * 0.5f, e1->global_y + e1->global_height * 0.5f);
							auto p2 = Vec2f(e2->global_x + e2->global_width * 0.5f, e2->global_y + e2->global_height * 0.5f);

							if (distance(pos, bezier_closest_point(pos, p1, p1 + Vec2f(50.f, 0.f), p2 - Vec2f(50.f, 0.f), p2, 4, 7)) < 3.f * thiz->element->global_scale)
							{
								thiz->editor->sel_type = cBPEditor::SelLink;
								thiz->editor->selected.l = input;
							}
						}
					}
				}
			}
			else if (is_mouse_scroll(action, key))
			{
				thiz->base_element->scale += pos.x() < 0.f ? 0.1f : -0.1f;
				thiz->base_element->scale = clamp(thiz->base_element->scale, 0.1f, 2.f);
			}
			else if (is_mouse_move(action, key) && (thiz->event_receiver->event_dispatcher->mouse_buttons[Mouse_Right] & KeyStateDown))
			{
				thiz->base_element->x += pos.x();
				thiz->base_element->y += pos.y();
			}
		}, new_mail_p(this));
	}

	virtual void update() override
	{
		for (auto i = 0; i < editor->bp->node_count(); i++)
		{
			auto n = editor->bp->node(i);
			for (auto j = 0; j < n->input_count(); j++)
			{
				auto input = n->input(j);
				auto output = input->link(0);
				if (output)
				{
					auto e1 = ((cElement*)output->user_data);
					auto e2 = ((cElement*)input->user_data);
					auto p1 = Vec2f(e1->global_x + e1->global_width * 0.5f, e1->global_y + e1->global_height * 0.5f);
					auto p2 = Vec2f(e2->global_x + e2->global_width * 0.5f, e2->global_y + e2->global_height * 0.5f);

					std::vector<Vec2f> points;
					path_bezier(points, p1, p1 + Vec2f(50.f, 0.f), p2 - Vec2f(50.f, 0.f), p2);
					element->canvas->stroke(points, editor->selected.l == input ? Vec4c(255, 255, 50, 255) : Vec4c(100, 100, 120, 255), 3.f * element->global_scale);
				}
			}
		}
		if (editor->dragging_slot)
		{
			auto e = ((cElement*)editor->dragging_slot->user_data);
			auto p1 = Vec2f(e->global_x + e->global_width * 0.5f, e->global_y + e->global_height * 0.5f);
			auto p2 = Vec2f(event_receiver->event_dispatcher->mouse_pos);

			std::vector<Vec2f> points;
			path_bezier(points, p1, p1 + Vec2f(editor->dragging_slot->type == BP::Slot::Output ? 50.f : -50.f, 0.f), p2, p2);
			element->canvas->stroke(points, Vec4c(255, 255, 50, 255), 3.f * element->global_scale);
		}
	}
};

struct cBPNode : Component
{
	cElement* element;
	cEventReceiver* event_receiver;
	cWindow* window;

	cBPEditor* editor;
	BP::Node* n;

	cBPNode() :
		Component("BPNode")
	{
	}

	virtual void start() override
	{
		element = (cElement*)(entity->find_component(cH("Element")));
		event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
		window = (cWindow*)(entity->find_component(cH("Window")));

		event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
			auto thiz = *(cBPNode**)c;
			if (is_mouse_down(action, key, true) && key == Mouse_Left)
			{
				thiz->editor->sel_type = cBPEditor::SelAir;
				thiz->editor->selected.n = thiz->n;
			}
		}, new_mail_p(this));

		window->add_pos_listener([](void* c) {
			auto n = *(BP::Node**)c;
			auto element = (cElement*)(((Entity*)n->user_data)->find_component(cH("Element")));
			n->pos.x() = element->x;
			n->pos.y() = element->y;
		}, new_mail_p(n));
	}

	virtual void update() override
	{
		if (n == editor->selected.n)
			element->background_frame_thickness = 4.f;
		else
			element->background_frame_thickness = 0.f;
	}
};

struct cBPSlot : Component
{
	cEventReceiver* event_receiver;

	cBPEditor* editor;
	BP::Slot* s;

	cBPSlot() :
		Component("BPSlot")
	{
	}

	virtual void start() override
	{
		event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
		if (s->type == BP::Slot::Input)
		{
			event_receiver->drag_hash = cH("input_slot");
			event_receiver->set_acceptable_drops({ cH("output_slot") });
		}
		else
		{
			event_receiver->drag_hash = cH("output_slot");
			event_receiver->set_acceptable_drops({ cH("input_slot") });
		}

		event_receiver->add_drag_and_drop_listener([](void* c, DragAndDrop action, cEventReceiver* er, const Vec2f& pos) {
			auto thiz = *(cBPSlot**)c;
			if (action == DragStart)
			{
				thiz->editor->dragging_slot = thiz->s;
				if (thiz->s->type == BP::Slot::Input)
					thiz->s->link_to(nullptr);
			}
			else if (action == DragEnd)
				thiz->editor->dragging_slot = nullptr;
			else if (action == Dropped)
			{
				auto oth = ((cBPSlot*)er->entity->find_component(cH("BPSlot")))->s;
				if (thiz->s->type == BP::Slot::Input)
					thiz->s->link_to(oth);
				else
					oth->link_to(thiz->s);
			}
		}, new_mail_p(this));
	}
};

void open_blueprint_editor(const std::wstring& filename, bool no_compile, const Vec2f& pos)
{
	auto e_container = get_docker_container_model()->copy();
	app.root->add_child(e_container);
	{
		auto c_element = (cElement*)e_container->find_component(cH("Element"));
		c_element->x = pos.x();
		c_element->y = pos.y();
		c_element->width = 800.f;
		c_element->height = 400.f;
	}

	auto e_docker = get_docker_model()->copy();
	e_container->add_child(e_docker);

	e_docker->child(0)->add_child(create_standard_docker_tab(app.font_atlas_pixel, L"Blueprint Editor", app.root));

	auto e_page = get_docker_page_model()->copy();
	e_docker->child(1)->add_child(e_page);
	{
		auto c_layout = cLayout::create();
		c_layout->type = LayoutVertical;
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;
		e_page->add_component(c_layout);
	}

	auto c_editor = new_component<cBPEditor>();
	c_editor->init(filename, no_compile);
	e_page->add_component(c_editor);

	auto e_menubar = create_standard_menubar();
	e_page->add_child(e_menubar);

	{
		auto e_menu = create_standard_menu();
		for (auto db : c_editor->dbs)
		{
			auto udts = db->get_udts();
			for (auto i = 0; i < udts.p->size(); i++)
			{
				auto udt = udts.p->at(i);
				auto e_item = create_standard_menu_item(app.font_atlas_pixel, 1.f, s2w(udt->name()));
				e_menu->add_child(e_item);
				struct Capture
				{
					cBPEditor* e;
					UdtInfo* u;
				}capture;
				capture.e = c_editor;
				capture.u = udt;
				((cEventReceiver*)e_item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
					if (is_mouse_clicked(action, key))
					{
						auto& capture = *(Capture*)c;
						capture.e->bp->add_node(capture.u->name(), "");
					}
				}, new_mail(&capture));
			}
			delete_mail(udts);
		}
		auto e_menu_btn = create_standard_menu_button(app.font_atlas_pixel, 1.f, L"Add", app.root, e_menu, true, SideS, true, false, true, nullptr);
		e_menubar->add_child(e_menu_btn);
	}

	{
		auto e_menu = create_standard_menu();
		{
			auto e_item = create_standard_menu_item(app.font_atlas_pixel, 1.f, L"Delete");
			e_menu->add_child(e_item);
			((cEventReceiver*)e_item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				if (is_mouse_clicked(action, key))
				{
					auto editor = *(cBPEditor**)c;
					editor->delete_selected();
				}
			}, new_mail_p(c_editor));
		}
		auto e_menu_btn = create_standard_menu_button(app.font_atlas_pixel, 1.f, L"Edit", app.root, e_menu, true, SideS, true, false, true, nullptr);
		e_menubar->add_child(e_menu_btn);
	}

	auto e_btn_run = create_standard_button(app.font_atlas_pixel, 1.f, L"Run");;
	e_page->add_child(e_btn_run);
	{
		auto c_event_receiver = (cEventReceiver*)e_btn_run->find_component(cH("EventReceiver"));
		struct Capture
		{
			cBPEditor* e;
			cText* t;
		}capture;
		capture.e = c_editor;
		capture.t = (cText*)e_btn_run->find_component(cH("Text"));
		c_event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
			if (is_mouse_clicked(action, key))
			{
				auto& capture = *(Capture*)c;
				capture.e->running = !capture.e->running;
				capture.t->set_text(capture.e->running ? L"Pause" : L"Run");
			}
		}, new_mail(&capture));
	}

	auto e_scene = Entity::create();
	e_page->add_child(e_scene);
	{
		auto c_element = cElement::create();
		c_element->clip_children = true;
		e_scene->add_component(c_element);

		e_scene->add_component(cEventReceiver::create());

		auto c_aligner = cAligner::create();
		c_aligner->width_policy = SizeFitParent;
		c_aligner->height_policy = SizeFitParent;
		e_scene->add_component(c_aligner);

		auto c_bp = new_component<cBP>();
		c_bp->editor = c_editor;
		e_scene->add_component(c_bp);
	}

	auto e_base = Entity::create();
	e_scene->add_child(e_base);
	{
		e_base->add_component(cElement::create());
	}

	auto bp = c_editor->bp;
	for (auto i = 0; i < bp->node_count(); i++)
	{
		auto n = bp->node(i);

		auto e_node = Entity::create();
		e_base->add_child(e_node);
		n->user_data = e_node;
		{
			auto c_element = cElement::create();
			c_element->x = n->pos.x();
			c_element->y = n->pos.y();
			c_element->inner_padding = Vec4f(8.f);
			c_element->background_color = Vec4c(255, 255, 255, 200);
			c_element->background_frame_color = Vec4c(252, 252, 50, 200);
			c_element->background_round_radius = 8.f;
			c_element->background_shadow_thickness = 8.f;
			e_node->add_component(c_element);

			e_node->add_component(cEventReceiver::create());

			e_node->add_component(cWindow::create());

			auto c_layout = cLayout::create();
			c_layout->type = LayoutVertical;
			c_layout->item_padding = 4.f;
			e_node->add_component(c_layout);

			auto c_node = new_component<cBPNode>();
			c_node->editor = c_editor;
			c_node->n = n;
			e_node->add_component(c_node);

			auto e_text_id = Entity::create();
			e_node->add_child(e_text_id);
			{
				e_text_id->add_component(cElement::create());

				auto c_text = cText::create(app.font_atlas_sdf);
				c_text->set_text(s2w(n->id()));
				c_text->sdf_scale = 0.8f;
				e_text_id->add_component(c_text);
			}

			auto e_text_type = Entity::create();
			e_node->add_child(e_text_type);
			{
				e_text_type->add_component(cElement::create());

				auto c_text = cText::create(app.font_atlas_sdf);
				c_text->set_text(s2w(n->udt->name()));
				c_text->color = Vec4c(50, 50, 50, 255);
				c_text->sdf_scale = 0.5f;
				e_text_type->add_component(c_text);
			}

			auto e_content = Entity::create();
			e_node->add_child(e_content);
			{
				e_content->add_component(cElement::create());

				auto c_aligner = cAligner::create();
				c_aligner->width_policy = SizeGreedy;
				e_content->add_component(c_aligner);

				auto c_layout = cLayout::create();
				c_layout->type = LayoutHorizontal;
				c_layout->item_padding = 16.f;
				e_content->add_component(c_layout);

				auto e_left = Entity::create();
				e_content->add_child(e_left);
				{
					e_left->add_component(cElement::create());

					auto c_aligner = cAligner::create();
					c_aligner->width_policy = SizeGreedy;
					e_left->add_component(c_aligner);

					auto c_layout = cLayout::create();
					c_layout->type = LayoutVertical;
					e_left->add_component(c_layout);

					for (auto j = 0; j < n->input_count(); j++)
					{
						auto input = n->input(j);

						auto e_input = Entity::create();
						e_left->add_child(e_input);
						{
							e_input->add_component(cElement::create());

							auto c_layout = cLayout::create();
							c_layout->type = LayoutVertical;
							c_layout->item_padding = 2.f;
							e_input->add_component(c_layout);
						}

						auto e_title = Entity::create();
						e_input->add_child(e_title);
						{
							e_title->add_component(cElement::create());

							auto c_layout = cLayout::create();
							c_layout->type = LayoutHorizontal;
							e_title->add_component(c_layout);
						}

						auto e_slot = Entity::create();
						e_title->add_child(e_slot);
						{
							auto c_element = cElement::create();
							auto r = app.font_atlas_sdf->pixel_height * 0.6f;
							c_element->width = r;
							c_element->height = r;
							c_element->background_round_radius = r * 0.5f;
							c_element->background_color = Vec4c(200, 200, 200, 255);
							e_slot->add_component(c_element);
							input->user_data = c_element;

							e_slot->add_component(cEventReceiver::create());

							auto c_slot = new_component<cBPSlot>();
							c_slot->editor = c_editor;
							c_slot->s = input;
							e_slot->add_component(c_slot);
						}

						auto e_text = Entity::create();
						e_title->add_child(e_text);
						{
							e_text->add_component(cElement::create());

							auto c_text = cText::create(app.font_atlas_sdf);
							c_text->sdf_scale = 0.6f;
							c_text->set_text(s2w(input->variable_info->name()));
							e_text->add_component(c_text);
						}

						auto e_data = Entity::create();
						e_input->add_child(e_data);
						{
							auto c_element = cElement::create();
							c_element->inner_padding = Vec4f(app.font_atlas_sdf->pixel_height, 0.f, 0.f, 0.f);
							e_data->add_component(c_element);

							auto c_layout = cLayout::create();
							c_layout->type = LayoutVertical;
							c_layout->item_padding = 2.f;
							e_data->add_component(c_layout);
						}

						auto type = input->variable_info->type();
						switch (type->tag())
						{
						case TypeTagAttributeES:
						{
							auto info = find_enum(c_editor->dbs, type->hash());
							std::vector<std::wstring> items;
							for (auto k = 0; k < info->item_count(); k++)
								items.push_back(s2w(info->item(k)->name()));
							int init_idx;
							info->find_item(*(int*)input->data(), &init_idx);
							auto e_combobox = create_standard_combobox(120.f, app.font_atlas_sdf, 0.5f, app.root, items, init_idx);
							e_data->add_child(e_combobox);

							struct Capture
							{
								BP::Slot* input;
								EnumInfo* e;
							}capture;
							capture.input = input;
							capture.e = info;
							((cCombobox*)e_combobox->find_component(cH("Combobox")))->add_changed_listener([](void* c, uint idx) {
								auto& capture = *(Capture*)c;
								auto v = capture.e->item(idx)->value();
								capture.input->set_data(&v);
							}, new_mail(&capture));
						}
						break;
						case TypeTagAttributeEM:
						{
							auto v = *(int*)input->data();

							auto info = find_enum(c_editor->dbs, type->hash());
							for (auto k = 0; k < info->item_count(); k++)
							{
								auto item = info->item(k);
								auto e_checkbox = create_standard_checkbox(app.font_atlas_sdf, 0.5f, s2w(item->name()), (v & item->value()) != 0);
								e_data->add_child(e_checkbox);

								struct Capture
								{
									BP::Slot* input;
									int v;
								}capture;
								capture.input = input;
								capture.v = item->value();
								((cCheckbox*)e_checkbox->find_component(cH("Checkbox")))->add_changed_listener([](void* c, bool checked) {
									auto& capture = *(Capture*)c;
									auto v = *(int*)capture.input->data();
									if (checked)
										v |= capture.v;
									else
										v &= ~capture.v;
									capture.input->set_data(&v);
								}, new_mail(&capture));
							}
						}
						break;
						case TypeTagAttributeV:
							switch (type->hash())
							{
							case cH("bool"):
							{
								auto e_checkbox = create_standard_checkbox(app.font_atlas_sdf, 0.5f, L"", (*(int*)input->data()) != 0);
								e_data->add_child(e_checkbox);

								((cCheckbox*)e_checkbox->find_component(cH("Checkbox")))->add_changed_listener([](void* c, bool checked) {
									auto input = *(BP::Slot**)c;
									auto v = checked ? 1 : 0;
									input->set_data(&v);
								}, new_mail_p(input));
							}
							break;
							case cH("int"):
								create_edit<int>(e_data, input);
								break;
							case cH("Vec(2+int)"):
								create_vec_edit<2, int>(e_data, input);
								break;
							case cH("Vec(3+int)"):
								create_vec_edit<3, int>(e_data, input);
								break;
							case cH("Vec(4+int)"):
								create_vec_edit<4, int>(e_data, input);
								break;
							case cH("uint"):
								create_edit<uint>(e_data, input);
								break;
							case cH("Vec(2+uint)"):
								create_vec_edit<2, uint>(e_data, input);
								break;
							case cH("Vec(3+uint)"):
								create_vec_edit<3, uint>(e_data, input);
								break;
							case cH("Vec(4+uint)"):
								create_vec_edit<4, uint>(e_data, input);
								break;
							case cH("float"):
								create_edit<float>(e_data, input);
								break;
							case cH("Vec(2+float)"):
								create_vec_edit<2, float>(e_data, input);
								break;
							case cH("Vec(3+float)"):
								create_vec_edit<3, float>(e_data, input);
								break;
							case cH("Vec(4+float)"):
								create_vec_edit<4, float>(e_data, input);
								break;
							case cH("uchar"):
								create_edit<uchar>(e_data, input);
								break;
							case cH("Vec(2+uchar)"):
								create_vec_edit<2, uchar>(e_data, input);
								break;
							case cH("Vec(3+uchar)"):
								create_vec_edit<3, uchar>(e_data, input);
								break;
							case cH("Vec(4+uchar)"):
								create_vec_edit<4, uchar>(e_data, input);
								break;
							case cH("std::basic_string(char)"):
							{
								auto& str = *(std::string*)input->data();

								auto e_edit = create_standard_edit(50.f, app.font_atlas_sdf, 0.5f);
								e_data->add_child(e_edit);
								{
									((cText*)e_edit->find_component(cH("Text")))->set_text(s2w(str));

									((cEdit*)e_edit->find_component(cH("Edit")))->add_changed_listener([](void* c, const wchar_t* text) {
										auto str = w2s(text);
										(*(BP::Slot**)c)->set_data(&str);
									}, new_mail_p(input));
								}
							}
							break;
							case cH("std::basic_string(wchar_t)"):
							{
								auto& str = *(std::wstring*)input->data();

								auto e_edit = create_standard_edit(50.f, app.font_atlas_sdf, 0.5f);
								e_data->add_child(e_edit);
								{
									((cText*)e_edit->find_component(cH("Text")))->set_text(str);

									((cEdit*)e_edit->find_component(cH("Edit")))->add_changed_listener([](void* c, const wchar_t* text) {
										auto str = std::wstring(text);
										(*(BP::Slot**)c)->set_data(&str);
									}, new_mail_p(input));
								}
							}
							break;
							}
							break;
						}
					}
				}

				auto e_right = Entity::create();
				e_content->add_child(e_right);
				{
					e_right->add_component(cElement::create());

					auto c_layout = cLayout::create();
					c_layout->type = LayoutVertical;
					e_right->add_component(c_layout);

					for (auto j = 0; j < n->output_count(); j++)
					{
						auto output = n->output(j);

						auto e_title = Entity::create();
						e_right->add_child(e_title);
						{
							e_title->add_component(cElement::create());

							auto c_aligner = cAligner::create();
							c_aligner->x_align = AlignxRight;
							e_title->add_component(c_aligner);

							auto c_layout = cLayout::create();
							c_layout->type = LayoutHorizontal;
							e_title->add_component(c_layout);
						}

						auto e_text = Entity::create();
						e_title->add_child(e_text);
						{
							e_text->add_component(cElement::create());

							auto c_text = cText::create(app.font_atlas_sdf);
							c_text->sdf_scale = 0.6f;
							c_text->set_text(s2w(output->variable_info->name()));
							e_text->add_component(c_text);
						}

						auto e_slot = Entity::create();
						e_title->add_child(e_slot);
						{
							auto c_element = cElement::create();
							auto r = app.font_atlas_sdf->pixel_height * 0.6f;
							c_element->width = r;
							c_element->height = r;
							c_element->background_round_radius = r * 0.5f;
							c_element->background_color = Vec4c(200, 200, 200, 255);
							e_slot->add_component(c_element);
							output->user_data = c_element;

							e_slot->add_component(cEventReceiver::create());

							auto c_slot = new_component<cBPSlot>();
							c_slot->editor = c_editor;
							c_slot->s = output;
							e_slot->add_component(c_slot);
						}
					}
				}
			}
		}
	}

	auto console_page = open_console([](void* c, const std::wstring& cmd, cConsole* console) {
		auto editor = *(cBPEditor**)c;
		auto& filename = editor->filename;
		auto bp = editor->bp;
		auto& dbs = editor->dbs;
		auto tokens = string_split(cmd);

		auto set_data = [&](const std::string& address, const std::string& value) {
			auto i = bp->find_input(address.c_str());
			if (i)
			{
				auto v = i->variable_info;
				auto type = v->type();
				auto value_before = serialize_value(dbs, type->tag(), type->hash(), i->data(), 2);
				auto data = new char[v->size()];
				unserialize_value(dbs, type->tag(), type->hash(), value, data);
				i->set_data(data);
				delete data;
				auto value_after = serialize_value(dbs, type->tag(), type->hash(), i->data(), 2);
				console->print(L"set value: " + s2w(address) + L", " + s2w(*value_before.p) + L" -> " + s2w(*value_after.p));
				delete_mail(value_before);
				delete_mail(value_after);
			}
			else
				console->print(L"input not found");
		};

		auto generate_graph_and_layout = [&]() {
			if (GRAPHVIZ_PATH == std::string(""))
				assert(0);
			auto dot_path = s2w(GRAPHVIZ_PATH) + L"/bin/dot.exe";

			std::string gv = "digraph bp {\nrankdir=LR\nnode [shape = Mrecord];\n";
			for (auto i = 0; i < bp->node_count(); i++)
			{
				auto src = bp->node(i);
				auto& name = src->id();

				auto str = "\t" + name + " [label = \"" + name + "|" + src->udt->name() + "|{{";
				for (auto j = 0; j < src->input_count(); j++)
				{
					auto input = src->input(j);
					auto& name = input->variable_info->name();
					str += "<" + name + ">" + name;
					if (j != src->input_count() - 1)
						str += "|";
				}
				str += "}|{";
				for (auto j = 0; j < src->output_count(); j++)
				{
					auto output = src->output(j);
					auto& name = output->variable_info->name();
					str += "<" + name + ">" + name;
					if (j != src->output_count() - 1)
						str += "|";
				}
				str += "}}\"];\n";

				gv += str;
			}
			for (auto i = 0; i < bp->node_count(); i++)
			{
				auto src = bp->node(i);

				for (auto j = 0; j < src->input_count(); j++)
				{
					auto input = src->input(j);
					if (input->link())
					{
						auto in_addr = input->get_address();
						auto out_addr = input->link()->get_address();
						auto in_sp = string_split(*in_addr.p, '.');
						auto out_sp = string_split(*out_addr.p, '.');
						delete_mail(in_addr);
						delete_mail(out_addr);

						gv += "\t" + out_sp[0] + ":" + out_sp[1] + " -> " + in_sp[0] + ":" + in_sp[1] + ";\n";
					}
				}
			}
			gv += "}\n";

			std::ofstream file("bp.gv");
			file << gv;
			file.close();

			exec(dot_path, L"-Tpng bp.gv -o bp.png", true);
			exec(dot_path, L"-Tplain bp.gv -y -o bp.graph.txt", true);
		};

		if (tokens[0] == L"help")
		{
			console->print(
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
				"  set-layout - set nodes' positions using 'bp.png' and 'bp.graph.txt', need do show graph first"
			);
		}
		else if (tokens[0] == L"show")
		{
			if (tokens[1] == L"udts")
			{
				for (auto db : dbs)
				{
					auto udts = db->get_udts();
					for (auto i = 0; i < udts.p->size(); i++)
						console->print(s2w(udts.p->at(i)->name()));
					delete_mail(udts);
				}
			}
			else if (tokens[1] == L"udt")
			{
				auto udt = find_udt(dbs, H(w2s(tokens[2]).c_str()));
				if (udt)
				{
					console->print(s2w(udt->name()));
					std::vector<VariableInfo*> inputs;
					std::vector<VariableInfo*> outputs;
					for (auto i_i = 0; i_i < udt->variable_count(); i_i++)
					{
						auto vari = udt->variable(i_i);
						auto attribute = std::string(vari->decoration());
						if (attribute.find('i') != std::string::npos)
							inputs.push_back(vari);
						if (attribute.find('o') != std::string::npos)
							outputs.push_back(vari);
					}
					console->print(L"[In]");
					for (auto& i : inputs)
						console->print(L"name:" + s2w(i->name()) + L" decoration:" + s2w(i->decoration()) + L" tag:" + s2w(get_name(i->type()->tag())) + L" type:" + s2w(i->type()->name()));
					console->print(L"[Out]");
					for (auto& o : outputs)
						console->print(L"name:" + s2w(o->name()) + L" decoration:" + s2w(o->decoration()) + L" tag:" + s2w(get_name(o->type()->tag())) + L" type:" + s2w(o->type()->name()));
				}
				else
					console->print(L"udt not found");
			}
			else if (tokens[1] == L"nodes")
			{
				for (auto i = 0; i < bp->node_count(); i++)
				{
					auto n = bp->node(i);
					console->print(L"id:" + s2w(n->id()) + L" type:" + s2w(n->udt->name()));
				}
			}
			else if (tokens[1] == L"node")
			{
				auto n = bp->find_node(w2s(tokens[2]).c_str());
				if (n)
				{
					console->print(L"[In]");
					for (auto i = 0; i < n->input_count(); i++)
					{
						auto input = n->input(i);
						auto v = input->variable_info;
						auto type = v->type();
						console->print(s2w(v->name()));
						Mail<std::string> link_address;
						if (input->link())
							link_address = input->link()->get_address();
						console->print(L"[" + (link_address.p ? s2w(*link_address.p) : L"") + L"]");
						delete_mail(link_address);
						auto str = serialize_value(dbs, type->tag(), type->hash(), input->data(), 2);
						console->print(std::wstring(L"   ") + (str.p->empty() ? L"-" : s2w(*str.p)));
						delete_mail(str);
					}
					console->print(L"[Out]");
					for (auto i = 0; i < n->output_count(); i++)
					{
						auto output = n->output(i);
						auto v = output->variable_info;
						auto type = v->type();
						console->print(s2w(v->name()));
						auto str = serialize_value(dbs, type->tag(), type->hash(), output->data(), 2);
						console->print(std::wstring(L"   ") + (str.p->empty() ? L"-" : s2w(*str.p)));
						delete_mail(str);
					}
				}
				else
					console->print(L"node not found");
			}
			else if (tokens[1] == L"graph")
			{
				if (!std::filesystem::exists(L"bp.png") || std::filesystem::last_write_time(L"bp.png") < std::filesystem::last_write_time(filename))
					generate_graph_and_layout();
				if (std::filesystem::exists(L"bp.png"))
				{
					exec(L"bp.png", L"", false);
					console->print(L"ok");
				}
				else
					console->print(L"bp.png not found, perhaps Graphviz is not available");
			}
			else
				console->print(L"unknow object to show");
		}
		else if (tokens[0] == L"add")
		{
			if (tokens[1] == L"node")
			{
				auto n = bp->add_node(w2s(tokens[2]), tokens[3] == L"-" ? "" : w2s(tokens[3]));
				if (n)
					console->print(L"node added: " + s2w(n->id()));
				else
					console->print(L"bad udt name or id already exist");
			}
			else if (tokens[1] == L"link")
			{
				auto out = bp->find_output(w2s(tokens[2]));
				auto in = bp->find_input(w2s(tokens[3]));
				if (out && in)
				{
					in->link_to(out);
					auto out_addr = in->link()->get_address();
					auto in_addr = in->get_address();
					console->print(L"link added: " + s2w(*out_addr.p) + L" -> " + s2w(*in_addr.p));
					delete_mail(out_addr);
					delete_mail(in_addr);
				}
				else
					console->print(L"wrong address");
			}
			else
				console->print(L"unknow object to add");
		}
		else if (tokens[0] == L"remove")
		{
			if (tokens[1] == L"node")
			{
				auto n = bp->find_node(w2s(tokens[2]));
				if (n)
				{
					bp->remove_node(n);
					console->print(L"node removed: " + tokens[2]);
				}
				else
					console->print(L"node not found");
			}
			else if (tokens[1] == L"link")
			{
				auto i = bp->find_input(w2s(tokens[3]));
				if (i)
				{
					i->link_to(nullptr);
					console->print(L"link removed: " + tokens[2]);
				}
				else
					console->print(L"input not found");
			}
			else
				console->print(L"unknow object to remove");
		}
		else if (tokens[0] == L"set")
			set_data(w2s(tokens[1]), w2s(tokens[2]));
		else if (tokens[0] == L"update")
		{
			bp->update();
			console->print(L"BP updated");
		}
		else if (tokens[0] == L"save")
		{
			BP::save_to_file(bp, filename);
			console->print(L"file saved");
		}
		else if (tokens[0] == L"set-layout")
		{
			if (!std::filesystem::exists(L"bp.graph.txt") || std::filesystem::last_write_time(L"bp.graph.txt") < std::filesystem::last_write_time(filename))
				generate_graph_and_layout();
			if (std::filesystem::exists(L"bp.graph.txt"))
			{
				auto str = get_file_string(L"bp.graph.txt");
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
				std::smatch match;
				while (std::regex_search(str, match, reg_node))
				{
					auto n = bp->find_node(match[1].str().c_str());
					if (n)
						n->pos = Vec2f(std::stof(match[2].str().c_str()), std::stof(match[3].str().c_str())) * 100.f;

					str = match.suffix();
				}
				console->print(L"ok");
			}
			else
				console->print(L"bp.graph.txt not found");
		}
		else
			console->print(L"unknow command");
	}, new_mail_p(c_editor), [](void* c) {
		auto editor = *(cBPEditor**)c;
		editor->console_tab = nullptr;
	}, new_mail_p(c_editor), filename + L":", Vec2f(850.f, 420.f));
	c_editor->console_tab = (cDockerTab*)console_page->parent()->parent()->child(0)->child(0)->find_component(cH("DockerTab"));

	open_image_viewer(c_editor->rt_id, Vec2f(350.f, 300.f));
}
