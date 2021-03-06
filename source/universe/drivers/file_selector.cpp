#include "../../graphics/font.h"
#include "list_private.h"
#include "../components/receiver_private.h"
#include "../components/text_private.h"
#include "file_selector_private.h"

namespace flame
{
	void dFileSelectorPrivate::folder_changed()
	{
		auto& folder = curr_folder();

		folder_text->set_text(folder.c_str());

		file_list.d->set_selected(nullptr);
		file_list.e->remove_all_children();
		file_list.items.clear();
		for (auto& it : std::filesystem::directory_iterator(folder))
		{
			auto& item = file_list.add(it.path().filename().wstring());

			item.r->add_mouse_click_listener([](Capture& c) {
				auto thiz = c.thiz<dFileSelectorPrivate>();
				thiz->path_text->set_text(thiz->file_list.find_data(c.data<EntityPrivate*>()));
			}, Capture().set_thiz(this).set_data(&item.e));

			if (it.is_directory())
			{
				item.r->add_mouse_dbclick_listener([](Capture& c) {
					auto thiz = c.thiz<dFileSelectorPrivate>();
					auto path = thiz->curr_folder() / thiz->file_list.find_data(c.data<EntityPrivate*>());
					thiz->set_folder(path);
				}, Capture().set_thiz(this).set_data(&item.e));

				item.t->set_text(std::wstring(Icon_FOLDER) + L" " + item.data);
			}
			else
			{
				item.r->add_mouse_dbclick_listener([](Capture& c) {
					auto thiz = c.thiz<dFileSelectorPrivate>();
					thiz->confirm();
				}, Capture().set_thiz(this).set_data(&item.e));
				item.t->set_text(std::wstring(Icon_FILE) + L" " + item.data);
			}
		}
	}

	void dFileSelectorPrivate::confirm()
	{
		if (!path_text->text.empty())
		{
			auto folder = curr_folder();
			std::ofstream file(recent_folders_path, std::ios::ate);
			file << folder.string() << "\n";
			file.close();
			auto path = folder / path_text->text;
			for (auto& cb : callbacks)
				cb->call(true, path.c_str());
		}
	}

	void dFileSelectorPrivate::set_folder(const std::filesystem::path& path)
	{
		folders.resize(folder_idx + 1);
		if (folders.empty() || path != folders.back())
		{
			folders.push_back(path);
			folder_idx++;
			folder_changed();
		}
	}

	void* dFileSelectorPrivate::add_callback(void (*callback)(Capture& c, bool ok, const wchar_t* text), const Capture& capture)
	{
		if (!callback)
		{
			auto slot = (uint)&capture;
			callback = [](Capture& c, bool ok, const wchar_t* text) {
				auto scr_ins = script::Instance::get_default();
				scr_ins->get_global("callbacks");
				scr_ins->get_member(nullptr, c.data<uint>());
				scr_ins->get_member("f");
				scr_ins->push_bool(ok);
				scr_ins->push_string(text ? w2s(text).c_str() : "");
				scr_ins->call(2);
				scr_ins->pop(2);
			};
			auto c = new Closure(callback, Capture().set_data(&slot));
			callbacks.emplace_back(c);
			return c;
		}
		auto c = new Closure(callback, capture);
		callbacks.emplace_back(c);
		return c;
	}

	void dFileSelectorPrivate::remove_callback(void* ret)
	{
		std::erase_if(callbacks, [&](const auto& i) {
			return i.get() == (decltype(i.get()))ret;
		});
	}

	void dFileSelectorPrivate::on_load_finished()
	{
		back_btn = entity->find_child("back_btn");
		fassert(back_btn);
		back_btn->get_component_t<cReceiverPrivate>()->add_mouse_click_listener([](Capture& c) {
			auto thiz = c.thiz<dFileSelectorPrivate>();
			if (thiz->folder_idx > 0)
				thiz->folder_idx--;
			thiz->folder_changed();
		}, Capture().set_thiz(this));

		forward_btn = entity->find_child("forward_btn");
		fassert(forward_btn);
		forward_btn->get_component_t<cReceiverPrivate>()->add_mouse_click_listener([](Capture& c) {
			auto thiz = c.thiz<dFileSelectorPrivate>();
			if (thiz->folder_idx < thiz->folders.size() - 1)
				thiz->folder_idx++;
			thiz->folder_changed();
		}, Capture().set_thiz(this));

		up_btn = entity->find_child("up_btn");
		fassert(up_btn);
		up_btn->get_component_t<cReceiverPrivate>()->add_mouse_click_listener([](Capture& c) {
			auto thiz = c.thiz<dFileSelectorPrivate>();
			thiz->set_folder(thiz->curr_folder().parent_path());
		}, Capture().set_thiz(this));

		folder_edit = entity->find_child("folder_edit");
		fassert(folder_edit);
		folder_text = folder_edit->get_component_t<cTextPrivate>();
		fassert(folder_text);

		file_list.build(entity->find_child("file_list"));
		fassert(file_list.e && file_list.d);

		clear_recent_btn = entity->find_child("clear_recent_btn");
		if (clear_recent_btn)
		{
			clear_recent_btn->get_component_t<cReceiverPrivate>()->add_mouse_click_listener([](Capture& c) {
				auto thiz = c.thiz<dFileSelectorPrivate>();
				thiz->recent_list.e->remove_all_children();
				std::ofstream file(recent_folders_path, std::ios::trunc);
				file.close();
			}, Capture().set_thiz(this));
		}

		recent_list.build(entity->find_child("recent_list"));
		fassert(recent_list.e && recent_list.d);

		path_edit = entity->find_child("path_edit");
		fassert(path_edit);
		path_text = path_edit->get_component_t<cTextPrivate>();
		fassert(path_text);

		ok_btn = entity->find_child("ok_btn");
		fassert(ok_btn);
		ok_btn->get_component_t<cReceiverPrivate>()->add_mouse_click_listener([](Capture& c) {
			auto thiz = c.thiz<dFileSelectorPrivate>();
			thiz->confirm();
		}, Capture().set_thiz(this));

		cancel_btn = entity->find_child("cancel_btn");
		fassert(cancel_btn);
		cancel_btn->get_component_t<cReceiverPrivate>()->add_mouse_click_listener([](Capture& c) {
			auto thiz = c.thiz<dFileSelectorPrivate>();
			for (auto& cb : thiz->callbacks)
				cb->call(false, nullptr);
		}, Capture().set_thiz(this));

		wchar_t buf[256];

		if (recent_folders_path.empty())
		{
			get_app_path(buf);
			recent_folders_path = std::filesystem::path(buf) / L"recent_folders.txt";
		}
		if (std::filesystem::exists(recent_folders_path))
		{
			std::ifstream file(recent_folders_path);
			while (!file.eof())
			{
				std::string line;
				std::getline(file, line);
				if (!line.empty())
				{
					auto& item = recent_list.add(line);
					item.t->set_text(item.data.filename());
					item.r->add_mouse_click_listener([](Capture& c) {
						auto thiz = c.thiz<dFileSelectorPrivate>();
						thiz->set_folder(thiz->recent_list.find_data(c.data<EntityPrivate*>()));
					}, Capture().set_thiz(this).set_data(&item.e));
				}
			}
			file.close();
		}

		get_current_path(buf);
		set_folder(buf);
	}

	dFileSelector* dFileSelector::create(void* parms)
	{
		return new dFileSelectorPrivate();
	}
}
