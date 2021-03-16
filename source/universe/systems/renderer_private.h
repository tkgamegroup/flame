#pragma once

#include <flame/graphics/graphics.h>
#include <flame/universe/systems/renderer.h>

namespace flame
{
	struct Window;

	namespace graphics
	{
		struct Device;
		struct Buffer;
		struct Image;
		struct Renderpass;
		struct Framebuffer;
		struct Pipeline;
		struct DescriptorSet;
		struct CommandBuffer;
		struct Swapchain;
	}

	struct EntityPrivate;
	struct cElementPrivate;
	struct cCameraPrivate;

	template <class T>
	struct GeometryBuffer
	{
		graphics::BufferUsageFlags usage;
		uint capacity = 0;
		graphics::AccessFlags access;
		T* pstag = nullptr;
		uint stag_num = 0;

		graphics::Device* d = nullptr;
		FlmPtr<graphics::Buffer> buf;
		FlmPtr<graphics::Buffer> stagbuf;

		void rebuild();
		void create(graphics::Device* d, graphics::BufferUsageFlags usage, uint capacity, graphics::AccessFlags access = graphics::AccessVertexAttributeRead);
		void push(uint cnt, const T* p);
		T* stag(uint cnt);
		void upload(graphics::CommandBuffer* cb);
	};

	struct ElementVertex
	{
		vec2 pos;
		vec2 uv;
		cvec4 col;

		void set(const vec2& _pos, const vec2& _uv, const cvec4& _col)
		{
			pos = _pos;
			uv = _uv;
			col = _col;
		}
	};

	struct ElementDrawCmd
	{
		enum Type
		{
			Fill,
			Stroke,
			Text,
			Scissor
		};

		Type type;
		uint res_id;
		std::vector<vec2> points;
		std::wstring text;
		cvec4 color;
		vec4 misc;
	};

	struct sRendererBridge : sRenderer
	{
		int find_element_res(const char* name) const override;
		void draw_text(uint layer, cElement* element, const vec2& pos, uint font_size, uint font_id,
			const wchar_t* text_beg, const wchar_t* text_end, const cvec4& color) override;
		void fill_rect(uint layer, cElement* element, const vec2& pos, const vec2& size, const cvec4& color) override;
		void stroke_rect(uint layer, cElement* element, const vec2& pos, const vec2& size, float thickness, const cvec4& color) override;
	};

	struct sRendererPrivate : sRendererBridge
	{
		struct ElementRes
		{
			ElementResType type;
			void* v;
			std::string name;
		};

		bool hdr;
		bool wireframe = false;
		bool always_update = false;

		Window* window;

		graphics::Canvas* canvas = nullptr;
		cCameraPrivate* camera = nullptr;

		graphics::Device* device;
		graphics::Swapchain* swapchain;
		FlmPtr<graphics::Renderpass> rp_rgba8c;
		FlmPtr<graphics::Renderpass> rp_rgba16c;
		std::vector<FlmPtr<graphics::Framebuffer>> fb_targets;
		FlmPtr<graphics::Pipeline> pl_element;
		FlmPtr<graphics::Pipeline> pl_blit8;
		FlmPtr<graphics::Pipeline> pl_blit16;
		FlmPtr<graphics::Pipeline> pl_gamma;

		GeometryBuffer<ElementVertex>	buf_element_vtx;
		GeometryBuffer<uint>			buf_element_idx;
		FlmPtr<graphics::Image>			img_wht;
		FlmPtr<graphics::DescriptorSet>	ds_element;

		std::vector<ElementRes> element_reses;

		bool dirty = true;

		vec2 tar_size;

		cElementPrivate* last_element;
		bool last_element_changed;

		std::vector<ElementDrawCmd> draw_layers[128];

		sRendererPrivate(sRendererParms* parms);

		void set_targets();

		void render(EntityPrivate* e, bool element_culled, bool node_culled);

		void set_shade_wireframe(bool v) override { wireframe = v; }
		void set_always_update(bool a) override { always_update = a; }

		void get_element_res(uint idx, ElementResType* type, void** v, char** name) const override;
		void set_element_res(int idx, ElementResType type, void* v, const char* name) override;
		int find_element_res(const std::string& name) const;

		void fill_rect(uint layer, cElementPrivate* element, const vec2& pos, const vec2& size, const cvec4& color);
		void stroke_rect(uint layer, cElementPrivate* element, const vec2& pos, const vec2& size, float thickness, const cvec4& color);
		void draw_text(uint layer, cElementPrivate* element, const vec2& pos, uint font_size, uint font_id,
			const wchar_t* text_beg, const wchar_t* text_end, const cvec4& color);

		graphics::Canvas* get_canvas() const override { return canvas; }

		cCamera* get_camera() const override { return (cCamera*)camera; }
		void set_camera(cCamera* c) override { camera = (cCameraPrivate*)c; }

		bool is_dirty() const override { return always_update || dirty; }
		void mark_dirty() override { dirty = true; }

		void record_element_drawing_commands(uint tar_idx, graphics::CommandBuffer* cb) override;
		void record_node_drawing_commands(uint tar_idx, graphics::CommandBuffer* cb) override;

		void on_added() override;

		void update() override;
	};

	inline void sRendererBridge::fill_rect(uint layer, cElement* element, const vec2& pos, const vec2& size, const cvec4& color)
	{
		((sRendererPrivate*)this)->fill_rect(layer, (cElementPrivate*)element, pos, size, color);
	}

	inline void sRendererBridge::stroke_rect(uint layer, cElement* element, const vec2& pos, const vec2& size, float thickness, const cvec4& color)
	{
		((sRendererPrivate*)this)->stroke_rect(layer, (cElementPrivate*)element, pos, size, thickness, color);
	}

	inline int sRendererBridge::find_element_res(const char* name) const
	{
		return ((sRendererPrivate*)this)->find_element_res(name);
	}
	inline void sRendererBridge::draw_text(uint layer, cElement* element, const vec2& pos, uint font_size, uint font_id,
		const wchar_t* text_beg, const wchar_t* text_end, const cvec4& color)
	{
		((sRendererPrivate*)this)->draw_text(layer, (cElementPrivate*)element, pos, font_size, font_id, text_beg, text_end, color);
	}
}
