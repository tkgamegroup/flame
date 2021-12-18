#pragma once

#include "../entity_private.h"
#include "file_selector.h"

namespace flame
{
	template <class T>
	struct ListItem
	{
		EntityPrivate* e;
		cTextPrivate* t;
		cReceiverPrivate* r;
		T data;
	};

	template <class T>
	struct List
	{
		EntityPrivate* e;
		cListPrivate* d;
		std::vector<ListItem<T>> items;

		void build(EntityPrivate* _e)
		{
			e = _e;
			d = e->get_component_t<cListPrivate>();
		}

		int find(EntityPrivate* e)
		{
			auto idx = 0;
			for (auto i : items)
			{
				if (i.e == e)
					return idx;
				idx++;
			}
			return -1;
		}

		T& find_data(EntityPrivate* e)
		{
			return items[find(e)].data;
		}

		ListItem<T>& add(const T& data)
		{
			auto& ret = items.emplace_back();
			ret.e = new EntityPrivate;
			ret.e->load(L"prefabs/list_item");
			ret.t = ret.e->get_component_t<cTextPrivate>();
			ret.r = ret.e->get_component_t<cReceiverPrivate>();
			ret.data = data;
			e->add_child(ret.e);
			return ret;
		}

		void remove(EntityPrivate* _e)
		{
			items.erase(items.begin() + find(_e));
			e->remove_child(_e);
		}
	};

	static std::filesystem::path recent_folders_path;

	struct cFileSelectorPrivate : cFileSelector
	{
		std::vector<std::filesystem::path> folders;
		int folder_idx = -1;

		EntityPrivate* back_btn;
		EntityPrivate* forward_btn;
		EntityPrivate* up_btn;

		EntityPrivate* folder_edit;
		cTextPrivate* folder_text;

		EntityPrivate* clear_recent_btn;
		List<std::filesystem::path> recent_list;
		List<std::wstring> file_list;

		EntityPrivate* path_edit;
		cTextPrivate* path_text;
		EntityPrivate* ok_btn;
		EntityPrivate* cancel_btn;

		std::vector<std::unique_ptr<Closure<void(Capture&, bool, const wchar_t*)>>> callbacks;

		void set_folder(const wchar_t* path) override { set_folder(std::filesystem::path(path)); }
		void set_folder(const std::filesystem::path& path);

		void* add_callback(void (*callback)(Capture& c, bool ok, const wchar_t* text), const Capture& capture) override;
		void remove_callback(void* ret) override;

		void on_load_finished() override;

		std::filesystem::path& curr_folder() { return folders[folder_idx]; }
		void folder_changed();
		void confirm();
	};
}
