#include "dialog.h"

DialogManager dialog_manager;

void Dialog::open(Dialog* dialog)
{
	dialog_manager.dialogs.emplace_back(dialog);
}

void Dialog::close()
{
	assert(!dialog_manager.dialogs.empty());
	if (dialog_manager.dialogs.front().get() == this)
		dialog_manager.open_new = true;
	add_event([this]() {
		graphics::Queue::get()->wait_idle();
		std::erase_if(dialog_manager.dialogs, [&](const auto& i) {
			return i.get() == this;
		});
		return false;
	});
}

void DialogManager::update()
{
	if (open_new)
	{
		if (!dialogs.empty())
		{
			ImGui::OpenPopup(dialogs.front()->title.c_str());
			open_new = false;
		}
	}
	if (!dialogs.empty())
		dialogs.front()->draw();
}

void MessageDialog::open(const std::string& title, const std::string& message)
{
	auto dialog = new MessageDialog;
	if (title == "[RestructurePrefabInstanceWarnning]")
	{
		dialog->title = "Cannot restructure Prefab Instance";
		dialog->message = "You cannot add/remove/reorder entity or component in Prefab Instance\n"
			"Edit it in that prefab";
	}
	else
	{
		dialog->title = title;
		dialog->message = message;
	}
	Dialog::open(dialog);
}

void MessageDialog::draw()
{
	if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextUnformatted(message.c_str());
		if (ImGui::Button("OK"))
		{
			close();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void YesNoDialog::open(const std::string& title, const std::function<void(bool)>& callback)
{
	auto dialog = new YesNoDialog;
	dialog->title = title;
	dialog->callback = callback;
	Dialog::open(dialog);
}

void YesNoDialog::draw()
{
	if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (ImGui::Button("Yes"))
		{
			if (callback)
				callback(true);
			close();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("No"))
		{
			if (callback)
				callback(false);
			close();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void InputDialog::open(const std::string& title, const std::function<void(bool, const std::string&)>& callback)
{
	auto dialog = new InputDialog;
	dialog->title = title;
	dialog->callback = callback;
	Dialog::open(dialog);
}

void InputDialog::draw()
{
	if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::InputText("##text", &text);
		if (ImGui::Button("OK"))
		{
			if (callback)
				callback(true, text);
			close();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			if (callback)
				callback(false, "");
			close();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void SelectResourceDialog::open(const std::string& title, const std::function<void(bool, const std::filesystem::path&)>& callback)
{
	auto dialog = new SelectResourceDialog;
	dialog->title = title;
	dialog->callback = callback;
	dialog->resource_panel.reset(app.project_path / L"assets");
	dialog->resource_panel.peeding_open_node = { dialog->resource_panel.folder_tree.get(), false };
	dialog->resource_panel.select_callback = [dialog](const std::filesystem::path& path) {
		dialog->path = path;
	};
	dialog->resource_panel.dbclick_callback = [dialog](const std::filesystem::path& path) {
		if (dialog->callback)
			dialog->callback(true, path);
		dialog->close();
		ImGui::CloseCurrentPopup();
	};
	Dialog::open(dialog);
}

void SelectResourceDialog::draw()
{
	if (ImGui::BeginPopupModal(title.c_str()))
	{
		ImGui::BeginChild("resource_panel", ImVec2(0, -ImGui::GetFontSize() - ImGui::GetStyle().ItemSpacing.y * 3));
		resource_panel.selected_path = path;
		resource_panel.draw();
		ImGui::EndChild();
		if (ImGui::Button("OK"))
		{
			if (callback)
				callback(true, path);
			close();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			if (callback)
				callback(false, L"");
			close();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}
