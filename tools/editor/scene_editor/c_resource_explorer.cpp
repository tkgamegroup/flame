#include <flame/foundation/bitmap.h>

#include "scene_editor.h"
#include "../bp_editor/bp_editor.h"

struct cThumbnail : Component
{
	cImage* image;

	std::wstring filename;
	Bitmap* thumbnail;
	uvec2* seat;


	cThumbnail() :
		Component("cThumbnail")
	{
		thumbnail = nullptr;
		seat = nullptr;
	}

	~cThumbnail()
	{
		if (thumbnail)
			thumbnail->release();
		if (seat)
			return_seat();
	}

	void return_seat()
	{
		for (auto it = scene_editor.resource_explorer->thumbnails_seats_occupied.begin(); it != scene_editor.resource_explorer->thumbnails_seats_occupied.end(); it++)
		{
			if (it->get() == seat)
			{
				scene_editor.resource_explorer->thumbnails_seats_free.push_back(std::move(*it));
				scene_editor.resource_explorer->thumbnails_seats_occupied.erase(it);
				break;
			}
		}
		seat = nullptr;
	}

	void on_event(EntityEvent e, void* t)
	{
		switch (e)
		{
		case EntityComponentAdded:
			if (t == this)
			{
				entity->get_component(cElement)->cmds.add([](Capture& c, graphics::Canvas* canvas) {
					c.thiz<cThumbnail>()->draw(canvas);
					return true;
				}, Capture().set_thiz(this));
				image = entity->get_component(cImage);

				add_work([](Capture& c) {
					auto thiz = c.thiz<cThumbnail>();
					uint w, h;
					char* data;
					get_thumbnail(64, thiz->filename.c_str(), &w, &h, &data);
					auto bitmap = Bitmap::create(w, h, 4, 1, (uchar*)data);
					bitmap->swap_channel(0, 2);
					thiz->thumbnail = bitmap;
				}, Capture().set_thiz(this));
			}
			break;
		}
	}

	void draw(graphics::Canvas* canvas)
	{
		if (thumbnail)
		{
			if (image->element->clipped)
			{
				if (seat)
				{
					return_seat();
					image->element->padding = vec4(0.f);
					image->id = 0;
					image->color = cvec4(100, 100, 100, 128);
				}
			}
			else
			{
				if (!seat)
				{
					if (!scene_editor.resource_explorer->thumbnails_seats_free.empty())
					{
						seat = scene_editor.resource_explorer->thumbnails_seats_free.front().get();
						scene_editor.resource_explorer->thumbnails_seats_occupied.push_back(std::move(scene_editor.resource_explorer->thumbnails_seats_free.front()));
						scene_editor.resource_explorer->thumbnails_seats_free.erase(scene_editor.resource_explorer->thumbnails_seats_free.begin());

						looper().add_event([](Capture& c) {
							auto thiz = c.thiz<cThumbnail>();
							auto image = thiz->image;
							auto& thumbnails_img_size = scene_editor.resource_explorer->thumbnails_img->size;
							auto& thumbnail_size = uvec2(thiz->thumbnail->get_width(), thiz->thumbnail->get_height());

							scene_editor.resource_explorer->thumbnails_img->set_pixels(*thiz->seat, thumbnail_size, thiz->thumbnail->get_data());

							auto h = (64 - thumbnail_size.x) * 0.5f;
							auto v = (64 - thumbnail_size.y) * 0.5f;
							image->element->padding = vec4(h, v, h, v);
							image->id = scene_editor.resource_explorer->thumbnails_img_idx << 16;
							image->uv0 = vec2(*thiz->seat) / thumbnails_img_size;
							image->uv1 = vec2(*thiz->seat + thumbnail_size) / thumbnails_img_size;
							image->color = cvec4(255);
						}, Capture().set_thiz(this), 0.f, FLAME_CHASH("update thumbnail"));
					}
				}
			}
		}
	}
};

cResourceExplorer::cResourceExplorer() :
	Component("cResourceExplorer")
{
	auto canvas = scene_editor.window->canvas;
	folder_img = Image::create_from_file(app.graphics_device, (app.resource_path / L"assets/folder.png").c_str());
	folder_img_idx = canvas->set_resource(-1, folder_img->default_view());
	file_img = Image::create_from_file(app.graphics_device, (app.resource_path / L"assets/file.png").c_str());
	file_img_idx = canvas->set_resource(-1, file_img->default_view());
	thumbnails_img = Image::create(app.graphics_device, Format_R8G8B8A8_UNORM, uvec2(1920, 1024), 1, 1, SampleCount_1, ImageUsageTransferDst | ImageUsageSampled);
	thumbnails_img->clear(ImageLayoutUndefined, ImageLayoutShaderReadOnly, cvec4(255));
	thumbnails_img_idx = canvas->set_resource(-1, thumbnails_img->default_view(), Sampler::get_default(FilterNearest));
	{
		auto x = 0;
		auto y = 0;
		while (true)
		{
			if (x + 64 > thumbnails_img->size.x)
			{
				x = 0;
				y += 64;
				if (y + 64 > thumbnails_img->size.y)
					break;
			}
			auto seat = new uvec2;
			seat->x() = x;
			seat->y() = y;
			thumbnails_seats_free.emplace_back(seat);
			x += 64;
		}
	}

	auto& ui = scene_editor.window->ui;

	ui.next_element_padding = 4.f;
	auto e_page = ui.e_begin_docker_page(L"Resource Explorer").second;
	{
		auto c_layout = ui.c_layout(LayoutVertical);
		c_layout->item_padding = 4.f;
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;
		c_layout->fence = 2;

		e_page->add_component(this);
	}

	base_path = app.resource_path;

		address_bar = ui.e_empty();
		ui.c_element();
		ui.c_layout(LayoutHorizontal);

		ui.e_begin_scrollbar(ScrollbarVertical, true);
			e_list = ui.e_begin_list(true);
			{
				c_list_element = e_list->get_component(cElement);

				c_list_layout = e_list->get_component(cLayout);
				c_list_layout->type = LayoutTile;
				c_list_layout->column = 4;
			}
				ui.e_begin_popup_menu();
					ui.e_menu_item(L"New Prefab", [](Capture& c) {
						auto& ui = scene_editor.window->ui;
						ui.e_input_dialog(L"name", [](Capture& c, bool ok, const wchar_t* text) {
							if (ok)
							{
								auto e = new<Entity>();
								Entity::save_to_file(e, (scene_editor.resource_explorer->curr_path / std::filesystem::path(text).replace_extension(L".prefab")).c_str());
								delete(e);
							}
						}, Capture());
					}, Capture());
					ui.e_menu_item(L"New BP", [](Capture& c) {
						auto& ui = scene_editor.window->ui;
						ui.e_input_dialog(L"name", [](Capture& c, bool ok, const wchar_t* text) {
							if (ok)
							{
								std::ofstream file((scene_editor.resource_explorer->curr_path / text).replace_extension(L".bp"));
								file << "<BP />";
								file.close();
							}
						}, Capture());
					}, Capture());
				ui.e_end_popup_menu();
			ui.e_end_list();
		ui.e_end_scrollbar();

	ui.e_end_docker_page();

	ev_file_changed = create_event(false);
	ev_end_file_watcher = add_file_watcher(base_path.c_str(), [](Capture& c, FileChangeType type, const wchar_t* filename) {
		set_event(scene_editor.resource_explorer->ev_file_changed);
	}, Capture(), true, false);

	navigate(base_path);
}

cResourceExplorer::~cResourceExplorer()
{
	if (scene_editor.window)
	{
		auto canvas = scene_editor.window->canvas;
		canvas->set_resource(folder_img_idx, nullptr);
		Image::destroy(folder_img);
		canvas->set_resource(file_img_idx, nullptr);
		Image::destroy(file_img);
		canvas->set_resource(thumbnails_img_idx, nullptr);
		Image::destroy(thumbnails_img);
	}

	destroy_event(ev_file_changed);
	set_event(ev_end_file_watcher);

	scene_editor.resource_explorer = nullptr;
}

Entity* cResourceExplorer::create_listitem(std::wstring_view title, uint img_id)
{
	auto& ui = scene_editor.window->ui;
	ui.push_style(FrameColorNormal, common(cvec4(0)));
	auto e_item = ui.e_list_item(L"", 0);
	ui.pop_style(FrameColorNormal);
	ui.c_layout(LayoutVertical)->item_padding = 4.f;
	ui.parents.push(e_item);
	ui.next_element_size = 64.f;
	ui.e_image(img_id << 16);
	ui.e_text(app.font_atlas->wrap_text(ui.style(FontSize).u.x, 64.f,
		title.c_str(), title.c_str() + title.size()).c_str());
	ui.parents.pop();
	return e_item;
}

void cResourceExplorer::navigate(const std::filesystem::path& path)
{
	curr_path = path;

	looper().add_event([](Capture& c) {
		auto& ui = scene_editor.window->ui;
		auto& base_path = scene_editor.resource_explorer->base_path;
		auto& curr_path = scene_editor.resource_explorer->curr_path;
		auto address_bar = scene_editor.resource_explorer->address_bar;
		auto list = scene_editor.resource_explorer->e_list;

		address_bar->remove_children(0, -1);
		ui.parents.push(address_bar);
		ui.push_style(ButtonColorNormal, common(cvec4(0)));
		ui.push_style(ButtonColorHovering, common(ui.style(FrameColorHovering).c));
		ui.push_style(ButtonColorActive, common(ui.style(FrameColorActive).c));

		ui.e_button(Icon_LEVEL_UP, [](Capture& c) {
			if (scene_editor.resource_explorer->curr_path != scene_editor.resource_explorer->base_path)
				scene_editor.resource_explorer->navigate(scene_editor.resource_explorer->curr_path.parent_path());
		}, Capture());

		std::vector<std::filesystem::path> stems;
		for (auto p = curr_path; ; p = p.parent_path())
		{
			stems.push_back(p);
			if (p == base_path)
				break;
		}
		std::reverse(stems.begin(), stems.end());

		for (auto i = 0; i < stems.size(); i++)
		{
			auto& s = stems[i];
			struct Capturing
			{
				wchar_t p[256];
			}capture;
			wcscpy_s(capture.p, s.c_str());
			ui.e_button(i == 0 ? s.c_str() : s.filename().c_str(), [](Capture& c) {
				auto& capture = c.data<Capturing>();
				scene_editor.resource_explorer->navigate(capture.p);
			}, Capture().set_data(&capture));

			std::vector<std::filesystem::path> sub_dirs;
			for (auto& it : std::filesystem::directory_iterator(s))
			{
				if (std::filesystem::is_directory(it.status()))
					sub_dirs.push_back(it.path());
			}
			if (!sub_dirs.empty())
			{
				ui.e_begin_button_menu(Icon_CARET_RIGHT);
				for (auto& p : sub_dirs)
				{
					struct Capturing
					{
						wchar_t p[256];
					}capture;
					wcscpy_s(capture.p, s.c_str());
					ui.e_menu_item(p.filename().c_str(), [](Capture& c) {
						auto& capture = c.data<Capturing>();
						scene_editor.resource_explorer->navigate(capture.p);
					}, Capture().set_data(&capture));
				}
				ui.e_end_button_menu();
			}
		}

		ui.pop_style(ButtonColorNormal);
		ui.pop_style(ButtonColorHovering);
		ui.pop_style(ButtonColorActive);
		ui.parents.pop();

		clear_all_works();
		looper().remove_all_events(FLAME_CHASH("update thumbnail"));

		list->get_component(cList)->set_selected(nullptr, false);
		list->remove_children(0, -1);

		std::vector<std::filesystem::path> dirs;
		std::vector<std::filesystem::path> files;
		for (auto& it : std::filesystem::directory_iterator(curr_path))
		{
			if (std::filesystem::is_directory(it.status()))
			{
				if (it.path().filename().wstring()[0] != L'.')
					dirs.push_back(it.path());
			}
			else
			{
				auto ext = it.path().extension();
				if (ext != L".ilk" && ext != L".exp")
					files.push_back(it.path());
			}
		}
		ui.parents.push(list);
		for (auto& p : dirs)
		{
			auto item = scene_editor.resource_explorer->create_listitem(p.filename().wstring(), scene_editor.resource_explorer->folder_img_idx);
			struct Capturing
			{
				wchar_t p[256];
			}capture;
			wcscpy_s(capture.p, p.c_str());
			item->get_component(cReceiver)->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const ivec2& pos) {
				if (is_mouse_clicked(action, key) && (action & KeyStateDouble))
				{
					auto& capture = c.data<Capturing>();
					scene_editor.resource_explorer->navigate(capture.p);
				}
				return true;
			}, Capture().set_data(&capture));
			ui.parents.push(item);
			ui.e_begin_popup_menu();
			ui.e_menu_item(L"Open", [](Capture& c) {
				auto& capture = c.data<Capturing>();
				scene_editor.resource_explorer->selected = capture.p;
				scene_editor.resource_explorer->navigate(scene_editor.resource_explorer->selected);
			}, Capture().set_data(&capture));
			ui.e_end_popup_menu();
			ui.parents.pop();
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

			auto item = scene_editor.resource_explorer->create_listitem(p.filename().wstring(), is_image_type ? 0 : scene_editor.resource_explorer->file_img_idx);
			if (is_image_type)
			{
				auto e_image = item->children[0];
				e_image->get_component(cImage)->color = cvec4(100, 100, 100, 128);

				auto c_thumbnail = new<cThumbnail>();
				c_thumbnail->filename = std::filesystem::canonical(p).wstring();
				e_image->add_component(c_thumbnail);
			}
			ui.parents.push(item);
			if (ext == L".prefab")
			{
				struct Capturing
				{
					wchar_t p[256];
				}capture;
				wcscpy_s(capture.p, p.c_str());
				auto er = item->get_component(cReceiver);
				er->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const ivec2& pos) {
					if (is_mouse_clicked(action, key) && (action & KeyStateDouble))
					{
						auto& capture = c.data<Capturing>();
						scene_editor.resource_explorer->selected = capture.p;
						scene_editor.load(scene_editor.resource_explorer->selected);
					}
					return true;
				}, Capture().set_data(&capture));
				ui.e_begin_popup_menu();
				ui.e_menu_item(L"Open", [](Capture& c) {
					c.thiz<cReceiver>()->send_mouse_event(KeyStateDown | KeyStateUp | KeyStateDouble, Mouse_Null, ivec2(0));
				}, Capture().set_thiz(er));
				ui.e_end_popup_menu();
			}
			else if (ext == L".bp")
			{
				struct Capturing
				{
					wchar_t p[256];
				}capture;
				wcscpy_s(capture.p, p.c_str());
				auto er = item->get_component(cReceiver);
				er->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const ivec2& pos) {
					if (is_mouse_clicked(action, key) && (action & KeyStateDouble))
					{
						auto& capture = c.data<Capturing>();
						scene_editor.resource_explorer->selected = capture.p;
						if (!bp_editor.window)
							new BPEditorWindow(scene_editor.resource_explorer->selected);
					}
					return true;
				}, Capture().set_data(&capture));
				ui.e_begin_popup_menu();
				ui.e_menu_item(L"Open", [](Capture& c) {
					c.thiz<cReceiver>()->send_mouse_event(KeyStateDown | KeyStateUp | KeyStateDouble, Mouse_Null, ivec2(0));
				}, Capture().set_thiz(er));
				ui.e_end_popup_menu();
			}
			ui.parents.pop();
		}
		ui.parents.pop();
	}, Capture().set_thiz(this));
}

void cResourceExplorer::on_component_added(Component* c)
{
	if (c->name_hash == FLAME_CHASH("cElement"))
	{
		((cElement*)c)->cmds.add([](Capture& c, graphics::Canvas* canvas) {
			scene_editor.resource_explorer->draw(canvas);
			return true;
		}, Capture());
	}
}

void cResourceExplorer::draw(graphics::Canvas* canvas)
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
		auto w = c_list_element->size.x - c_list_element->padding.xz().sum();
		c_list_layout->set_column(max(1U, uint(w / (c_list_layout->item_padding + 64.f))));
	}
}
