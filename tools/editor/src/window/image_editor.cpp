#include <flame/global.h>
#include <flame/engine/core/input.h>
#include <flame/engine/graphics/command_buffer.h>

#include "image_editor.h"

static int _magic_number = 0;

ImageEditor::ImageEditor(std::shared_ptr<flame::Texture> _texture) :
	ImageViewer(_texture->filename != "" ? _texture->filename : "Image - " + std::to_string(_magic_number++), _texture),
	penID(-1)
{
}

void ImageEditor::on_menu_bar()
{
	if (ImGui::BeginMenu("Filter"))
	{
		ImGui::EndMenu();
	}
}

void ImageEditor::on_top_area()
{
	enum Mode
	{
		ModeTerrainBlendMap = 0
	};

	const char *modeNames[] = {
		"terrain blend map"
	};
	static int mode = ModeTerrainBlendMap;
	ImGui::Combo("mode", &mode, modeNames, TK_ARRAYSIZE(modeNames));

	switch (mode)
	{
		case ModeTerrainBlendMap:
			if (penID < -1 || penID > 3)
				penID = -1;
			ImGui::RadioButton("Null", &penID, -1);
			ImGui::SameLine();
			ImGui::RadioButton("R", &penID, 0);
			ImGui::SameLine();
			ImGui::RadioButton("G", &penID, 1);
			ImGui::SameLine();
			ImGui::RadioButton("B", &penID, 2);
			ImGui::SameLine();
			ImGui::RadioButton("A", &penID, 3);
			break;
	}
}

void ImageEditor::on_mouse_overing_image(ImVec2 image_pos)
{
	if (flame::mouse.button[0].pressing && penID != -1)
	{
		auto x = flame::mouse.x - image_pos.x;
		auto y = flame::mouse.y - image_pos.y;

		staging_buffer->map(texture->get_linear_offset(x, y), texture->bpp / 8);
		auto pixel = (unsigned char*)staging_buffer->mapped;
		switch (penID)
		{
			case 0:
				pixel[0] = 0; pixel[1] = 0; pixel[2] = 255; pixel[3] = 255;
				break;
			case 1:
				pixel[0] = 0; pixel[1] = 255; pixel[2] = 0; pixel[3] = 255;
				break;
			case 2:
				pixel[0] = 255; pixel[1] = 0; pixel[2] = 0; pixel[3] = 255;
				break;
			case 3:
				//pixel[0] = 0; pixel[1] = 0; pixel[2] = 0; pixel[3] = 0;
				pixel[3] = 0;
				break;
		}
		staging_buffer->unmap();

		{
			VkBufferImageCopy r = {};
			r.bufferOffset = texture->get_linear_offset(x, y);
			r.imageOffset.x = x;
			r.imageOffset.y = y;
			r.imageExtent.width = 1;
			r.imageExtent.height = 1;
			r.imageExtent.depth = 1;
			r.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			r.imageSubresource.layerCount = 1;

			auto cb = flame::begin_once_command_buffer();
			texture->transition_layout(cb, texture->layout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
			vkCmdCopyBufferToImage(cb->v, staging_buffer->v, texture->v, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 1, &r);
			texture->transition_layout(cb, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, texture->layout);
			flame::end_once_command_buffer(cb);
		}
	}
}
