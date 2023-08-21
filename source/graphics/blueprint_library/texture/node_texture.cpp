#include "node_texture.h"
#include "../../../foundation/typeinfo.h"
#include "../../buffer_private.h"
#include "../../image_private.h"
#include "../../shader_private.h"
#include "../../command_private.h"
#include "../../texture.h"
#include "../../noise.h"
#include "../../extension.h"

namespace flame
{
	namespace graphics
	{
		void add_node_templates_texture(BlueprintNodeLibraryPtr library)
		{
			library->add_template("New Texture",
				{
					{
						.name = "Format",
						.allowed_types = { TypeInfo::get<Format>() },
						.default_value = TypeInfo::serialize_t(Format_R8G8B8A8_UNORM)
					},
					{
						.name = "Extent",
						.allowed_types = { TypeInfo::get<uvec3>() },
						.default_value = "64,64,1"
					},
					{
						.name = "Color",
						.allowed_types = { TypeInfo::get<vec4>() },
						.default_value = "0,0,0,1"
					}
				},
				{
					{
						.name = "Texture",
						.allowed_types = { TypeInfo::get<Texture>() }
					}
				},
				[](BlueprintArgument* inputs, BlueprintArgument* outputs) {
					auto format = *(Format*)inputs[0].data;
					auto extent = *(uvec3*)inputs[1].data;
					auto color = *(vec4*)inputs[2].data;
					auto& texture = *(Texture*)outputs[0].data;
					texture.type = TextureImage;
					if (texture.image)
					{
						auto old_image = texture.image;
						add_event([old_image]() {
							delete old_image;
							return false;
						});
						texture.image = nullptr;
					}
					if (extent.x > 0 && extent.y > 0 && extent.z > 0)
					{
						texture.image = Image::create(format, extent, ImageUsageSampled | ImageUsageTransferDst);
						texture.image->clear(color, ImageLayoutShaderReadOnly);
					}
				},
				nullptr,
				[](BlueprintArgument* inputs, BlueprintArgument* outputs) {
					auto texture = *(Texture*)outputs[0].data;
					if (texture.image)
					{
						auto old_image = texture.image;
						add_event([old_image]() {
							delete old_image;
							return false; 
						});
					}
				},
				nullptr,
				[](BlueprintArgument* inputs, BlueprintArgument* outputs, BlueprintNodePreview* preview) {
					auto& texture = *(Texture*)outputs[0].data;
					preview->type = "image"_h;
					preview->data = texture.image;
				}
			);
			library->add_template("Voronoi Texture",
				{
				},
				{
					{
						.name = "Texture",
						.allowed_types = { TypeInfo::get<Texture>() }
					}
				},
				[](BlueprintArgument* inputs, BlueprintArgument* outputs) {
					auto& texture = *(Texture*)outputs[0].data;
					texture.type = TextureVoronoi;
					if (texture.image)
					{
						auto old_image = texture.image;
						add_event([old_image]() {
							delete old_image;
							return false;
						});
						texture.image = nullptr;
					}
					const auto preview_size = 256;
					if (!texture.image)
						// the image is only for preview
						texture.image = Image::create(Format_R8_UNORM, uvec3(preview_size, preview_size, 1), ImageUsageSampled | ImageUsageTransferSrc | ImageUsageTransferDst);
					for (auto x = 0; x < preview_size; x++)
					{
						for (auto y = 0; y < preview_size; y++)
						{
							texture.image->set_pixel(x, y, 0, 0, 
								vec4(voronoi_noise(vec2(x, y) * 0.05f), 0.f, 0.f, 0.f));
						}
					}
					texture.image->upload_pixels(0, 0, preview_size, preview_size, 0, 0);
				},
				nullptr,
				[](BlueprintArgument* inputs, BlueprintArgument* outputs) {
					auto& texture = *(Texture*)outputs[0].data;
					if (texture.image)
					{
						auto old_image = texture.image;
						add_event([old_image]() {
							delete old_image;
							return false;
						});
					}
				},
				nullptr,
				[](BlueprintArgument* inputs, BlueprintArgument* outputs, BlueprintNodePreview* preview) {
					auto& texture = *(Texture*)outputs[0].data;
					preview->type = "image"_h;
					preview->data = texture.image;
				}
			);
		}
	}
}
