#include <flame/universe/application.h>
#include <flame/foundation/bitmap.h>
#include <flame/foundation/system.h>

using namespace flame;

struct Item
{
	std::filesystem::path path;
};

struct ShowItem
{
	uint idx;
	uint64 file_size;
	std::unique_ptr<graphics::Image> image;
	float thumbnail_scale;
};

auto image_size = 500;

struct App : UniverseApplication
{
	std::filesystem::path directory;
	std::vector<Item> all_items;
	std::vector<ShowItem> show_list;

	void init();
	void open(const std::filesystem::path& path);
	void select_random();
};

App app;

void App::init()
{
	app.create(true, "Media Browser", uvec2(1280, 720), WindowFrame | WindowResizable | WindowMaximized);
	app.always_render = false;

	app.main_window->imgui_callbacks.add([this]() {
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 2.0f));
		ImGui::Begin("Main", nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus);
		ImGui::PopStyleVar(2);
		ImGui::Text("items: %d", all_items.size());
		if (ImGui::Button("Random"))
			select_random();
		if (!all_items.empty())
		{
			auto& style = ImGui::GetStyle();
			ImGui::BeginChild("items", ImVec2(0, -ImGui::GetFontSize() - style.ItemSpacing.y * 2));
			auto window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
			auto spacing = style.ItemSpacing.x;
			auto hovered_idx = -1;
			for (auto i = 0; i < show_list.size(); i++)
			{
				auto& item = show_list[i];
				ImGui::PushID(i);

				auto pressed = ImGui::InvisibleButton("", (vec2)item.image->size * item.thumbnail_scale + vec2(4));
				auto p0 = (vec2)ImGui::GetItemRectMin();
				auto p1 = (vec2)ImGui::GetItemRectMax();
				auto active = ImGui::IsItemActive();
				auto hovered = ImGui::IsItemHovered();
				if (hovered)
					hovered_idx = i;
				ImU32 col;
				if (active)			col = ImGui::GetColorU32(ImGuiCol_ButtonActive);
				else if (hovered)	col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
				else				col = ImColor(0, 0, 0, 0);
				auto draw_list = ImGui::GetWindowDrawList();
				draw_list->AddRectFilled(p0, p1, col);
				draw_list->AddImage(item.image.get(), p0 + vec2(2), p1 - vec2(2));

				ImGui::PopID();
				float next_x2 = ImGui::GetItemRectMax().x + spacing + image_size;
				if (i + 1 < all_items.size() && next_x2 < window_visible_x2)
					ImGui::SameLine();
			}
			ImGui::EndChild();

			if (hovered_idx != -1)
			{
				auto& item = show_list[hovered_idx];
				ImGui::Text("%s %d %dx%d", all_items[item.idx].path.string().c_str(), (uint)item.file_size, item.image->size.x, item.image->size.y);
			}
		}
		ImGui::End();
	});
}

void App::open(const std::filesystem::path& path)
{
	main_window->native->set_title("Media Browser - " + path.string());

	directory = path;
	for (auto& it : std::filesystem::recursive_directory_iterator(path))
	{
		Item item;
		item.path = it.path();
		all_items.push_back(item);
	}
}

void App::select_random()
{
	graphics::Queue::get(nullptr)->wait_idle();
	show_list.clear();
	auto find_idx = [&](int i) {
		for (auto& it : show_list)
		{
			if (it.idx == i)
				return true;
		}
		return false;
	};
	auto n = min((int)all_items.size(), 5);
	for (auto i = 0; i < n; i++)
	{
		auto idx = linearRand(0, (int)all_items.size() - 1);
		if (!find_idx(idx))
		{
			auto& path = all_items[idx].path;
			auto ext = path.extension();
			if (ext == L".jpg" || ext == L".jpeg" || ext == L".png")
			{
				std::unique_ptr<Bitmap> bmp(Bitmap::create(path, 4));
				auto img = graphics::Image::create(nullptr, bmp.get());
				if (img->size.x > image_size && img->size.y > image_size)
				{
					auto aspect = (float)img->size.x / (float)img->size.y;
					if (aspect < 4.f)
					{
						auto& item = show_list.emplace_back();
						item.idx = idx;
						item.file_size = get_file_size(path);
						item.image.reset(img);
						item.thumbnail_scale = (float)image_size / (float)img->size.y;
						continue;
					}
				}
				delete img;
			}
		}
		i--;
	}
}

int main(int argc, char** args)
{
	app.init();

	std::filesystem::path preferences_path = L"preferences.ini";

	auto preferences_i = parse_ini_file(preferences_path);
	for (auto& e : preferences_i.get_section_entries("path"))
	{
		app.open(e.value);
		break;
	}

	app.run();

	return 0;
}
