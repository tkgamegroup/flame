#pragma once

#include <flame/universe/system.h>

namespace flame
{
	namespace graphics
	{
		struct CommandBuffer;
		struct Canvas;
	}

	struct cElement;
	struct cCamera;

	struct sRendererParms
	{
		bool hdr = true;
	};

	struct sRenderer : System
	{
		inline static auto type_name = "flame::sRenderer";
		inline static auto type_hash = ch(type_name);

		sRenderer() :
			System(type_name, type_hash)
		{
		}

		virtual void set_shade_wireframe(bool v) = 0;
		virtual void set_always_update(bool a) = 0;

		virtual void fill_rect(uint layer, cElement* element, const vec2& pos, const vec2& size, const cvec4& color) = 0;

		virtual graphics::Canvas* get_canvas() const = 0;

		virtual cCamera* get_camera() const = 0;
		virtual void set_camera(cCamera* camera) = 0;

		virtual bool is_dirty() const = 0;
		virtual void mark_dirty() = 0;

		virtual void record_element_drawing_commands(uint tar_idx, graphics::CommandBuffer* cb) = 0;
		virtual void record_node_drawing_commands(uint tar_idx, graphics::CommandBuffer* cb) = 0;

		FLAME_UNIVERSE_EXPORTS static sRenderer* create(void* parms = nullptr);
	};
}
