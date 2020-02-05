#include <flame/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/device.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/image.h>
#include <flame/graphics/shader.h>
#include <flame/universe/systems/event_dispatcher.h>
#include <flame/universe/systems/2d_renderer.h>
#include <flame/universe/ui/utils.h>

#include "../renderpath/canvas/canvas.h"

#include "../app.h"
#include "../data_tracker.h"
#include "blueprint_editor.h"
#include "console.h"
#include "image_viewer.h"

#include <functional>

namespace flame
{
	struct DstImage$
	{
		AttributeP<Image> img$o;
		AttributeE<TargetType$> type$o;
		AttributeP<Imageview> view$o;

		AttributeD<uint> idx$o;

		__declspec(dllexport) void update$(BP* scene)
		{
			if (img$o.frame == -1)
			{
				if (idx$o.v > 0)
					app.s_2d_renderer->canvas->set_image(idx$o.v, nullptr);
				if (img$o.v)
					Image::destroy(img$o.v);
				if (view$o.v)
					Imageview::destroy(view$o.v);
				auto d = Device::default_one();
				if (d)
				{
					img$o.v = Image::create(d, Format_R8G8B8A8_UNORM, Vec2u(800, 600), 1, 1, SampleCount_1, ImageUsage$(ImageUsageTransferDst | ImageUsageAttachment | ImageUsageSampled));
					(img$o.v)->init(Vec4c(0, 0, 0, 255));
				}
				else
					img$o.v = nullptr;
				type$o.v = TargetImageview;
				type$o.frame = scene->frame;
				if (img$o.v)
				{
					view$o.v = Imageview::create(img$o.v);
					idx$o.v = app.s_2d_renderer->canvas->set_image(-1, view$o.v);
				}
				img$o.frame = scene->frame;
				view$o.frame = scene->frame;
				idx$o.frame = scene->frame;
			}
		}

		__declspec(dllexport) ~DstImage$()
		{
			if (idx$o.v > 0)
				app.s_2d_renderer->canvas->set_image(idx$o.v, nullptr);
			if (img$o.v)
				Image::destroy(img$o.v);
			if (view$o.v)
				Imageview::destroy(view$o.v);
		}
	};

	struct CmdBufs$
	{
		AttributeD<Array<Commandbuffer*>> out$o;

		__declspec(dllexport) void active_update$(BP* scene)
		{
			if (out$o.frame == -1)
			{
				for (auto i = 0; i < out$o.v.s; i++)
					Commandbuffer::destroy(out$o.v[i]);
				auto d = Device::default_one();
				if (d)
				{
					out$o.v.resize(1);
					out$o.v[0] = Commandbuffer::create(d->gcp);
				}
				else
					out$o.v.resize(0);
				out$o.frame = scene->frame;
			}

			app.extra_cbs.push_back(out$o.v[0]);
		}

		__declspec(dllexport) ~CmdBufs$()
		{
			for (auto i = 0; i < out$o.v.s; i++)
				Commandbuffer::destroy(out$o.v[i]);
		}
	};
}

const auto dot_path = s2w(GRAPHVIZ_PATH) + L"/bin/dot.exe";

struct cBPEditor : Component
{
	std::wstring filename;
	std::wstring filepath;
	BP* bp;
	bool changed;
	bool locked;

	cText* tab_text;
	Entity* e_add_node_menu;
	cEdit* add_node_menu_filter;
	Vec2f add_pos;
	Entity* e_base;
	cDockerTab* console_tab;

	enum SelType
	{
		SelAir,
		SelLibrary,
		SelSubGraph,
		SelNode,
		SelSlot,
		SelLink
	}sel_type_;
	union
	{
		void* plain;
		BP::Library* library;
		BP::SubGraph* subgraph;
		BP::Node* node;
		BP::Slot* slot;
		BP::Slot* link;
	}selected_;

	BP::Slot* dragging_slot;

	bool running;

	cBPEditor() :
		Component("BPEditor")
	{
		bp = nullptr;
		changed = false;
		locked = false;

		console_tab = nullptr;
	}

	~cBPEditor()
	{
		BP::destroy(bp);

		if (console_tab)
		{
			looper().add_event([](void* c) {
				auto tab = *(cDockerTab**)c;
				tab->take_away(true);
			}, new_mail_p(console_tab));
		}
	}

	Entity* selected_entity()
	{
		switch (sel_type_)
		{
		case SelLibrary:
			return (Entity*)selected_.library->user_data;
		case SelSubGraph:
			return (Entity*)selected_.subgraph->user_data;
		case SelNode:
			return (Entity*)selected_.node->user_data;
		}
		return nullptr;
	}

	void deselect()
	{
		auto e = selected_entity();
		if (e)
			e->get_component(cElement)->set_frame_thickness(0.f);

		sel_type_ = SelAir;
		selected_.plain = nullptr;
	}

	void select(SelType t, void* p)
	{
		deselect();
		sel_type_ = t;
		selected_.plain = p;

		auto e = selected_entity();
		if (e)
			e->get_component(cElement)->set_frame_thickness(4.f);
	}

	void set_changed(bool v)
	{
		if (changed != v)
		{
			changed = v;
			tab_text->set_text((filename + (changed ? L" *" : L"")).c_str());
		}
	}

	void reset_add_node_menu_filter()
	{
		add_node_menu_filter->text->set_text(L"");

		for (auto i = 1; i < e_add_node_menu->child_count(); i++)
			e_add_node_menu->child(i)->set_visibility(true);
	}

	void refresh_add_node_menu()
	{
		e_add_node_menu->remove_child((Entity*)INVALID_POINTER);

		std::vector<UdtInfo*> all_udts;
		auto add_udt = [&](UdtInfo* u) {
			{
				auto f = only_not_null(u->find_function("update"), u->find_function("active_update"));
				if (!f.first)
					return;
				if (!check_function(f.first, "D#void", {}))
					return;
			}
			auto no_input_output = true;
			for (auto i = 0; i < u->variable_count(); i++)
			{
				auto v = u->variable(i);
				std::string decoration = v->decoration();
				if (decoration.find('i') != std::string::npos || decoration.find('o') != std::string::npos)
				{
					no_input_output = false;
					break;
				}
			}
			if (!no_input_output)
				all_udts.push_back(u);
		};
		for (auto i = 0; i < global_db_count(); i++)
		{
			auto udts = global_db(i)->get_udts();
			for (auto j = 0; j < udts.s; j++)
				add_udt(udts.v[j]);
		}
		for (auto i = 0; i < bp->db_count(); i++)
		{
			auto udts = bp->dbs()[i]->get_udts();
			for (auto j = 0; j < udts.s; j++)
				add_udt(udts.v[j]);
		}
		std::sort(all_udts.begin(), all_udts.end(), [](UdtInfo* a, UdtInfo* b) {
			return std::string(a->type()->name()) < std::string(b->type()->name());
		});
		ui::push_parent(e_add_node_menu);
		ui::e_begin_layout(LayoutHorizontal, 4.f);
		ui::e_text(Icon_SEARCH);
		ui::e_edit(150.f)->get_component(cText)->data_changed_listeners.add([](void* c, Component* t, uint hash, void*) {
			auto menu = *(Entity**)c;
			auto str = ((cText*)t)->text();
			for (auto i = 1; i < menu->child_count(); i++)
			{
				auto item = menu->child(i);
				item->set_visibility(str[0] ? (std::wstring(item->get_component(cText)->text()).find(str) != std::string::npos) : true);
			}
		}, new_mail_p(e_add_node_menu));
		add_node_menu_filter = ui::current_entity()->get_component(cEdit);
		ui::e_end_layout();
		ui::e_menu_item(L"Enum", [](void* c) {
		}, new_mail_p(this));
		ui::e_menu_item(L"Var", [](void* c) {
		}, new_mail_p(this));
		ui::e_menu_item(L"Array", [](void* c) {
		}, new_mail_p(this));
		for (auto udt : all_udts)
		{
			struct Capture
			{
				cBPEditor* e;
				UdtInfo* u;
			}capture;
			capture.e = this;
			capture.u = udt;
			ui::e_menu_item(s2w(udt->type()->name()).c_str(), [](void* c) {
				auto& capture = *(Capture*)c;
				capture.e->reset_add_node_menu_filter();
				capture.e->add_node(capture.u->type()->name(), "", capture.e->add_pos);
			}, new_mail(&capture));
		}
		ui::pop_parent();
	}

	Entity* create_library_entity(BP::Library* m);
	Entity* create_node_entity(BP::Node* n);
	Entity* create_subgraph_entity(BP::SubGraph* p);

	void link_test_nodes()
	{
		auto n_dst = bp->find_node("test_dst");
		if (!n_dst)
		{
			n_dst = bp->add_node("D#DstImage", "test_dst");
			n_dst->pos = Vec2f(0.f, -200.f);
			n_dst->used_by_editor = true;
		}
		{
			auto s = bp->find_input("*.rt_dst.type");
			if (s)
				s->link_to(n_dst->find_output("type"));
		}
		{
			auto s = bp->find_input("*.rt_dst.v");
			if (s)
				s->link_to(n_dst->find_output("view"));
		}
		auto n_cbs = bp->find_node("test_cbs");
		if (!n_cbs)
		{
			n_cbs = bp->add_node("D#CmdBufs", "test_cbs");
			n_cbs->pos = Vec2f(200.f, -200.f);
			n_cbs->used_by_editor = true;
		}
		{
			auto s = bp->find_input("*.make_cmd.cbs");
			if (s)
				s->link_to(n_cbs->find_output("out"));
		}
	}

	void load(const std::wstring& _filename)
	{
		filename = _filename;
		filepath = std::filesystem::path(filename).parent_path().wstring();
		if (bp)
			BP::destroy(bp);
		bp = BP::create_from_file(filename.c_str());
		link_test_nodes();

		refresh_add_node_menu();

		e_base->remove_child((Entity*)INVALID_POINTER);

		for (auto i = 0; i < bp->library_count(); i++)
			create_library_entity(bp->library(i));
		for (auto i = 0; i < bp->subgraph_count(); i++)
			create_subgraph_entity(bp->subgraph(i));
		for (auto i = 0; i < bp->node_count(); i++)
			create_node_entity(bp->node(i));

		set_changed(false);

		sel_type_ = SelAir;
		selected_.plain = nullptr;
		dragging_slot = nullptr;
		running = false;
	}

	void set_add_pos_center()
	{
		add_pos = e_base->parent()->get_component(cElement)->size_ * 0.5f - e_base->get_component(cElement)->pos_;
	}

	BP::Node* add_node(const std::string& type_name, const std::string& id, const Vec2f& pos)
	{
		auto n = bp->add_node(type_name.c_str(), id.c_str());
		n->pos = pos;
		create_node_entity(n);
		set_changed(true);
		return n;
	}

	void remove_library(BP::Library* l)
	{
		struct Capture
		{
			cBPEditor* e;
			BP::Library* l;
		}capture;
		capture.e = this;
		capture.l = l;
		looper().add_event([](void* c) {
			auto& capture = *(Capture*)c;
			auto bp = capture.e->bp;

			auto m_db = capture.l->db();
			for (auto i = 0; i < bp->node_count(); i++)
			{
				auto n = bp->node(i);
				auto udt = n->udt();
				if (udt && udt->db() == m_db)
				{
					auto e = (Entity*)n->user_data;
					e->parent()->remove_child(e);
				}
			}
			auto e = (Entity*)capture.l->user_data;
			e->parent()->remove_child(e);

			bp->remove_library(capture.l);
			capture.e->refresh_add_node_menu();
			capture.e->set_changed(true);
		}, new_mail(&capture));
	}

	void remove_subgraph(BP::SubGraph* s)
	{
		struct Capture
		{
			cBPEditor* e;
			BP::SubGraph* s;
		}capture;
		capture.e = this;
		capture.s = s;
		looper().add_event([](void* c) {
			auto& capture = *(Capture*)c;
			auto e = (Entity*)capture.s->user_data;
			e->parent()->remove_child(e);
			capture.e->bp->remove_subgraph(capture.s);
			capture.e->refresh_add_node_menu();
			capture.e->set_changed(true);
		}, new_mail(&capture));
	}

	bool remove_node(BP::Node* n)
	{
		if (selected_.node->id() == "test_dst" || selected_.node->id() == "test_cbs")
			return false;
		looper().add_event([](void* c) {
			auto n = *(BP::Node**)c;
			auto e = (Entity*)n->user_data;
			e->parent()->remove_child(e);
			n->scene()->remove_node(n);
		}, new_mail_p(n));
		set_changed(true);
		return true;
	}

	void duplicate_selected()
	{
		switch (sel_type_)
		{
		case SelNode:
		{
			auto n = bp->add_node(selected_.node->type_name(), "");
			n->pos = add_pos;
			for (auto i = 0; i < n->input_count(); i++)
			{
				auto src = selected_.node->input(i);
				auto dst = n->input(i);
				dst->set_data(src->data());
				dst->link_to(src->link(0));
			}
			create_node_entity(n);
			set_changed(true);
		}
			break;
		}
	}

	void delete_selected()
	{
		switch (sel_type_)
		{
		case SelLibrary:
		{
			std::wstring str;
			auto l_db = selected_.library->db();
			for (auto i = 0; i < bp->node_count(); i++)
			{
				auto n = bp->node(i);
				auto udt = n->udt();
				if (udt && udt->db() == l_db)
					str += L"id: " + s2w(n->id()) + L", type: " + s2w(n->type_name()) + L"\n";
			}

			struct Capture
			{
				cBPEditor* e;
				BP::Library* l;
			}capture;
			capture.e = this;
			capture.l = selected_.library;
			ui::e_confirm_dialog((L"The node(s):\n" + str + L"will be removed, sure to remove the library?").c_str(), [](void* c, bool yes) {
				auto& capture = *(Capture*)c;

				if (yes)
				{
					capture.e->remove_library(capture.l);
					capture.e->set_changed(true);
				}
			}, new_mail(&capture));
		}
			break;
		case SelSubGraph:
			break;
		case SelNode:
			if (!remove_node(selected_.node))
				ui::e_message_dialog(L"Cannot Remove Test Nodes");
			break;
		case SelLink:
			selected_.link->link_to(nullptr);
			set_changed(true);
			break;
		}

		deselect();
	}

	void update_gv()
	{
		auto gv_filename = filepath + L"/bp.gv";
		if (!std::filesystem::exists(gv_filename) || std::filesystem::last_write_time(gv_filename) < std::filesystem::last_write_time(filename))
		{
			if (!GRAPHVIZ_PATH[0])
				assert(0);

			std::string gv = "digraph bp {\nrankdir=LR\nnode [shape = Mrecord];\n";
			for (auto i = 0; i < bp->node_count(); i++)
			{
				auto n = bp->node(i);
				auto name = n->id();
				auto str = sfmt("\t%s[label = \"%s|%s|{{", name, name, n->type_name());
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
						auto in_sp = ssplit(input->get_address().str(), '.');
						auto out_sp = ssplit(input->link()->get_address().str(), '.');

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

	bool generate_graph_image()
	{
		update_gv();
		auto png_filename = filepath + L"/bp.png";
		if (!std::filesystem::exists(png_filename) || std::filesystem::last_write_time(png_filename) < std::filesystem::last_write_time(filename))
			exec(dot_path.c_str(), (wchar_t*)(L"-Tpng " + filepath + L"/bp.gv -y -o " + png_filename).c_str(), true);

		return std::filesystem::exists(png_filename);
	}

	bool auto_set_layout()
	{
		update_gv();
		auto graph_filename = filepath + L"/bp.graph";
		if (!std::filesystem::exists(graph_filename) || std::filesystem::last_write_time(graph_filename) < std::filesystem::last_write_time(filename))
			exec(dot_path.c_str(), (wchar_t*)(L"-Tplain" + filepath + L"/bp.gv -y -o " + graph_filename).c_str(), true);
		if (!std::filesystem::exists(graph_filename))
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
				((Entity*)n->user_data)->get_component(cElement)->set_pos(n->pos);
			}

			str = match.suffix();
		}

		return true;
	}

	void on_component_added(Component* c) override
	{
		if (c->name_hash == FLAME_CHASH("cElement"))
		{
			((cElement*)c)->cmds.add([](void* c, graphics::Canvas* canvas) {
				(*(cBPEditor**)c)->draw(canvas);
			}, new_mail_p(this));
		}
	}

	void draw(graphics::Canvas* canvas)
	{
		if (running)
			bp->update();
	}
};

struct cBPScene : Component
{
	cElement* element;
	cEventReceiver* event_receiver;
	cElement* base_element;

	cText* scale_text;
	cBPEditor* editor;

	float link_stick_out;

	cBPScene() :
		Component("cBPScene")
	{
	}

	void for_each_link(const std::function<bool(const Vec2f&p1, const Vec2f & p2, BP::Slot* input)>& l);
	void on_component_added(Component* c) override;
	void draw(graphics::Canvas* canvas);
};

struct cBPObject : Component
{
	cElement* element;
	cEventReceiver* event_receiver;

	cBPEditor* editor;
	cBPEditor::SelType t;
	void* p;
	bool moved;

	cBPObject() :
		Component("cBPObject")
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
					auto thiz = *(cBPObject**)c;
					auto pos = ((cElement*)e)->pos_;
					switch (thiz->t)
					{
					case cBPEditor::SelLibrary:
						((BP::Library*)thiz->p)->pos = pos;
						break;
					case cBPEditor::SelSubGraph:
						((BP::SubGraph*)thiz->p)->pos = pos;
						break;
					case cBPEditor::SelNode:
						((BP::Node*)thiz->p)->pos = pos;
						break;
					}
					thiz->moved = true;
				}
			}, new_mail_p(this));
		}
		else if (c->name_hash == FLAME_CHASH("cEventReceiver"))
		{
			event_receiver = (cEventReceiver*)c;
			event_receiver->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
				auto thiz = *(cBPObject**)c;
				if (is_mouse_down(action, key, true) && key == Mouse_Left)
					thiz->editor->select(thiz->t, thiz->p);
			}, new_mail_p(this));
			event_receiver->data_changed_listeners.add([](void* c, Component* er, uint hash, void*) {
				auto thiz = *(cBPObject**)c;
				if (hash == FLAME_CHASH("state"))
				{
					switch (((cEventReceiver*)er)->state)
					{
					case EventReceiverActive:
						thiz->moved = false;
						break;
					default:
						if (thiz->moved)
						{
							thiz->editor->set_changed(true);
							thiz->moved = false;
						}
					}
				}
			}, new_mail_p(this));
		}
	}
};

struct cBPSlot : Component
{
	cElement* element;
	cText* text;
	cEventReceiver* event_receiver;
	cDataTracker* tracker;

	cBPEditor* editor;
	BP::Slot* s;

	cBPSlot() :
		Component("cBPSlot")
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
				auto thiz = *(cBPSlot**)c;
				auto editor = thiz->editor;
				auto s = thiz->s;
				if (action == DragStart)
				{
					editor->dragging_slot = s;
					if (s->io() == BP::Slot::In)
					{
						s->link_to(nullptr);
						editor->set_changed(true);
					}
				}
				else if (action == DragEnd)
					editor->dragging_slot = nullptr;
				else if (action == Dropped)
				{
					auto oth = er->entity->get_component(cBPSlot)->s;
					if (s->io() == BP::Slot::In)
					{
						if (s->link_to(oth))
							editor->set_changed(true);
					}
					else
					{
						if (oth->link_to(thiz->s))
							editor->set_changed(true);
					}
				}
			}, new_mail_p(this));
		}
	}
};

void cBPScene::for_each_link(const std::function<bool(const Vec2f & p1, const Vec2f & p2, BP::Slot* l)>& callback)
{
	auto bp = editor->bp;
	const auto process = [&](BP::Slot* output, BP::Slot* input) {
		if (!output->user_data && !input->user_data)
			return true;

		auto get_pos = [&](BP::Slot* s) {
			if (s->user_data)
			{
				auto e = ((cBPSlot*)s->user_data)->element;
				return e->global_pos + e->global_size * 0.5f;
			}
			else
			{
				auto scn = s->node()->scene();
				BP::SubGraph* sg;
				while (scn != bp)
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

	for (auto i = 0; i < bp->subgraph_count(); i++)
	{
		auto s = bp->subgraph(i);
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
	for (auto i = 0; i < bp->node_count(); i++)
	{
		auto n = bp->node(i);
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

void cBPScene::on_component_added(Component* c)
{
	if (c->name_hash == FLAME_CHASH("cElement"))
	{
		element = (cElement*)c;
		element->cmds.add([](void* c, graphics::Canvas* canvas) {
			(*(cBPScene**)c)->draw(canvas);
		}, new_mail_p(this));
	}
	else if (c->name_hash == FLAME_CHASH("cEventReceiver"))
	{
		event_receiver = (cEventReceiver*)c;
		event_receiver->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& _pos) {
			auto thiz = *(cBPScene**)c;
			auto editor = thiz->editor;
			auto pos = (Vec2f)_pos;
			auto line_width = 3.f * thiz->base_element->global_scale;

			if (is_mouse_down(action, key, true) && key == Mouse_Left)
			{
				editor->deselect();

				thiz->for_each_link([&](const Vec2f& p1, const Vec2f& p2, BP::Slot* l) {
					if (segment_distance(p1, p2, pos) < line_width)
					{
						editor->select(cBPEditor::SelLink, l);
						return false;
					}
					return true;
				});
			}
		}, new_mail_p(this));
	}
}

void cBPScene::draw(graphics::Canvas* canvas)
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
			canvas->stroke(points.size(), points.data(), editor->selected_.link == l ? Vec4c(255, 255, 50, 255) : Vec4c(100, 100, 120, 255), line_width);
		}
		return true;
	});
	if (editor->dragging_slot)
	{
		auto e = ((cBPSlot*)editor->dragging_slot->user_data)->element;

		std::vector<Vec2f> points;
		points.push_back(e->global_pos + e->global_size * 0.5f);
		points.push_back(Vec2f(event_receiver->dispatcher->mouse_pos));
		canvas->stroke(points.size(), points.data(), Vec4c(255, 255, 50, 255), line_width);
	}
}

Entity* cBPEditor::create_library_entity(BP::Library* l)
{
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
		auto c_module = new_u_object<cBPObject>();
		c_module->editor = this;
		c_module->t = SelLibrary;
		c_module->p = l;
		e_library->add_component(c_module);
	}
	ui::push_parent(e_library);
	ui::e_begin_layout(LayoutVertical, 4.f)->get_component(cElement)->inner_padding_ = Vec4f(8.f);
	ui::e_text(l->filename())->get_component(cElement)->inner_padding_ = Vec4f(4.f, 2.f, 4.f, 2.f);
	ui::e_text(L"library")->get_component(cText)->color = Vec4c(50, 50, 50, 255);
	ui::e_end_layout();
	ui::e_empty();
	ui::c_element();
	ui::c_event_receiver()->pass = (Entity*)INVALID_POINTER;
	ui::c_aligner(SizeFitParent, SizeFitParent);
	ui::c_bring_to_front();
	ui::pop_parent();

	ui::pop_parent();

	return e_library;
}

Entity* cBPEditor::create_subgraph_entity(BP::SubGraph* s)
{
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
		auto c_package = new_u_object<cBPObject>();
		c_package->editor = this;
		c_package->t = SelSubGraph;
		c_package->p = s;
		e_subgraph->add_component(c_package);
	}
	ui::push_parent(e_subgraph);
	ui::e_begin_layout(LayoutVertical, 4.f)->get_component(cElement)->inner_padding_ = Vec4f(8.f);
	ui::e_text(s2w(s->id()).c_str())->get_component(cElement)->inner_padding_ = Vec4f(4.f, 2.f, 4.f, 2.f);
	ui::c_event_receiver();
	ui::c_edit();
	ui::current_entity()->get_component(cText)->data_changed_listeners.add([](void* c, Component* t, uint hash, void*) {
		if (hash == FLAME_CHASH("text"))
			(*(BP::SubGraph**)c)->set_id(w2s(((cText*)t)->text()).c_str());
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
			auto c_slot = new_u_object<cBPSlot>();
			c_slot->editor = this;
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
			auto c_slot = new_u_object<cBPSlot>();
			c_slot->editor = this;
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
	ui::c_event_receiver()->pass = (Entity*)INVALID_POINTER;
	ui::c_aligner(SizeFitParent, SizeFitParent);
	ui::c_bring_to_front();
	ui::pop_parent();

	ui::pop_parent();

	return e_subgraph;
}

template <class T>
void create_edit(cBPEditor* editor, BP::Slot* input)
{
	auto& data = *(T*)input->data();

	ui::push_style_1u(ui::FontSize, 12);
	auto e_edit = create_drag_edit(std::is_floating_point<T>::value);
	struct Capture
	{
		cBPEditor* editor;
		BP::Slot* input;
		cText* drag_text;
	}capture;
	capture.editor = editor;
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
			capture.editor->set_changed(true);
		}
	}, new_mail(&capture));
	ui::pop_style(ui::FontSize);

	auto c_tracker = new_u_object<cDigitalDataTracker<T>>();
	c_tracker->data = &data;
	ui::current_parent()->add_component(c_tracker);
}

template <uint N, class T>
void create_vec_edit(cBPEditor* editor, BP::Slot* input)
{
	auto& data = *(Vec<N, T>*)input->data();

	struct Capture
	{
		cBPEditor* editor;
		BP::Slot* input;
		uint i;
		cText* drag_text;
	}capture;
	capture.editor = editor;
	capture.input = input;
	ui::push_style_1u(ui::FontSize, 12);
	for (auto i = 0; i < N; i++)
	{
		ui::e_begin_layout(LayoutHorizontal, 4.f);
		auto e_edit = create_drag_edit(std::is_floating_point<T>::value);
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
				capture.editor->set_changed(true);
			}
		}, new_mail(&capture));
		ui::e_text(s2w(Vec<N, T>::coord_name(i)).c_str());
		ui::e_end_layout();
	}
	ui::pop_style(ui::FontSize);

	auto c_tracker = new_u_object<cDigitalVecDataTracker<N, T>>();
	c_tracker->data = &data;
	ui::current_parent()->add_component(c_tracker);
}

Entity* cBPEditor::create_node_entity(BP::Node* n)
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
		auto c_node = new_u_object<cBPObject>();
		c_node->editor = this;
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
	}, new_mail_p(n));
	ui::pop_style(ui::FontSize);

	auto udt = n->udt();
	if (udt)
	{
		auto module_name = std::filesystem::path(udt->db()->module_name());
		module_name = module_name.lexically_relative(std::filesystem::canonical(std::filesystem::path(filename).parent_path()));
		ui::e_text((module_name.wstring() + L"\n" + s2w(n->type_name())).c_str())->get_component(cText)->color = Vec4c(50, 50, 50, 255);
	}
	std::string udt_name = n->type_name();
	if (n->id() == "test_dst")
	{
		ui::e_button(L"Show", [](void* c) {
			open_image_viewer(*(uint*)(*(BP::Node**)c)->find_output("idx")->data(), Vec2f(1495.f, 339.f));
		}, new_mail_p(n));
	}
	//else if (udt_name == "D#graphics::Shader")
	//{
	//	auto e_edit = create_standard_button(app.font_atlas_pixel, 0.9f, L"Edit Shader");
	//	e_content->add_child(e_edit);

	//	struct Capture
	//	{
	//		cBPEditor* e;
	//		BP::Node* n;
	//	}capture;
	//	capture.e = this;
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
	//						auto editor = *(cBPEditor**)c;
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
	//						cBPEditor* e;
	//						BP::Node* n;
	//						cText* t;
	//					}_capture;
	//					_capture.e = capture.e;
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
			if (bp->find_output_export(input) != -1)
				c_text->set_text(L"  p");
			ui::pop_style(ui::FontSize);
			auto c_slot = new_u_object<cBPSlot>();
			c_slot->editor = this;
			c_slot->s = input;
			c_slot->text = c_text;
			ui::current_entity()->add_component(c_slot);
			input->user_data = c_slot;
			ui::e_begin_popup_menu(false);
				struct Capture
				{
					cBPEditor* editor;
					BP::Slot* s;
				}capture;
				capture.editor = this;
				capture.s = input;
				ui::e_menu_item(L"Add To Exports", [](void* c) {
					auto& capture = *(Capture*)c;
					capture.editor->bp->add_input_export(capture.s);
					capture.editor->set_changed(true);
					((cBPSlot*)capture.s->user_data)->text->set_text(L"  p");
				}, new_mail(&capture));
				ui::e_menu_item(L"Remove From Exports", [](void* c) {
					auto& capture = *(Capture*)c;
					capture.editor->bp->remove_input_export(capture.s);
					capture.editor->set_changed(true);
					((cBPSlot*)capture.s->user_data)->text->set_text(L"");
				}, new_mail(&capture));
			ui::e_end_popup_menu();

			ui::e_text(s2w(input->name()).c_str());
		ui::e_end_layout();

		auto e_data = ui::e_begin_layout(LayoutVertical, 2.f);
		e_data->get_component(cElement)->inner_padding_ = Vec4f(ui::style_1u(ui::FontSize), 0.f, 0.f, 0.f);
		extra_global_db_count = bp->db_count();
		extra_global_dbs = bp->dbs();
		auto type = input->type();
		auto base_hash = type->base_hash();
		ui::push_style_1u(ui::FontSize, 12);
		switch (type->tag())
		{
		case TypeEnumSingle:
		{
			auto info = find_enum(base_hash);
			create_enum_combobox(info, 120.f);

			struct Capture
			{
				cBPEditor* editor;
				BP::Slot* input;
				EnumInfo* e;
			}capture;
			capture.editor = this;
			capture.input = input;
			capture.e = info;
			e_data->child(0)->get_component(cCombobox)->data_changed_listeners.add([](void* c, Component* cb, uint hash, void*) {
				auto& capture = *(Capture*)c;
				if (hash == FLAME_CHASH("index"))
				{
					auto v = capture.e->item(((cCombobox*)cb)->idx)->value();
					capture.input->set_data(&v);
					capture.editor->set_changed(true);
				}
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
			create_enum_checkboxs(info);
			for (auto k = 0; k < info->item_count(); k++)
			{
				auto item = info->item(k);

				struct Capture
				{
					cBPEditor* editor;
					BP::Slot* input;
					int v;
				}capture;
				capture.editor = this;
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
						capture.editor->set_changed(true);
					}
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
					cBPEditor* editor;
					BP::Slot* input;
				}capture;
				capture.editor = this;
				capture.input = input;
				e_checkbox->get_component(cCheckbox)->data_changed_listeners.add([](void* c, Component* cb, uint hash, void*) {
					auto& capture = *(Capture*)c;
					if (hash == FLAME_CHASH("checked"))
					{
						auto v = (((cCheckbox*)cb)->checked) ? 1 : 0;
						capture.input->set_data(&v);
						capture.editor->set_changed(true);
					}
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
				struct Capture
				{
					cBPEditor* editor;
					BP::Slot* input;
				}capture;
				capture.editor = this;
				capture.input = input;
				e_edit->get_component(cText)->data_changed_listeners.add([](void* c, Component* t, uint hash, void*) {
					auto& capture = *(Capture*)c;
					if (hash == FLAME_CHASH("text"))
					{
						capture.input->set_data(&StringA(w2s(((cText*)t)->text())));
						capture.editor->set_changed(true);
					}
				}, new_mail(&capture));

				auto c_tracker = new_u_object<cStringADataTracker>();
				c_tracker->data = input->data();
				e_data->add_component(c_tracker);
			}
			break;
			case FLAME_CHASH("StringW"):
			{
				auto e_edit = ui::e_edit(50.f);
				struct Capture
				{
					cBPEditor* editor;
					BP::Slot* input;
				}capture;
				capture.editor = this;
				capture.input = input;
				e_edit->get_component(cText)->data_changed_listeners.add([](void* c, Component* t, uint hash, void*) {
					auto& capture = *(Capture*)c;
					if (hash == FLAME_CHASH("text"))
					{
						capture.input->set_data(&StringW(((cText*)t)->text()));
						capture.editor->set_changed(true);
					}
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
			if (bp->find_output_export(output) != -1)
				c_text->set_text(L"  p");
			ui::pop_style(ui::FontSize);
			auto c_slot = new_u_object<cBPSlot>();
			c_slot->editor = this;
			c_slot->s = output;
			c_slot->text = c_text;
			ui::current_entity()->add_component(c_slot);
			output->user_data = c_slot;
			ui::e_begin_popup_menu(false);
			struct Capture
			{
				cBPEditor* editor;
				BP::Slot* s;
			}capture;
			capture.editor = this;
			capture.s = output;
				ui::e_menu_item(L"Add To Exports", [](void* c) {
					auto& capture = *(Capture*)c;
					capture.editor->bp->add_output_export(capture.s);
					capture.editor->set_changed(true);
					((cBPSlot*)capture.s->user_data)->text->set_text(L"  p");
				}, new_mail(&capture));
				ui::e_menu_item(L"Remove From Exports", [](void* c) {
					auto& capture = *(Capture*)c;
					capture.editor->bp->remove_output_export(capture.s);
					capture.editor->set_changed(true);
					((cBPSlot*)capture.s->user_data)->text->set_text(L"");
				}, new_mail(&capture));
			ui::e_end_popup_menu();
		ui::e_end_layout();
	}
	ui::e_end_layout();

	ui::e_end_layout();
	ui::e_end_layout();
	ui::e_empty();
	ui::c_element();
	ui::c_event_receiver()->pass = (Entity*)INVALID_POINTER;
	ui::c_aligner(SizeFitParent, SizeFitParent);
	ui::c_bring_to_front();
	ui::pop_parent();

	ui::pop_parent();

	return e_node;
}

void open_blueprint_editor(const std::wstring& filename, const Vec2f& pos)
{
	ui::push_parent(app.root);
	ui::next_element_pos = pos;
	ui::next_element_size = Vec2f(1900.f, 711.f);
	ui::e_begin_docker_floating_container();
	ui::e_begin_docker();
	auto e_tab = ui::e_begin_docker_page(filename.c_str()).tab;
	{
		auto c_layout = ui::c_layout(LayoutVertical);
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;
		c_layout->fence = 2;
	}
	auto c_editor = new_u_object<cBPEditor>();
	ui::current_entity()->add_component(c_editor);
	c_editor->tab_text = e_tab->get_component(cText);

	auto e_menubar = ui::e_begin_menu_bar();
	ui::e_begin_menubar_menu(L"Blueprint");
	ui::e_menu_item(L"Save", [](void* c) {
		auto editor = *(cBPEditor**)c;
		BP::save_to_file(editor->bp, editor->filename.c_str());
		editor->set_changed(false);
	}, new_mail_p(c_editor));
	ui::e_end_menubar_menu();
	auto menu_add = ui::e_begin_menubar_menu(L"Add");
	{
		struct Capture
		{
			cBPEditor* e;
			cMenu* m;
		}capture;
		capture.e = c_editor;
		capture.m = menu_add->get_component(cMenu);
		menu_add->get_component(cEventReceiver)->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
			auto& capture = *(Capture*)c;

			if (ui::is_menu_can_open(capture.m, action, key))
			{
				auto base = capture.e->e_base;
				capture.e->set_add_pos_center();
			}
		}, new_mail(&capture));
	}
		ui::e_menu_item(L"Library", [](void* c) {
			auto editor = *(cBPEditor**)c;
			ui::e_input_dialog(L"library", [](void* c, bool ok, const wchar_t* text) {
				auto editor = *(cBPEditor**)c;
				auto bp = editor->bp;

				if (ok && text[0])
				{
					auto l = bp->add_library(text);
					if (!l)
						ui::e_message_dialog(L"Add Library Failed");
					else
					{
						l->pos = editor->add_pos;
						editor->create_library_entity(l);
						editor->refresh_add_node_menu();
						editor->set_changed(true);
					}
				}
			}, new_mail_p(editor));
		}, new_mail_p(c_editor));
		ui::e_menu_item(L"Sub Graph", [](void* c) {
			auto editor = *(cBPEditor**)c;
			ui::e_input_dialog(L"bp", [](void* c, bool ok, const wchar_t* text) {
				auto editor = *(cBPEditor**)c;
				auto bp = editor->bp;

				if (ok && text[0])
				{
					auto s = bp->add_subgraph(text, "");
					if (!s)
						ui::e_message_dialog(L"Add Sub Graph Failed");
					else
					{
						s->pos = editor->add_pos;
						editor->create_subgraph_entity(s);
						editor->refresh_add_node_menu();
						editor->set_changed(true);
					}
				}
			}, new_mail_p(editor));
		}, new_mail_p(c_editor));
		c_editor->e_add_node_menu = ui::e_begin_sub_menu(L"Nodes")->get_component(cMenu)->items;
		ui::e_end_sub_menu();
	ui::e_end_menubar_menu();
	ui::e_begin_menubar_menu(L"Edit");
	ui::e_menu_item(L"Duplicate", [](void* c) {
		auto editor = *(cBPEditor**)c;
		editor->set_add_pos_center();
		editor->duplicate_selected();
	}, new_mail_p(c_editor));
	ui::e_menu_item(L"Delete", [](void* c) {
		auto editor = *(cBPEditor**)c;
		editor->delete_selected();
	}, new_mail_p(c_editor));
	ui::e_end_menubar_menu();
	ui::e_begin_menubar_menu(L"Tools");
	ui::e_menu_item(L"Generate Graph Image", [](void* c) {
		auto editor = *(cBPEditor**)c;
		editor->generate_graph_image();
	}, new_mail_p(c_editor));
	ui::e_menu_item(L"Auto Set Layout", [](void* c) {
		auto editor = *(cBPEditor**)c;
		editor->auto_set_layout();
	}, new_mail_p(c_editor));
	ui::e_menu_item(L"Link Test Nodes", [](void* c) {
		auto editor = *(cBPEditor**)c;
		editor->link_test_nodes();
	}, new_mail_p(c_editor));
	ui::e_end_menubar_menu();
	ui::e_begin_menubar_menu(L"Window");
	ui::e_menu_item(L"Console", [](void* c) {
		auto editor = *(cBPEditor**)c;
		if (!editor->console_tab)
		{

			auto console_page = open_console([](void* c, const std::wstring& cmd, cConsole* console) {
				auto editor = *(cBPEditor**)c;
				auto& filename = editor->filename;
				auto bp = editor->bp;
				auto tokens = ssplit(cmd);

				if (editor->locked)
				{
					console->print(L"bp is locked");
					return;
				}

				extra_global_db_count = bp->db_count();
				extra_global_dbs = bp->dbs();

				auto set_data = [&](const std::string& address, const std::string& value) {
					auto i = bp->find_input(address.c_str());
					if (i)
					{
						auto type = i->type();
						auto value_before = type->serialize(i->raw_data(), 2);
						auto data = new char[i->size()];
						type->unserialize(value, data);
						i->set_data((char*)data + sizeof(AttributeBase));
						((cBPSlot*)i->user_data)->tracker->update_view();
						delete[] data;
						auto value_after = type->serialize(i->raw_data(), 2);
						console->print(L"set value: " + s2w(address) + L", " + s2w(value_before) + L" -> " + s2w(value_after));
						editor->set_changed(true);
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
						for (auto i = 0; i < global_db_count(); i++)
						{
							auto udts = global_db(i)->get_udts();
							for (auto i = 0; i < udts.s; i++)
								all_udts.push_back(udts.v[i]);
						}
						for (auto i = 0; i < bp->db_count(); i++)
						{
							auto udts = bp->dbs()[i]->get_udts();
							for (auto i = 0; i < udts.s; i++)
								all_udts.push_back(udts.v[i]);
						}
						std::sort(all_udts.begin(), all_udts.end(), [](UdtInfo* a, UdtInfo* b) {
							return std::string(a->type()->name()) < std::string(b->type()->name());
						});
						for (auto udt : all_udts)
							console->print(s2w(udt->type()->name()));
					}
					else if (tokens[1] == L"udt")
					{
						auto udt = find_udt(FLAME_HASH(w2s(tokens[2]).c_str()));
						if (udt)
						{
							console->print(s2w(udt->type()->name()));
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
								console->print(wsfmt(L"name:%s decoration:%s type:%s", s2w(i->name()).c_str(), s2w(i->decoration()).c_str(), s2w(i->type()->name()).c_str()));
							console->print(L"[Out]");
							for (auto& o : outputs)
								console->print(wsfmt(L"name:%s decoration:%s type:%s", s2w(o->name()).c_str(), s2w(o->decoration()).c_str(), s2w(o->type()->name()).c_str()));
						}
						else
							console->print(L"udt not found");
					}
					else if (tokens[1] == L"nodes")
					{
						for (auto i = 0; i < bp->node_count(); i++)
						{
							auto n = bp->node(i);
							console->print(wsfmt(L"id:%s type:%s", s2w(n->id()).c_str(), s2w(n->udt()->type()->name()).c_str()));
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
								auto type = input->type();
								console->print(s2w(input->name()));
								std::string link_address;
								if (input->link())
									link_address = input->link()->get_address().str();
								console->print(wsfmt(L"[%s]", s2w(link_address).c_str()));
								auto str = s2w(type->serialize(input->raw_data(), 2));
								if (str.empty())
									str = L"-";
								console->print(wsfmt(L"   %s", str.c_str()));
							}
							console->print(L"[Out]");
							for (auto i = 0; i < n->output_count(); i++)
							{
								auto output = n->output(i);
								auto type = output->type();
								console->print(s2w(output->name()));
								auto str = s2w(type->serialize(output->raw_data(), 2).c_str());
								if (str.empty())
									str = L"-";
								console->print(wsfmt(L"   %s", str.c_str()));
							}
						}
						else
							console->print(L"node not found");
					}
					else if (tokens[1] == L"graph")
					{
						if (!editor->generate_graph_image())
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
							console->print(wsfmt(L"node added: %s", s2w(n->id()).c_str()));
						else
							console->print(L"bad udt name or id already exist");
					}
					else if (tokens[1] == L"link")
					{
						auto out = bp->find_output(w2s(tokens[2]).c_str());
						auto in = bp->find_input(w2s(tokens[3]).c_str());
						if (out && in)
						{
							in->link_to(out);
							auto out_addr = in->link()->get_address();
							auto in_addr = in->get_address();
							console->print(wsfmt(L"link added: %s -> %s", s2w(out_addr.str()).c_str(), s2w(in_addr.str()).c_str()));
							editor->set_changed(true);
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
						auto n = bp->find_node(w2s(tokens[2]).c_str());
						if (n)
						{
							if (!editor->remove_node(n))
								printf("cannot remove test nodes\n");
							else
								console->print(wsfmt(L"node removed: %s", tokens[2].c_str()));
						}
						else
							console->print(L"node not found");
					}
					else if (tokens[1] == L"link")
					{
						auto i = bp->find_input(w2s(tokens[3]).c_str());
						if (i)
						{
							i->link_to(nullptr);
							console->print(wsfmt(L"link removed: %s", tokens[2].c_str()));
							editor->set_changed(true);
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
					BP::save_to_file(bp, filename.c_str());
					editor->set_changed(false);
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

				extra_global_db_count = 0;
				extra_global_dbs = nullptr;

			}, new_mail_p(editor), [](void* c) {
				auto editor = *(cBPEditor**)c;
				editor->console_tab = nullptr;
			}, new_mail_p(editor), editor->filename + L":", Vec2f(1495.f, 10.f));
			editor->console_tab = console_page->parent()->parent()->child(0)->child(0)->get_component(cDockerTab);
		}
	}, new_mail_p(c_editor));
	ui::e_end_menubar_menu();
	ui::e_end_menu_bar();

	ui::e_begin_layout()->get_component(cElement)->clip_children = true;
	ui::c_aligner(SizeFitParent, SizeFitParent);

	auto e_scene = ui::e_begin_layout();
	ui::c_event_receiver();
	ui::c_aligner(SizeFitParent, SizeFitParent);
	auto c_bp_scene = new_u_object<cBPScene>();
	c_bp_scene->editor = c_editor;
	e_scene->add_component(c_bp_scene);

	auto e_base = ui::e_empty();
	c_bp_scene->base_element = ui::c_element();
	c_editor->e_base = e_base;

	auto e_overlayer = ui::e_empty();
	ui::c_element();
	{
		auto c_event_receiver = ui::c_event_receiver();
		c_event_receiver->pass = (Entity*)INVALID_POINTER;
		c_event_receiver->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
			auto c_bp_scene = *(cBPScene**)c;
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
		}, new_mail_p(c_bp_scene));
	}
	ui::c_aligner(SizeFitParent, SizeFitParent);

	c_editor->load(filename);

	ui::e_end_layout();

	{
		ui::next_entity = Entity::create();
		struct Capture
		{
			cBPEditor* editor;
			Entity* e;
		}capture;
		capture.editor = c_editor;
		capture.e = ui::next_entity;
		ui::e_button(L"Run", [](void* c) {
			auto& capture = *(Capture*)c;
			capture.editor->running = !capture.editor->running;
			capture.e->get_component(cText)->set_text(capture.editor->running ? L"Pause" : L"Run");

			if (capture.editor->running)
				capture.editor->bp->time = 0.f;
		}, new_mail(&capture));
	}

	ui::e_text(L"100%");
	ui::c_aligner(AlignxLeft, AlignyBottom);
	c_bp_scene->scale_text = ui::current_entity()->get_component(cText);

	ui::e_end_layout();

	ui::e_end_docker_page();
	ui::e_end_docker();
	ui::e_end_docker_floating_container();
	ui::pop_parent();
}
