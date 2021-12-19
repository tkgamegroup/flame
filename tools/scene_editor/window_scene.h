#pragma once

#include "app.h"

struct WindowScene : View
{
	graphics::Image* render_tar = nullptr;

	//Entity* e_prefab = nullptr;

	WindowScene();

	void open_scene(const std::filesystem::path& path);
	void open_prefab(const std::filesystem::path& path);

	void on_draw() override;
};

extern WindowScene window_scene;
