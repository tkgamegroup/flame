#include <flame/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/foundation/bitmap.h>
#include <flame/graphics/device.h>
#include <flame/graphics/image.h>
#include <flame/universe/systems/2d_renderer.h>
#include <flame/universe/ui/utils.h>

#include "../renderpath/canvas/canvas.h"

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


	cThumbnail() :
		Component("cThumbnail")
	{
		thumbnail = nullptr;
		seat = nullptr;
	}

	~cThumbnail()
	{
		if (thumbnail)
			Bitmap::destroy(thumbnail);
		if (seat)
			return_seat();
	}

	void return_seat();
	virtual void on_component_added(Component* c) override;
	void draw(graphics::Canvas* canvas);
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

	void* ev_file_changed;
	void* ev_end_file_watcher;

	cResourceExplorer() :
		Component("cResourceExplorer")
	{
		auto canvas = app.s_2d_renderer->canvas;
		folder_img = Image::create_from_file(app.d, L"../art/ui/imgs/folder.png");
		folder_img_v = Imageview::create(folder_img);
		folder_img_idx = canvas->set_image(-1, folder_img_v);
		file_img = Image::create_from_file(app.d, L"../art/ui/imgs/file.png");
		file_img_v = Imageview::create(file_img);
		file_img_idx = canvas->set_image(-1, file_img_v);
		thumbnails_img = Image::create(app.d, Format_R8G8B8A8_UNORM, Vec2u(1920, 1024), 1, 1, SampleCount_1, ImageUsage$(ImageUsageTransferDst | ImageUsageSampled));
		thumbnails_img->init(Vec4c(255));
		thumbnails_img_v = Imageview::create(thumbnails_img);
		thumbnails_img_idx = canvas->set_image(-1, thumbnails_img_v, FilterNearest);
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
		auto canvas = app.s_2d_renderer->canvas;
		canvas->set_image(folder_img_idx, nullptr);
		Imageview::destroy(folder_img_v);
		Image::destroy(folder_img);
		canvas->set_image(file_img_idx, nullptr);
		Imageview::destroy(file_img_v);
		Image::destroy(file_img);
		canvas->set_image(thumbnails_img_idx, nullptr);
		Imageview::destroy(thumbnails_img_v);
		Image::destroy(thumbnails_img);

		destroy_event(ev_file_changed);
		set_event(ev_end_file_watcher);
	}

	Entity* create_listitem(const std::wstring& title, uint img_id)
	{
		auto e_item = ui::e_list_item(L"", false);
		ui::c_layout(LayoutVertical)->item_padding = 4.f;
		ui::push_parent(e_item);
		ui::e_image(img_id << 16, Vec2f(64.f));
		ui::e_text(app.font_atlas_pixel->slice_text_by_width(title.c_str(), title.size(), ui::style_1u(ui::FontSize), 64.f).v);
		ui::pop_parent();
		return e_item;
	}

	void navigate(const std::filesystem::path& path)
	{
		curr_path = path;

		looper().add_event([](void* c) {
			auto thiz = *(cResourceExplorer**)c;
			auto& base_path = thiz->base_path;
			auto& curr_path = thiz->curr_path;
			auto address_bar = thiz->address_bar;
			auto list = thiz->e_list;

			address_bar->remove_child((Entity*)INVALID_POINTER);
			ui::push_parent(address_bar);
			ui::push_style_4c(ui::ButtonColorNormal, Vec4c(0));

			ui::e_button(Icon_LEVEL_UP, [](void* c) {
				auto thiz = *(cResourceExplorer**)c;
				if (thiz->curr_path != thiz->base_path)
					thiz->navigate(thiz->curr_path.parent_path());
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
				struct Capture
				{
					cResourceExplorer* e;
					std::wstring p;
				}capture;
				capture.e = thiz;
				capture.p = s.wstring();
				ui::e_button(s.filename().c_str(), [](void* c) {
					auto& capture = *(Capture*)c;
					capture.e->navigate(capture.p);
				}, new_mail(&capture));

				std::vector<std::filesystem::path> sub_dirs;
				for (std::filesystem::directory_iterator end, it(s); it != end; it++)
				{
					if (std::filesystem::is_directory(it->status()))
						sub_dirs.push_back(it->path());
				}
				if (!sub_dirs.empty())
				{
					ui::e_begin_button_menu(Icon_CARET_RIGHT);
					for (auto& p : sub_dirs)
					{
						struct Capture
						{
							cResourceExplorer* e;
							std::wstring p;
						}capture;
						capture.e = thiz;
						capture.p = p.wstring();
						ui::e_menu_item(p.filename().c_str(), [](void* c) {
							auto& capture = *(Capture*)c;
							capture.e->navigate(capture.p);
						}, new_mail(&capture));
					}
					ui::e_end_button_menu();
				}
			}

			ui::pop_style(ui::ButtonColorNormal);
			ui::pop_parent();

			clear_all_works();
			looper().clear_events(FLAME_CHASH("update thumbnail"));

			list->get_component(cList)->set_selected(nullptr, false);
			list->remove_child((Entity*)INVALID_POINTER);

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
			ui::push_parent(list);
			ui::push_style_4c(ui::FrameColorNormal, Vec4c(0));
			for (auto& p : dirs)
			{
				auto item = thiz->create_listitem(p.filename().wstring(), thiz->folder_img_idx);
				struct Capture
				{
					cResourceExplorer* e;
					std::filesystem::path p;
				}capture;
				capture.e = thiz;
				capture.p = p;
				item->get_component(cEventReceiver)->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					auto& capture = *(Capture*)c;
					if (is_mouse_clicked(action, key, true))
						capture.e->navigate(capture.p);
				}, new_mail(&capture));
				ui::push_parent(item);
				ui::e_begin_popup_menu();
				ui::e_menu_item(L"Open", [](void* c) {
					auto& capture = *(Capture*)c;
					capture.e->selected = capture.p;
					capture.e->navigate(capture.e->selected);
				}, new_mail(&capture));
				ui::e_end_popup_menu();
				ui::pop_parent();
			}
			for (auto& p : files)
			{
				auto is_image_type = false;
				auto ext = p.extension();
				if (ext == L".bmp" ||
					ext == L".jpg" ||
					ext == L".png" ||
					ext == L".gif" ||
					ext == L".mp4")
					is_image_type = true;

				auto item = thiz->create_listitem(p.filename().wstring(), is_image_type ? 0 : thiz->file_img_idx);
				if (is_image_type)
				{
					auto e_image = item->child(0);
					e_image->get_component(cImage)->color = Vec4c(100, 100, 100, 128);

					auto c_thumbnail = new_u_object<cThumbnail>();
					c_thumbnail->explorer = thiz;
					c_thumbnail->filename = std::filesystem::canonical(p).wstring();
					e_image->add_component(c_thumbnail);
				}
				ui::push_parent(item);
				struct Capture
				{
					cResourceExplorer* e;
					std::filesystem::path p;
				}capture;
				capture.e = thiz;
				capture.p = p;
				if (ext == L".prefab")
				{
					ui::e_begin_popup_menu();
					ui::e_menu_item(L"Open", [](void* c) {
						auto& capture = *(Capture*)c;
						capture.e->selected = capture.p;
						open_scene_editor(capture.e->selected, Vec2f(450.f, 20.f));
					}, new_mail(&capture));
					ui::e_end_popup_menu();
				}
				else if (p.filename() == L"bp")
				{
					ui::e_begin_popup_menu();
					ui::e_menu_item(L"Open", [](void* c) {
						auto& capture = *(Capture*)c;
						capture.e->selected = capture.p;
						open_blueprint_editor(capture.e->selected, Vec2f(4.f, 10.f));
					}, new_mail(&capture));
				}
				ui::pop_parent();
			}
			ui::pop_style(ui::FrameColorNormal);
			ui::pop_parent();
		}, new_mail_p(this));
	}

	void on_component_added(Component* c) override
	{
		if (c->name_hash == FLAME_CHASH("cElement"))
		{
			((cElement*)c)->cmds.add([](void* c, graphics::Canvas* canvas) {
				(*(cResourceExplorer**)c)->draw(canvas);
			}, new_mail_p(this));
		}
	}

	void draw(graphics::Canvas* canvas)
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
			auto w = c_list_element->size_.x() - c_list_element->inner_padding_h();
			c_list_layout->set_column(max(1U, uint(w / (c_list_layout->item_padding + 64.f))));
		}
	}
};

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

void cThumbnail::on_component_added(Component* c)
{
	if (c->name_hash == FLAME_CHASH("cElement"))
	{
		((cElement*)c)->cmds.add([](void* c, graphics::Canvas* canvas) {
			(*(cThumbnail**)c)->draw(canvas);
		}, new_mail_p(this));
	}
	else if (c->name_hash == FLAME_CHASH("cImage"))
	{
		image = (cImage*)c;
		add_work([](void* c) {
			auto thiz = *(cThumbnail**)c;
			uint w, h;
			char* data;
			get_thumbnail(64, thiz->filename.c_str(), &w, &h, &data);
			auto bitmap = Bitmap::create(Vec2u(w, h), 4, 32, (uchar*)data, true);
			bitmap->swap_channel(0, 2);
			thiz->thumbnail = bitmap;
		}, new_mail_p(this));
	}
}

void cThumbnail::draw(graphics::Canvas* canvas)
{
	if (thumbnail)
	{
		if (image->element->cliped)
		{
			if (seat)
			{
				return_seat();
				image->element->inner_padding_ = Vec4f(0.f);
				image->id = 0;
				image->color = Vec4c(100, 100, 100, 128);
			}
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
					
					looper().add_event([](void* c) {
						auto thiz = *(cThumbnail**)c;
						auto image = thiz->image;
						auto explorer = thiz->explorer;
						auto& thumbnails_img_size = explorer->thumbnails_img->size;
						auto& thumbnail_size = thiz->thumbnail->size;

						explorer->thumbnails_img->set_pixels(thiz->seat->pos, thumbnail_size, thiz->thumbnail->data);

						auto h = (64 - thumbnail_size.x()) * 0.5f;
						auto v = (64 - thumbnail_size.y()) * 0.5f;
						image->element->inner_padding_ = Vec4f(h, v, h, v);
						image->id = explorer->thumbnails_img_idx << 16;
						image->uv0 = Vec2f(thiz->seat->pos) / thumbnails_img_size;
						image->uv1 = Vec2f(thiz->seat->pos + thumbnail_size) / thumbnails_img_size;
						image->color = Vec4c(255);
					}, new_mail_p(this), nullptr, false, 0.f, FLAME_CHASH("update thumbnail"));
				}
			}
		}
	}
}

void open_resource_explorer(const std::wstring& path, const Vec2f& pos)
{
	ui::push_parent(app.root);
	ui::next_element_pos = pos;
	ui::next_element_size = Vec2f(1914.f, 274.f);
	ui::e_begin_docker_floating_container();
	ui::e_begin_docker();
	auto e_page = ui::e_begin_docker_page(L"Resource Explorer").page;
	e_page->get_component(cElement)->inner_padding_ = 4.f;
	{
		auto c_layout = ui::c_layout(LayoutVertical);
		c_layout->item_padding = 4.f;
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;
		c_layout->fence = 2;
	}
	auto c_explorer = new_u_object<cResourceExplorer>();
	c_explorer->base_path = path;
	e_page->add_component(c_explorer);

	c_explorer->address_bar = ui::e_empty();
	ui::c_element();
	ui::c_layout(LayoutHorizontal);

	ui::e_begin_scroll_view1(ScrollbarVertical, Vec2f(0.f));
	c_explorer->e_list = ui::e_begin_list(true);
	{
		c_explorer->c_list_element = c_explorer->e_list->get_component(cElement);

		c_explorer->c_list_layout = c_explorer->e_list->get_component(cLayout);
		c_explorer->c_list_layout->type = LayoutGrid;
		c_explorer->c_list_layout->column_ = 4;
	}
	ui::e_begin_popup_menu();
	ui::e_menu_item(L"New Prefab", [](void* c) {
		auto explorer = *(cResourceExplorer**)c;
		ui::e_input_dialog(L"name", [](void* c, bool ok, const wchar_t* text) {
			auto explorer = *(cResourceExplorer**)c;
			if (ok)
			{
				auto e = Entity::create();
				Entity::save_to_file(e, (explorer->curr_path / std::filesystem::path(text).replace_extension(L".prefab")).c_str());
				Entity::destroy(e);
			}
		}, new_mail_p(explorer));
	}, new_mail_p(c_explorer));
	ui::e_menu_item(L"New BP", [](void* c) {
		auto explorer = *(cResourceExplorer**)c;
		ui::e_input_dialog(L"name", [](void* c, bool ok, const wchar_t* text) {
			auto explorer = *(cResourceExplorer**)c;
			if (ok)
			{
				auto p = explorer->curr_path / text;
				std::filesystem::create_directory(p);
				auto bp = BP::create();
				bp->save_to_file(bp, (p / L"bp").c_str());
				BP::destroy(bp);
			}
		}, new_mail_p(explorer));
	}, new_mail_p(c_explorer));
	ui::e_end_popup_menu();
	ui::e_end_list();
	ui::e_end_scroll_view1();

	c_explorer->ev_file_changed = create_event(false);
	c_explorer->ev_end_file_watcher = add_file_watcher(path.c_str(), [](void* c, FileChangeType type, const wchar_t* filename) {
		auto explorer = *(cResourceExplorer**)c;

		set_event(explorer->ev_file_changed);
	}, new_mail_p(c_explorer), true, false);

	ui::e_end_docker_page();
	ui::e_end_docker();
	ui::e_end_docker_floating_container();
	ui::pop_parent();

	c_explorer->navigate(path);
}
