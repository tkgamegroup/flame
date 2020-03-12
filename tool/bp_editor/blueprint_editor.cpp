#include <flame/universe/ui/typeinfo_utils.h>
#include "app.h"

#include <functional>

#include <flame/reflect_macros.h>

struct R(DstImage)
{
	BP::Node* n;

	BASE1;
	RV(Image*, img, o);
	RV(TargetType, type, o);
	RV(Imageview*, view, o);
	RV(uint, idx, o);

	__declspec(dllexport) void RF(update)(uint frame)
	{
		if (img_s()->frame() == -1)
		{
			if (idx > 0)
				app.canvas->set_image(idx, nullptr);
			if (img)
				Image::destroy(img);
			if (view)
				Imageview::destroy(view);
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
	}

	__declspec(dllexport) RF(~DstImage)()
	{
		if (idx > 0)
			app.canvas->set_image(idx, nullptr);
		if (img)
			Image::destroy(img);
		if (view)
			Imageview::destroy(view);
	}
};

struct R(CmdBufs)
{
	BP::Node* n;

	BASE1;
	RV(Array<Commandbuffer*>, out, o);

	__declspec(dllexport) void RF(active_update)(uint frame)
	{
		if (out_s()->frame() == -1)
		{
			for (auto i = 0; i < out.s; i++)
				Commandbuffer::destroy(out[i]);
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

	__declspec(dllexport) RF(~CmdBufs)()
	{
		for (auto i = 0; i < out.s; i++)
			Commandbuffer::destroy(out[i]);
	}
};

struct cSlot : Component
{
	cElement* element;
	cText* text;
	cEventReceiver* event_receiver;
	cDataTracker* tracker;

	BP::Slot* s;

	cSlot() :
		Component("cSlot")
	{
		element = nullptr;
		event_receiver = nullptr;
		tracker = nullptr;
	}

	void on_component_added(Component* c) override
	{
		if (c->name_hash == FLAME_CHASH("cElement"))
			element = (cElement*)c;
		else if (c->name_hash == FLAME_CHASH("cText"))
			text = (cText*)c;
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
				auto s = thiz->s;
				if (action == DragStart)
				{
					app.editor->dragging_slot = s;
					if (s->io() == BP::Slot::In)
					{
						s->link_to(nullptr);
						app.set_changed(true);
					}
				}
				else if (action == DragEnd)
					app.editor->dragging_slot = nullptr;
				else if (action == Dropped)
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
		}
	}
};

struct cScene : Component
{
	cElement* element;
	cEventReceiver* event_receiver;
	cElement* base_element;

	cText* scale_text;

	float link_stick_out;

	cScene() :
		Component("cScene")
	{
	}

	void for_each_link(const std::function<bool(const Vec2f& p1, const Vec2f& p2, BP::Slot* input)>& callback)
	{
		const auto process = [&](BP::Slot* output, BP::Slot* input) {
			if (!output->user_data && !input->user_data)
				return true;

			auto get_pos = [&](BP::Slot* s) {
				if (s->user_data)
				{
					auto e = ((cSlot*)s->user_data)->element;
					return e->global_pos + e->global_size * 0.5f;
				}
				else
				{
					auto scn = s->node()->scene();
					BP::SubGraph* sg;
					while (scn != app.bp)
					{
						sg = scn->scene();
						if (!sg)
							break;
						scn = sg->scene();
					}

					auto e = ((Entity*)sg->user_data)->get_component(cElement);
					auto ret = e->global_pos;
					if (s->io() == BP::Slot::Out)
						ret.x() += e->size_.x();
					return ret;
				}
			};

			if (!callback(get_pos(output), get_pos(input), input))
				return false;
			return true;
		};

		for (auto i = 0; i < app.bp->subgraph_count(); i++)
		{
			auto s = app.bp->subgraph(i);
			auto sbp = s->bp();
			for (auto j = 0; j < sbp->output_export_count(); j++)
			{
				auto output = sbp->output_export(j);
				for (auto k = 0; k < output->link_count(); k++)
				{
					auto input = output->link(k);
					if (input->node()->scene()->scene() != s)
					{
						if (!process(output, input))
							return;
					}
				}
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
					if (!process(output, output->link(k)))
						return;
				}
			}
		}
	}

	void on_component_added(Component* c) override
	{
		if (c->name_hash == FLAME_CHASH("cElement"))
		{
			element = (cElement*)c;
			element->cmds.add([](void* c, graphics::Canvas* canvas) {
				(*(cScene**)c)->draw(canvas);
				return true;
			}, new_mail_p(this));
		}
		else if (c->name_hash == FLAME_CHASH("cEventReceiver"))
		{
			event_receiver = (cEventReceiver*)c;
			event_receiver->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& _pos) {
				auto thiz = *(cScene**)c;
				auto pos = (Vec2f)_pos;
				auto line_width = 3.f * thiz->base_element->global_scale;
				if (is_mouse_down(action, key, true) && key == Mouse_Left)
				{
					app.deselect();

					thiz->for_each_link([&](const Vec2f& p1, const Vec2f& p2, BP::Slot* l) {
						if (segment_distance(p1, p2, pos) < line_width)
						{
							app.select(SelLink, l);
							return false;
						}
						return true;
					});
				}
				return true;
			}, new_mail_p(this));
		}
	}

	void draw(graphics::Canvas* canvas)
	{
		link_stick_out = 50.f * base_element->global_scale;
		auto line_width = 3.f * base_element->global_scale;

		if (element->cliped)
			return;

		for_each_link([&](const Vec2f& p1, const Vec2f& p2, BP::Slot* l) {
			if (rect_overlapping(rect(element->pos_, element->size_), Vec4f(min(p1, p2), max(p1, p2))))
			{
				std::vector<Vec2f> points;
				points.push_back(p1);
				points.push_back(p2);
				canvas->stroke(points.size(), points.data(), app.selected.link == l ? Vec4c(255, 255, 50, 255) : Vec4c(100, 100, 120, 255), line_width);
			}
			return true;
		});
		if (app.editor->dragging_slot)
		{
			auto e = ((cSlot*)app.editor->dragging_slot->user_data)->element;

			std::vector<Vec2f> points;
			points.push_back(e->global_pos + e->global_size * 0.5f);
			points.push_back(Vec2f(event_receiver->dispatcher->mouse_pos));
			canvas->stroke(points.size(), points.data(), Vec4c(255, 255, 50, 255), line_width);
		}
	}
};

struct cSceneObject : Component
{
	cElement* element;
	cEventReceiver* event_receiver;

	SelType t;
	void* p;
	bool moved;

	cSceneObject() :
		Component("cSceneObject")
	{
		moved = false;
	}

	void on_component_added(Component* c) override
	{
		if (c->name_hash == FLAME_CHASH("cElement"))
		{
			element = (cElement*)c;
			element->data_changed_listeners.add([](void* c, Component* e, uint hash, void*) {
				if (hash == FLAME_CHASH("pos"))
				{
					auto thiz = *(cSceneObject**)c;
					auto pos = ((cElement*)e)->pos_;
					switch (thiz->t)
					{
					case SelLibrary:
						((BP::Library*)thiz->p)->pos = pos;
						break;
					case SelSubGraph:
						((BP::SubGraph*)thiz->p)->pos = pos;
						break;
					case SelNode:
						((BP::Node*)thiz->p)->pos = pos;
						break;
					}
					thiz->moved = true;
				}
				return true;
			}, new_mail_p(this));
		}
		else if (c->name_hash == FLAME_CHASH("cEventReceiver"))
		{
			event_receiver = (cEventReceiver*)c;
			event_receiver->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
				auto thiz = *(cSceneObject**)c;
				if (is_mouse_down(action, key, true) && key == Mouse_Left)
					app.select(thiz->t, thiz->p);
				return true;
			}, new_mail_p(this));
			event_receiver->state_listeners.add([](void* c, EventReceiverState state) {
				auto thiz = *(cSceneObject**)c;
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
	auto tnp = ui::e_begin_docker_page(app.filepath.c_str());
	{
		auto c_layout = ui::c_layout(LayoutVertical);
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;
		c_layout->fence = 2;
	}
	tnp.second->add_component(this);
	tab_text = tnp.first->get_component(cText);

		ui::e_begin_layout()->get_component(cElement)->clip_children = true;
		ui::c_aligner(SizeFitParent, SizeFitParent);

			ui::e_begin_layout();
			ui::c_event_receiver();
			ui::c_aligner(SizeFitParent, SizeFitParent);
			auto c_bp_scene = new_u_object<cScene>();
			ui::current_entity()->add_component(c_bp_scene);

				e_base = ui::e_empty();
				c_bp_scene->base_element = ui::c_element();

				auto e_overlayer = ui::e_empty();
				ui::c_element();
				{
					auto c_event_receiver = ui::c_event_receiver();
					c_event_receiver->pass_checkers.add([](void*, cEventReceiver*, bool* pass) {
						*pass = true;
						return true;
					}, Mail<>());
					c_event_receiver->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
						auto c_bp_scene = *(cScene**)c;
						if (is_mouse_scroll(action, key))
						{
							auto s = clamp(c_bp_scene->base_element->scale_ + (pos.x() > 0.f ? 0.1f : -0.1f), 0.1f, 2.f);
							c_bp_scene->base_element->set_scale(s);
							c_bp_scene->scale_text->set_text((std::to_wstring(int(s * 100)) + L"%").c_str());
						}
						else if (is_mouse_move(action, key))
						{
							auto ed = c_bp_scene->event_receiver->dispatcher;
							if ((ed->key_states[Key_Ctrl] & KeyStateDown) && (ed->mouse_buttons[Mouse_Left] & KeyStateDown))
								c_bp_scene->base_element->set_pos(Vec2f(pos), true);
						}
						return true;
					}, new_mail_p(c_bp_scene));
				}
				ui::c_aligner(SizeFitParent, SizeFitParent);

				on_load();

			ui::e_end_layout();

			ui::next_entity = Entity::create();
			ui::e_button(L"Run", [](void* c) {
				app.running = !app.running;
				(*(Entity**)c)->get_component(cText)->set_text(app.running ? L"Pause" : L"Run");

				if (app.running)
					app.bp->time = 0.f;
			}, new_mail_p(ui::next_entity));

			ui::e_text(L"100%");
			ui::c_aligner(AlignxLeft, AlignyBottom);
			c_bp_scene->scale_text = ui::current_entity()->get_component(cText);

		ui::e_end_layout();

	ui::e_end_docker_page();
}

cEditor::~cEditor()
{
	app.editor = nullptr;
}

static Entity* selected_entity()
{
	switch (app.sel_type)
	{
	case SelLibrary:
		return (Entity*)app.selected.library->user_data;
	case SelSubGraph:
		return (Entity*)app.selected.subgraph->user_data;
	case SelNode:
		return (Entity*)app.selected.node->user_data;
	}
	return nullptr;
}

void cEditor::on_deselect()
{
	auto e = selected_entity();
	if (e)
		e->get_component(cElement)->set_frame_thickness(0.f);
}

void cEditor::on_select()
{
	auto e = selected_entity();
	if (e)
		e->get_component(cElement)->set_frame_thickness(4.f);
}

void cEditor::set_add_pos_center()
{
	add_pos = e_base->parent()->get_component(cElement)->size_ * 0.5f - e_base->get_component(cElement)->pos_;
}

void cEditor::on_changed()
{
	std::wstring title = L"editor";
	if (app.changed)
		title += L"*";
	tab_text->set_text(title.c_str());
}

void cEditor::on_load()
{
	e_base->remove_child(0, -1);

	for (auto i = 0; i < app.bp->library_count(); i++)
		on_add_library(app.bp->library(i));
	for (auto i = 0; i < app.bp->node_count(); i++)
		on_add_node(app.bp->node(i));
	for (auto i = 0; i < app.bp->subgraph_count(); i++)
		on_add_subgraph(app.bp->subgraph(i));

	dragging_slot = nullptr;

	on_changed();
}

void cEditor::on_add_library(BP::Library* l)
{
	l->pos = add_pos;

	ui::push_parent(e_base);

		auto e_library = ui::e_empty();
		l->user_data = e_library;
		{
			auto c_element = ui::c_element();
			c_element->pos_ = l->pos;
			c_element->color_ = Vec4c(255, 200, 190, 200);
			c_element->frame_color_ = Vec4c(252, 252, 50, 200);
			ui::c_event_receiver();
			ui::c_layout(LayoutVertical)->fence = 1;
			ui::c_moveable();
			auto c_library = new_u_object<cSceneObject>();
			c_library->t = SelLibrary;
			c_library->p = l;
			e_library->add_component(c_library);
		}
		ui::push_parent(e_library);
			ui::e_begin_layout(LayoutVertical, 4.f)->get_component(cElement)->inner_padding_ = Vec4f(8.f);
				ui::e_text(l->filename())->get_component(cElement)->inner_padding_ = Vec4f(4.f, 2.f, 4.f, 2.f);
				ui::e_text(L"library")->get_component(cText)->color = Vec4c(50, 50, 50, 255);
			ui::e_end_layout();
			ui::e_empty();
			ui::c_element();
			ui::c_event_receiver()->pass_checkers.add([](void*, cEventReceiver*, bool* pass) {
				*pass = true;
				return true;
			}, Mail<>());
			ui::c_aligner(SizeFitParent, SizeFitParent);
			ui::c_bring_to_front();
		ui::pop_parent();

	ui::pop_parent();
}

template <class T>
void create_edit(cEditor* editor, BP::Slot* input)
{
	auto& data = *(T*)input->data();

	ui::push_style_1u(ui::FontSize, 12);
	auto e_edit = ui::e_drag_edit(std::is_floating_point<T>::value);
	struct Capture
	{
		BP::Slot* input;
		cText* drag_text;
	}capture;
	capture.input = input;
	capture.drag_text = e_edit->child(1)->get_component(cText);
	e_edit->child(0)->get_component(cText)->data_changed_listeners.add([](void* c, Component* t, uint hash, void*) {
		auto& capture = *(Capture*)c;
		if (hash == FLAME_CHASH("text"))
		{
			auto text = ((cText*)t)->text();
			auto data = sto_s<T>(text);
			capture.input->set_data(&data);
			capture.drag_text->set_text(text);
			app.set_changed(true);
		}
		return true;
	}, new_mail(&capture));
	ui::pop_style(ui::FontSize);

	auto c_tracker = new_u_object<cDigitalDataTracker<T>>();
	c_tracker->data = &data;
	ui::current_parent()->add_component(c_tracker);
}

template <uint N, class T>
void create_vec_edit(cEditor* editor, BP::Slot* input)
{
	auto& data = *(Vec<N, T>*)input->data();

	struct Capture
	{
		BP::Slot* input;
		uint i;
		cText* drag_text;
	}capture;
	capture.input = input;
	ui::push_style_1u(ui::FontSize, 12);
	for (auto i = 0; i < N; i++)
	{
		ui::e_begin_layout(LayoutHorizontal, 4.f);
		auto e_edit = ui::e_drag_edit(std::is_floating_point<T>::value);
		capture.i = i;
		capture.drag_text = e_edit->child(1)->get_component(cText);
		e_edit->child(0)->get_component(cText)->data_changed_listeners.add([](void* c, Component* t, uint hash, void*) {
			auto& capture = *(Capture*)c;
			if (hash == FLAME_CHASH("text"))
			{
				auto text = ((cText*)t)->text();
				auto data = *(Vec<N, T>*)capture.input->data();
				data[capture.i] = sto_s<T>(text);
				capture.input->set_data(&data);
				capture.drag_text->set_text(text);
				app.set_changed(true);
			}
			return true;
		}, new_mail(&capture));
		ui::e_text(s2w(Vec<N, T>::coord_name(i)).c_str());
		ui::e_end_layout();
	}
	ui::pop_style(ui::FontSize);

	auto c_tracker = new_u_object<cDigitalVecDataTracker<N, T>>();
	c_tracker->data = &data;
	ui::current_parent()->add_component(c_tracker);
}

void cEditor::on_add_node(BP::Node* n)
{
	ui::push_parent(e_base);
		auto e_node = ui::e_empty();
		n->user_data = e_node;
		{
			auto c_element = ui::c_element();
			c_element->pos_ = n->pos;
			c_element->color_ = Vec4c(255, 255, 255, 200);
			c_element->frame_color_ = Vec4c(252, 252, 50, 200);
			ui::c_event_receiver();
			ui::c_layout(LayoutVertical)->fence = 1;
			ui::c_moveable();
			auto c_node = new_u_object<cSceneObject>();
			c_node->t = SelNode;
			c_node->p = n;
			e_node->add_component(c_node);
		}
		ui::push_parent(e_node);
			ui::e_begin_layout(LayoutVertical, 4.f)->get_component(cElement)->inner_padding_ = Vec4f(8.f);
				ui::push_style_1u(ui::FontSize, 21);
				ui::e_text(s2w(n->id()).c_str())->get_component(cElement)->inner_padding_ = Vec4f(4.f, 2.f, 4.f, 2.f);
				ui::c_event_receiver();
				ui::c_edit();
				ui::current_entity()->get_component(cText)->data_changed_listeners.add([](void* c, Component* t, uint hash, void*) {
					if (hash == FLAME_CHASH("text"))
						(*(BP::Node**)c)->set_id(w2s(((cText*)t)->text()).c_str());
					return true;
				}, new_mail_p(n));
				ui::pop_style(ui::FontSize);

				auto udt = n->udt();
				if (udt)
				{
					auto module_name = std::filesystem::path(udt->db()->module_name());
					module_name = module_name.lexically_relative(std::filesystem::canonical(app.fileppath));
					ui::e_text((module_name.wstring() + L"\n" + s2w(n->type_name())).c_str())->get_component(cText)->color = Vec4c(50, 50, 50, 255);
				}
				std::string udt_name = n->type_name();
				if (n->id() == "test_dst")
				{
					ui::e_button(L"Show", [](void* c) {
						//open_image_viewer(*(uint*)(*(BP::Node**)c)->find_output("idx")->data(), Vec2f(1495.f, 339.f));
					}, new_mail_p(n));
				}
				//else if (udt_name == "D#graphics::Shader")
				//{
				//	auto e_edit = create_standard_button(app.font_atlas_pixel, 0.9f, L"Edit Shader");
				//	e_content->add_child(e_edit);

				//	struct Capture
				//	{
				//		BP::Node* n;
				//	}capture;
				//	capture.n = n;
				//	e_edit->get_component(cEventReceiver)->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
				//		auto& capture = *(Capture*)c;

				//		if (is_mouse_clicked(action, key))
				//		{
				//			capture.e->locked = true;
				//			auto t = create_topmost(capture.e->entity, false, false, true, Vec4c(255, 255, 255, 235), true);
				//			{
				//				t->get_component(cElement)->inner_padding_ = Vec4f(4.f);

				//				auto c_layout = cLayout::create(LayoutVertical);
				//				c_layout->width_fit_children = false;
				//				c_layout->height_fit_children = false;
				//				t->add_component(c_layout);
				//			}

				//			auto e_buttons = Entity::create();
				//			t->add_child(e_buttons);
				//			{
				//				e_buttons->add_component(cElement::create());

				//				auto c_layout = cLayout::create(LayoutHorizontal);
				//				c_layout->item_padding = 4.f;
				//				e_buttons->add_component(c_layout);
				//			}

				//			auto e_back = create_standard_button(app.font_atlas_pixel, 1.f, L"Back");
				//			e_buttons->add_child(e_back);
				//			{
				//				e_back->get_component(cEventReceiver)->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
				//					if (is_mouse_clicked(action, key))
				//					{
				//						destroy_topmost(editor->entity, false);
				//						editor->locked = false;
				//					}
				//				}, new_mail_p(capture.e));
				//			}

				//			auto e_compile = create_standard_button(app.font_atlas_pixel, 1.f, L"Compile");
				//			e_buttons->add_child(e_compile);

				//			auto e_text_tip = Entity::create();
				//			e_buttons->add_child(e_text_tip);
				//			{
				//				e_text_tip->add_component(cElement::create());

				//				auto c_text = cText::create(app.font_atlas_pixel);
				//				c_text->set_text(L"(Do update first to get popper result)");
				//				e_text_tip->add_component(c_text);
				//			}

				//			auto filename = ssplit(*(std::wstring*)capture.n->find_input("filename")->data(), L':')[0];

				//			auto e_text_view = Entity::create();
				//			{
				//				auto c_element = cElement::create();
				//				c_element->clip_children = true;
				//				e_text_view->add_component(c_element);

				//				auto c_aligner = cAligner::create();
				//				c_aligner->width_policy_ = SizeFitParent;
				//				c_aligner->height_policy_ = SizeFitParent;
				//				e_text_view->add_component(c_aligner);

				//				auto c_layout = cLayout::create(LayoutVertical);
				//				c_layout->width_fit_children = false;
				//				c_layout->height_fit_children = false;
				//				e_text_view->add_component(c_layout);
				//			}

				//			auto e_text = Entity::create();
				//			e_text_view->add_child(e_text);
				//			{
				//				e_text->add_component(cElement::create());

				//				auto c_text = cText::create(app.font_atlas_pixel);
				//				auto file = get_file_string(capture.e->filepath + L"/" + filename);
				//				c_text->set_text(s2w(file).c_str());
				//				c_text->auto_width_ = false;
				//				e_text->add_component(c_text);

				//				e_text->add_component(cEventReceiver::create());

				//				e_text->add_component(cEdit::create());

				//				auto c_aligner = cAligner::create();
				//				c_aligner->width_policy_ = SizeFitParent;
				//				e_text->add_component(c_aligner);

				//				{
				//					struct _Capture
				//					{
				//						BP::Node* n;
				//						cText* t;
				//					}_capture;
				//					_capture.n = capture.n;
				//					_capture.t = c_text;
				//					e_compile->get_component(cEventReceiver)->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
				//						auto& capture = *(_Capture*)c;
				//						if (is_mouse_clicked(action, key))
				//						{
				//							auto i_filename = capture.n->find_input("filename");
				//							std::ofstream file(capture.e->filepath + L"/" + *(std::wstring*)i_filename->data());
				//							auto str = w2s(capture.t->text());
				//							str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
				//							file.write(str.c_str(), str.size());
				//							file.close();
				//							i_filename->set_frame(capture.n->scene()->frame);
				//						}
				//					}, new_mail(&_capture));
				//				}
				//			}

				//			auto e_scrollbar_container = wrap_standard_scrollbar(e_text_view, ScrollbarVertical, true, default_style.font_size);
				//			e_scrollbar_container->get_component(cAligner)->height_factor_ = 2.f / 3.f;
				//			t->add_child(e_scrollbar_container);
				//		}
				//	}, new_mail(&capture));
				//}
				ui::e_begin_layout(LayoutHorizontal, 16.f);
				ui::c_aligner(SizeGreedy, SizeFixed);

					ui::e_begin_layout(LayoutVertical);
					ui::c_aligner(SizeGreedy, SizeFixed);
						for (auto i = 0; i < n->input_count(); i++)
						{
							auto input = n->input(i);

							ui::e_begin_layout(LayoutVertical, 2.f);

							ui::e_begin_layout(LayoutHorizontal);
							ui::e_empty();
							{
								auto c_element = ui::c_element();
								auto r = ui::style_1u(ui::FontSize);
								c_element->size_ = r;
								c_element->roundness_ = r * 0.5f;
								c_element->roundness_lod = 2;
								c_element->color_ = Vec4c(200, 200, 200, 255);
							}
							ui::c_event_receiver();
							ui::push_style_1u(ui::FontSize, 9);
							auto c_text = ui::c_text();
							c_text->auto_width_ = false;
							c_text->auto_height_ = false;
							if (app.bp->find_output_export(input) != -1)
								c_text->set_text(L"  p");
							ui::pop_style(ui::FontSize);
							auto c_slot = new_u_object<cSlot>();
							c_slot->s = input;
							c_slot->text = c_text;
							ui::current_entity()->add_component(c_slot);
							input->user_data = c_slot;
							ui::e_begin_popup_menu(false);
							ui::e_menu_item(L"Add To Exports", [](void* c) {
								auto s = *(BP::Slot**)c;
								app.bp->add_input_export(s);
								app.set_changed(true);
								((cSlot*)s->user_data)->text->set_text(L"  p");
							}, new_mail_p(input));
							ui::e_menu_item(L"Remove From Exports", [](void* c) {
								auto s = *(BP::Slot**)c;
								app.bp->remove_input_export(s);
								app.set_changed(true);
								((cSlot*)s->user_data)->text->set_text(L"");
							}, new_mail_p(input));
							ui::e_end_popup_menu();

							ui::e_text(s2w(input->name()).c_str());
							ui::e_end_layout();

							auto e_data = ui::e_begin_layout(LayoutVertical, 2.f);
							e_data->get_component(cElement)->inner_padding_ = Vec4f(ui::style_1u(ui::FontSize), 0.f, 0.f, 0.f);
							extra_global_db_count = app.bp->db_count();
							extra_global_dbs = app.bp->dbs();
							auto type = input->type();
							auto base_hash = type->base_hash();
							ui::push_style_1u(ui::FontSize, 12);
							switch (type->tag())
							{
							case TypeEnumSingle:
							{
								auto info = find_enum(base_hash);
								ui::create_enum_combobox(info, 120.f);

								struct Capture
								{
									BP::Slot* input;
									EnumInfo* e;
								}capture;
								capture.input = input;
								capture.e = info;
								e_data->child(0)->get_component(cCombobox)->data_changed_listeners.add([](void* c, Component* cb, uint hash, void*) {
									auto& capture = *(Capture*)c;
									if (hash == FLAME_CHASH("index"))
									{
										auto v = capture.e->item(((cCombobox*)cb)->idx)->value();
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
								ui::create_enum_checkboxs(info);
								for (auto k = 0; k < info->item_count(); k++)
								{
									auto item = info->item(k);

									struct Capture
									{
										BP::Slot* input;
										int v;
									}capture;
									capture.input = input;
									capture.v = item->value();
									e_data->child(k)->child(0)->get_component(cCheckbox)->data_changed_listeners.add([](void* c, Component* cb, uint hash, void*) {
										auto& capture = *(Capture*)c;
										if (hash == FLAME_CHASH("checked"))
										{
											auto v = *(int*)capture.input->data();
											if (((cCheckbox*)cb)->checked)
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
									auto e_checkbox = ui::e_checkbox(L"");

									struct Capture
									{
										BP::Slot* input;
									}capture;
									capture.input = input;
									e_checkbox->get_component(cCheckbox)->data_changed_listeners.add([](void* c, Component* cb, uint hash, void*) {
										auto& capture = *(Capture*)c;
										if (hash == FLAME_CHASH("checked"))
										{
											auto v = (((cCheckbox*)cb)->checked) ? 1 : 0;
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
								case FLAME_CHASH("Vec(2+int)"):
									create_vec_edit<2, int>(this, input);
									break;
								case FLAME_CHASH("Vec(3+int)"):
									create_vec_edit<3, int>(this, input);
									break;
								case FLAME_CHASH("Vec(4+int)"):
									create_vec_edit<4, int>(this, input);
									break;
								case FLAME_CHASH("uint"):
									create_edit<uint>(this, input);
									break;
								case FLAME_CHASH("Vec(2+uint)"):
									create_vec_edit<2, uint>(this, input);
									break;
								case FLAME_CHASH("Vec(3+uint)"):
									create_vec_edit<3, uint>(this, input);
									break;
								case FLAME_CHASH("Vec(4+uint)"):
									create_vec_edit<4, uint>(this, input);
									break;
								case FLAME_CHASH("float"):
									create_edit<float>(this, input);
									break;
								case FLAME_CHASH("Vec(2+float)"):
									create_vec_edit<2, float>(this, input);
									break;
								case FLAME_CHASH("Vec(3+float)"):
									create_vec_edit<3, float>(this, input);
									break;
								case FLAME_CHASH("Vec(4+float)"):
									create_vec_edit<4, float>(this, input);
									break;
								case FLAME_CHASH("uchar"):
									create_edit<uchar>(this, input);
									break;
								case FLAME_CHASH("Vec(2+uchar)"):
									create_vec_edit<2, uchar>(this, input);
									break;
								case FLAME_CHASH("Vec(3+uchar)"):
									create_vec_edit<3, uchar>(this, input);
									break;
								case FLAME_CHASH("Vec(4+uchar)"):
									create_vec_edit<4, uchar>(this, input);
									break;
								case FLAME_CHASH("StringA"):
								{
									auto e_edit = ui::e_edit(50.f);
									e_edit->get_component(cText)->data_changed_listeners.add([](void* c, Component* t, uint hash, void*) {
										auto s = *(BP::Slot**)c;
										if (hash == FLAME_CHASH("text"))
										{
											s->set_data(&StringA(w2s(((cText*)t)->text())));
											app.set_changed(true);
										}
										return true;
									}, new_mail_p(input));

									auto c_tracker = new_u_object<cStringADataTracker>();
									c_tracker->data = input->data();
									e_data->add_component(c_tracker);
								}
								break;
								case FLAME_CHASH("StringW"):
								{
									auto e_edit = ui::e_edit(50.f);
									e_edit->get_component(cText)->data_changed_listeners.add([](void* c, Component* t, uint hash, void*) {
										auto s = *(BP::Slot**)c;
										if (hash == FLAME_CHASH("text"))
										{
											s->set_data(&StringW(((cText*)t)->text()));
											app.set_changed(true);
										}
										return true;
									}, new_mail_p(input));

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
							ui::pop_style(ui::FontSize);
							ui::e_end_layout();

							ui::e_end_layout();

							c_slot->tracker = e_data->get_component(cDataTracker);
						}
					ui::e_end_layout();

					ui::e_begin_layout(LayoutVertical);
					ui::c_aligner(SizeGreedy, SizeFixed);
						for (auto i = 0; i < n->output_count(); i++)
						{
							auto output = n->output(i);

							ui::e_begin_layout(LayoutHorizontal);
							ui::c_aligner(AlignxRight, AlignyFree);
							ui::e_text(s2w(output->name()).c_str());

							ui::e_empty();
							{
								auto c_element = ui::c_element();
								auto r = ui::style_1u(ui::FontSize);
								c_element->size_ = r;
								c_element->roundness_ = r * 0.5f;
								c_element->roundness_lod = 2;
								c_element->color_ = Vec4c(200, 200, 200, 255);
							}
							ui::c_event_receiver();
							ui::push_style_1u(ui::FontSize, 9);
							auto c_text = ui::c_text();
							c_text->auto_width_ = false;
							c_text->auto_height_ = false;
							if (app.bp->find_output_export(output) != -1)
								c_text->set_text(L"  p");
							ui::pop_style(ui::FontSize);
							auto c_slot = new_u_object<cSlot>();
							c_slot->s = output;
							c_slot->text = c_text;
							ui::current_entity()->add_component(c_slot);
							output->user_data = c_slot;
							ui::e_begin_popup_menu(false);
							ui::e_menu_item(L"Add To Exports", [](void* c) {
								auto s = *(BP::Slot**)c;
								app.bp->add_output_export(s);
								app.set_changed(true);
								((cSlot*)s->user_data)->text->set_text(L"  p");
							}, new_mail_p(output));
							ui::e_menu_item(L"Remove From Exports", [](void* c) {
								auto s = *(BP::Slot**)c;
								app.bp->remove_output_export(s);
								app.set_changed(true);
								((cSlot*)s->user_data)->text->set_text(L"");
							}, new_mail_p(output));
							ui::e_end_popup_menu();
							ui::e_end_layout();
						}
					ui::e_end_layout();
				ui::e_end_layout();
			ui::e_end_layout();
			ui::e_empty();
			ui::c_element();
			ui::c_event_receiver()->pass_checkers.add([](void*, cEventReceiver*, bool* pass) {
				*pass = true;
				return true;
			}, Mail<>());
			ui::c_aligner(SizeFitParent, SizeFitParent);
			ui::c_bring_to_front();
		ui::pop_parent();
	ui::pop_parent();
}

void cEditor::on_add_subgraph(BP::SubGraph* s)
{
	s->pos = add_pos;

	ui::push_parent(e_base);
		auto e_subgraph = ui::e_empty();
		s->user_data = e_subgraph;
		{
			auto c_element = ui::c_element();
			c_element->pos_ = s->pos;
			c_element->color_ = Vec4c(190, 255, 200, 200);
			c_element->frame_color_ = Vec4c(252, 252, 50, 200);
			ui::c_event_receiver();
			ui::c_layout(LayoutVertical)->fence = 1;
			ui::c_moveable();
			auto c_subgraph = new_u_object<cSceneObject>();
			c_subgraph->t = SelSubGraph;
			c_subgraph->p = s;
			e_subgraph->add_component(c_subgraph);
		}
		ui::push_parent(e_subgraph);
			ui::e_begin_layout(LayoutVertical, 4.f)->get_component(cElement)->inner_padding_ = Vec4f(8.f);
				ui::e_text(s2w(s->id()).c_str())->get_component(cElement)->inner_padding_ = Vec4f(4.f, 2.f, 4.f, 2.f);
				ui::c_event_receiver();
				ui::c_edit();
				ui::current_entity()->get_component(cText)->data_changed_listeners.add([](void* c, Component* t, uint hash, void*) {
					if (hash == FLAME_CHASH("text"))
						(*(BP::SubGraph**)c)->set_id(w2s(((cText*)t)->text()).c_str());
					return true;
				}, new_mail_p(s));
				ui::e_text(s->bp()->filename())->get_component(cText)->color = Vec4c(50, 50, 50, 255);

				ui::e_begin_layout(LayoutHorizontal, 16.f);
				ui::c_aligner(SizeGreedy, SizeFixed);
					auto bp = s->bp();
					ui::e_begin_layout(LayoutVertical);
					ui::c_aligner(SizeGreedy, SizeFixed);
						for (auto i = 0; i < bp->input_export_count(); i++)
						{
							auto s = bp->input_export(i);

							ui::e_begin_layout(LayoutHorizontal);
							ui::e_empty();
							{
								auto c_element = ui::c_element();
								auto r = ui::style_1u(ui::FontSize);
								c_element->size_ = r;
								c_element->roundness_ = r * 0.5f;
								c_element->roundness_lod = 2;
								c_element->color_ = Vec4c(200, 200, 200, 255);
								ui::c_event_receiver();
								auto c_slot = new_u_object<cSlot>();
								c_slot->s = s;
								ui::current_entity()->add_component(c_slot);
								s->user_data = c_slot;
							}
							ui::e_text(s2w(s->get_address().str()).c_str());
							ui::e_end_layout();
						}
					ui::e_end_layout();
					ui::e_begin_layout(LayoutVertical);
					ui::c_aligner(SizeGreedy, SizeFixed);
						for (auto i = 0; i < bp->output_export_count(); i++)
						{
							auto s = bp->output_export(i);

							ui::e_begin_layout(LayoutHorizontal);
							ui::c_aligner(AlignxRight, AlignyFree);
							ui::e_text(s2w(s->get_address().str()).c_str());
							ui::e_empty();
							{
								auto c_element = ui::c_element();
								auto r = ui::style_1u(ui::FontSize);
								c_element->size_ = r;
								c_element->roundness_ = r * 0.5f;
								c_element->roundness_lod = 2;
								c_element->color_ = Vec4c(200, 200, 200, 255);
								ui::c_event_receiver();
								auto c_slot = new_u_object<cSlot>();
								c_slot->s = s;
								ui::current_entity()->add_component(c_slot);
								s->user_data = c_slot;
							}
							ui::e_end_layout();
						}
					ui::e_end_layout();
				ui::e_end_layout();
			ui::e_end_layout();

			ui::e_empty();
			ui::c_element();
			ui::c_event_receiver()->pass_checkers.add([](void*, cEventReceiver*, bool* pass) {
				*pass = true;
				return true;
			}, Mail<>());
			ui::c_aligner(SizeFitParent, SizeFitParent);
			ui::c_bring_to_front();
		ui::pop_parent();
	ui::pop_parent();
}

void cEditor::on_remove_library(BP::Library* l)
{
	auto m_db = l->db();
	for (auto i = 0; i < app.bp->node_count(); i++)
	{
		auto n = app.bp->node(i);
		auto udt = n->udt();
		if (udt && udt->db() == m_db)
		{
			auto e = (Entity*)n->user_data;
			e->parent()->remove_child(e);
		}
	}
	auto e = (Entity*)l->user_data;
	e->parent()->remove_child(e);
}

void cEditor::on_remove_node(BP::Node* n)
{
	auto e = (Entity*)n->user_data;
	e->parent()->remove_child(e);
}

void cEditor::on_remove_subgraph(BP::SubGraph* s)
{
	auto e = (Entity*)s->user_data;
	e->parent()->remove_child(e);
}

void cEditor::on_data_changed(BP::Slot* s)
{
	((cSlot*)s->user_data)->tracker->update_view();
}

