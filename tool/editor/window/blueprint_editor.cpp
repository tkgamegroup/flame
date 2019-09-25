#include <flame/foundation/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/device.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/image.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/font.h>
#include <flame/graphics/canvas.h>
#include <flame/universe/default_style.h>
#include <flame/universe/topmost.h>
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
#include <flame/universe/components/splitter.h>
#include <flame/universe/components/window.h>

#include "../app.h"
#include "../data_tracker.h"
#include "blueprint_editor.h"
#include "console.h"
#include "image_viewer.h"

template<class T>
void create_edit(Entity* parent, BP::Slot* input)
{
	auto& data = *(T*)input->data();

	auto c_tracker = new_component<cDigitalDataTracker<T>>();
	c_tracker->data = &data;
	parent->add_component(c_tracker);

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
	auto& data = *(Vec<N, T>*)input->data();

	auto c_tracker = new_component<cDigitalVecDataTracker<N, T>>();
	c_tracker->data = &data;
	parent->add_component(c_tracker);

	struct Capture
	{
		BP::Slot* input;
		uint i;
	}capture;
	capture.input = input;
	for (auto i = 0; i < N; i++)
	{
		auto e_edit = create_standard_edit(50.f, app.font_atlas_sdf, 0.5f);
		parent->add_child(wrap_standard_text(e_edit, false, app.font_atlas_sdf, 0.5f, s2w(Vec<N, T>::coord_name(i))));
		capture.i = i;
		((cEdit*)e_edit->find_component(cH("Edit")))->add_changed_listener([](void* c, const wchar_t* text) {
			auto& capture = *(Capture*)c;
			auto data = *(Vec<N, T>*)capture.input->data();
			data[capture.i] = text[0] ? sto<T>(text) : 0;
			capture.input->set_data(&data);
		}, new_mail(&capture));
	}
}

struct cBPEditor : Component
{
	std::wstring filename;
	std::wstring filepath;
	BP* bp;
	std::vector<TypeinfoDatabase*> dbs;
	bool locked;

	Entity* e_add_node_menu;
	Entity* e_base;
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
	bool cb_recorded;

	std::vector<std::pair<cElement*, uint>> tips;

	cBPEditor() :
		Component("BPEditor")
	{
		bp = nullptr;
		locked = false;

		console_tab = nullptr;

		rt = graphics::Image::create(app.d, Format_R8G8B8A8_UNORM, Vec2u(400, 300), 1, 1, SampleCount_1, ImageUsage$(ImageUsageTransferDst | ImageUsageAttachment | ImageUsageSampled));
		rt->init(Vec4c(0, 0, 0, 255));
		rt_v = Imageview::create(rt);
		rt_id = app.canvas->find_free_image();
		rt_cbs.resize(1);
		rt_cbs[0] = Commandbuffer::create(app.d->gcp);

		app.canvas->set_image(rt_id, rt_v);
	}

	~cBPEditor()
	{
		if (bp)
			BP::destroy(bp);

		if (console_tab)
		{
			looper().add_delay_event([](void* c) {
				auto tab = *(cDockerTab**)c;
				tab->take_away(true);
			}, new_mail_p(console_tab));
		}

		app.canvas->set_image(rt_id, nullptr);
	}

	void load(const std::wstring& _filename, bool no_compile)
	{
		filename = _filename;
		filepath = std::filesystem::path(filename).parent_path().wstring();
		if (bp)
			BP::destroy(bp);
		bp = BP::create_from_file(filename, no_compile);
		dbs.clear();
		for (auto i = 0; i < bp->dependency_count(); i++)
			dbs.push_back(bp->dependency_typeinfodatabase(i));
		dbs.push_back(bp->db());

		e_base->remove_all_children();
		for (auto i = 0; i < bp->node_count(); i++)
			create_node_entity(bp->node(i));

		e_add_node_menu->remove_all_children();

		std::vector<UdtInfo*> all_udts;
		for (auto db : dbs)
		{
			auto udts = db->get_udts();
			for (auto i = 0; i < udts.p->size(); i++)
			{
				auto u = udts.p->at(i);
				if (u->name().find('(') != std::string::npos)
					continue;
				{
					auto f = u->find_function("update");
					if (!(f && f->return_type()->equal(TypeTagVariable, cH("void")) && f->parameter_count() == 0))
						continue;
				}
				auto no_input_output = true;
				for (auto i = 0; i < u->variable_count(); i++)
				{
					auto v = u->variable(i);
					if (v->decoration().find('i') != std::string::npos || v->decoration().find('o') != std::string::npos)
					{
						no_input_output = false;
						break;
					}
				}
				if (!no_input_output)
					all_udts.push_back(u);
			}
			delete_mail(udts);
		}
		std::sort(all_udts.begin(), all_udts.end(), [](UdtInfo* a, UdtInfo* b) {
			return a->name() < b->name();
		});
		for (auto udt : all_udts)
		{
			auto e_item = create_standard_menu_item(app.font_atlas_pixel, 1.f, s2w(udt->name()));
			e_add_node_menu->add_child(e_item);
			struct Capture
			{
				cBPEditor* e;
				UdtInfo* u;
			}capture;
			capture.e = this;
			capture.u = udt;
			((cEventReceiver*)e_item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				auto& capture = *(Capture*)c;
				if (is_mouse_clicked(action, key))
				{
					destroy_topmost(app.root);
					capture.e->add_node(capture.u->name(), "");
				}
			}, new_mail(&capture));
		}
		{
			auto e_item = create_standard_menu_item(app.font_atlas_pixel, 1.f, L"template..");
			e_add_node_menu->add_child(e_item);
			((cEventReceiver*)e_item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				auto editor = *(cBPEditor**)c;
				if (is_mouse_clicked(action, key))
				{
					destroy_topmost(app.root);

					auto t = create_topmost(editor->entity, false, false, true, Vec4c(255, 255, 255, 235), true);
					{
						t->add_component(cLayout::create(LayoutFree));
					}

					auto e_dialog = Entity::create();
					t->add_child(e_dialog);
					{
						e_dialog->add_component(cElement::create());

						auto c_aligner = cAligner::create();
						c_aligner->x_align = AlignxMiddle;
						c_aligner->y_align = AlignyMiddle;
						e_dialog->add_component(c_aligner);

						auto c_layout = cLayout::create(LayoutVertical);
						c_layout->item_padding = 4.f;
						e_dialog->add_component(c_layout);
					}

					auto e_name = create_standard_edit(100.f, app.font_atlas_pixel, 1.f);
					e_dialog->add_child(e_name);

					auto e_buttons = Entity::create();
					e_dialog->add_child(e_buttons);
					{
						e_buttons->add_component(cElement::create());

						auto c_layout = cLayout::create(LayoutHorizontal);
						c_layout->item_padding = 4.f;
						e_buttons->add_component(c_layout);
					}

					auto e_btn_ok = create_standard_button(app.font_atlas_pixel, 1.f, L"Ok");
					e_buttons->add_child(e_btn_ok);
					{
						struct Capture
						{
							cBPEditor* e;
							cText* t;
						}capture;
						capture.e = editor;
						capture.t = (cText*)e_name->find_component(cH("Text"));
						((cEventReceiver*)e_btn_ok->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
							auto& capture = *(Capture*)c;
							if (is_mouse_clicked(action, key))
							{
								destroy_topmost(capture.e->entity, false);
								auto name = w2s(capture.t->text());
								auto db = capture.e->bp->db();
								if (db->find_udt(H(name.c_str())))
									capture.e->add_node(name, "");
								else
								{
									if (capture.e->running)
										capture.e->add_tip(L"Cannot Add New Template Node While Running");
									else
									{
										auto file = SerializableNode::create_from_xml_file(capture.e->filename);
										auto n_nodes = file->find_node("nodes");
										auto n_node = n_nodes->new_node("node");
										n_node->new_attr("type", name);
										{
											std::string id;
											auto bp = capture.e->bp;
											for (auto i = 0; i < bp->node_count() + 1; i++)
											{
												id = "node_" + std::to_string(i);
												if (!bp->find_node(id))
													break;
											}
											n_node->new_attr("id", id);
										}
										n_node->new_attr("pos", "0.0;0.0");
										SerializableNode::save_to_xml_file(file, capture.e->filename);
										SerializableNode::destroy(file);

										capture.e->load(capture.e->filename, false);
									}
								}
							}
						}, new_mail(&capture));
					}

					auto e_btn_cancel = create_standard_button(app.font_atlas_pixel, 1.f, L"Cancel");
					e_buttons->add_child(e_btn_cancel);
					{
						((cEventReceiver*)e_btn_cancel->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
							if (is_mouse_clicked(action, key))
								destroy_topmost(*(Entity**)c, false);
						}, new_mail_p(editor->entity));
					}
				}
			}, new_mail_p(this));
		}

		sel_type = SelAir;
		selected.n = nullptr;
		dragging_slot = nullptr;
		running = false;
		cb_recorded = false;
	}

	Entity* create_node_entity(BP::Node* n);

	BP::Node* add_node(const std::string& type_name, const std::string& id)
	{
		auto n = bp->add_node(type_name, id);
		create_node_entity(n);
		return n;
	}

	void remove_node(BP::Node* n)
	{
		looper().add_delay_event([](void* c) {
			auto e = *(Entity**)c;
			e->parent()->remove_child(e);
		}, new_mail_p(n->user_data));
		bp->remove_node(n);
	}

	void delete_selected()
	{
		switch (sel_type)
		{
		case SelNode:
			remove_node(selected.n);
			break;
		case SelLink:
			selected.l->link_to(nullptr);
			break;
		}
		sel_type = SelAir;
		selected.n = nullptr;
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

			auto str = "\t" + name + " [label = \"" + name + "|" + src->udt()->name() + "|{{";
			for (auto j = 0; j < src->input_count(); j++)
			{
				auto input = src->input(j);
				auto& name = input->vi()->name();
				str += "<" + name + ">" + name;
				if (j != src->input_count() - 1)
					str += "|";
			}
			str += "}|{";
			for (auto j = 0; j < src->output_count(); j++)
			{
				auto output = src->output(j);
				auto& name = output->vi()->name();
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

	bool auto_set_layout()
	{
		if (!std::filesystem::exists(L"bp.graph.txt") || std::filesystem::last_write_time(L"bp.graph.txt") < std::filesystem::last_write_time(filename))
			generate_graph_and_layout();
		if (!std::filesystem::exists(L"bp.graph.txt"))
			return false;

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
			{
				n->pos = Vec2f(std::stof(match[2].str().c_str()), std::stof(match[3].str().c_str())) * 100.f;
				((cElement*)((Entity*)n->user_data)->find_component(cH("Element")))->pos = n->pos;
			}

			str = match.suffix();
		}

		return true;
	}

	void add_tip(const std::wstring& text)
	{
		auto e_tip = Entity::create();
		entity->add_child(e_tip);
		{
			auto c_element = cElement::create();
			c_element->pos.y() = tips.size() * (app.font_atlas_sdf->pixel_height + 20.f);
			c_element->inner_padding = Vec4f(8.f);
			c_element->color = Vec4c(0, 0, 0, 255);
			e_tip->add_component(c_element);
			tips.emplace_back(c_element, 180);

			auto c_text = cText::create(app.font_atlas_sdf);
			c_text->color = Vec4c(255);
			c_text->set_text(text);
			e_tip->add_component(c_text);
		}
	}

	virtual void update() override
	{
		if (running)
		{
			bp->update();
			if (cb_recorded)
				app.extra_cbs.push_back((Commandbuffer*)rt_cbs[0]);
		}

		for (auto it = tips.begin(); it != tips.end(); )
		{
			it->second--;
			if (it->second == 0)
			{
				for (auto _it = it + 1; _it != tips.end(); _it++)
					_it->first->pos.y() = it->first->pos.y();
				entity->remove_child(it->first->entity);
				it = tips.erase(it);
			}
			else
			{
				if (it->second < 60)
					it->first->alpha = it->second / 60.f;
				it++;
			}
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

	virtual void start() override;

	virtual void update() override;
};

struct cBPNode : Component
{
	cElement* element;
	cEventReceiver* event_receiver;

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

		event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
			auto thiz = *(cBPNode**)c;
			if (is_mouse_down(action, key, true) && key == Mouse_Left)
			{
				thiz->editor->sel_type = cBPEditor::SelNode;
				thiz->editor->selected.n = thiz->n;
			}
		}, new_mail_p(this));
	}

	virtual void update() override
	{
		if (n == editor->selected.n)
			element->frame_thickness = 4.f;
		else
			element->frame_thickness = 0.f;

		n->pos = element->pos;
	}
};

struct cBPSlot : Component
{
	cElement* element;
	cEventReceiver* event_receiver;
	cDataTracker* tracker;

	cBPEditor* editor;
	BP::Slot* s;

	cBPSlot() :
		Component("BPSlot")
	{
		element = nullptr;
		event_receiver = nullptr;
		tracker = nullptr;
	}

	virtual void start() override
	{
		element = (cElement*)entity->find_component(cH("Element"));
		event_receiver = (cEventReceiver*)entity->find_component(cH("EventReceiver"));

		if (s->type() == BP::Slot::Input)
		{
			event_receiver->drag_hash = cH("input_slot");
			event_receiver->set_acceptable_drops({ cH("output_slot") });
			tracker = (cDataTracker*)entity->parent()->parent()->child(1)->find_component(cH("DataTracker"));
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
				if (thiz->s->type() == BP::Slot::Input)
					thiz->s->link_to(nullptr);
			}
			else if (action == DragEnd)
				thiz->editor->dragging_slot = nullptr;
			else if (action == Dropped)
			{
				auto oth = ((cBPSlot*)er->entity->find_component(cH("BPSlot")))->s;
				if (thiz->s->type() == BP::Slot::Input)
					thiz->s->link_to(oth);
				else
					oth->link_to(thiz->s);
			}
		}, new_mail_p(this));
	}
};

void cBP::start()
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
						auto e1 = ((cBPSlot*)output->user_data)->element;
						auto e2 = ((cBPSlot*)input->user_data)->element;
						auto p1 = (e1->global_pos + e1->global_size) * 0.5f;
						auto p2 = (e2->global_pos + e2->global_size) * 0.5f;

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
			thiz->base_element->scale += pos.x() > 0.f ? 0.1f : -0.1f;
			thiz->base_element->scale = clamp(thiz->base_element->scale, 0.1f, 2.f);
		}
		else if (is_mouse_move(action, key) && (thiz->event_receiver->event_dispatcher->mouse_buttons[Mouse_Right] & KeyStateDown))
			thiz->base_element->pos += pos;
	}, new_mail_p(this));
}

void cBP::update()
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
				auto e1 = ((cBPSlot*)output->user_data)->element;
				auto e2 = ((cBPSlot*)input->user_data)->element;
				if (e1 && e2)
				{
					auto p1 = (e1->global_pos + e1->global_size) * 0.5f;
					auto p2 = (e2->global_pos + e2->global_size) * 0.5f;

					std::vector<Vec2f> points;
					path_bezier(points, p1, p1 + Vec2f(50.f, 0.f), p2 - Vec2f(50.f, 0.f), p2);
					element->canvas->stroke(points, editor->selected.l == input ? Vec4c(255, 255, 50, 255) : Vec4c(100, 100, 120, 255), 3.f * base_element->global_scale);
				}
			}
		}
	}
	if (editor->dragging_slot)
	{
		auto e = ((cBPSlot*)editor->dragging_slot->user_data)->element;
		auto p1 = (e->global_pos + e->global_size) * 0.5f;
		auto p2 = Vec2f(event_receiver->event_dispatcher->mouse_pos);

		std::vector<Vec2f> points;
		path_bezier(points, p1, p1 + Vec2f(editor->dragging_slot->type() == BP::Slot::Output ? 50.f : -50.f, 0.f), p2, p2);
		element->canvas->stroke(points, Vec4c(255, 255, 50, 255), 3.f * base_element->global_scale);
	}
}

Entity* cBPEditor::create_node_entity(BP::Node* n)
{
	auto e_node = Entity::create();
	e_base->add_child(e_node);
	n->user_data = e_node;
	{
		auto c_element = cElement::create();
		c_element->pos = n->pos;
		c_element->color = Vec4c(255, 255, 255, 200);
		c_element->frame_color = Vec4c(252, 252, 50, 200);
		e_node->add_component(c_element);

		e_node->add_component(cEventReceiver::create());

		auto c_layout = cLayout::create(LayoutFree);
		c_layout->width_fit_children = true;
		c_layout->height_fit_children = true;
		e_node->add_component(c_layout);

		e_node->add_component(cMoveable::create());

		auto c_node = new_component<cBPNode>();
		c_node->editor = this;
		c_node->n = n;
		e_node->add_component(c_node);
	}
	{
		auto e_content = Entity::create();
		e_node->add_child(e_content);
		{
			auto c_element = cElement::create();
			c_element->inner_padding = Vec4f(8.f);
			e_content->add_component(c_element);

			auto c_layout = cLayout::create(LayoutVertical);
			c_layout->item_padding = 4.f;
			e_content->add_component(c_layout);
		}

		auto e_text_id = Entity::create();
		e_content->add_child(e_text_id);
		{
			auto c_element = cElement::create();
			c_element->inner_padding = Vec4f(4.f, 2.f, 4.f, 2.f);
			e_text_id->add_component(c_element);

			auto c_text = cText::create(app.font_atlas_sdf);
			c_text->set_text(s2w(n->id()));
			c_text->sdf_scale = 0.8f;
			e_text_id->add_component(c_text);

			e_text_id->add_component(cEventReceiver::create());

			auto c_edit = cEdit::create();
			c_edit->add_changed_listener([](void* c, const wchar_t* text) {
				auto n = *(BP::Node**)c;
				n->set_id(w2s(text));
			}, new_mail_p(n));
			e_text_id->add_component(c_edit);
		}

		auto e_text_type = Entity::create();
		e_content->add_child(e_text_type);
		{
			e_text_type->add_component(cElement::create());

			auto c_text = cText::create(app.font_atlas_sdf);
			c_text->set_text(s2w(n->udt()->name()));
			c_text->color = Vec4c(50, 50, 50, 255);
			c_text->sdf_scale = 0.5f;
			e_text_type->add_component(c_text);
		}

		if (n->udt()->name() == "graphics::Shader")
		{
			auto e_btn_edit = create_standard_button(app.font_atlas_sdf, 0.5f, L"Edit Shader");
			e_content->add_child(e_btn_edit);

			struct Capture
			{
				cBPEditor* e;
				BP::Node* n;
			}capture;
			capture.e = this;
			capture.n = n;
			((cEventReceiver*)e_btn_edit->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				auto& capture = *(Capture*)c;
				if (is_mouse_clicked(action, key))
				{
					capture.e->locked = true;
					auto t = create_topmost(capture.e->entity, false, false, true, Vec4c(255, 255, 255, 235), true);
					{
						((cElement*)t->find_component(cH("Element")))->inner_padding = Vec4f(4.f);

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

					auto e_btn_back = create_standard_button(app.font_atlas_pixel, 1.f, L"Back");
					e_buttons->add_child(e_btn_back);
					{
						((cEventReceiver*)e_btn_back->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
							if (is_mouse_clicked(action, key))
							{
								auto editor = *(cBPEditor**)c;
								destroy_topmost(editor->entity, false);
								editor->locked = false;
							}
						}, new_mail_p(capture.e));
					}

					auto e_btn_compile = create_standard_button(app.font_atlas_pixel, 1.f, L"Compile");
					e_buttons->add_child(e_btn_compile);

					auto e_text_tip = Entity::create();
					e_buttons->add_child(e_text_tip);
					{
						e_text_tip->add_component(cElement::create());

						auto c_text = cText::create(app.font_atlas_pixel);
						c_text->set_text(L"(Do update first to get popper result)");
						e_text_tip->add_component(c_text);
					}

					auto e_main = Entity::create();
					t->add_child(e_main);
					{
						e_main->add_component(cElement::create());

						auto c_aligner = cAligner::create();
						c_aligner->width_policy = SizeFitParent;
						c_aligner->height_policy = SizeFitParent;
						e_main->add_component(c_aligner);

						auto c_layout = cLayout::create(LayoutVertical);
						c_layout->width_fit_children = false;
						c_layout->height_fit_children = false;
						e_main->add_component(c_layout);
					}


					auto filename = *(std::wstring*)capture.n->find_input("filename")->data();
					auto prefix = *(std::string*)capture.n->find_input("prefix")->data();
					auto inputs = (std::vector<void*>*)capture.n->find_input("inputs")->data_p();
					auto outputs = (std::vector<void*>*)capture.n->find_input("outputs")->data_p();
					auto pll = (Pipelinelayout*)capture.n->find_input("pll")->data_p();
					auto autogen_code = *(bool*)capture.n->find_input("autogen_code")->data();

					{
						auto e_text_view = Entity::create();
						{
							auto c_element = cElement::create();
							c_element->clip_children = true;
							e_text_view->add_component(c_element);

							auto c_aligner = cAligner::create();
							c_aligner->width_policy = SizeFitParent;
							c_aligner->height_policy = SizeFitParent;
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
							auto _prefix = s2w(prefix);
							if (autogen_code)
							{
								auto code = get_shader_autogen_code(shader_stage_from_filename(filename), inputs, outputs, pll);
								_prefix += s2w(*code.p);
								delete_mail(code);
							}
							c_text->set_text(_prefix);
							c_text->auto_width = false;
							e_text->add_component(c_text);
						}

						auto e_scrollbar_container = wrap_standard_scrollbar(e_text_view, ScrollbarVertical, true, app.font_atlas_pixel->pixel_height);
						((cAligner*)e_scrollbar_container->find_component(cH("Aligner")))->height_factor = 1.f / 3.f;
						e_main->add_child(e_scrollbar_container);
					}

					auto e_spliter = Entity::create();
					e_main->add_child(e_spliter);
					{
						auto c_element = cElement::create();
						c_element->size.y() = 8.f;
						e_spliter->add_component(c_element);

						e_spliter->add_component(cEventReceiver::create());

						e_spliter->add_component(cStyleColor::create(Vec4c(0), default_style.frame_color_hovering, default_style.frame_color_active));

						auto c_splitter = cSplitter::create();
						c_splitter->type = SplitterVertical;
						e_spliter->add_component(c_splitter);

						auto c_aligner = cAligner::create();
						c_aligner->width_policy = SizeFitParent;
						e_spliter->add_component(c_aligner);
					}

					{
						auto e_text_view = Entity::create();
						{
							auto c_element = cElement::create();
							c_element->clip_children = true;
							e_text_view->add_component(c_element);

							auto c_aligner = cAligner::create();
							c_aligner->width_policy = SizeFitParent;
							c_aligner->height_policy = SizeFitParent;
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
							c_text->set_text(s2w(file));
							c_text->auto_width = false;
							e_text->add_component(c_text);

							e_text->add_component(cEventReceiver::create());

							e_text->add_component(cEdit::create());

							auto c_aligner = cAligner::create();
							c_aligner->width_policy = SizeFitParent;
							e_text->add_component(c_aligner);

							{
								struct _Capture
								{
									cBPEditor* e;
									BP::Node* n;
									cText* t;
								}_capture;
								_capture.e = capture.e;
								_capture.n = capture.n;
								_capture.t = c_text;
								((cEventReceiver*)e_btn_compile->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
									auto& capture = *(_Capture*)c;
									if (is_mouse_clicked(action, key))
									{
										auto i_filename = capture.n->find_input("filename");
										std::ofstream file(capture.e->filepath + L"/" + *(std::wstring*)i_filename->data());
										auto str = w2s(capture.t->text());
										str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
										file.write(str.c_str(), str.size());
										file.close();
										i_filename->set_frame(looper().frame);
									}
								}, new_mail(&_capture));
							}
						}

						auto e_scrollbar_container = wrap_standard_scrollbar(e_text_view, ScrollbarVertical, true, app.font_atlas_pixel->pixel_height);
						((cAligner*)e_scrollbar_container->find_component(cH("Aligner")))->height_factor = 2.f / 3.f;
						e_main->add_child(e_scrollbar_container);
					}
				}
			}, new_mail(&capture));
		}

		auto e_main = Entity::create();
		e_content->add_child(e_main);
		{
			e_main->add_component(cElement::create());

			auto c_aligner = cAligner::create();
			c_aligner->width_policy = SizeGreedy;
			e_main->add_component(c_aligner);

			auto c_layout = cLayout::create(LayoutHorizontal);
			c_layout->item_padding = 16.f;
			e_main->add_component(c_layout);

			auto e_left = Entity::create();
			e_main->add_child(e_left);
			{
				e_left->add_component(cElement::create());

				auto c_aligner = cAligner::create();
				c_aligner->width_policy = SizeGreedy;
				e_left->add_component(c_aligner);

				e_left->add_component(cLayout::create(LayoutVertical));

				for (auto j = 0; j < n->input_count(); j++)
				{
					auto input = n->input(j);

					auto e_input = Entity::create();
					e_left->add_child(e_input);
					{
						e_input->add_component(cElement::create());

						auto c_layout = cLayout::create(LayoutVertical);
						c_layout->item_padding = 2.f;
						e_input->add_component(c_layout);
					}

					auto e_title = Entity::create();
					e_input->add_child(e_title);
					{
						e_title->add_component(cElement::create());

						e_title->add_component(cLayout::create(LayoutHorizontal));
					}

					auto e_slot = Entity::create();
					e_title->add_child(e_slot);
					{
						auto c_element = cElement::create();
						auto r = app.font_atlas_sdf->pixel_height * 0.6f;
						c_element->size = r;
						c_element->round_radius = r * 0.5f;
						c_element->color = Vec4c(200, 200, 200, 255);
						e_slot->add_component(c_element);

						e_slot->add_component(cEventReceiver::create());

						auto c_slot = new_component<cBPSlot>();
						c_slot->editor = this;
						c_slot->s = input;
						e_slot->add_component(c_slot);
						input->user_data = c_slot;
					}

					auto e_text = Entity::create();
					e_title->add_child(e_text);
					{
						e_text->add_component(cElement::create());

						auto c_text = cText::create(app.font_atlas_sdf);
						c_text->sdf_scale = 0.6f;
						c_text->set_text(s2w(input->vi()->name()));
						e_text->add_component(c_text);
					}

					auto e_data = Entity::create();
					e_input->add_child(e_data);
					{
						auto c_element = cElement::create();
						c_element->inner_padding = Vec4f(app.font_atlas_sdf->pixel_height, 0.f, 0.f, 0.f);
						e_data->add_component(c_element);

						auto c_layout = cLayout::create(LayoutVertical);
						c_layout->item_padding = 2.f;
						e_data->add_component(c_layout);
					}

					auto type = input->vi()->type();
					switch (type->tag())
					{
					case TypeTagAttributeES:
					{
						auto info = find_enum(dbs, type->hash());
						create_enum_combobox(info, 120.f, app.font_atlas_sdf, 0.5f, e_data);

						auto c_tracker = new_component<cEnumSingleDataTracker>();
						c_tracker->data = (int*)input->data();
						c_tracker->info = info;
						e_data->add_component(c_tracker);

						struct Capture
						{
							BP::Slot* input;
							EnumInfo* e;
						}capture;
						capture.input = input;
						capture.e = info;
						((cCombobox*)e_data->child(0)->find_component(cH("Combobox")))->add_changed_listener([](void* c, int idx) {
							auto& capture = *(Capture*)c;
							auto v = capture.e->item(idx)->value();
							capture.input->set_data(&v);
						}, new_mail(&capture));
					}
						break;
					case TypeTagAttributeEM:
					{
						auto v = *(int*)input->data();

						auto info = find_enum(dbs, type->hash());

						auto c_tracker = new_component<cEnumMultiDataTracker>();
						c_tracker->data = (int*)input->data();
						c_tracker->info = info;
						e_data->add_component(c_tracker);

						for (auto k = 0; k < info->item_count(); k++)
						{
							auto item = info->item(k);
							auto e_checkbox = create_standard_checkbox(app.font_atlas_sdf, 0.5f, s2w(item->name()));
							e_data->add_child(e_checkbox);
						}
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
							((cCheckbox*)e_data->child(k)->find_component(cH("Checkbox")))->add_changed_listener([](void* c, bool checked) {
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
							auto c_tracker = new_component<cBoolDataTracker>();
							c_tracker->data = (bool*)input->data();
							e_data->add_component(c_tracker);

							auto e_checkbox = create_standard_checkbox(app.font_atlas_sdf, 0.5f, L"");
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
							auto c_tracker = new_component<cStringDataTracker>();
							c_tracker->data = (std::string*)input->data();
							e_data->add_component(c_tracker);

							auto e_edit = create_standard_edit(50.f, app.font_atlas_sdf, 0.5f);
							e_data->add_child(e_edit);
							((cEdit*)e_edit->find_component(cH("Edit")))->add_changed_listener([](void* c, const wchar_t* text) {
								auto str = w2s(text);
								(*(BP::Slot**)c)->set_data(&str);
							}, new_mail_p(input));
						}
							break;
						case cH("std::basic_string(wchar_t)"):
						{
							auto c_tracker = new_component<cWStringDataTracker>();
							c_tracker->data = (std::wstring*)input->data();
							e_data->add_component(c_tracker);

							auto e_edit = create_standard_edit(50.f, app.font_atlas_sdf, 0.5f);
							e_data->add_child(e_edit);
							((cEdit*)e_edit->find_component(cH("Edit")))->add_changed_listener([](void* c, const wchar_t* text) {
								auto str = std::wstring(text);
								(*(BP::Slot**)c)->set_data(&str);
							}, new_mail_p(input));
						}
							break;
						}
						break;
					}
				}
			}

			auto e_right = Entity::create();
			e_main->add_child(e_right);
			{
				e_right->add_component(cElement::create());

				e_right->add_component(cLayout::create(LayoutVertical));

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

						e_title->add_component(cLayout::create(LayoutHorizontal));
					}

					auto e_text = Entity::create();
					e_title->add_child(e_text);
					{
						e_text->add_component(cElement::create());

						auto c_text = cText::create(app.font_atlas_sdf);
						c_text->sdf_scale = 0.6f;
						c_text->set_text(s2w(output->vi()->name()));
						e_text->add_component(c_text);
					}

					auto e_slot = Entity::create();
					e_title->add_child(e_slot);
					{
						auto c_element = cElement::create();
						auto r = app.font_atlas_sdf->pixel_height * 0.6f;
						c_element->size = r;
						c_element->round_radius = r * 0.5f;
						c_element->color = Vec4c(200, 200, 200, 255);
						e_slot->add_component(c_element);

						e_slot->add_component(cEventReceiver::create());

						auto c_slot = new_component<cBPSlot>();
						c_slot->editor = this;
						c_slot->s = output;
						e_slot->add_component(c_slot);
						output->user_data = c_slot;
					}
				}
			}
		}

		auto e_bring_to_front = Entity::create();
		e_node->add_child(e_bring_to_front);
		{
			e_bring_to_front->add_component(cElement::create());

			auto c_event_receiver = cEventReceiver::create();
			c_event_receiver->penetrable = true;
			e_bring_to_front->add_component(c_event_receiver);

			auto c_aligner = cAligner::create();
			c_aligner->width_policy = SizeFitParent;
			c_aligner->height_policy = SizeFitParent;
			e_bring_to_front->add_component(c_aligner);

			e_bring_to_front->add_component(cBringToFront::create());
		}
	}

	return e_node;
}

void open_blueprint_editor(const std::wstring& filename, bool no_compile, const Vec2f& pos)
{
	auto e_container = get_docker_container_model()->copy();
	app.root->add_child(e_container);
	{
		auto c_element = (cElement*)e_container->find_component(cH("Element"));
		c_element->pos = pos;
		c_element->size.x() = 1052.f;
		c_element->size.y() = 963.f;
	}

	auto e_docker = get_docker_model()->copy();
	e_container->add_child(e_docker, 0);

	e_docker->child(0)->add_child(create_standard_docker_tab(app.font_atlas_pixel, L"Blueprint Editor", app.root));

	auto e_page = get_docker_page_model()->copy();
	{
		auto c_layout = cLayout::create(LayoutVertical);
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;
		e_page->add_component(c_layout);
	}
	e_docker->child(1)->add_child(e_page);

	auto c_editor = new_component<cBPEditor>();
	e_page->add_component(c_editor);

	auto e_menubar = create_standard_menubar();
	e_page->add_child(e_menubar);
	{
		auto e_menu = create_standard_menu();
		{
			auto e_item = create_standard_menu_item(app.font_atlas_pixel, 1.f, L"Save");
			e_menu->add_child(e_item);
			((cEventReceiver*)e_item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				auto editor = *(cBPEditor**)c;
				if (is_mouse_clicked(action, key))
				{
					destroy_topmost(app.root);
					editor->bp->save_to_file(editor->bp, editor->filename);
				}
			}, new_mail_p(c_editor));
		}
		{
			auto e_item = create_standard_menu_item(app.font_atlas_pixel, 1.f, L"Dependency Manager");
			e_menu->add_child(e_item);
			((cEventReceiver*)e_item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				auto editor = *(cBPEditor**)c;
				if (is_mouse_clicked(action, key))
				{
					destroy_topmost(app.root);
				}
			}, new_mail_p(c_editor));
		}
		{
			auto e_item = create_standard_menu_item(app.font_atlas_pixel, 1.f, L"Reload");
			e_menu->add_child(e_item);
			((cEventReceiver*)e_item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				auto editor = *(cBPEditor**)c;
				if (is_mouse_clicked(action, key))
				{
					destroy_topmost(app.root);
					if (editor->running)
						editor->add_tip(L"Cannot Reload While Running");
					else
						editor->load(editor->filename, false);
				}
			}, new_mail_p(c_editor));
		}
		{
			auto e_item = create_standard_menu_item(app.font_atlas_pixel, 1.f, L"Reload (No Compile)");
			e_menu->add_child(e_item);
			((cEventReceiver*)e_item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				auto editor = *(cBPEditor**)c;
				if (is_mouse_clicked(action, key))
				{
					destroy_topmost(app.root);
					if (editor->running)
						editor->add_tip(L"Cannot Reload While Running");
					else
						editor->load(editor->filename, true);
				}
			}, new_mail_p(c_editor));
		}
		auto e_menu_btn = create_standard_menu_button(app.font_atlas_pixel, 1.f, L"Blueprint", app.root, e_menu, true, SideS, true, false, true, nullptr);
		e_menubar->add_child(e_menu_btn);
	}
	{
		auto e_menu = create_standard_menu();
		c_editor->e_add_node_menu = e_menu;
		auto e_menu_btn = create_standard_menu_button(app.font_atlas_pixel, 1.f, L"Add Node", app.root, e_menu, true, SideS, true, false, true, nullptr);
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
					destroy_topmost(app.root);
					auto editor = *(cBPEditor**)c;
					editor->delete_selected();
				}
			}, new_mail_p(c_editor));
		}
		auto e_menu_btn = create_standard_menu_button(app.font_atlas_pixel, 1.f, L"Edit", app.root, e_menu, true, SideS, true, false, true, nullptr);
		e_menubar->add_child(e_menu_btn);
	}
	{
		auto e_menu = create_standard_menu();
		{
			auto e_item = create_standard_menu_item(app.font_atlas_pixel, 1.f, L"Auto Set Layout");
			e_menu->add_child(e_item);
			((cEventReceiver*)e_item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				if (is_mouse_clicked(action, key))
				{
					destroy_topmost(app.root);
					auto editor = *(cBPEditor**)c;
					editor->auto_set_layout();
				}
			}, new_mail_p(c_editor));
		}
		auto e_menu_btn = create_standard_menu_button(app.font_atlas_pixel, 1.f, L"Layout", app.root, e_menu, true, SideS, true, false, true, nullptr);
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

				if (capture.e->running)
				{
					auto bp = capture.e->bp;
					bp->graphics_device = app.d;
					auto rt_dst = bp->find_node("rt_dst");
					if (rt_dst)
					{
						rt_dst->find_input("type")->set_data_i(TargetImageview);
						rt_dst->find_input("v")->set_data_p(capture.e->rt_v);
					}
					auto make_cmd = bp->find_node("make_cmd");
					if (make_cmd)
					{
						make_cmd->find_input("cbs")->set_data_p(&capture.e->rt_cbs);
						capture.e->cb_recorded = true;
					}
				}
				else
					capture.e->cb_recorded = false;
			}
		}, new_mail(&capture));
	}

	auto e_clipper = Entity::create();
	e_page->add_child(e_clipper);
	{
		auto c_element = cElement::create();
		c_element->clip_children = true;
		e_clipper->add_component(c_element);

		auto c_aligner = cAligner::create();
		c_aligner->width_policy = SizeFitParent;
		c_aligner->height_policy = SizeFitParent;
		e_clipper->add_component(c_aligner);

		e_clipper->add_component(cLayout::create(LayoutFree));
	}

	auto e_scene = Entity::create();
	e_clipper->add_child(e_scene);
	{
		e_scene->add_component(cElement::create());

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
	c_editor->e_base = e_base;

	c_editor->load(filename, no_compile);

	auto console_page = open_console([](void* c, const std::wstring& cmd, cConsole* console) {
		auto editor = *(cBPEditor**)c;
		auto& filename = editor->filename;
		auto bp = editor->bp;
		auto& dbs = editor->dbs;
		auto tokens = string_split(cmd);

		if (editor->locked)
		{
			console->print(L"bp is locked");
			return;
		}

		auto set_data = [&](const std::string& address, const std::string& value) {
			auto i = bp->find_input(address.c_str());
			if (i)
			{
				auto v = i->vi();
				auto type = v->type();
				auto value_before = serialize_value(dbs, type->tag(), type->hash(), i->data(), 2);
				auto data = new char[v->size()];
				unserialize_value(dbs, type->tag(), type->hash(), value, data);
				i->set_data(data);
				((cBPSlot*)i->user_data)->tracker->update_view();
				delete data;
				auto value_after = serialize_value(dbs, type->tag(), type->hash(), i->data(), 2);
				console->print(L"set value: " + s2w(address) + L", " + s2w(*value_before.p) + L" -> " + s2w(*value_after.p));
				delete_mail(value_before);
				delete_mail(value_after);
			}
			else
				console->print(L"input not found");
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
				"  auto-set-layout - set nodes' positions using 'bp.png' and 'bp.graph.txt', need do show graph first"
			);
		}
		else if (tokens[0] == L"show")
		{
			if (tokens[1] == L"udts")
			{
				std::vector<UdtInfo*> all_udts;
				for (auto db : dbs)
				{
					auto udts = db->get_udts();
					for (auto i = 0; i < udts.p->size(); i++)
						all_udts.push_back(udts.p->at(i));
					delete_mail(udts);
				}
				std::sort(all_udts.begin(), all_udts.end(), [](UdtInfo* a, UdtInfo* b) {
					return a->name() < b->name();
				});
				for (auto udt : all_udts)
					console->print(s2w(udt->name()));
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
					console->print(L"id:" + s2w(n->id()) + L" type:" + s2w(n->udt()->name()));
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
						auto v = input->vi();
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
						auto v = output->vi();
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
					editor->generate_graph_and_layout();
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
				auto n = editor->add_node(w2s(tokens[2]), tokens[3] == L"-" ? "" : w2s(tokens[3]));
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
					editor->remove_node(n);
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
		else if (tokens[0] == L"auto-set-layout")
		{
			if (editor->auto_set_layout())
				console->print(L"ok");
			else
				console->print(L"bp.graph.txt not found");
		}
		else
			console->print(L"unknow command");
	}, new_mail_p(c_editor), [](void* c) {
		auto editor = *(cBPEditor**)c;
		editor->console_tab = nullptr;
	}, new_mail_p(c_editor), filename + L":", Vec2f(1505.f, 20.f));
	c_editor->console_tab = (cDockerTab*)console_page->parent()->parent()->child(0)->child(0)->find_component(cH("DockerTab"));

	open_image_viewer(c_editor->rt_id, Vec2f(20.f, 655.f));
}
