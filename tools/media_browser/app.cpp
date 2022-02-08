#include <flame/universe/application.h>
#include <flame/foundation/bitmap.h>
#include <flame/foundation/system.h>

using namespace flame;

struct File
{
	std::filesystem::path path;
	std::u8string path_u8;
	uint64 size = 0;
};

struct ShowItem
{
	uint idx;
	std::unique_ptr<graphics::Image> image;
	float thumbnail_scale;

	File& file() const;
};

auto thumbnail_size = 500;

struct App : UniverseApplication
{
	std::filesystem::path directory;
	std::vector<File> files;
	std::vector<ShowItem> show_items;
	int showing_item_idx = -1;
	float showing_item_scl = 1.f;
	vec2 showing_item_off = vec2(0.f);
	int peeding_delete_idx = -1;

	void init();
	void open_dir(const std::filesystem::path& path);
	ShowItem& add_show_item(int idx, graphics::ImagePtr image);
	void select_random();
	void reset_showing_item();
};

App app;

File& ShowItem::file() const
{
	return app.files[idx];
}

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

		if (peeding_delete_idx != -1)
		{
			graphics::Queue::get(nullptr)->wait_idle();
			auto& item = show_items[peeding_delete_idx];
			move_to_recycle_bin(item.file().path);
			item.file().path.clear();
			show_items.erase(show_items.begin() + peeding_delete_idx);
			if (showing_item_idx == peeding_delete_idx && peeding_delete_idx >= show_items.size())
				showing_item_idx = (int)show_items.size() - 1;
			reset_showing_item();

			peeding_delete_idx = -1;
		}

		if (showing_item_idx == -1)
		{
			if (ImGui::Button("Select Dir"))
				ifd::FileDialog::Instance().Open("OpenDialog", "Select a directory", "");
			ImGui::SameLine();
			ImGui::Text("items: %d", files.size());
			if (ImGui::Button("Random"))
				select_random();
			if (!files.empty())
			{
				auto& style = ImGui::GetStyle();
				ImGui::BeginChild("items", ImVec2(0, -ImGui::GetFontSize() - style.ItemSpacing.y * 2));
				auto window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
				auto spacing = style.ItemSpacing.x;
				auto hovered_idx = -1;
				for (auto i = 0; i < show_items.size(); i++)
				{
					auto& item = show_items[i];
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
					float next_x2 = ImGui::GetItemRectMax().x + spacing + thumbnail_size;
					if (i + 1 < files.size() && next_x2 < window_visible_x2)
						ImGui::SameLine();

					if (pressed)
					{
						showing_item_idx = i;
						reset_showing_item();
					}
				}
				ImGui::EndChild();

				if (hovered_idx != -1)
				{
					auto& item = show_items[hovered_idx];
					ImGui::Text("%s %d %dx%d", item.file().path_u8.c_str(), (uint)item.file().size, item.image->size.x, item.image->size.y);

					if (ImGui::IsKeyPressed(Keyboard_Del) || ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
						peeding_delete_idx = hovered_idx;
				}
			}
			if (ifd::FileDialog::Instance().IsDone("OpenDialog"))
			{
				if (ifd::FileDialog::Instance().HasResult())
					app.open_dir(ifd::FileDialog::Instance().GetResultFormated());
				ifd::FileDialog::Instance().Close();
			}
		}
		else
		{
			auto& item = show_items[showing_item_idx];

			auto& io = ImGui::GetIO();
			auto draw_list = ImGui::GetWindowDrawList();
			draw_list->AddImage(item.image.get(), showing_item_off, showing_item_off + (vec2)item.image->size * showing_item_scl);
			
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsKeyPressed(Keyboard_Esc))
				showing_item_idx = -1;
			else
			{
				if (auto disp = (vec2)io.MouseDelta; disp.x != 0.f || disp.y != 0.f)
				{
					if (io.MouseDown[ImGuiMouseButton_Left])
						showing_item_off += disp;
				}
				if (auto scroll = io.MouseWheel; scroll != 0.f)
				{
					auto p = ((vec2)io.MousePos - showing_item_off) / showing_item_scl;
					auto scl1 = showing_item_scl;
					if (scroll > 0.f)
						showing_item_scl = showing_item_scl * 1.05f + 0.05f;
					else
						showing_item_scl = max(0.1f, showing_item_scl / 1.05f - 0.05f);
					showing_item_off += p * (scl1 - showing_item_scl);
				}
				if (ImGui::IsKeyPressed(Keyboard_Left))
				{
					if (showing_item_idx > 0)
					{
						showing_item_idx--;
						reset_showing_item();
					}
				}
				if (ImGui::IsKeyPressed(Keyboard_Right))
				{
					if (showing_item_idx < show_items.size() - 1)
					{
						showing_item_idx++;
						reset_showing_item();
					}
				}
			}
		}

		ImGui::End();
	});
}

void App::open_dir(const std::filesystem::path& path)
{
	main_window->native->set_title("Media Browser - " + path.string());

	directory = path;
	files.clear();
	for (auto& it : std::filesystem::recursive_directory_iterator(path))
	{
		auto ext = it.path().extension();
		if (ext == L".jpg" || ext == L".jpeg" || ext == L".png")
		{
			File item;
			item.path = it.path();
			item.path_u8 = item.path.u8string();
			files.push_back(item);
		}
	}
}

ShowItem& App::add_show_item(int idx, graphics::ImagePtr image)
{
	auto& file = files[idx];
	if (!file.size)
		file.size = std::filesystem::file_size(file.path);
	auto& ret = show_items.emplace_back();
	ret.idx = idx;
	ret.image.reset(image);
	ret.thumbnail_scale = (float)thumbnail_size / (float)image->size.y;
	return ret;
}

void App::select_random()
{
	graphics::Queue::get(nullptr)->wait_idle();
	show_items.clear();
	auto valid_idx = [&](int idx) {
		if (files[idx].path.empty())
			return false;
		for (auto& item : show_items)
		{
			if (item.idx == idx)
				return false;
		}
		return true;
	};
	auto n = min((int)files.size(), 5);
	for (auto i = 0; i < n; i++)
	{
		auto idx = linearRand(0, (int)files.size() - 1);
		if (!valid_idx(idx)) continue;
		auto& path = files[idx].path;
		auto img = graphics::Image::create(nullptr, path, false);
		if (!img) continue;
		if (img->size.x > thumbnail_size && img->size.y > thumbnail_size)
		{
			auto aspect = (float)img->size.x / (float)img->size.y;
			if (aspect < 4.f)
			{
				add_show_item(idx, img);
				continue;
			}
		}
		delete img;
	}
}

void App::reset_showing_item()
{
	showing_item_off = vec2(0.f);
	showing_item_scl = 1.f;
}

int main(int argc, char** args)
{
	srand(time(0));

	app.init();

	std::filesystem::path preferences_path = L"preferences.ini";

	auto preferences_i = parse_ini_file(preferences_path);
	for (auto& e : preferences_i.get_section_entries("directory"))
	{
		app.open_dir(e.value);
		break;
	}

	app.run();

	std::ofstream preferences_o(preferences_path);
	if (!app.directory.empty())
	{
		preferences_o << "[directory]\n";
		preferences_o << app.directory.string() << "\n";
	}
	preferences_o.close();

	return 0;
}
