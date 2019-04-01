#pragma once

#include <flame/engine/graphics/buffer.h>
#include <flame/engine/graphics/texture.h>
#include <flame/engine/ui/ui.h>
#include <flame/engine/ui/imageviewer.h>

struct ImageEditor : flame::ui::ImageViewer
{
	int penID;

	ImageEditor(std::shared_ptr<flame::Texture> _texture);
	virtual void on_menu_bar() override;
	virtual void on_top_area() override;
	virtual void on_mouse_overing_image(ImVec2 image_pos) override;
};
