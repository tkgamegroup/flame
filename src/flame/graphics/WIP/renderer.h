#pragma once

#include <memory>

#include <flame/global.h>
#include <flame/spare_list.h>
#include <flame/math.h>
#include <flame/engine/core/object.h>
#include <flame/engine/graphics/resource.h>
#include <flame/engine/graphics/graphics.h>
#include <flame/engine/graphics/display_layer.h>

namespace flame
{
	struct CameraComponent;
	struct Model;
	struct Buffer;
	struct DescriptorSet;
	struct Framebuffer;
	struct CommandBuffer;
	struct Scene;

	struct Renderer : Object
	{

	};

	struct PlainRenderer
	{
		static bool first;

		std::unique_ptr<CommandBuffer> cb;

		static Buffer *last_bone_buffer_mode0;
		static Buffer *last_bone_buffer_mode2;
		static Buffer *last_bone_buffer_mode3;

		static std::shared_ptr<RenderPass> renderpass_color;
		static std::shared_ptr<RenderPass> renderpass_color_and_depth;
		static std::shared_ptr<RenderPass> renderpass_color_clear;
		static std::shared_ptr<RenderPass> renderpass_color_clear_and_depth;

		enum Mode
		{
			mode_just_color,
			mode_color_and_front_light,
			mode_just_texture,
			mode_wireframe
		};

		struct DrawData
		{
			Mode mode;
			Buffer *vbuffer0 = nullptr;
			Buffer *vbuffer1 = nullptr;
			Buffer *ibuffer = nullptr;

			struct ObjData
			{
				glm::mat4 mat;
				struct GeoData
				{
					int index_count;
					int first_index = 0;
					int vertex_offset = 0;
					int instance_count = 1;
					int first_instance = 0;
				};
				std::vector<GeoData> geo_data;
				Buffer *bone_buffer = nullptr;
				glm::vec4 color;

				void fill_with_model(Model *m);
				void fill_with_model_texture_mode(Model *m);
			};

			std::vector<ObjData> obj_data;
		};

		PlainRenderer();
		void render(Framebuffer *framebuffer, bool clear, CameraComponent *camera, DrawData *data);
		void do_render(CommandBuffer *cb, CameraComponent *camera, DrawData *data);
		void add_to_drawlist();
	};

	struct LinesRenderer
	{
		static bool first;

		static std::shared_ptr<RenderPass> renderpass_color;

		std::unique_ptr<CommandBuffer> cb;

		struct Vertex
		{
			glm::vec3 position;
			glm::vec3 color;
		};

		struct DrawData
		{
			Buffer *vertex_buffer;
			int vertex_count;
		};

		LinesRenderer();
		void render(Framebuffer *framebuffer, bool clear, CameraComponent *camera, DrawData *data);
		void add_to_drawlist();
	};

	enum { MaxLightCount = 256 };
	enum { MaxStaticModelInstanceCount = 1024 };
	enum { MaxAnimatedModelInstanceCount = 8 };
	enum { MaxTerrainCount = 8 };
	enum { MaxWaterCount = 8 };

	enum { MaxShadowCount = 4 };
	enum { EnvrSizeCx = 128 * 4 };
	enum { EnvrSizeCy = 128 * 2 };
	enum { ShadowMapCx = 2048 };
	enum { ShadowMapCy = 2048 };

	struct DeferredRenderer : Renderer
	{
		static bool defe_inited;
		static bool shad_inited;
		static std::shared_ptr<RenderPass> renderpass_color;
		static std::shared_ptr<RenderPass> renderpass_color16;
		static std::shared_ptr<RenderPass> renderpass_color_and_depth;
		static std::shared_ptr<RenderPass> renderpass_color32_and_depth;
		static std::shared_ptr<RenderPass> renderpass_defe;

		bool sky_dirty;
		bool ambient_dirty;
		bool light_count_dirty;
		bool static_model_instance_count_dirty;
		bool animated_model_instance_count_dirty;
		bool terrain_count_dirty;
		bool water_count_dirty;

		int static_indirect_count;
		int animated_indirect_count;

		std::unique_ptr<CommandBuffer> cb_defe;
		std::unique_ptr<CommandBuffer> cb_shad;

		std::unique_ptr<Buffer> constantBuffer;
		std::unique_ptr<Buffer> matrixBuffer;
		std::unique_ptr<Buffer> staticModelInstanceMatrixBuffer;
		std::unique_ptr<Buffer> animatedModelInstanceMatrixBuffer;
		std::unique_ptr<Buffer> terrainBuffer;
		std::unique_ptr<Buffer> waterBuffer;
		std::unique_ptr<Buffer> lightBuffer;
		std::unique_ptr<Buffer> ambientBuffer;
		std::unique_ptr<Buffer> staticObjectIndirectBuffer;
		std::unique_ptr<Buffer> animatedObjectIndirectBuffer;
		std::unique_ptr<Texture> envrImage;
		std::unique_ptr<Texture> mainImage;
		std::unique_ptr<Texture> depthImage;
		std::unique_ptr<Texture> albedoAlphaImage;
		std::unique_ptr<Texture> normalHeightImage;
		std::unique_ptr<Texture> specRoughnessImage;
		std::unique_ptr<DescriptorSet> ds_mrt;
		std::unique_ptr<DescriptorSet> ds_mrtAnim;
		std::unique_ptr<DescriptorSet> ds_mrtAnim_bone;
		std::unique_ptr<DescriptorSet> ds_terrain;
		std::unique_ptr<DescriptorSet> ds_water;
		std::unique_ptr<DescriptorSet> ds_defe;
		std::unique_ptr<DescriptorSet> ds_comp;

		bool enable_shadow;
		std::unique_ptr<Buffer> shadowBuffer;
		std::unique_ptr<Texture> esmImage;
		std::unique_ptr<Texture> esmDepthImage;
		std::shared_ptr<Framebuffer> fb_esm[MaxShadowCount * 6];
		std::unique_ptr<DescriptorSet> ds_esm;
		std::unique_ptr<DescriptorSet> ds_esmAnim;

		DisplayLayer *dst;

		Resource resource;

		std::shared_ptr<Framebuffer> framebuffer;

		virtual bool on_message(Object *sender, Message msg) override;

		DeferredRenderer(bool _enable_shadow, DisplayLayer *_dst);
		~DeferredRenderer();
		void render(Scene *scene, CameraComponent *camera);
		void add_to_drawlist();

	private:
		void create_resolution_related();

		SpareList lights;
		SpareList static_model_instances;
		SpareList animated_model_instances;
		SpareList terrains;
		SpareList waters;
		SpareList shadow_lights;

		struct LightAux
		{
			long long attribute_updated_frame;
			long long shadow_updated_frame;
		};

		struct ModelInstanceAux
		{
			long long matrix_updated_frame;
		};

		struct TerrainAux
		{
			long long attribute_updated_frame;
			long long blend_map_updated_frame;
		};

		struct WaterAux
		{
			long long attribute_updated_frame;
		};

		LightAux light_auxes[MaxLightCount];
		ModelInstanceAux static_model_instance_auxes[MaxStaticModelInstanceCount];
		ModelInstanceAux animated_model_instance_auxes[MaxAnimatedModelInstanceCount];
		TerrainAux terrain_auxes[MaxTerrainCount];
		WaterAux water_auxes[MaxWaterCount];
	};
}
