#include <flame/foundation/blueprint.h>
#include <flame/graphics/font.h>
#include <flame/universe/topmost.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/scrollbar.h>
#include <flame/universe/components/tree.h>
#include <flame/universe/components/menu.h>
#include <flame/universe/components/list.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/window.h>

#include "../app.h"
#include "resource_explorer.h"
#include "blueprint_editor.h"
#include "scene_editor.h"

struct cResourceExplorer : Component
{
	std::filesystem::path base_path;
	std::filesystem::path curr_path;

	Entity* address_bar;
	Entity* list;

	std::filesystem::path selected;
	Entity* blank_menu;
	Entity* pf_menu;
	Entity* bp_menu;

	void* ev_file_changed;
	void* ev_end_file_watcher;

	cResourceExplorer() :
		Component("ResourceExplorer")
	{
	}

	~cResourceExplorer()
	{
		destroy_event(ev_file_changed);
		set_event(ev_end_file_watcher);
	}

	void navigate(const std::filesystem::path& path)
	{
		curr_path = path;

		looper().add_delay_event([](void* c) {
			auto thiz = *(cResourceExplorer**)c;
			auto& base_path = thiz->base_path;
			auto& curr_path = thiz->curr_path;
			auto address_bar = thiz->address_bar;
			auto list = thiz->list;

			address_bar->remove_all_children();

			std::vector<std::filesystem::path> stems;
			for (auto p = curr_path; ; p = p.parent_path())
			{
				stems.push_back(p);
				if (p == base_path)
					break;
			}
			std::reverse(stems.begin(), stems.end());

			for (auto& s : stems)
			{
				auto e_stem = create_standard_button(app.font_atlas_pixel, 1.f, s.filename().wstring());
				address_bar->add_child(e_stem);
				{
					((cStyleColor*)e_stem->find_component(cH("StyleColor")))->color_normal.a() = 0;

					struct Capture
					{
						cResourceExplorer* e;
						std::wstring p;
					}capture;
					capture.e = thiz;
					capture.p = s.wstring();
					((cEventReceiver*)e_stem->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
						auto& capture = *(Capture*)c;
						if (is_mouse_down(action, key, true) && key == Mouse_Left)
							capture.e->navigate(capture.p);
					}, new_mail(&capture));
				}

				std::vector<std::filesystem::path> sub_dirs;
				for (std::filesystem::directory_iterator end, it(s); it != end; it++)
				{
					if (std::filesystem::is_directory(it->status()))
						sub_dirs.push_back(it->path());
				}
				if (!sub_dirs.empty())
				{
					auto e_stem_popup = create_standard_menu();
					for (auto& p : sub_dirs)
					{
						auto e_item = create_standard_menu_item(app.font_atlas_pixel, 1.f, p.filename().wstring());
						e_stem_popup->add_child(e_item);
						struct Capture
						{
							cResourceExplorer* e;
							std::wstring p;
						}capture;
						capture.e = thiz;
						capture.p = p.wstring();
						((cEventReceiver*)e_item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
							auto& capture = *(Capture*)c;
							if (is_mouse_down(action, key, true) && key == Mouse_Left)
							{
								destroy_topmost(app.root);
								capture.e->navigate(capture.p);
							}
						}, new_mail(&capture));
					}
					auto e_stem_popup_btn = create_standard_menu_button(app.font_atlas_pixel, 1.f, Icon_CARET_RIGHT, app.root, e_stem_popup, false, SideS, false, false, true, nullptr);
					address_bar->add_child(e_stem_popup_btn);
				}
			}

			list->remove_all_children();
			((cList*)list->find_component(cH("List")))->selected = nullptr;

			std::vector<std::filesystem::path> dirs;
			std::vector<std::filesystem::path> files;
			for (std::filesystem::directory_iterator end, it(curr_path); it != end; it++)
			{
				if (std::filesystem::is_directory(it->status()))
				{
					if (it->path().filename().wstring()[0] != L'.')
						dirs.push_back(it->path());
				}
				else
				{
					auto ext = it->path().extension();
					if (ext != L".ilk" && ext != L".exp")
						files.push_back(it->path());
				}
			}
			auto upward_item = create_standard_listitem(app.font_atlas_pixel, 1.f, Icon_FOLDER_O + std::wstring(L" .."));
			((cListItem*)upward_item->find_component(cH("ListItem")))->unselected_color_normal.a() = 0;
			list->add_child(upward_item);
			((cEventReceiver*)upward_item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				auto thiz = *(cResourceExplorer**)c;
				if (is_mouse_clicked(action, key, true))
				{
					if (thiz->curr_path != thiz->base_path)
						thiz->navigate(thiz->curr_path.parent_path());
				}
			}, new_mail_p(thiz));
			for (auto& p : dirs)
			{
				auto item = create_standard_listitem(app.font_atlas_pixel, 1.f, Icon_FOLDER_O + std::wstring(L" ") + p.filename().wstring());
				((cListItem*)item->find_component(cH("ListItem")))->unselected_color_normal.a() = 0;
				list->add_child(item);
				struct Capture
				{
					cResourceExplorer* e;
					std::filesystem::path p;
				}capture;
				capture.e = thiz;
				capture.p = p;
				((cEventReceiver*)item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
					auto& capture = *(Capture*)c;
					if (is_mouse_clicked(action, key, true))
						capture.e->navigate(capture.p);
				}, new_mail(&capture));
			}
			for (auto& p : files)
			{
				auto item = create_standard_listitem(app.font_atlas_pixel, 1.f, Icon_FILE_O + std::wstring(L" ") + p.filename().wstring());
				((cListItem*)item->find_component(cH("ListItem")))->unselected_color_normal.a() = 0;
				list->add_child(item);
				struct Capture
				{
					cResourceExplorer* e;
					std::filesystem::path p;
				}capture;
				capture.e = thiz;
				capture.p = p;
				((cEventReceiver*)item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
					auto& capture = *(Capture*)c;
					if (is_mouse_down(action, key, true) && key == Mouse_Right)
					{
						capture.e->selected = capture.p;
						auto fn = capture.p.filename().wstring();
						auto ext = capture.p.extension().wstring();
						if (ext == L".prefab")
							popup_menu(capture.e->pf_menu, app.root, pos);
						else if (fn == L"bp")
							popup_menu(capture.e->bp_menu, app.root, pos);
					}
				}, new_mail(&capture));
			}

		}, new_mail_p(this));
	}

	virtual void update() override
	{
		if (wait_event(ev_file_changed, 0))
		{
			while (!std::filesystem::exists(curr_path))
			{
				curr_path = curr_path.parent_path();
				assert(curr_path != base_path);
			}

			navigate(curr_path);
		}

	}
};

void open_resource_explorer(const std::wstring& path, const Vec2f& pos)
{
	auto e_container = get_docker_container_model()->copy();
	app.root->add_child(e_container);
	{
		auto c_element = (cElement*)e_container->find_component(cH("Element"));
		c_element->pos = pos;
		c_element->size.x() = 300.f;
		c_element->size.y() = 600.f;
	}

	auto e_docker = get_docker_model()->copy();
	e_container->add_child(e_docker, 0);

	e_docker->child(0)->add_child(create_standard_docker_tab(app.font_atlas_pixel, L"Resource Explorer", app.root));

	auto e_page = get_docker_page_model()->copy();
	{
		((cElement*)e_page->find_component(cH("Element")))->inner_padding = Vec4f(4.f);

		auto c_layout = cLayout::create(LayoutVertical);
		c_layout->item_padding = 4.f;
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;
		c_layout->fence = 2;
		e_page->add_component(c_layout);
	}
	auto c_explorer = new_component<cResourceExplorer>();
	{
		c_explorer->base_path = path;

		c_explorer->blank_menu = create_standard_menu();
		{
			{
				auto item = create_standard_menu_item(app.font_atlas_pixel, 1.f, L"New Prefab");
				c_explorer->blank_menu->add_child(item);
				((cEventReceiver*)item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
					auto c_explorer = *(cResourceExplorer**)c;
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						destroy_topmost(app.root);
						popup_input_dialog(c_explorer->entity, L"name", [](void* c, bool ok, const std::wstring& text) {
							auto explorer = *(cResourceExplorer**)c;

							if (ok)
							{
								auto e = Entity::create();
								Entity::save_to_file(app.dbs, e, explorer->curr_path / ext_replace(text, L".prefab"));
								Entity::destroy(e);
							}
						}, new_mail_p(c_explorer));
					}
				}, new_mail_p(c_explorer));
			}
			{
				auto item = create_standard_menu_item(app.font_atlas_pixel, 1.f, L"New BP");
				c_explorer->blank_menu->add_child(item);
				((cEventReceiver*)item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
					auto c_explorer = *(cResourceExplorer**)c;
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						destroy_topmost(app.root);
						popup_input_dialog(c_explorer->entity, L"name", [](void* c, bool ok, const std::wstring& text) {
							auto explorer = *(cResourceExplorer**)c;

							if (ok)
							{
								auto p = explorer->curr_path / text;
								std::filesystem::create_directory(p);
								auto bp = BP::create();
								bp->save_to_file(bp, p / L"bp");
								BP::destroy(bp);
							}
						}, new_mail_p(c_explorer));
					}
				}, new_mail_p(c_explorer));
			}
		}

		c_explorer->pf_menu = create_standard_menu();
		{
			auto mi_open = create_standard_menu_item(app.font_atlas_pixel, 1.f, L"Open");
			c_explorer->pf_menu->add_child(mi_open);
			((cEventReceiver*)mi_open->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				auto c_explorer = *(cResourceExplorer**)c;
				if (is_mouse_down(action, key, true) && key == Mouse_Left)
				{
					destroy_topmost(app.root);
					open_scene_editor(c_explorer->selected, Vec2f(450.f, 20.f));
				}
			}, new_mail_p(c_explorer));
		}

		c_explorer->bp_menu = create_standard_menu();
		{
			{
				auto item = create_standard_menu_item(app.font_atlas_pixel, 1.f, L"Open");
				c_explorer->bp_menu->add_child(item);
				((cEventReceiver*)item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
					auto c_explorer = *(cResourceExplorer**)c;
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						destroy_topmost(app.root);
						open_blueprint_editor(c_explorer->selected, false, Vec2f(450.f, 20.f));
					}
				}, new_mail_p(c_explorer));
			}
			{
				auto item = create_standard_menu_item(app.font_atlas_pixel, 1.f, L"Open (No Compile)");
				c_explorer->bp_menu->add_child(item);
				((cEventReceiver*)item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
					auto c_explorer = *(cResourceExplorer**)c;
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						destroy_topmost(app.root);
						open_blueprint_editor(c_explorer->selected, true, Vec2f(450.f, 20.f));
					}
				}, new_mail_p(c_explorer));
			}
		}
	}
	e_page->add_component(c_explorer);
	e_docker->child(1)->add_child(e_page);

	auto e_address_bar = Entity::create();
	e_page->add_child(e_address_bar);
	{
		e_address_bar->add_component(cElement::create());

		e_address_bar->add_component(cLayout::create(LayoutHorizontal));
	}
	c_explorer->address_bar = e_address_bar;

	auto e_list = create_standard_list(true);
	{
		auto c_event_receiver = (cEventReceiver*)e_list->find_component(cH("EventReceiver"));
		c_event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
			auto exploter = *(cResourceExplorer**)c;

			if (is_mouse_down(action, key, true) && key == Mouse_Right)
				popup_menu(exploter->blank_menu, app.root, pos);
		}, new_mail_p(c_explorer));
		e_list->add_component(c_event_receiver);
	}
	c_explorer->list = e_list;

	e_page->add_child(wrap_standard_scrollbar(e_list, ScrollbarVertical, true, 1.f));

	c_explorer->ev_file_changed = create_event(false);
	c_explorer->ev_end_file_watcher = add_file_watcher(path, [](void* c, FileChangeType type, const std::wstring& filename) {
		auto explorer = *(cResourceExplorer**)c;

		set_event(explorer->ev_file_changed);
	}, new_mail_p(c_explorer), true, false);

	c_explorer->navigate(path);
}
