//#include "canvas.h"
//
//namespace flame
//{
//	namespace graphics
//	{
//		struct RenderPreferencesPrivate : RenderPreferences
//		{
//			std::unique_ptr<RenderpassPrivate> r16_renderpass;
//			std::unique_ptr<RenderpassPrivate> mesh_renderpass;
//			std::unique_ptr<RenderpassPrivate> depth_renderpass;
//			std::unique_ptr<RenderpassPrivate> pickup_renderpass;
//			std::vector<MaterialPipeline> material_pipelines[MaterialUsageCount];
//			std::unique_ptr<PipelinePrivate> element_pipeline;
//			std::unique_ptr<PipelinePrivate> sky_pipeline;
//			PipelineLayoutPrivate* mesh_pipeline_layout;
//			PipelineLayoutPrivate* terrain_pipeline_layout;
//			PipelinePrivate* mesh_wireframe_pipeline;
//			PipelinePrivate* mesh_armature_wireframe_pipeline;
//			PipelinePrivate* terrain_wireframe_pipeline;
//			PipelinePrivate* mesh_pickup_pipeline;
//			PipelinePrivate* mesh_armature_pickup_pipeline;
//			PipelinePrivate* terrain_pickup_pipeline;
//			PipelinePrivate* mesh_outline_pipeline;
//			PipelinePrivate* mesh_armature_outline_pipeline;
//			PipelinePrivate* terrain_outline_pipeline;
//			std::unique_ptr<PipelinePrivate> line_pipeline;
//			std::unique_ptr<PipelinePrivate> triangle_pipeline;
//			std::unique_ptr<PipelinePrivate> blurh_pipeline[10];
//			std::unique_ptr<PipelinePrivate> blurv_pipeline[10];
//			std::unique_ptr<PipelinePrivate> blurh_depth_pipeline;
//			std::unique_ptr<PipelinePrivate> blurv_depth_pipeline;
//			std::unique_ptr<PipelinePrivate> blit_8_pipeline;
//			std::unique_ptr<PipelinePrivate> blit_16_pipeline;
//			std::unique_ptr<PipelinePrivate> blend_8_pipeline;
//			std::unique_ptr<PipelinePrivate> blend_16_pipeline;
//			std::unique_ptr<PipelinePrivate> filter_bright_pipeline;
//			std::unique_ptr<PipelinePrivate> downsample_pipeline;
//			std::unique_ptr<PipelinePrivate> upsample_pipeline;
//			std::unique_ptr<PipelinePrivate> gamma_pipeline;
//		};
//
//		struct ArmatureDeformerPrivate : ArmatureDeformer
//		{
//			MeshPrivate* mesh;
//			//ShaderBuffer poses_buffer;
//			std::unique_ptr<DescriptorSetPrivate> descriptorset;
//
//			ArmatureDeformerPrivate(RenderPreferencesPrivate* preferences, MeshPrivate* mesh);
//			void release() override { delete this; }
//			void set_pose(uint id, const mat4& pose) override;
//		};
//
//		const auto msaa_sample_count = SampleCount_8;
//		const auto shadow_map_size = uvec2(2048);
//
//		struct MeshWeight
//		{
//			ivec4 ids;
//			vec4 weights;
//		};
//
//		struct MeshResourcePrivate
//		{
//			ShaderGeometryBuffer<MeshVertex> vertex_buffer;
//			ShaderGeometryBuffer<MeshWeight> weight_buffer;
//			ShaderGeometryBuffer<uint> index_buffer;
//			uint material_id;
//		};
//
//		struct ModelResourcePrivate
//		{
//			CanvasPrivate* canvas;
//
//			ModelResourcePrivate(CanvasPrivate* canvas) :
//				canvas(canvas)
//			{
//			}
//
//			~ModelResourcePrivate();
//
//			std::string name;
//			ModelPrivate* model;
//			std::vector<uint> materials;
//			std::vector<std::unique_ptr<MeshResourcePrivate>> meshes;
//		};
//
//		struct MeshInfo
//		{
//			MeshResourcePrivate* res;
//			mat4 transform;
//			bool cast_shadow;
//			ArmatureDeformerPrivate* deformer;
//			ShadeFlags flags;
//			void* userdata;
//		};
//
//		struct TerrainInfo
//		{
//			uvec2 blocks;
//			vec3 scale;
//			vec3 coord;
//			float tess_levels;
//			uint height_tex_id;
//			uint normal_tex_id;
//			uint material_id;
//			ShadeFlags flags;
//			void* userdata;
//		};
//
//		struct DirectionalLight
//		{
//			vec3 dir;
//			vec3 side;
//			vec3 up;
//			vec3 color;
//			bool cast_shadow;
//			float shadow_distance;
//		};
//
//		struct PointLight
//		{
//			vec3 coord;
//			vec3 color;
//			bool cast_shadow;
//			float shadow_distance;
//		};
//
//		struct Cmd
//		{
//			enum Type
//			{
//				DrawElement,
//				DrawMesh,
//				DrawTerrain,
//				DrawLines,
//				DrawTriangles,
//				SetScissor,
//				SetViewport,
//				Blur,
//				Bloom
//			};
//
//			Type type;
//
//			Cmd(Type t) : type(t) {}
//			virtual ~Cmd() {}
//		};
//
//		struct CmdDrawTerrain : Cmd
//		{
//			std::vector<uint> entries;
//
//			CmdDrawTerrain() : Cmd(DrawTerrain) {}
//		};
//
//		struct CmdDrawLines : Cmd
//		{
//			uint count = 0;
//
//			CmdDrawLines() : Cmd(DrawLines) {}
//		};
//
//		struct CmdDrawTriangles : Cmd
//		{
//			uint count = 0;
//
//			CmdDrawTriangles() : Cmd(DrawTriangles) {}
//		};
//
//		struct CmdSetScissor : Cmd
//		{
//			Rect scissor;
//
//			CmdSetScissor(const Rect& _scissor) : Cmd(SetScissor) { scissor = _scissor; }
//		};
//
//		struct CmdSetViewport : Cmd
//		{
//			Rect viewport;
//
//			CmdSetViewport(const Rect& _viewport) : Cmd(SetViewport) { viewport = _viewport; }
//		};
//
//		struct CmdBlur : Cmd
//		{
//			Rect range;
//			uint radius;
//
//			CmdBlur(const Rect& _range, uint _radius) : Cmd(Blur) { range = _range; radius = _radius; }
//		};
//
//		struct CmdBloom : Cmd
//		{
//			CmdBloom() : Cmd(Bloom) {}
//		};
//
//		struct CanvasPrivate : Canvas
//		{
//			float shadow_distance = 100.f;
//			uint csm_levels = 3;
//			float csm_factor = 0.3f;
//
//			cvec4 clear_color = cvec4(0, 0, 0, 255);
//
//			float fovy;
//			float aspect;
//			float zNear;
//			float zFar;
//			vec3 camera_coord;
//			mat3 camera_dirs;
//			mat4 view_matrix;
//			mat4 view_inv_matrix;
//			mat4 proj_matrix;
//			mat4 proj_view_matrix;
//
//			std::unique_ptr<ImagePrivate> default_sky_box_image;
//			std::unique_ptr<ImagePrivate> default_sky_irr_image;
//			std::unique_ptr<ImagePrivate> default_sky_rad_image;
//			std::unique_ptr<DescriptorSetPrivate> sky_descriptorset;
//
//			//ShaderBuffer mesh_matrix_buffer;
//			std::unique_ptr<DescriptorSetPrivate> mesh_descriptorset;
//
//			//ShaderBuffer material_info_buffer;
//			std::unique_ptr<DescriptorSetPrivate> material_descriptorset;
//
//			std::unique_ptr<ImagePrivate> shadow_depth_image;
//			std::unique_ptr<ImagePrivate> shadow_depth_back_image;
//			std::unique_ptr<FramebufferPrivate> shadow_depth_back_framebuffer;
//			std::unique_ptr<DescriptorSetPrivate> shadow_depth_back_descriptorset;
//
//			//ShaderBuffer light_sets_buffer;
//			//ShaderBuffer light_infos_buffer;
//			//ShaderBuffer shadow_matrices_buffer;
//			std::vector<std::unique_ptr<ImagePrivate>> directional_shadow_maps;
//			std::vector<std::unique_ptr<FramebufferPrivate>> directional_light_depth_framebuffers;
//			std::vector<std::unique_ptr<FramebufferPrivate>> directional_shadow_map_framebuffers;
//			std::vector<std::unique_ptr<DescriptorSetPrivate>> directional_shadow_map_descriptorsets;
//			std::vector<std::unique_ptr<ImagePrivate>> point_shadow_maps;
//			std::vector<std::unique_ptr<FramebufferPrivate>> point_light_depth_framebuffers;
//			std::vector<std::unique_ptr<FramebufferPrivate>> point_shadow_map_framebuffers;
//			std::vector<std::unique_ptr<DescriptorSetPrivate>> point_shadow_map_descriptorsets;
//
//			std::unique_ptr<DescriptorSetPrivate> light_descriptorset;
//
//			//ShaderBuffer terrain_info_buffer;
//			std::unique_ptr<DescriptorSetPrivate> terrain_descriptorset;
//
//			std::vector<ImageViewPrivate*> output_imageviews;
//			std::vector<std::unique_ptr<FramebufferPrivate>> output_framebuffers;
//			std::vector<std::unique_ptr<DescriptorSetPrivate>> output_descriptorsets;
//
//			std::unique_ptr<ImagePrivate> hdr_image;
//			std::unique_ptr<FramebufferPrivate> hdr_framebuffer;
//			std::unique_ptr<DescriptorSetPrivate> hdr_descriptorset;
//
//			std::unique_ptr<ImagePrivate> depth_image;
//
//			std::vector<std::unique_ptr<FramebufferPrivate>> mesh_framebuffers;
//
//			std::unique_ptr<ImagePrivate> back_image;
//			std::vector<std::unique_ptr<FramebufferPrivate>> back_framebuffers;
//			std::vector<std::unique_ptr<DescriptorSetPrivate>> back_nearest_descriptorsets;
//			std::vector<std::unique_ptr<DescriptorSetPrivate>> back_linear_descriptorsets;
//
//			std::unique_ptr<ImagePrivate> pickup_image;
//			std::unique_ptr<FramebufferPrivate> pickup_framebuffer;
//
//			std::vector<std::vector<vec2>> paths;
//
//			ShaderGeometryBuffer<Line> line_buffer;
//			ShaderGeometryBuffer<Triangle> triangle_buffer;
//
//			std::vector<MeshInfo> meshes;
//			std::vector<TerrainInfo> terrains;
//			std::vector<DirectionalLight> directional_lights;
//			std::vector<PointLight> point_lights;
//
//			std::vector<std::unique_ptr<Cmd>> cmds;
//
//			uvec2 output_size;
//			Rect curr_scissor;
//			Rect curr_viewport;
//		};
//	}
//}
