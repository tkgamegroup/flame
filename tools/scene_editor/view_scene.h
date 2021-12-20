#pragma once

#include "app.h"

struct View_Scene : View
{
	graphics::Image* render_tar = nullptr;

	//Entity* e_prefab = nullptr;

	View_Scene();

	void open_scene(const std::filesystem::path& path);
	void open_prefab(const std::filesystem::path& path);

	void on_draw() override;
};

extern View_Scene view_scene;
