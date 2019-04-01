#include <regex>

#include <flame/global.h>
#include <flame/engine/ui/fileselector.h>
#include "log_dog.h"

LogDog::Column::Column() :
	match_index(0)
{
}

LogDog::LogDog() :
	Window("Log Dog"),
	log_file_timestamp(0),
	curr_page(0)
{
	strcpy(match_regex, R"((\[[\w\s:]+\])[\s]*(INFO|DEBUG|WARNING|ERROR)[:\s]*(.*))");
	
	auto col1 = new Column;
	strcpy(col1->name, "Time");
	col1->match_index = 1;
	columns.emplace_back(col1);
	auto col2 = new Column;
	strcpy(col2->name, "Level");
	col2->match_index = 2;
	columns.emplace_back(col2);
	auto col3 = new Column;
	strcpy(col3->name, "Text");
	col3->match_index = 3;
	columns.emplace_back(col3);

	set_log_filename("F:\\SVN_branch_gamma\\bin\\PTGame.as.log");
}

LogDog::~LogDog()
{
	log_dog = nullptr;
}

struct SelectFileDialog : flame::ui::FileSelector
{
	LogDog *dst;

	SelectFileDialog(LogDog *_dst) :
		FileSelector("Select File", flame::ui::FileSelectorSave, "", flame::ui::WindowModal | flame::ui::WindowNoSavedSettings,
			flame::ui::FileSelectorNoRightArea),
		dst(_dst)
	{
		first_cx = 800;
		first_cy = 600;

		callback = [this](std::string s) {
			dst->set_log_filename(s);
			return true;
		};
	}
};

void LogDog::set_log_filename(const std::string &filename)
{
	log_filename = filename;
	std::ifstream file(log_filename);
	if (!file.good())
		return;

	std::regex regex(match_regex);
	while (!file.eof())
	{
		std::string line;
		std::getline(file, line);
		std::smatch match;
		if (std::regex_search(line, match, regex))
		{
			std::vector<std::string> row(columns.size());
			for (auto i = 0; i < columns.size(); i++)
			{
				auto c = columns[i].get();
				if (c->match_index < match.size())
					row[i] = match[c->match_index].str();
			}
			logs.push_back(row);
		}
	}

	curr_page = 0;
}

void LogDog::on_show()
{
	ImGui::BeginTabBar("tabbar"/*, ImGuiTabBarFlags_NoReorder*/);
	if (ImGui::TabItem("File"))
	{
		ImGui::TextUnformatted(log_filename.c_str());
		ImGui::SameLine();
		if (ImGui::IconButton(ICON_FA_ELLIPSIS_H))
			new SelectFileDialog(this);
	}
	if (ImGui::TabItem("Columns"))
	{
		if (ImGui::IsItemActive())
		{
			auto mouseY = ImGui::GetMousePos().y;
			if (mouseY < ImGui::GetItemRectMin().y || mouseY > ImGui::GetItemRectMax().y)
			{
				int cut = 1;
			}
		}

		if (ImGui::InputText("Match Regex", match_regex, TK_ARRAYSIZE(match_regex)))
		{

		}
		ImGui::BeginChild("columns", ImVec2(350, 150), true);
		ImGui::Columns(3, "_column_columns");
		ImGui::Separator();
		ImGui::Text("Name"); ImGui::NextColumn();
		ImGui::Text("Match Index"); ImGui::NextColumn();
		ImGui::Text("Operatation"); ImGui::NextColumn();
		ImGui::Separator();
		auto op = 0;
		int op_obj;
		for (auto i = 0; i < columns.size(); i++)
		{
			auto c = columns[i].get();
			ImGui::PushID(i);
			ImGui::InputText("##name", c->name, TK_ARRAYSIZE(c->name)); ImGui::NextColumn();
			ImGui::InputInt("##index", &c->match_index); ImGui::NextColumn();
			if (ImGui::IconButton(ICON_FA_TRASH))
			{
				op = 1;
				op_obj = i;
			}
			ImGui::SameLine();
			if (ImGui::IconButton(ICON_FA_ARROW_UP))
			{
				op = 2;
				op_obj = i;
			}
			ImGui::SameLine();
			if (ImGui::IconButton(ICON_FA_ARROW_DOWN))
			{
				op = 3;
				op_obj = i;
			}
			ImGui::NextColumn();
			ImGui::PopID();
		}
		switch (op)
		{
			case 1:
				columns.erase(columns.begin() + op_obj);
				break;
			case 2:
				if (op_obj > 0)
					std::swap(columns[op_obj], columns[op_obj - 1]);
				break;
			case 3:
				if (op_obj < columns.size() - 1)
					std::swap(columns[op_obj], columns[op_obj + 1]);
				break;
		}
		ImGui::Columns(1);
		ImGui::EndChild();
		ImGui::SameLine();
		ImGui::Text("Columns");
		if (ImGui::Button("New Column"))
		{
			auto col = new Column;
			strcpy(col->name, "NewColumn");
			columns.emplace_back(new Column);
		}
	}
	if (ImGui::TabItem("Log"))
	{
		static bool show_error;
		ImGui::Checkbox_2in1("Error", &show_error);
		ImGui::Columns(columns.size(), "_log_columns");
		ImGui::Separator();
		for (auto &c : columns)
		{
			ImGui::Text(c->name);
			ImGui::NextColumn();
		}
		ImGui::Separator();
		const int element_per_page = 15;
		int total_page = logs.size() / element_per_page;
		for (int i = 0; i < element_per_page; i++)
		{
			auto index = curr_page * element_per_page + i;
			if (index >= logs.size())
				break;
			auto &l = logs[index];
			for (auto j = 0; j < columns.size(); j++)
			{
				ImGui::Text(l[j].c_str());
				ImGui::NextColumn();
			}
		}
		ImGui::Columns(1);
		ImGui::Separator();
		ImGui::Text("%d/%d", curr_page + 1, total_page);
		ImGui::SameLine();
		if (ImGui::IconButton(ICON_FA_CARET_LEFT))
		{
			if (curr_page > 0)
				curr_page--;
		}
		ImGui::SameLine();
		if (ImGui::IconButton(ICON_FA_CARET_RIGHT))
		{
			if (curr_page < total_page - 1)
				curr_page++;
		}
	}
	ImGui::EndTabBar();
}

LogDog *log_dog = nullptr;
