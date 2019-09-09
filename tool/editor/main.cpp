#include <flame/foundation/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/network/network.h>
#include <flame/graphics/device.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/image.h>
#include <flame/graphics/font.h>
#include <flame/graphics/canvas.h>
#include <flame/universe/default_style.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/edit.h>
#include <flame/universe/components/image.h>
#include <flame/universe/components/event_dispatcher.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/checkbox.h>
#include <flame/universe/components/toggle.h>
#include <flame/universe/components/combobox.h>
#include <flame/universe/components/tree.h>
#include <flame/universe/components/scrollbar.h>
#include <flame/universe/components/window.h>

using namespace flame;
using namespace graphics;

bool bp_running;

union
{
	BP::Node* n;
	BP::Slot* l;
}bp_selected;
BP::Slot* dragging_slot;

struct cBP : Component
{
	cElement* element;
	cEventReceiver* event_receiver;
	cElement* base_element;

	BP* bp;

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
				bp_selected.n = nullptr;
				auto bp = thiz->bp;

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
								bp_selected.l = input;
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
						
					std::vector<Vec2f> points;
					path_bezier(points, p1, p1 + Vec2f(50.f, 0.f), p2 - Vec2f(50.f, 0.f), p2);
					element->canvas->stroke(points, bp_selected.l == input ? Vec4c(255, 255, 50, 255) : Vec4c(100, 100, 120, 255), 3.f * element->global_scale);
				}
			}
		}
		if (dragging_slot)
		{
			auto e = ((cElement*)dragging_slot->user_data);
			auto p1 = Vec2f(e->global_x + e->global_width * 0.5f, e->global_y + e->global_height * 0.5f);
			auto p2 = Vec2f(event_receiver->event_dispatcher->mouse_pos);

			std::vector<Vec2f> points;
			path_bezier(points, p1, p1 + Vec2f(dragging_slot->type == BP::Slot::Output ? 50.f : -50.f, 0.f), p2, p2);
			element->canvas->stroke(points, Vec4c(255, 255, 50, 255), 3.f * element->global_scale);
		}
	}
};

struct cBPNode : Component
{
	cElement* element;
	cEventReceiver* event_receiver;
	cWindow* window;

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
				bp_selected.n = thiz->n;
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
		if (n == bp_selected.n)
			element->background_frame_thickness = 4.f;
		else
			element->background_frame_thickness = 0.f;
	}
};

struct cBPSlot : Component
{
	cEventReceiver* event_receiver;

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
				dragging_slot = thiz->s;
				if (thiz->s->type == BP::Slot::Input)
					thiz->s->link_to(nullptr);
			}
			else if (action == DragEnd)
				dragging_slot = nullptr;
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

struct App
{
	Window* w;
	Device* d;
	Semaphore* render_finished;
	SwapchainResizable* scr;
	Fence* fence;
	std::vector<Commandbuffer*> cbs;

	FontAtlas* font_atlas_pixel;
	FontAtlas* font_atlas_sdf;
	Canvas* canvas;
	int rt_frame;

	Entity* root;
	cElement* c_element_root;
	cText* c_text_fps;

	std::wstring filename;
	BP* bp;

	std::vector<TypeinfoDatabase*> dbs;

	Image* bp_rt;
	Imageview* bp_rt_v;
	std::vector<void*> bp_cbs;

	void* ev_1;
	void* ev_2;
	void* ev_3;

	void run()
	{
		auto sc = scr->sc();
		auto sc_frame = scr->sc_frame();

		if (sc_frame > rt_frame)
		{
			canvas->set_render_target(TargetImages, sc ? &sc->images() : nullptr);
			rt_frame = sc_frame;
		}

		if (sc)
		{
			sc->acquire_image();
			fence->wait();

			c_element_root->width = w->size.x();
			c_element_root->height = w->size.y();
			c_text_fps->set_text(std::to_wstring(looper().fps));
			root->update();

			if (bp_running)
				bp->update();

			std::vector<Commandbuffer*> _cbs;

			{
				auto img_idx = sc->image_index();
				auto cb = cbs[img_idx];
				canvas->record(cb, img_idx);
				_cbs.push_back(cb);
			}

			if (bp_running)
				_cbs.push_back((Commandbuffer*)bp_cbs[0]);

			d->gq->submit(_cbs, sc->image_avalible(), render_finished, fence);
			d->gq->present(sc, render_finished);
		}

		if (wait_event(ev_1, 0))
		{
			fence->wait();

			set_event(ev_2);
			wait_event(ev_3, -1);
		}
	}

	void set_data(const std::string& address, const std::string& value)
	{
		auto i = bp->find_input(address.c_str());
		if (i)
		{
			set_event(ev_1);
			wait_event(ev_2, -1);

			auto v = i->variable_info;
			auto type = v->type();
			auto value_before = serialize_value(dbs, type->tag(), type->hash(), i->data(), 2);
			auto data = new char[v->size()];
			unserialize_value(dbs, type->tag(), type->hash(), value, data);
			i->set_data(data);
			delete data;
			auto value_after = serialize_value(dbs, type->tag(), type->hash(), i->data(), 2);
			printf("set value: %s, %s -> %s\n", address.c_str(), *value_before.p, *value_after.p);
			delete_mail(value_before);
			delete_mail(value_after);

			set_event(ev_3);
		}
		else
			printf("input not found\n");
	}

	void generate_graph_and_layout()
	{
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
	}

}app;

void create_directory_tree_node(const std::filesystem::path& path, Entity* parent)
{
	auto e_tree_node = create_standard_tree_node(app.font_atlas_pixel, Icon_FOLDER_O + std::wstring(L" ") + path.filename().wstring());
	parent->add_child(e_tree_node);
	auto e_sub_tree = e_tree_node->child(1);
	for (std::filesystem::directory_iterator end, it(path); it != end; it++)
	{
		if (std::filesystem::is_directory(it->status()))
		{
			if (it->path().filename().wstring() != L"build")
				create_directory_tree_node(it->path(), e_sub_tree);
		}
		else
		{
			auto e_tree_leaf = create_standard_tree_leaf(app.font_atlas_pixel, Icon_FILE_O + std::wstring(L" ") + it->path().filename().wstring());
			e_sub_tree->add_child(e_tree_leaf);
		}
	}
}

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

int main(int argc, char **args)
{
	if (argc != 2)
	{
		printf("argc is not 2, exit\n");
		return 0;
	}

	app.filename = s2w(args[1]);
	app.bp = BP::create_from_file(app.filename, true);
	if (!app.bp)
	{
		printf("bp not found, exit\n");
		return 0;
	}
	bp_running = false;
	bp_selected.n = nullptr;
	dragging_slot = nullptr;

	for (auto i = 0; i < app.bp->dependency_count(); i++)
		app.dbs.push_back(app.bp->dependency_typeinfodatabase(i));
	app.dbs.push_back(app.bp->typeinfodatabase);

	app.ev_1 = create_event(false);
	app.ev_2 = create_event(false);
	app.ev_3 = create_event(false);

	std::thread([&]() {
		app.w = Window::create("Editor", Vec2u(1280, 720), WindowFrame | WindowResizable);
		app.d = Device::create(true);
		app.render_finished = Semaphore::create(app.d);
		app.scr = SwapchainResizable::create(app.d, app.w);
		app.fence = Fence::create(app.d);
		auto sc = app.scr->sc();
		app.canvas = Canvas::create(app.d, TargetImages, &sc->images());
		app.cbs.resize(sc->images().size());
		for (auto i = 0; i < app.cbs.size(); i++)
			app.cbs[i] = Commandbuffer::create(app.d->gcp);

		app.bp_rt = Image::create(app.d, Format_R8G8B8A8_UNORM, Vec2u(400, 300), 1, 1, SampleCount_1, ImageUsage$(ImageUsageTransferDst | ImageUsageAttachment | ImageUsageSampled));
		app.bp_rt->init(Vec4c(0, 0, 0, 255));
		app.bp_rt_v = Imageview::create(app.bp_rt);
		app.bp_cbs.resize(1);
		app.bp_cbs[0] = Commandbuffer::create(app.d->gcp);

		app.bp->graphics_device = app.d;
		app.bp->find_input("rt_dst.type")->set_data_i(TargetImageview);
		app.bp->find_input("rt_dst.v")->set_data_p(app.bp_rt_v);
		app.bp->find_input("make_cmd.cmdbufs")->set_data_p(&app.bp_cbs);

		auto font14 = Font::create(L"c:/windows/fonts/msyh.ttc", 14);
		auto font_awesome14 = Font::create(L"../asset/font_awesome.ttf", 14);
		auto font32 = Font::create(L"c:/windows/fonts/msyh.ttc", 32);
		auto font_awesome32 = Font::create(L"../asset/font_awesome.ttf", 32);
		app.font_atlas_pixel = FontAtlas::create(app.d, FontDrawPixel, { font14, font_awesome14 });
		app.font_atlas_sdf = FontAtlas::create(app.d, FontDrawSdf, { font32, font_awesome32 });
		app.font_atlas_pixel->index = 1;
		app.font_atlas_sdf->index = 2;
		app.canvas->set_image(app.font_atlas_pixel->index, Imageview::create(app.font_atlas_pixel->image(), Imageview2D, 0, 1, 0, 1, SwizzleOne, SwizzleOne, SwizzleOne, SwizzleR));
		app.canvas->set_image(app.font_atlas_sdf->index, Imageview::create(app.font_atlas_sdf->image()));
		app.canvas->set_image(3, app.bp_rt_v);
		app.canvas->set_clear_color(Vec4c(100, 100, 100, 255));
		default_style.set_to_light();

		app.root = Entity::create();
		{
			app.c_element_root = cElement::create(app.canvas);
			app.root->add_component(app.c_element_root);

			app.root->add_component(cEventDispatcher::create(app.w));

			app.root->add_component(cLayout::create());
		}

		auto e_fps = Entity::create();
		app.root->add_child(e_fps);
		{
			e_fps->add_component(cElement::create());

			auto c_text = cText::create(app.font_atlas_pixel);
			app.c_text_fps = c_text;
			e_fps->add_component(c_text);

			auto c_aligner = cAligner::create();
			c_aligner->x_align = AlignxLeft;
			c_aligner->y_align = AlignyBottom;
			e_fps->add_component(c_aligner);
		}

		{
			auto e_tab = get_docker_tab_model();
			((cText*)e_tab->find_component(cH("Text")))->font_atlas = app.font_atlas_pixel;
			((cDockerTab*)e_tab->find_component(cH("DockerTab")))->root = app.root;
		}

		{
			auto e_container = get_docker_container_model()->copy();
			app.root->add_child(e_container);
			{
				auto c_element = (cElement*)e_container->find_component(cH("Element"));
				c_element->x = 20.f;
				c_element->y = 20.f;
				c_element->width = 300.f;
				c_element->height = 600.f;
			}

			auto e_docker = get_docker_model()->copy();
			e_container->add_child(e_docker);
			auto e_tabbar = e_docker->child(0);
			auto e_pages = e_docker->child(1);

			auto e_tab = get_docker_tab_model()->copy();
			((cText*)e_tab->find_component(cH("Text")))->set_text(L"Resource Explorer");
			e_tabbar->add_child(e_tab);

			auto e_page = get_docker_page_model()->copy();
			{
				auto c_layout = cLayout::create();
				c_layout->type = LayoutVertical;
				c_layout->item_padding = 4.f;
				c_layout->width_fit_children = false;
				c_layout->height_fit_children = false;
				e_page->add_component(c_layout);
			}
			e_pages->add_child(e_page);

			auto e_tree = Entity::create();
			{
				auto c_element = cElement::create();
				c_element->inner_padding = Vec4f(4.f);
				e_tree->add_component(c_element);

				auto c_aligner = cAligner::create();
				c_aligner->width_policy = SizeFitParent;
				c_aligner->height_policy = SizeFitParent;
				e_tree->add_component(c_aligner);

				auto c_layout = cLayout::create();
				c_layout->type = LayoutVertical;
				c_layout->item_padding = 4.f;
				c_layout->width_fit_children = false;
				c_layout->height_fit_children = false;
				e_tree->add_component(c_layout);

				e_tree->add_component(cTree::create());
			}

			create_directory_tree_node(L"../renderpath", e_tree);

			e_page->add_child(wrap_standard_scrollbar(e_tree, ScrollbarVertical, true));
		}

		{
			auto e_container = get_docker_container_model()->copy();
			app.root->add_child(e_container);
			{
				auto c_element = (cElement*)e_container->find_component(cH("Element"));
				c_element->x = 350.f;
				c_element->y = 20.f;
				c_element->width = 800.f;
				c_element->height = 400.f;
			}

			auto e_docker = get_docker_model()->copy();
			e_container->add_child(e_docker);
			auto e_tabbar = e_docker->child(0);
			auto e_pages = e_docker->child(1);

			auto e_tab = get_docker_tab_model()->copy();
			((cText*)e_tab->find_component(cH("Text")))->set_text(L"Blueprint Editor");
			e_tabbar->add_child(e_tab);

			auto e_page = get_docker_page_model()->copy();
			e_pages->add_child(e_page);
			{
				((cElement*)e_page->find_component(cH("Element")))->inner_padding = Vec4f(8.f);

				auto c_layout = cLayout::create();
				c_layout->type = LayoutVertical;
				c_layout->item_padding = 4.f;
				c_layout->width_fit_children = false;
				c_layout->height_fit_children = false;
				e_page->add_component(c_layout);
			}

			auto e_btn_run = Entity::create();
			e_page->add_child(e_btn_run);
			{
				auto c_element = cElement::create();
				c_element->inner_padding = Vec4f(4.f, 2.f, 4.f, 2.f);
				e_btn_run->add_component(c_element);

				auto c_text = cText::create(app.font_atlas_pixel);
				c_text->set_text(L"Run");
				e_btn_run->add_component(c_text);

				auto c_event_receiver = cEventReceiver::create();
				c_event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
					if (is_mouse_clicked(action, key))
					{
						auto c_text = *(cText**)c;
						bp_running = !bp_running;
						c_text->set_text(bp_running ? L"Pause" : L"Run");
					}
				}, new_mail_p(c_text));
				e_btn_run->add_component(c_event_receiver);

				e_btn_run->add_component(cStyleBackgroundColor::create(default_style.button_color_normal, default_style.button_color_hovering, default_style.button_color_active));
			}

			auto e_scene = Entity::create();
			e_page->add_child(e_scene);
			{
				e_scene->add_component(cElement::create());

				e_scene->add_component(cEventReceiver::create());

				auto c_aligner = cAligner::create();
				c_aligner->width_policy = SizeFitParent;
				c_aligner->height_policy = SizeFitParent;
				e_scene->add_component(c_aligner);

				auto c_bp = new_component<cBP>();
				c_bp->bp = app.bp;
				e_scene->add_component(c_bp);
			}

			auto e_base = Entity::create();
			e_scene->add_child(e_base);
			{
				e_base->add_component(cElement::create());
			}

			for (auto i = 0; i < app.bp->node_count(); i++)
			{
				auto n = app.bp->node(i);

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
									auto info = find_enum(app.dbs, type->hash());
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

									auto info = find_enum(app.dbs, type->hash());
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
									c_slot->s = output;
									e_slot->add_component(c_slot);
								}
							}
						}
					}
				}
			}
		}

		{
			auto e_container = get_docker_container_model()->copy();
			app.root->add_child(e_container);
			{
				auto c_element = (cElement*)e_container->find_component(cH("Element"));
				c_element->x = 350.f;
				c_element->y = 420.f;
				c_element->width = 400.f;
				c_element->height = 340.f;
			}

			auto e_docker = get_docker_model()->copy();
			e_container->add_child(e_docker);
			auto e_tabbar = e_docker->child(0);
			auto e_pages = e_docker->child(1);

			auto e_tab = get_docker_tab_model()->copy();
			((cText*)e_tab->find_component(cH("Text")))->set_text(L"Render Target");
			e_tabbar->add_child(e_tab);

			auto e_page = get_docker_page_model()->copy();
			e_pages->add_child(e_page);
			{
				((cElement*)e_page->find_component(cH("Element")))->inner_padding = Vec4f(8.f);

				auto c_layout = cLayout::create();
				c_layout->width_fit_children = false;
				c_layout->height_fit_children = false;
				e_page->add_component(c_layout);
			}

			auto e_image = Entity::create();
			e_page->add_child(e_image);
			{
				auto c_element = cElement::create();
				c_element->width = 400;
				c_element->height = 300;
				e_image->add_component(c_element);

				auto c_image = cImage::create();
				c_image->id = 3;
				e_image->add_component(c_image);
			}
		}

		{
			auto e_container = get_docker_container_model()->copy();
			app.root->add_child(e_container);
			{
				auto c_element = (cElement*)e_container->find_component(cH("Element"));
				c_element->x = 850.f;
				c_element->y = 420.f;
				c_element->width = 400.f;
				c_element->height = 300.f;
			}

			auto e_docker = get_docker_model()->copy();
			e_container->add_child(e_docker);
			auto e_tabbar = e_docker->child(0);
			auto e_pages = e_docker->child(1);

			auto e_tab = get_docker_tab_model()->copy();
			((cText*)e_tab->find_component(cH("Text")))->set_text(L"Console");
			e_tabbar->add_child(e_tab);

			auto e_page = get_docker_page_model()->copy();
			e_pages->add_child(e_page);
			{
				((cElement*)e_page->find_component(cH("Element")))->inner_padding = Vec4f(8.f);

				auto c_layout = cLayout::create();
				c_layout->width_fit_children = false;
				c_layout->height_fit_children = false;
				e_page->add_component(c_layout);
			}

			auto e_log = Entity::create();
			e_page->add_child(e_log);
		}

		set_event(app.ev_1);
		wait_event(app.ev_2, -1);
		looper().loop([](void* c) {
			auto app = (*(App**)c);
			app->run();
		}, new_mail_p(&app));
	}).detach();

	wait_event(app.ev_1, -1);

	set_event(app.ev_2);

	printf("\"%s\":\n", w2s(app.filename).c_str());

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
				"  show graph - use Graphviz to show graph\n"
				"  add node [type_name] [id] - add a node (id of '-' means don't care)\n"
				"  add link [out_adress] [in_adress] - add a link\n"
				"  remove node [id] - remove a node\n"
				"  remove link [in_adress] - remove a link\n"
				"  set [in_adress] [value] - set value for input\n"
				"  update - update this blueprint\n"
				"  save - save this blueprint\n"
				"  set-layout - set nodes' positions using 'bp.png' and 'bp.graph.txt', need do show graph first\n"
			);
		}
		else if (s_command_line == "show")
		{
			scanf("%s", command_line);
			auto s_what = std::string(command_line);

			if (s_what == "udts")
			{
				std::vector<UdtInfo*> available_udts;
				{
					for (auto db : app.dbs)
					{
						auto udts = db->get_udts();
						for (auto i = 0; i < udts.p->size(); i++)
							available_udts.push_back((*udts.p)[i]);
						delete_mail(udts);
					}
					std::sort(available_udts.begin(), available_udts.end(), [](UdtInfo* a, UdtInfo* b) {
						return std::string(a->name()) < std::string(b->name());
					});
				}
				for (auto u : available_udts)
					printf("%s\n", u->name().c_str());
			}
			else if (s_what == "udt")
			{
				scanf("%s", command_line);
				auto s_name = std::string(command_line);

				auto udt = find_udt(app.dbs, H(s_name.c_str()));
				if (udt)
				{
					printf("%s:\n", udt->name().c_str());
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
					printf("[In]\n");
					for (auto &i : inputs)
						printf(" name:%s attribute:%s tag:%s type:%s\n", i->name().c_str(), i->decoration(), get_name(i->type()->tag()), i->type()->name().c_str());
					printf("[Out]\n");
					for (auto &o : outputs)
						printf(" name:%s attribute:%s tag:%s type:%s\n", o->name().c_str(), o->decoration(), get_name(o->type()->tag()), o->type()->name().c_str());
				}
				else
					printf("udt not found\n");
			}
			else if (s_what == "nodes")
			{
				for (auto i = 0; i < app.bp->node_count(); i++)
				{
					auto n = app.bp->node(i);
					printf("id:%s type:%s\n", n->id().c_str(), n->udt->name());
				}
			}
			else if (s_what == "node")
			{
				scanf("%s", command_line);
				auto s_id = std::string(command_line);

				auto n = app.bp->find_node(s_id.c_str());
				if (n)
				{
					printf("[In]\n");
					for (auto i = 0; i < n->input_count(); i++)
					{
						auto input = n->input(i);
						auto v = input->variable_info;
						auto type = v->type();
						printf(" %s\n", v->name().c_str());
						Mail<std::string> link_address;
						if (input->link())
							link_address = input->link()->get_address();
						printf("   [%s]->\n", link_address.p ? link_address.p->c_str() : "");
						delete_mail(link_address);
						auto str = serialize_value(app.dbs, type->tag(), type->hash(), input->data(), 2);
						printf("   %s\n", str.p->empty() ? "-" : str.p->c_str());
						delete_mail(str);
					}
					printf("[Out]\n");
					for (auto i = 0; i < n->output_count(); i++)
					{
						auto output = n->output(i);
						auto v = output->variable_info;
						auto type = v->type();
						printf(" %s\n", output->variable_info->name().c_str());
						auto str = serialize_value(app.dbs, type->tag(), type->hash(), output->data(), 2);
						printf("   %s\n", str.p->empty() ? "-" : str.p->c_str());
						delete_mail(str);
					}
				}
				else
					printf("node not found\n");
			}
			else if (s_what == "graph")
			{
				if (!std::filesystem::exists(L"bp.png") || std::filesystem::last_write_time(L"bp.png") < std::filesystem::last_write_time(app.filename))
					app.generate_graph_and_layout();
				if (std::filesystem::exists(L"bp.png"))
				{
					exec(L"bp.png", L"", false);
					printf("ok\n");
				}
				else
					printf("bp.png not found, perhaps Graphviz is not available\n");
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
				auto s_tn = std::string(command_line);

				scanf("%s", command_line);
				auto s_id = std::string(command_line);

				auto n = app.bp->add_node(s_tn.c_str(), s_id == "-" ? nullptr : s_id.c_str());
				if (!n)
					printf("bad udt name or id already exist\n");
				else
					printf("node added: %s\n", n->id().c_str());
			}
			else if (s_what == "link")
			{
				scanf("%s", command_line);
				auto s_out_address = std::string(command_line);

				scanf("%s", command_line);
				auto s_in_address = std::string(command_line);

				auto out = app.bp->find_output(s_out_address.c_str());
				auto in = app.bp->find_input(s_in_address.c_str());
				if (out && in)
				{
					in->link_to(out);
					auto out_addr = in->link()->get_address();
					auto in_addr = in->get_address();
					printf("link added: %s -> %s\n", out_addr.p->c_str(), in_addr.p->c_str());
					delete_mail(out_addr);
					delete_mail(in_addr);
				}
				else
					printf("wrong address\n");
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

				auto n = app.bp->find_node(s_id.c_str());
				if (n)
				{
					app.bp->remove_node(n);
					printf("node removed: %s\n", s_id.c_str());
				}
				else
					printf("node not found\n");
			}
			else if (s_what == "link")
			{
				scanf("%s", command_line);
				auto s_in_address = std::string(command_line);

				auto i = app.bp->find_input(s_in_address.c_str());
				if (i)
				{
					i->link_to(nullptr);
					printf("link removed: %s\n", s_in_address.c_str());
				}
				else
					printf("input not found\n");
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

			app.set_data(s_address, s_value);
		}
		else if (s_command_line == "update")
		{
			app.bp->update();
			printf("BP updated\n");
		}
		else if (s_command_line == "save")
		{
		BP::save_to_file(app.bp, app.filename);
			printf("file saved\n");
		}
		else if (s_command_line == "set-layout")
		{
			if (!std::filesystem::exists(L"bp.graph.txt") || std::filesystem::last_write_time(L"bp.graph.txt") < std::filesystem::last_write_time(app.filename))
				app.generate_graph_and_layout();
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
					auto n = app.bp->find_node(match[1].str().c_str());
					if (n)
						n->pos = Vec2f(std::stof(match[2].str().c_str()), std::stof(match[3].str().c_str())) * 100.f;

					str = match.suffix();
				}
				printf("ok\n");
			}
			else
				printf("bp.graph.txt not found\n");
		}
		else
			printf("unknow command\n");
	}

	return 0;
}
