#include <flame/graphics/extension.h>
#include <flame/graphics/gui.h>
#include <flame/graphics/application.h>

using namespace flame;
using namespace graphics;

struct App : GraphicsApplication
{
	std::string strs;

	void on_gui() override
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Main", nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoSavedSettings);
		ImGui::PopStyleVar(2);

		ImGui::InputTextMultiline("##strs", &strs, ImVec2(400, 200), ImGuiInputTextFlags_AllowTabInput);
		auto i = 0;
		for (auto sp : SUS::split(strs, '\n'))
		{
			auto hash_str = str(sh(sp));
			ImGui::TextUnformatted(hash_str.c_str());
			ImGui::SameLine();
			ImGui::PushID(i);
			if (ImGui::Button("Copy"))
				ImGui::SetClipboardText(hash_str.c_str());
			ImGui::PopID();
			i++;
		}

		ImGui::End();
	}
};

static App app;

int main(int argc, char** args)
{
	app.create("String Hasher", uvec2(800, 600), WindowFrame | WindowResizable, true, true);
	app.run();

	return 0;
}
