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
#include <flame/universe/components/list.h>
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

namespace flame
{
	struct DstImage$
	{
		AttributeV<Vec2u> size$i;

		AttributeP<void> img$o;
		AttributeE<TargetType$> type$o;
		AttributeP<void> view$o;

		AttributeV<uint> idx$o;

		__declspec(dllexport) DstImage$()
		{
			size$i.v = Vec2u(400, 300);
		}

		__declspec(dllexport) void update$()
		{
			if (size$i.frame > img$o.frame)
			{
				if (idx$o.v > 0)
					app.canvas->set_image(idx$o.v, nullptr);
				if (img$o.v)
					Image::destroy((Image*)img$o.v);
				if (view$o.v)
					Imageview::destroy((Imageview*)view$o.v);
				auto d = (Device*)bp_env().graphics_device;
				if (d && size$i.v.x() > 0 && size$i.v.y() > 0)
				{
					img$o.v = Image::create(d, Format_R8G8B8A8_UNORM, size$i.v, 1, 1, SampleCount_1, ImageUsage$(ImageUsageTransferDst | ImageUsageAttachment | ImageUsageSampled));
					((Image*)img$o.v)->init(Vec4c(0, 0, 0, 255));
				}
				else
					img$o.v = nullptr;
				type$o.v = TargetImageview;
				type$o.frame = size$i.frame;
				if (img$o.v)
				{
					view$o.v = Imageview::create((Image*)img$o.v);
					idx$o.v = app.canvas->set_image(-1, (Imageview*)view$o.v);
				}
				img$o.frame = size$i.frame;
				view$o.frame = size$i.frame;
				idx$o.frame = size$i.frame;
			}
		}

		__declspec(dllexport) ~DstImage$()
		{
			if (idx$o.v > 0)
				app.canvas->set_image(idx$o.v, nullptr);
			if (img$o.v)
				Image::destroy((Image*)img$o.v);
			if (view$o.v)
				Imageview::destroy((Imageview*)view$o.v);
		}
	};

	struct CmdBufs$
	{
		AttributeV<std::vector<void*>> out$o;

		__declspec(dllexport) CmdBufs$()
		{
		}

		__declspec(dllexport) void update$()
		{
			if (out$o.frame == -1)
			{
				for (auto i = 0; i < out$o.v.size(); i++)
					Commandbuffer::destroy((Commandbuffer*)out$o.v[i]);
				auto d = (Device*)bp_env().graphics_device;
				if (d)
				{
					out$o.v.resize(1);
					out$o.v[0] = Commandbuffer::create(d->gcp);
				}
				else
					out$o.v.clear();
				out$o.frame = 0;
			}

			app.extra_cbs.push_back((Commandbuffer*)out$o.v[0]);
		}

		__declspec(dllexport) ~CmdBufs$()
		{
			for (auto i = 0; i < out$o.v.size(); i++)
				Commandbuffer::destroy((Commandbuffer*)out$o.v[i]);
		}
	};
}

struct cBPEditor : Component
{
	std::wstring filename;
	std::wstring filepath;
	BP* bp;
	std::vector<TypeinfoDatabase*> dbs;
	bool locked;

	Entity* e_add_node_menu;
	cEdit* add_node_menu_filter;
	Vec2f add_pos;
	Entity* e_base;
	Entity* e_exports_main;
	Entity* e_slot_menu;
	cDockerTab* console_tab;

	enum SelType
	{
		SelAir,
		SelModule,
		SelImport,
		SelNode,
		SelSlot,
		SelLink
	}sel_type;
	union
	{
		BP::Module* m;
		BP::Import* i;
		BP::Node* n;
		BP::Slot* s;
		BP::Slot* l;
	}selected;
	BP::Slot* dragging_slot;

	bool running;

	std::vector<std::pair<cElement*, uint>> tips;

	cBPEditor() :
		Component("BPEditor")
	{
		bp = nullptr;
		locked = false;

		console_tab = nullptr;
	}

	~cBPEditor()
	{
		BP::destroy(bp);

		if (console_tab)
		{
			looper().add_delay_event([](void* c) {
				auto tab = *(cDockerTab**)c;
				tab->take_away(true);
			}, new_mail_p(console_tab));
		}
	}

	void reset_add_node_menu_filter()
	{
		add_node_menu_filter->text->set_text(L"");

		for (auto i = 1; i < e_add_node_menu->child_count(); i++)
			e_add_node_menu->child(i)->visible = true;
	}

	Entity* create_module_entity(BP::Module* m);
	Entity* create_node_entity(BP::Node* n);
	Entity* create_import_entity(BP::Import* i);
	Entity* create_exports_entity();
	Entity* create_export_entity(BP::Export* e);
	void refresh_exports_entity();

	void load(const std::wstring& _filename, bool no_compile)
	{
		filename = _filename;
		filepath = std::filesystem::path(filename).parent_path().wstring();
		if (bp)
			BP::destroy(bp);
		bp = BP::create_from_file(filename, no_compile);
		dbs.clear();
		for (auto i = 0; i < bp->module_count(); i++)
			dbs.push_back(bp->module(i)->db());
		if (bp->self_module())
			dbs.push_back(bp->self_module()->db());

		e_base->remove_all_children();

		for (auto i = 0; i < bp->module_count(); i++)
			create_module_entity(bp->module(i));
		for (auto i = 0; i < bp->impt_count(); i++)
			create_import_entity(bp->impt(i));
		for (auto i = 0; i < bp->node_count(); i++)
			create_node_entity(bp->node(i));
		create_exports_entity();
		refresh_exports_entity();

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
		{
			auto e_edit = create_standard_edit(150.f, app.font_atlas_pixel, 1.f);
			auto item = wrap_standard_text(e_edit, true, app.font_atlas_pixel, 1.f, Icon_SEARCH);
			e_add_node_menu->add_child(item);

			add_node_menu_filter = (cEdit*)e_edit->find_component(cH("Edit"));
			add_node_menu_filter->add_changed_listener([](void* c, const wchar_t* text) {
				auto menu = *(Entity**)c;
				for (auto i = 1; i < menu->child_count(); i++)
				{
					auto item = menu->child(i);
					item->visible = text[0] ? (((cText*)item->find_component(cH("Text")))->text().find(text) != std::string::npos) : true;
				}
			}, new_mail_p(e_add_node_menu));
		}
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

					capture.e->reset_add_node_menu_filter();

					capture.e->add_node(capture.u->name(), "", capture.e->add_pos);
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

					editor->reset_add_node_menu_filter();

					popup_input_dialog(editor->entity, L"template", [](void* c, bool ok, const std::wstring& text) {
						auto editor = *(cBPEditor**)c;
						auto bp = editor->bp;
						auto name = w2s(text);

						if (bp->self_module() && bp->self_module()->db()->find_udt(H(name.c_str())))
							editor->add_node(name, "", editor->add_pos);
						else
						{
							if (editor->running)
								editor->add_tip(L"Cannot Add New Template Node While Running");
							else
							{
								auto file = SerializableNode::create_from_xml_file(editor->filename);
								auto n_nodes = file->find_node("nodes");
								auto n_node = n_nodes->new_node("node");
								n_node->new_attr("type", name);
								{
									std::string id;
									for (auto i = 0; i < bp->node_count() + 1; i++)
									{
										id = "node_" + std::to_string(i);
										if (!bp->find_node(id))
											break;
									}
									n_node->new_attr("id", id);
								}
								n_node->new_attr("pos", to_string(editor->add_pos));
								SerializableNode::save_to_xml_file(file, editor->filename);
								SerializableNode::destroy(file);

								editor->load(editor->filename, false);
							}
						}
					}, new_mail_p(editor));
				}
			}, new_mail_p(this));
		}

		sel_type = SelAir;
		selected.n = nullptr;
		dragging_slot = nullptr;
		running = false;
	}

	BP::Node* add_node(const std::string& type_name, const std::string& id, const Vec2f& pos)
	{
		auto n = bp->add_node(type_name, id);
		n->pos = pos;
		create_node_entity(n);
		return n;
	}

	void remove_module(BP::Module* m)
	{
		struct Capture
		{
			BP* bp;
			BP::Module* m;
		}capture;
		capture.bp = bp;
		capture.m = m;
		looper().add_delay_event([](void* c) {
			auto& capture = *(Capture*)c;

			auto m_db = capture.m->db();
			for (auto i = 0; i < capture.bp->node_count(); i++)
			{
				auto n = capture.bp->node(i);
				if (n->udt()->db() == m_db)
				{
					auto e = (Entity*)n->user_data;
					e->parent()->remove_child(e);
				}
			}
			auto e = (Entity*)capture.m->user_data;
			e->parent()->remove_child(e);

			capture.bp->remove_module(capture.m);
		}, new_mail(&capture));
	}

	void remove_node(BP::Node* n)
	{
		looper().add_delay_event([](void* c) {
			auto n = *(BP::Node**)c;
			auto e = (Entity*)n->user_data;
			e->parent()->remove_child(e);
			n->bp()->remove_node(n);
		}, new_mail_p(n));
	}

	void delete_selected()
	{
		switch (sel_type)
		{
		case SelModule:
			if (selected.m->filename() == L"bp.dll")
				add_tip(L"Cannot Remove Self Module");
			else if (selected.m->filename() == L"flame_foundation.dll")
				add_tip(L"Cannot Remove 'foundation' Module");
			else
			{
				std::wstring str;
				auto m_db = selected.m->db();
				for (auto i = 0; i < bp->node_count(); i++)
				{
					auto n = bp->node(i);
					auto udt = n->udt();
					if (udt->db() == m_db)
						str += L"id: " + s2w(n->id()) + L", type: " + s2w(udt->name()) + L"\n";
				}

				struct Capture
				{
					cBPEditor* e;
					BP::Module* m;
				}capture;
				capture.e = this;
				capture.m = selected.m;
				popup_confirm_dialog(entity, L"The node(s):\n" + str + L"will be removed, sure to remove module?", [](void* c, bool yes) {
					auto& capture = *(Capture*)c;

					if (yes)
						capture.e->remove_module(capture.m);
				}, new_mail(&capture));
			}
			break;
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
			bp->update();

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
	bool can_popup_menu;

	float bezier_extent;

	cBP() :
		Component("BP")
	{
		can_popup_menu = false;
	}

	virtual void start() override;

	virtual void update() override;
};

struct cBPModule : Component
{
	cElement* element;
	cEventReceiver* event_receiver;

	cBPEditor* editor;
	BP::Module* m;

	cBPModule() :
		Component("BPModule")
	{
	}

	virtual void start() override
	{
		element = (cElement*)(entity->find_component(cH("Element")));
		event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));

		event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
			auto thiz = *(cBPModule**)c;
			if (is_mouse_down(action, key, true) && key == Mouse_Left)
			{
				thiz->editor->sel_type = cBPEditor::SelModule;
				thiz->editor->selected.m = thiz->m;
			}
		}, new_mail_p(this));
	}

	virtual void update() override
	{
		if (m == editor->selected.m)
			element->frame_thickness = 4.f;
		else
			element->frame_thickness = 0.f;

		m->pos = element->pos;
	}
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

		event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
			auto thiz = *(cBPSlot**)c;
			auto editor = thiz->editor;

			if (is_mouse_down(action, key, true) && key == Mouse_Right)
			{
				editor->sel_type = cBPEditor::SelSlot;
				editor->selected.s = thiz->s;
				popup_menu(editor->e_slot_menu, app.root, pos);
			}
		}, new_mail_p(this));
	}
};

struct cBPImport : Component
{
	cElement* element;
	cEventReceiver* event_receiver;

	cBPEditor* editor;
	BP::Import* i;

	cBPImport() :
		Component("BPImport")
	{
	}

	virtual void start() override
	{
		element = (cElement*)(entity->find_component(cH("Element")));
		event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));

		event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
			auto thiz = *(cBPImport**)c;
			if (is_mouse_down(action, key, true) && key == Mouse_Left)
			{
				thiz->editor->sel_type = cBPEditor::SelImport;
				thiz->editor->selected.i = thiz->i;
			}
		}, new_mail_p(this));
	}

	virtual void update() override
	{
		if (i == editor->selected.i)
			element->frame_thickness = 4.f;
		else
			element->frame_thickness = 0.f;

		i->pos = element->pos;
	}
};

struct cBPExport : Component
{
	cElement* element;

	BP* bp;

	cBPExport() :
		Component("BPExport")
	{
	}

	virtual void start() override
	{
		element = (cElement*)(entity->find_component(cH("Element")));
	}

	virtual void update() override
	{
		bp->expts_node_pos = element->pos;
	}
};

void cBP::start()
{
	element = (cElement*)(entity->find_component(cH("Element")));
	event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
	base_element = (cElement*)(entity->child(0)->find_component(cH("Element")));

	event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
		auto thiz = *(cBP**)c;
		auto editor = thiz->editor;

		if (is_mouse_down(action, key, true) && key == Mouse_Left)
		{
			editor->sel_type = cBPEditor::SelAir;
			editor->selected.n = nullptr;
			auto bp = editor->bp;

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
						auto p1 = e1->global_pos + e1->global_size * 0.5f;
						auto p2 = e2->global_pos + e2->global_size * 0.5f;

						if (distance(pos, bezier_closest_point(pos, p1, p1 + Vec2f(thiz->bezier_extent, 0.f), p2 - Vec2f(thiz->bezier_extent, 0.f), p2, 4, 7)) < 3.f * thiz->element->global_scale)
						{
							editor->sel_type = cBPEditor::SelLink;
							editor->selected.l = input;
						}
					}
				}
			}
		}
		else if (is_mouse_down(action, key, true) && key == Mouse_Right)
			thiz->can_popup_menu = true;
		else if (thiz->can_popup_menu && is_mouse_up(action, key, true) && key == Mouse_Right)
		{
			popup_menu(editor->e_add_node_menu, app.root, pos);
			editor->add_pos = pos - thiz->element->global_pos;
			thiz->can_popup_menu = false;
		}
	}, new_mail_p(this));
}

void cBP::update()
{
	bezier_extent = 50.f * base_element->global_scale;

	auto bp = editor->bp;
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
				if (e1 && e2)
				{
					auto p1 = e1->global_pos + e1->global_size * 0.5f;
					auto p2 = e2->global_pos + e2->global_size * 0.5f;

					std::vector<Vec2f> points;
					path_bezier(points, p1, p1 + Vec2f(bezier_extent, 0.f), p2 - Vec2f(bezier_extent, 0.f), p2);
					element->canvas->stroke(points, editor->selected.l == input ? Vec4c(255, 255, 50, 255) : Vec4c(100, 100, 120, 255), 3.f * base_element->global_scale);
				}
			}
		}
	}
	if (editor->dragging_slot)
	{
		auto e = ((cBPSlot*)editor->dragging_slot->user_data)->element;
		auto p1 = e->global_pos + e->global_size * 0.5f;
		auto p2 = Vec2f(event_receiver->event_dispatcher->mouse_pos);

		std::vector<Vec2f> points;
		path_bezier(points, p1, p1 + Vec2f(editor->dragging_slot->type() == BP::Slot::Output ? bezier_extent : -bezier_extent, 0.f), p2, p2);
		element->canvas->stroke(points, Vec4c(255, 255, 50, 255), 3.f * base_element->global_scale);
	}
}

Entity* cBPEditor::create_module_entity(BP::Module* m)
{
	auto e_module = Entity::create();
	e_base->add_child(e_module);
	m->user_data = e_module;
	{
		auto c_element = cElement::create();
		c_element->pos = m->pos;
		c_element->color = Vec4c(255, 200, 190, 200);
		c_element->frame_color = Vec4c(252, 252, 50, 200);
		e_module->add_component(c_element);

		e_module->add_component(cEventReceiver::create());

		auto c_layout = cLayout::create(LayoutVertical);
		c_layout->fence = 1;
		e_module->add_component(c_layout);

		e_module->add_component(cMoveable::create());

		auto c_module = new_component<cBPModule>();
		c_module->editor = this;
		c_module->m = m;
		e_module->add_component(c_module);
	}
	{
		auto e_content = Entity::create();
		e_module->add_child(e_content);
		{
			auto c_element = cElement::create();
			c_element->inner_padding = Vec4f(8.f);
			e_content->add_component(c_element);

			auto c_layout = cLayout::create(LayoutVertical);
			c_layout->item_padding = 4.f;
			e_content->add_component(c_layout);
		}

		auto e_text_filename = Entity::create();
		e_content->add_child(e_text_filename);
		{
			auto c_element = cElement::create();
			c_element->inner_padding = Vec4f(4.f, 2.f, 4.f, 2.f);
			e_text_filename->add_component(c_element);

			auto c_text = cText::create(app.font_atlas_sdf);
			c_text->set_text(m->filename());
			c_text->sdf_scale = 0.8f;
			e_text_filename->add_component(c_text);
		}

		auto e_text_type = Entity::create();
		e_content->add_child(e_text_type);
		{
			e_text_type->add_component(cElement::create());

			auto c_text = cText::create(app.font_atlas_sdf);
			c_text->set_text(L"module");
			c_text->color = Vec4c(50, 50, 50, 255);
			c_text->sdf_scale = 0.5f;
			e_text_type->add_component(c_text);
		}
	}

	auto e_bring_to_front = Entity::create();
	e_module->add_child(e_bring_to_front);
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

	return e_module;
}

Entity* cBPEditor::create_import_entity(BP::Import* i)
{
	auto e_import = Entity::create();
	e_base->add_child(e_import);
	i->user_data = e_import;
	{
		auto c_element = cElement::create();
		c_element->pos = i->pos;
		c_element->color = Vec4c(190, 255, 200, 200);
		c_element->frame_color = Vec4c(252, 252, 50, 200);
		e_import->add_component(c_element);

		e_import->add_component(cEventReceiver::create());

		auto c_layout = cLayout::create(LayoutVertical);
		c_layout->fence = 1;
		e_import->add_component(c_layout);

		e_import->add_component(cMoveable::create());

		auto c_import = new_component<cBPImport>();
		c_import->editor = this;
		c_import->i = i;
		e_import->add_component(c_import);
	}
	{
		auto e_content = Entity::create();
		e_import->add_child(e_content);
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
			c_text->sdf_scale = 0.8f;
			c_text->set_text(s2w(i->id()));
			e_text_id->add_component(c_text);

			e_text_id->add_component(cEventReceiver::create());

			auto c_edit = cEdit::create();
			c_edit->add_changed_listener([](void* c, const wchar_t* text) {
				(*(BP::Import**)c)->set_id(w2s(text));
			}, new_mail_p(i));
			e_text_id->add_component(c_edit);
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
		}

		auto j = 0;

		auto e_left = Entity::create();
		e_main->add_child(e_left);
		{
			e_left->add_component(cElement::create());

			auto c_aligner = cAligner::create();
			c_aligner->width_policy = SizeGreedy;
			e_left->add_component(c_aligner);

			e_left->add_component(cLayout::create(LayoutVertical));

			for (; j < i->bp()->expt_count(); j++)
			{
				auto expt = i->bp()->expt(j);
				auto slot = expt->slot();
				if (slot->type() == BP::Slot::Output)
					break;

				auto e_title = Entity::create();
				e_left->add_child(e_title);
				{
					e_title->add_component(cElement::create());

					e_title->add_component(cLayout::create(LayoutHorizontal));

					auto e_slot = Entity::create();
					e_title->add_child(e_slot);
					{
						auto c_element = cElement::create();
						auto r = app.font_atlas_sdf->pixel_height * 0.6f;
						c_element->size = r;
						c_element->roundness = r * 0.5f;
						c_element->color = Vec4c(200, 200, 200, 255);
						e_slot->add_component(c_element);

						e_slot->add_component(cEventReceiver::create());

						auto c_slot = new_component<cBPSlot>();
						c_slot->editor = this;
						c_slot->s = slot;
						e_slot->add_component(c_slot);
						slot->user_data = c_slot;
					}

					auto e_text = Entity::create();
					e_title->add_child(e_text);
					{
						e_text->add_component(cElement::create());

						auto c_text = cText::create(app.font_atlas_sdf);
						c_text->sdf_scale = 0.6f;
						c_text->set_text(s2w(expt->alias()));
						e_text->add_component(c_text);
					}
				}
			}
		}

		auto e_right = Entity::create();
		e_main->add_child(e_right);
		{
			e_right->add_component(cElement::create());

			e_right->add_component(cLayout::create(LayoutVertical));

			for (; j < i->bp()->expt_count(); j++)
			{
				auto expt = i->bp()->expt(j);
				auto slot = expt->slot();

				auto e_title = Entity::create();
				e_right->add_child(e_title);
				{
					e_title->add_component(cElement::create());

					auto c_aligner = cAligner::create();
					c_aligner->x_align = AlignxRight;
					e_title->add_component(c_aligner);

					e_title->add_component(cLayout::create(LayoutHorizontal));

					auto e_text = Entity::create();
					e_title->add_child(e_text);
					{
						e_text->add_component(cElement::create());

						auto c_text = cText::create(app.font_atlas_sdf);
						c_text->sdf_scale = 0.6f;
						c_text->set_text(s2w(expt->alias()));
						e_text->add_component(c_text);
					}

					auto e_slot = Entity::create();
					e_title->add_child(e_slot);
					{
						auto c_element = cElement::create();
						auto r = app.font_atlas_sdf->pixel_height * 0.6f;
						c_element->size = r;
						c_element->roundness = r * 0.5f;
						c_element->color = Vec4c(200, 200, 200, 255);
						e_slot->add_component(c_element);

						e_slot->add_component(cEventReceiver::create());

						auto c_slot = new_component<cBPSlot>();
						c_slot->editor = this;
						c_slot->s = slot;
						e_slot->add_component(c_slot);
						slot->user_data = c_slot;
					}
				}
			}
		}
	}

	auto e_bring_to_front = Entity::create();
	e_import->add_child(e_bring_to_front);
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

	return e_import;
}

Entity* cBPEditor::create_exports_entity()
{
	auto e_exports = Entity::create();
	e_base->add_child(e_exports);
	{
		auto c_element = cElement::create();
		c_element->pos = bp->expts_node_pos;
		c_element->color = Vec4c(190, 200, 255, 200);
		c_element->frame_color = Vec4c(252, 252, 50, 200);
		e_exports->add_component(c_element);

		e_exports->add_component(cEventReceiver::create());

		auto c_layout = cLayout::create(LayoutVertical);
		c_layout->fence = 1;
		e_exports->add_component(c_layout);

		e_exports->add_component(cMoveable::create());

		auto c_export = new_component<cBPExport>();
		c_export->bp = bp;
		e_exports->add_component(c_export);
	}
	{
		auto e_content = Entity::create();
		e_exports->add_child(e_content);
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
			c_text->set_text(L"Exports");
			c_text->sdf_scale = 0.8f;
			e_text_id->add_component(c_text);
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
		}
		e_exports_main = e_main;
	}

	auto e_bring_to_front = Entity::create();
	e_exports->add_child(e_bring_to_front);
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

	return e_exports;
}

Entity* cBPEditor::create_export_entity(BP::Export* e)
{
	auto s = e->slot();

	auto e_export = Entity::create();
	e->user_data = e_export;
	{
		auto c_element = cElement::create();
		c_element->inner_padding = Vec4f(0.f, 0.f, 4.f + app.font_atlas_sdf->pixel_height * 0.5f, 0.f);
		e_export->add_component(c_element);

		auto c_text = cText::create(app.font_atlas_sdf);
		c_text->sdf_scale = 0.5f;
		auto out_addr = s->get_address();
		c_text->set_text(s2w(e->alias()) + L" (" + s2w(*out_addr.p) + L")");
		delete_mail(out_addr);
		e_export->add_component(c_text);

		if (s->type() == BP::Slot::Output)
		{
			auto c_aligner = cAligner::create();
			c_aligner->x_align = AlignxRight;
			e_export->add_component(c_aligner);
		}

		e_export->add_component(cLayout::create(LayoutFree));
	}

	auto e_close = Entity::create();
	e_export->add_child(e_close);
	{
		e_close->add_component(cElement::create());

		auto c_text = cText::create(app.font_atlas_sdf);
		c_text->color = Vec4c(200, 40, 20, 255);
		c_text->sdf_scale = 0.5f;
		c_text->set_text(Icon_TIMES);
		e_close->add_component(c_text);

		auto c_event_receiver = cEventReceiver::create();
		e_close->add_component(c_event_receiver);

		auto c_aligner = cAligner::create();
		c_aligner->x_align = AlignxRight;
		e_close->add_component(c_aligner);
	}

	return e_export;
}

void cBPEditor::refresh_exports_entity()
{
	e_exports_main->remove_all_children();

	auto i = 0;

	auto e_left = Entity::create();
	e_exports_main->add_child(e_left);
	{
		e_left->add_component(cElement::create());

		auto c_aligner = cAligner::create();
		c_aligner->width_policy = SizeGreedy;
		e_left->add_component(c_aligner);

		e_left->add_component(cLayout::create(LayoutVertical));

		for (; i < bp->expt_count(); i++)
		{
			auto e = bp->expt(i);
			if (e->slot()->type() == BP::Slot::Output)
				break;

			e_left->add_child(create_export_entity(e));
		}
	}

	auto e_right = Entity::create();
	e_exports_main->add_child(e_right);
	{
		e_right->add_component(cElement::create());

		e_right->add_component(cLayout::create(LayoutVertical));

		for (; i < bp->expt_count(); i++)
		{
			auto e = bp->expt(i);

			e_right->add_child(create_export_entity(e));
		}
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

		auto c_layout = cLayout::create(LayoutVertical);
		c_layout->fence = 1;
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
			c_text->sdf_scale = 0.8f;
			c_text->set_text(s2w(n->id()));
			e_text_id->add_component(c_text);

			e_text_id->add_component(cEventReceiver::create());

			auto c_edit = cEdit::create();
			c_edit->add_changed_listener([](void* c, const wchar_t* text) {
				(*(BP::Node**)c)->set_id(w2s(text));
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

		auto udt_name = n->udt()->name();
		if (udt_name == "DstImage")
		{
			auto e_show = create_standard_button(app.font_atlas_sdf, 0.5f, L"Show");
			e_content->add_child(e_show);
			((cEventReceiver*)e_show->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				if (is_mouse_clicked(action, key))
					open_image_viewer(*(uint*)(*(BP::Node**)c)->find_output("idx")->data(), Vec2f(1495.f, 339.f));
			}, new_mail_p(n));
		}
		else if (udt_name == "graphics::Shader")
		{
			auto e_edit = create_standard_button(app.font_atlas_sdf, 0.5f, L"Edit Shader");
			e_content->add_child(e_edit);

			struct Capture
			{
				cBPEditor* e;
				BP::Node* n;
			}capture;
			capture.e = this;
			capture.n = n;
			((cEventReceiver*)e_edit->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
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

					auto e_back = create_standard_button(app.font_atlas_pixel, 1.f, L"Back");
					e_buttons->add_child(e_back);
					{
						((cEventReceiver*)e_back->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
							if (is_mouse_clicked(action, key))
							{
								auto editor = *(cBPEditor**)c;
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
					auto inputs = (AttributeP<std::vector<void*>>*)capture.n->find_input("inputs")->raw_data();
					auto outputs = (AttributeP < std::vector<void*>>*)capture.n->find_input("outputs")->raw_data();
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
								auto code = get_shader_autogen_code(shader_stage_from_filename(filename), get_attribute_vec(*inputs), get_attribute_vec(*outputs), pll);
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
								((cEventReceiver*)e_compile->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
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
						c_element->roundness = r * 0.5f;
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

						create_enum_checkboxs(info, app.font_atlas_sdf, 0.5f, e_data);
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
							((cCheckbox*)e_data->child(k)->child(0)->find_component(cH("Checkbox")))->add_changed_listener([](void* c, bool checked) {
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

							auto e_checkbox = create_standard_checkbox();
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
						c_element->roundness = r * 0.5f;
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

	return e_node;
}

void open_blueprint_editor(const std::wstring& filename, bool no_compile, const Vec2f& pos)
{
	auto e_container = get_docker_container_model()->copy();
	app.root->add_child(e_container);
	{
		auto c_element = (cElement*)e_container->find_component(cH("Element"));
		c_element->pos = pos;
		c_element->size.x() = 1483.f;
		c_element->size.y() = 711.f;
	}

	auto e_docker = get_docker_model()->copy();
	e_container->add_child(e_docker, 0);

	e_docker->child(0)->add_child(create_standard_docker_tab(app.font_atlas_pixel, L"Blueprint Editor", app.root));

	auto e_page = get_docker_page_model()->copy();
	{
		auto c_layout = cLayout::create(LayoutVertical);
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;
		c_layout->fence = 3;
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
		{
			auto e_item = create_standard_menu_item(app.font_atlas_pixel, 1.f, L"Module");
			e_menu->add_child(e_item);
			((cEventReceiver*)e_item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				auto editor = *(cBPEditor**)c;
				if (is_mouse_clicked(action, key))
				{
					destroy_topmost(app.root);

					if (editor->running)
						editor->add_tip(L"Cannot Add Module While Running");
					else
					{
						popup_input_dialog(editor->entity, L"module", [](void* c, bool ok, const std::wstring& text) {
							auto editor = *(cBPEditor**)c;
							auto bp = editor->bp;

							if (ok)
							{
								auto m = bp->add_module(text);
								if (!m)
									editor->add_tip(L"Add Module Failed");
								else
								{
									m->pos = editor->add_pos;
									editor->create_module_entity(m);
								}
							}
						}, new_mail_p(editor));
					}
				}
			}, new_mail_p(c_editor));
		}
		{
			auto e_item = create_standard_menu_item(app.font_atlas_pixel, 1.f, L"Import");
			e_menu->add_child(e_item);
			((cEventReceiver*)e_item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				auto editor = *(cBPEditor**)c;
				if (is_mouse_clicked(action, key))
				{
					destroy_topmost(app.root);

					if (editor->running)
						editor->add_tip(L"Cannot Add Import While Running");
					else
					{
						popup_input_dialog(editor->entity, L"bp", [](void* c, bool ok, const std::wstring& text) {
							auto editor = *(cBPEditor**)c;
							auto bp = editor->bp;

							if (ok)
							{
								auto i = bp->add_impt(text, "");
								if (!i)
									editor->add_tip(L"Add Import Failed");
								else
								{
									i->pos = editor->add_pos;
									editor->create_import_entity(i);
								}
							}
						}, new_mail_p(editor));
					}
				}
			}, new_mail_p(c_editor));
		}
		{
			auto e_sub_menu = create_standard_menu();
			c_editor->e_add_node_menu = e_sub_menu;
			e_menu->add_child(create_standard_menu_button(app.font_atlas_pixel, 1.f, L"Nodes", app.root, e_sub_menu, true, SideE, false, true, false, Icon_CARET_RIGHT));
		}
		auto e_menu_btn = create_standard_menu_button(app.font_atlas_pixel, 1.f, L"Add", app.root, e_menu, true, SideS, true, false, true, nullptr);
		e_menubar->add_child(e_menu_btn);
		struct Capture
		{
			cBPEditor* e;
			cMenuButton* b;
		}capture;
		capture.e = c_editor;
		capture.b = (cMenuButton*)e_menu_btn->find_component(cH("MenuButton"));
		((cEventReceiver*)e_menu_btn->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
			auto& capture = *(Capture*)c;

			if (capture.b->can_open(action, key))
			{
				auto base = capture.e->e_base;
				capture.e->add_pos = ((cElement*)base->parent()->find_component(cH("Element")))->size * 0.5f - ((cElement*)base->find_component(cH("Element")))->pos;
			}
		}, new_mail(&capture));
	}
	{
		auto e_menu = create_standard_menu();
		{
			auto e_item = create_standard_menu_item(app.font_atlas_pixel, 1.f, L"Delete");
			e_menu->add_child(e_item);
			((cEventReceiver*)e_item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				auto editor = *(cBPEditor**)c;

				if (is_mouse_clicked(action, key))
				{
					destroy_topmost(app.root);

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
				auto editor = *(cBPEditor**)c;

				if (is_mouse_clicked(action, key))
				{
					destroy_topmost(app.root);

					editor->auto_set_layout();
				}
			}, new_mail_p(c_editor));
		}
		auto e_menu_btn = create_standard_menu_button(app.font_atlas_pixel, 1.f, L"Layout", app.root, e_menu, true, SideS, true, false, true, nullptr);
		e_menubar->add_child(e_menu_btn);
	}

	auto e_run = create_standard_button(app.font_atlas_pixel, 1.f, L"Run");;
	e_page->add_child(e_run);
	{
		auto c_event_receiver = (cEventReceiver*)e_run->find_component(cH("EventReceiver"));
		struct Capture
		{
			cBPEditor* e;
			cText* t;
		}capture;
		capture.e = c_editor;
		capture.t = (cText*)e_run->find_component(cH("Text"));
		c_event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
			auto& capture = *(Capture*)c;

			if (is_mouse_clicked(action, key))
			{
				capture.e->running = !capture.e->running;
				capture.t->set_text(capture.e->running ? L"Pause" : L"Run");

				if (capture.e->running)
					capture.e->bp->graphics_device = app.d;
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

		e_scene->add_component(cLayout::create(LayoutFree));
	}
	auto c_bp = new_component<cBP>();
	c_bp->editor = c_editor;
	e_scene->add_component(c_bp);

	auto e_base = Entity::create();
	e_scene->add_child(e_base);
	{
		e_base->add_component(cElement::create());
	}
	c_editor->e_base = e_base;

	{
		auto e_menu = create_standard_menu();
		c_editor->e_slot_menu = e_menu;
		{
			auto item = create_standard_menu_item(app.font_atlas_pixel, 1.f, L"Add To Export");
			e_menu->add_child(item);
			((cEventReceiver*)item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				auto editor = *(cBPEditor**)c;
				if (is_mouse_down(action, key, true) && key == Mouse_Left)
				{
					destroy_topmost(app.root);
					popup_input_dialog(editor->entity, L"alias", [](void* c, bool ok, const std::wstring& text) {
						auto editor = *(cBPEditor**)c;

						if (ok)
						{
							editor->bp->add_expt(editor->selected.s, w2s(text));
							editor->refresh_exports_entity();
						}
					}, new_mail_p(editor));
				}
			}, new_mail_p(c_editor));
		}
	}

	auto e_overlayer = Entity::create();
	e_scene->add_child(e_overlayer);
	{
		e_overlayer->add_component(cElement::create());

		auto c_event_receiver = cEventReceiver::create();
		c_event_receiver->penetrable = true;
		c_event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
			auto c_bp = *(cBP**)c;
			if (is_mouse_scroll(action, key))
			{
				c_bp->base_element->scale += pos.x() > 0.f ? 0.1f : -0.1f;
				c_bp->base_element->scale = clamp(c_bp->base_element->scale, 0.1f, 2.f);
			}
			else if (is_mouse_move(action, key) && (c_bp->event_receiver->event_dispatcher->mouse_buttons[Mouse_Right] & KeyStateDown))
			{
				c_bp->base_element->pos += pos;
				c_bp->can_popup_menu = false;
			}
		}, new_mail_p(c_bp));
		e_overlayer->add_component(c_event_receiver);

		auto c_aligner = cAligner::create();
		c_aligner->width_policy = SizeFitParent;
		c_aligner->height_policy = SizeFitParent;
		e_overlayer->add_component(c_aligner);
	}

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
				auto value_before = serialize_value(dbs, type->tag(), type->hash(), i->raw_data(), 2);
				auto data = new char[v->size()];
				unserialize_value(dbs, type->tag(), type->hash(), value, data);
				i->set_data((char*)data + sizeof(AttributeBase));
				((cBPSlot*)i->user_data)->tracker->update_view();
				delete data;
				auto value_after = serialize_value(dbs, type->tag(), type->hash(), i->raw_data(), 2);
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
						auto str = serialize_value(dbs, type->tag(), type->hash(), input->raw_data(), 2);
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
						auto str = serialize_value(dbs, type->tag(), type->hash(), output->raw_data(), 2);
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
				auto n = editor->add_node(w2s(tokens[2]), tokens[3] == L"-" ? "" : w2s(tokens[3]), Vec2f(0.f));
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
	}, new_mail_p(c_editor), filename + L":", Vec2f(1495.f, 10.f));
	c_editor->console_tab = (cDockerTab*)console_page->parent()->parent()->child(0)->child(0)->find_component(cH("DockerTab"));
}
