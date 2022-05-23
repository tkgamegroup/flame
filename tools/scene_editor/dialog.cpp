#include "dialog.h"

DialogManager dialog_manager;

void Dialog::open(Dialog* dialog)
{
	dialog_manager.dialogs.emplace_back(dialog);
	ImGui::OpenPopup(dialog->title.c_str());
}

void Dialog::close()
{
	assert(!dialog_manager.dialogs.empty());
	ImGui::CloseCurrentPopup();
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
	for (auto& d : dialogs)
		d->draw();
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
			close();
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
		}
		ImGui::SameLine();
		if (ImGui::Button("No"))
		{
			if (callback)
				callback(false);
			close();
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
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			if (callback)
				callback(false, "");
			close();
		}
		ImGui::EndPopup();
	}
}

void SelectResourceDialog::open(const std::string& title, const std::function<void(bool, const std::filesystem::path&)>& callback)
{
	auto dialog = new SelectResourceDialog;
	dialog->title = title;
	dialog->callback = callback;
	dialog->explorer.reset(app.project_path / L"assets");
	dialog->explorer.peeding_open_node = { dialog->explorer.folder_tree.get(), false };
	dialog->explorer.select_callback = [dialog](const std::filesystem::path& path) {
		dialog->path = path;
	};
	dialog->explorer.dbclick_callback = [dialog](const std::filesystem::path& path) {
		if (dialog->callback)
			dialog->callback(true, path);
		dialog->close();
	};
	Dialog::open(dialog);
}

void SelectResourceDialog::draw()
{
	if (ImGui::BeginPopupModal(title.c_str()))
	{
		ImGui::BeginChild("explorer", ImVec2(0, -ImGui::GetFontSize() - ImGui::GetStyle().ItemSpacing.y * 3));
		explorer.selected_path = path;
		explorer.draw();
		ImGui::EndChild();
		if (ImGui::Button("OK"))
		{
			if (callback)
				callback(true, path);
			close();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			if (callback)
				callback(false, L"");
			close();
		}
		ImGui::EndPopup();
	}
}
