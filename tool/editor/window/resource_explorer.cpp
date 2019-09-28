#include <flame/foundation/blueprint.h>
#include <flame/foundation/bitmap.h>
#include <flame/graphics/device.h>
#include <flame/graphics/image.h>
#include <flame/graphics/font.h>
#include <flame/graphics/canvas.h>
#include <flame/universe/topmost.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/image.h>
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

struct cResourceExplorer;

struct Seat
{
	Vec2u pos;
};

struct cThumbnail : Component
{
	cImage* image;

	cResourceExplorer* explorer;
	std::wstring filename;
	Bitmap* thumbnail;
	Seat* seat;

	cThumbnail();
	~cThumbnail();
	void return_seat();
	virtual void start() override;
	virtual void update() override;
};

struct cResourceExplorer : Component
{
	std::filesystem::path base_path;
	std::filesystem::path curr_path;

	Entity* address_bar;
	Entity* e_list;
	cElement* c_list_element;
	cLayout* c_list_layout;
	Image* folder_img;
	Imageview* folder_img_v;
	uint folder_img_idx;
	Image* file_img;
	Imageview* file_img_v;
	uint file_img_idx;
	Image* thumbnails_img;
	Imageview* thumbnails_img_v;
	uint thumbnails_img_idx;
	Vec2u thumbnails_img_pos;
	std::vector<std::unique_ptr<Seat>> thumbnails_seats_free;
	std::vector<std::unique_ptr<Seat>> thumbnails_seats_occupied;

	std::filesystem::path selected;
	Entity* blank_menu;
	Entity* dir_menu;
	Entity* pf_menu;
	Entity* bp_menu;

	void* ev_file_changed;
	void* ev_end_file_watcher;

	cResourceExplorer() :
		Component("ResourceExplorer")
	{
		folder_img = Image::create_from_file(app.d, L"../asset/ui/imgs/folder.png");
		folder_img_v = Imageview::create(folder_img);
		folder_img_idx = app.canvas->set_image(-1, folder_img_v);
		file_img = Image::create_from_file(app.d, L"../asset/ui/imgs/file.png");
		file_img_v = Imageview::create(file_img);
		file_img_idx = app.canvas->set_image(-1, file_img_v);
		thumbnails_img = Image::create(app.d, Format_R8G8B8A8_UNORM, Vec2u(1920, 1024), 1, 1, SampleCount_1, ImageUsage$(ImageUsageTransferDst | ImageUsageSampled));
		thumbnails_img->init(Vec4c(255));
		thumbnails_img_v = Imageview::create(thumbnails_img);
		thumbnails_img_idx = app.canvas->set_image(-1, thumbnails_img_v, FilterNearest);
		{
			auto x = 0;
			auto y = 0;
			while (true)
			{
				if (x + 64 > thumbnails_img->size.x())
				{
					x = 0;
					y += 64;
					if (y + 64 > thumbnails_img->size.y())
						break;
				}
				auto seat = new Seat;
				seat->pos.x() = x;
				seat->pos.y() = y;
				thumbnails_seats_free.emplace_back(seat);
				x += 64;
			}
		}
	}

	~cResourceExplorer()
	{
		app.canvas->set_image(folder_img_idx, nullptr);
		Imageview::destroy(folder_img_v);
		Image::destroy(folder_img);
		app.canvas->set_image(file_img_idx, nullptr);
		Imageview::destroy(file_img_v);
		Image::destroy(file_img);
		app.canvas->set_image(thumbnails_img_idx, nullptr);
		Imageview::destroy(thumbnails_img_v);
		Image::destroy(thumbnails_img);

		destroy_event(ev_file_changed);
		set_event(ev_end_file_watcher);
	}

	Entity* create_listitem(const std::wstring& title, uint img_id)
	{
		auto e_item = Entity::create();
		{
			e_item->add_component(cElement::create());

			e_item->add_component(cEventReceiver::create());

			e_item->add_component(cStyleColor::create(Vec4c(0), Vec4c(0), Vec4c(0)));

			auto c_layout = cLayout::create(LayoutVertical);
			c_layout->item_padding = 4.f;
			e_item->add_component(c_layout);

			auto c_listitem = cListItem::create();
			c_listitem->unselected_color_normal = Vec4c(0);
			e_item->add_component(c_listitem);
		}

		auto e_image = Entity::create();
		e_item->add_child(e_image);
		{
			auto c_element = cElement::create();
			c_element->size = 64.f;
			e_image->add_component(c_element);

			auto c_image = cImage::create();
			c_image->id = img_id;
			e_image->add_component(c_image);
		}

		auto e_title = Entity::create();
		e_item->add_child(e_title);
		{
			e_title->add_component(cElement::create());

			auto c_text = cText::create(app.font_atlas_pixel);
			{
				auto str = app.font_atlas_pixel->slice_text_by_width(title, 64.f);
				c_text->set_text(*str.p);
				delete_mail(str);
			}
			e_title->add_component(c_text);
		}

		return e_item;
	}

	void navigate(const std::filesystem::path& path)
	{
		wait_all_works();
		looper().clear_delay_events();

		curr_path = path;

		looper().add_delay_event([](void* c) {
			auto thiz = *(cResourceExplorer**)c;
			auto& base_path = thiz->base_path;
			auto& curr_path = thiz->curr_path;
			auto address_bar = thiz->address_bar;
			auto list = thiz->e_list;

			address_bar->remove_all_children();
			auto e_upward = create_standard_button(app.font_atlas_pixel, 1.f, Icon_LEVEL_UP);
			address_bar->add_child(e_upward);
			((cStyleColor*)e_upward->find_component(cH("StyleColor")))->color_normal.a() = 0;
			((cEventReceiver*)e_upward->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				auto thiz = *(cResourceExplorer**)c;
				if (is_mouse_clicked(action, key))
				{
					if (thiz->curr_path != thiz->base_path)
						thiz->navigate(thiz->curr_path.parent_path());
				}
			}, new_mail_p(thiz));

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
			for (auto& p : dirs)
			{
				auto item = thiz->create_listitem(p.filename().wstring(), thiz->folder_img_idx);
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
					else if (is_mouse_down(action, key, true) && key == Mouse_Right)
					{
						capture.e->selected = capture.p;
						popup_menu(capture.e->dir_menu, app.root, pos);
					}
				}, new_mail(&capture));
			}
			for (auto& p : files)
			{
				auto is_image_type = false;
				auto ext = p.extension();
				if (ext == L".bmp" ||
					ext == L".jpg" ||
					ext == L".png")
					is_image_type = true;

				auto item = thiz->create_listitem(p.filename().wstring(), is_image_type ? 0 : thiz->file_img_idx);
				list->add_child(item);
				if (is_image_type)
				{
					auto e_image = item->child(0);
					((cImage*)e_image->find_component(cH("Image")))->color = Vec4c(100, 100, 100, 128);

					auto c_thumbnail = new_component<cThumbnail>();
					c_thumbnail->explorer = thiz;
					c_thumbnail->filename = std::filesystem::canonical(p).wstring();
					e_image->add_component(c_thumbnail);
				}
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
		else
		{
			auto w = c_list_element->size.x() - c_list_element->inner_padding_horizontal();
			c_list_layout->column = max(1U, uint(w / (c_list_layout->item_padding + 64.f)));
		}
	}
};

cThumbnail::cThumbnail() :
	Component("Thumbnail")
{
	thumbnail = nullptr;
	seat = nullptr;
}

cThumbnail::~cThumbnail()
{
	if (thumbnail)
		Bitmap::destroy(thumbnail);
	if (seat)
		return_seat();
}

void cThumbnail::return_seat()
{
	for (auto it = explorer->thumbnails_seats_occupied.begin(); it != explorer->thumbnails_seats_occupied.end(); it++)
	{
		if (it->get() == seat)
		{
			explorer->thumbnails_seats_free.push_back(std::move(*it));
			explorer->thumbnails_seats_occupied.erase(it);
			break;
		}
	}
	seat = nullptr;
}

void cThumbnail::start()
{
	image = (cImage*)entity->find_component(cH("Image"));

	add_work([](void* c) {
		auto thiz = *(cThumbnail**)c;
		uint w, h;
		char* data;
		get_thumbnail(64, thiz->filename, &w, &h, &data);
		auto bitmap = Bitmap::create(Vec2u(w, h), 4, 32, (uchar*)data, true);
		bitmap->swap_channel(0, 2);
		thiz->thumbnail = bitmap;
	}, new_mail_p(this));
}

void cThumbnail::update()
{
	if (thumbnail)
	{
		if (image->element->cliped)
		{
			if (seat)
				return_seat();
			image->element->inner_padding = Vec4f(0.f);
			image->id = 0;
			image->color = Vec4c(100, 100, 100, 128);
		}
		else
		{
			if (!seat)
			{
				if (!explorer->thumbnails_seats_free.empty())
				{
					seat = explorer->thumbnails_seats_free.front().get();
					explorer->thumbnails_seats_occupied.push_back(std::move(explorer->thumbnails_seats_free.front()));
					explorer->thumbnails_seats_free.erase(explorer->thumbnails_seats_free.begin());
					
					looper().add_delay_event([](void* c) {
						auto thiz = *(cThumbnail**)c;
						auto image = thiz->image;
						auto explorer = thiz->explorer;
						auto& thumbnails_img_size = explorer->thumbnails_img->size;
						auto& thumbnail_size = thiz->thumbnail->size;

						explorer->thumbnails_img->set_pixels(thiz->seat->pos.x(), thiz->seat->pos.y(), thumbnail_size.x(), thumbnail_size.y(), thiz->thumbnail->data);

						{
							auto h = (64 - thumbnail_size.x()) * 0.5f;
							auto v = (64 - thumbnail_size.y()) * 0.5f;
							image->element->inner_padding = Vec4f(h, v, h, v);
						}
						image->id = explorer->thumbnails_img_idx;
						image->uv0 = Vec2f(thiz->seat->pos) / thumbnails_img_size;
						image->uv1 = Vec2f(thiz->seat->pos + thumbnail_size) / thumbnails_img_size;
						image->color = Vec4c(255);
					}, new_mail_p(this));
				}
			}
		}
	}
}

void open_resource_explorer(const std::wstring& path, const Vec2f& pos)
{
	auto e_container = get_docker_container_model()->copy();
	app.root->add_child(e_container);
	{
		auto c_element = (cElement*)e_container->find_component(cH("Element"));
		c_element->pos = pos;
		c_element->size.x() = 1914.f;
		c_element->size.y() = 274.f;
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

		c_explorer->dir_menu = create_standard_menu();
		{
			{
				auto item = create_standard_menu_item(app.font_atlas_pixel, 1.f, L"Goto");
				c_explorer->dir_menu->add_child(item);
				((cEventReceiver*)item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
					auto explorer = *(cResourceExplorer**)c;
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						destroy_topmost(app.root);
						explorer->navigate(explorer->selected);
					}
				}, new_mail_p(c_explorer));
			}
		}

		c_explorer->blank_menu = create_standard_menu();
		{
			{
				auto item = create_standard_menu_item(app.font_atlas_pixel, 1.f, L"New Prefab");
				c_explorer->blank_menu->add_child(item);
				((cEventReceiver*)item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
					auto explorer = *(cResourceExplorer**)c;
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						destroy_topmost(app.root);
						popup_input_dialog(explorer->entity, L"name", [](void* c, bool ok, const std::wstring& text) {
							auto explorer = *(cResourceExplorer**)c;

							if (ok)
							{
								auto e = Entity::create();
								Entity::save_to_file(app.dbs, e, explorer->curr_path / ext_replace(text, L".prefab"));
								Entity::destroy(e);
							}
						}, new_mail_p(explorer));
					}
				}, new_mail_p(c_explorer));
			}
			{
				auto item = create_standard_menu_item(app.font_atlas_pixel, 1.f, L"New BP");
				c_explorer->blank_menu->add_child(item);
				((cEventReceiver*)item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
					auto explorer = *(cResourceExplorer**)c;
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						destroy_topmost(app.root);
						popup_input_dialog(explorer->entity, L"name", [](void* c, bool ok, const std::wstring& text) {
							auto explorer = *(cResourceExplorer**)c;

							if (ok)
							{
								auto p = explorer->curr_path / text;
								std::filesystem::create_directory(p);
								auto bp = BP::create();
								bp->save_to_file(bp, p / L"bp");
								BP::destroy(bp);
							}
						}, new_mail_p(explorer));
					}
				}, new_mail_p(c_explorer));
			}
		}

		c_explorer->pf_menu = create_standard_menu();
		{
			auto mi_open = create_standard_menu_item(app.font_atlas_pixel, 1.f, L"Open");
			c_explorer->pf_menu->add_child(mi_open);
			((cEventReceiver*)mi_open->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				auto explorer = *(cResourceExplorer**)c;
				if (is_mouse_down(action, key, true) && key == Mouse_Left)
				{
					destroy_topmost(app.root);
					open_scene_editor(explorer->selected, Vec2f(450.f, 20.f));
				}
			}, new_mail_p(c_explorer));
		}

		c_explorer->bp_menu = create_standard_menu();
		{
			{
				auto item = create_standard_menu_item(app.font_atlas_pixel, 1.f, L"Open");
				c_explorer->bp_menu->add_child(item);
				((cEventReceiver*)item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
					auto explorer = *(cResourceExplorer**)c;
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						destroy_topmost(app.root);
						open_blueprint_editor(explorer->selected, false, Vec2f(450.f, 20.f));
					}
				}, new_mail_p(c_explorer));
			}
			{
				auto item = create_standard_menu_item(app.font_atlas_pixel, 1.f, L"Open (No Compile)");
				c_explorer->bp_menu->add_child(item);
				((cEventReceiver*)item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
					auto explorer = *(cResourceExplorer**)c;
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						destroy_topmost(app.root);
						open_blueprint_editor(explorer->selected, true, Vec2f(450.f, 20.f));
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
		c_explorer->c_list_element = (cElement*)e_list->find_component(cH("Element"));

		auto c_layout = (cLayout*)e_list->find_component(cH("Layout"));
		c_layout->type = LayoutGrid;
		c_layout->column = 4;
		c_explorer->c_list_layout = c_layout;

		auto c_event_receiver = (cEventReceiver*)e_list->find_component(cH("EventReceiver"));
		c_event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
			auto exploter = *(cResourceExplorer**)c;

			if (is_mouse_down(action, key, true) && key == Mouse_Right)
				popup_menu(exploter->blank_menu, app.root, pos);
		}, new_mail_p(c_explorer));
		e_list->add_component(c_event_receiver);
	}
	c_explorer->e_list = e_list;

	e_page->add_child(wrap_standard_scrollbar(e_list, ScrollbarVertical, true, 1.f));

	c_explorer->ev_file_changed = create_event(false);
	c_explorer->ev_end_file_watcher = add_file_watcher(path, [](void* c, FileChangeType type, const std::wstring& filename) {
		auto explorer = *(cResourceExplorer**)c;

		set_event(explorer->ev_file_changed);
	}, new_mail_p(c_explorer), true, false);

	c_explorer->navigate(path);
}
