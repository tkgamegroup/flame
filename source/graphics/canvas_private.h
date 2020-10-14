#include <flame/graphics/canvas.h>

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;
		struct BufferPrivate;
		struct ImagePrivate;
		struct ImageViewPrivate;
		struct ImageAtlasPrivate;
		struct FontAtlasPrivate;
		struct MaterialPrivate;
		struct ModelPrivate;
		struct FramebufferPrivate;
		struct DescriptorSetPrivate;
		struct CommandBufferPrivate;

		template <class T, BufferUsageFlags U>
		struct TBuffer
		{
			DevicePrivate* d = nullptr;
			uint count = 0;
			std::unique_ptr<BufferPrivate> buf;
			std::unique_ptr<BufferPrivate> stg;
			T* beg = nullptr;
			T* end = nullptr;

			void _build()
			{
				T* temp = nullptr;
				auto n = 0;
				if (end)
				{
					n = stg_num();
					temp = new T[n];
					memcpy(temp, stg->mapped, sizeof(T) * n);
				}
				buf.reset(new BufferPrivate(d, count * sizeof(T), BufferUsageTransferDst | U, MemoryPropertyDevice));
				stg.reset(new BufferPrivate(d, buf->size, BufferUsageTransferSrc, MemoryPropertyHost | MemoryPropertyCoherent));
				stg->map();
				beg = (T*)stg->mapped;
				end = beg;
				if (temp)
				{
					push(n, temp);
					delete[]temp;
				}
			}

			void create(DevicePrivate* _d, uint _count)
			{
				d = _d;
				count = _count;
				_build();
			}

			void stg_rewind()
			{
				end = (T*)stg->mapped;
			}

			uint stg_num()
			{
				return end - stg->mapped;
			}

			void push(const T& t)
			{
				auto n = stg_num();
				if (n >= count)
				{
					count *= 2;
					_build();
				}
				*end = t;
				end++;
			}

			void push(uint cnt, const T* p)
			{
				auto n = stg_num();
				if (n + cnt >= count)
				{
					count = (n + cnt) * 2;
					_build();
				}
				memcpy(end, p, sizeof(T) * cnt);
				end += cnt;
			}

			void push(uint off, const T& t)
			{
				beg[off] = t;
			}

			void upload(CommandBufferPrivate* cb)
			{
				BufferCopy cpy;
				cpy.size = (char*)end - stg->mapped;
				cb->copy_buffer(stg.get(), buf.get(), { &cpy, 1 });
			}

			void upload(CommandBufferPrivate* cb, uint off, uint n)
			{
				BufferCopy cpy;
				cpy.src_off = cpy.dst_off = off * sizeof(T);
				cpy.size = n * sizeof(T);
				cb->copy_buffer(stg.get(), buf.get(), { &cpy, 1 });
			}

			void barrier(CommandBufferPrivate* cb)
			{
				cb->buffer_barrier(buf.get(), AccessTransferWrite, AccessVertexAttributeRead);
			}
		};

		const auto msaa_sample_count = SampleCount_8;
		const auto shadow_map_size = Vec2u(2048);

		struct ElementVertex
		{
			Vec2f position;
			Vec2f uv;
			Vec4c color;
		};

		struct MeshVertex
		{
			Vec3f position;
			Vec2f uv;
			Vec3f normal;
		};

		struct MeshWeight
		{
			Vec4i ids;
			Vec4f weights;
		};

		struct ElementResource
		{
			std::string name; 
			ResourceType type = ResourceImage;
			void* p;
		};

		struct MaterialResource
		{
			std::string name;
			MaterialPrivate* material;
			std::vector<std::pair<uint, std::unique_ptr<ImagePrivate>>> textures;
		};

		struct ModelResource
		{
			struct Mesh
			{
				TBuffer<MeshVertex, BufferUsageVertex> vertex_buffer;
				TBuffer<MeshWeight, BufferUsageVertex> weight_buffer;
				TBuffer<uint, BufferUsageIndex> index_buffer;
				uint material_index;
			};

			std::string name;
			ModelPrivate* model;
			std::vector<uint> materials;
			std::vector<std::unique_ptr<Mesh>> meshes;
		};

		struct ArmatureDeformerPrivate : ArmatureDeformer
		{
			MeshPrivate* mesh;
			TBuffer<Mat4f, BufferUsageStorage> poses_buffer;
			std::unique_ptr<DescriptorSetPrivate> descriptorset;
			Vec2i dirty_range = Vec2i(0);

			ArmatureDeformerPrivate(DevicePrivate* device, MeshPrivate* mesh);
			void release() override { delete this; }
			void set_pose(uint id, const Mat4f& pose) override;
		};

		struct RenderDataS
		{
			float fovy;
			float aspect;
			float zNear;
			float zFar;
			Vec3f camera_coord;
			float dummy1;
			Vec4f frustum_planes[6];

			Vec2f fb_size;
			float shadow_distance;
			uint csm_levels;
			Vec4f dummy3[3];

			Mat4f view_inv;
			Mat4f view;
			Mat4f proj;
			Mat4f proj_view;
		};

		struct MeshMatrixS
		{
			Mat4f transform;
			Mat4f normal_matrix;
		};

		struct MaterialInfoS
		{
			Vec4f color;
			float metallic;
			float roughness;
			float alpha_test;
			float dummy1;
			int color_map_index = -1;
			int metallic_roughness_ao_map_index = -1;
			int normal_hegiht_map_index = -1;
			int dummy2;
		};

		struct LightIndicesS
		{
			uint directional_lights_count;
			uint point_lights_count;
			uint point_light_indices[1022];
		};

		struct DirectionalLightInfoS
		{
			Vec3f dir;
			float distance;
			Vec3f color;
			int dummy1;

			int shadow_map_index;
			float dummy2;
			Vec2f dummy3;
			Vec4f dummy4;
			Mat4f shadow_matrices[4];
		};

		struct PointLightInfoS
		{
			Vec3f coord;
			float distance;
			Vec3f color;
			int shadow_map_index;
		};

		struct TerrainInfoS
		{
			Vec3f coord;
			float dummy1;

			Vec2u size;
			uint height_tex_id;
			uint color_tex_id;

			Vec3f extent;
			float tess_levels;
		};

		struct Cmd
		{
			enum Type
			{
				DrawElement,
				DrawMesh,
				DrawTerrain,
				SetScissor,
				Blur,
				Bloom
			};

			Type type;

			Cmd(Type t) : type(t) {}
			virtual ~Cmd() {}

			union
			{
				struct
				{
					uint count;
				}d3;
			}v;
		};

		struct CmdDrawElement : Cmd
		{
			uint id;
			uint vertices_count = 0;
			uint indices_count = 0;

			CmdDrawElement(uint _id) : Cmd(DrawElement) { id = _id; }
		};

		struct CmdDrawMesh : Cmd
		{
			std::vector<std::tuple<uint, ModelResource::Mesh*, bool, ArmatureDeformerPrivate*>> meshes;

			CmdDrawMesh() : Cmd(DrawMesh) {}
		};

		struct CmdDrawTerrain : Cmd
		{
			uint idx;

			CmdDrawTerrain() : Cmd(DrawTerrain) {}
		};

		struct CmdSetScissor : Cmd
		{
			Vec4f scissor;

			CmdSetScissor(const Vec4f& _scissor) : Cmd(SetScissor) { scissor = _scissor; }
		};

		struct CmdBlur : Cmd
		{
			Vec4f range;
			uint radius;

			CmdBlur(const Vec4f& _range, uint _radius) : Cmd(Blur) { range = _range; radius = _radius; }
		};

		struct CmdBloom : Cmd
		{
			CmdBloom() : Cmd(Bloom) {}
		};

		struct CanvasBridge : Canvas
		{
			void set_target(uint views_count, ImageView* const* views) override;

			int find_resource(ResourceType type, const char* name) override;
			uint set_resource(ResourceType type, int slot, void* p, const char* name) override;

			void record(CommandBuffer* cb, uint image_index) override;
		};

		struct CanvasPrivate : CanvasBridge
		{
			DevicePrivate* device;

			bool hdr;
			bool msaa_3d;

			Vec4c clear_color = Vec4c(0, 0, 0, 255);

			std::unique_ptr<ImagePrivate> white_image;
			std::vector<ElementResource> element_resources;
			std::vector<std::pair<std::string, ImageViewPrivate*>> texture_resources;
			std::vector<std::unique_ptr<MaterialResource>> material_resources;
			std::vector<std::unique_ptr<ModelResource>> model_resources;

			TBuffer<ElementVertex, BufferUsageVertex> element_vertex_buffer;
			TBuffer<uint, BufferUsageIndex> element_index_buffer;
			std::unique_ptr<DescriptorSetPrivate> element_descriptorset;

			TBuffer<RenderDataS, BufferUsageUniform> render_data_buffer;
			std::unique_ptr<DescriptorSetPrivate> render_data_descriptorset;

			TBuffer<MeshMatrixS, BufferUsageStorage> mesh_matrix_buffer;
			std::unique_ptr<DescriptorSetPrivate> mesh_descriptorset;

			TBuffer<MaterialInfoS, BufferUsageStorage> material_info_buffer;
			std::unique_ptr<DescriptorSetPrivate> material_descriptorset;

			std::unique_ptr<ImagePrivate> shadow_depth_image;
			std::unique_ptr<ImagePrivate> shadow_blur_pingpong_image;
			std::unique_ptr<FramebufferPrivate> shadow_blur_pingpong_image_framebuffer;
			std::unique_ptr<DescriptorSetPrivate> shadow_blur_pingpong_image_descriptorset;

			TBuffer<LightIndicesS, BufferUsageStorage> light_indices_buffer;

			TBuffer<DirectionalLightInfoS, BufferUsageStorage> directional_light_info_buffer;
			std::vector<std::unique_ptr<ImagePrivate>> directional_light_shadow_maps;
			std::vector<std::unique_ptr<FramebufferPrivate>> directional_light_shadow_map_depth_framebuffers;
			std::vector<std::unique_ptr<FramebufferPrivate>> directional_light_shadow_map_framebuffers;
			std::vector<std::unique_ptr<DescriptorSetPrivate>> directional_light_shadow_map_descriptorsets;
			uint used_directional_light_shadow_maps_count = 0;

			TBuffer<PointLightInfoS, BufferUsageStorage> point_light_info_buffer;
			std::vector<std::unique_ptr<ImagePrivate>> point_light_shadow_maps;
			std::vector<std::unique_ptr<FramebufferPrivate>> point_light_shadow_map_depth_framebuffers;
			std::vector<std::unique_ptr<FramebufferPrivate>> point_light_shadow_map_framebuffers;
			std::vector<std::unique_ptr<DescriptorSetPrivate>> point_light_shadow_map_descriptorsets;
			uint used_point_light_shadow_maps_count = 0;

			std::unique_ptr<DescriptorSetPrivate> light_descriptorset;

			TBuffer<TerrainInfoS, BufferUsageStorage> terrain_info_buffer;
			std::unique_ptr<DescriptorSetPrivate> terrain_descriptorset;

			std::vector<ImageViewPrivate*> target_imageviews;
			std::vector<std::unique_ptr<FramebufferPrivate>> target_framebuffers;
			std::vector<std::unique_ptr<DescriptorSetPrivate>> target_descriptorsets;

			std::unique_ptr<ImagePrivate> dst_image;
			std::unique_ptr<FramebufferPrivate> dst_framebuffer;
			std::unique_ptr<DescriptorSetPrivate> dst_descriptorset;

			std::unique_ptr<ImagePrivate> msaa_image;
			std::unique_ptr<ImagePrivate> msaa_resolve_image;
			std::unique_ptr<DescriptorSetPrivate> msaa_descriptorset;

			std::unique_ptr<ImagePrivate> depth_image;

			std::vector<std::unique_ptr<FramebufferPrivate>> forward_framebuffers;

			std::unique_ptr<ImagePrivate> back_image;
			std::vector<std::unique_ptr<FramebufferPrivate>> back_framebuffers;
			std::vector<std::unique_ptr<DescriptorSetPrivate>> back_nearest_descriptorsets;
			std::vector<std::unique_ptr<DescriptorSetPrivate>> back_linear_descriptorsets;

			std::vector<std::vector<Vec2f>> paths;

			std::vector<std::unique_ptr<Cmd>> cmds;
			CmdDrawElement* last_element_cmd = nullptr;
			CmdDrawMesh* last_mesh_cmd = nullptr;

			Vec2u target_size;
			Vec4f curr_scissor;

			PipelinePrivate* pl_element;
			PipelinePrivate* pl_forward;
			PipelinePrivate* pl_forward_armature;
			PipelinePrivate* pl_terrain;

			CanvasPrivate(DevicePrivate* d, bool hdr, bool msaa_3d);

			void release() override { delete this; }

			Vec4c get_clear_color() const override { return clear_color; }
			void set_clear_color(const Vec4c& color) override { clear_color = color; }

			ImageView* get_target(uint idx) const override { return target_imageviews.empty() ? nullptr : (ImageView*)target_imageviews[idx]; }
			void set_target(std::span<ImageViewPrivate*> views);

			void* get_resource(ResourceType type, uint slot, ResourceType* real_type = nullptr) override;
			int find_resource(ResourceType type, const std::string& name);
			uint set_resource(ResourceType type, int slot, void* p, const std::string& name);

			void add_draw_element_cmd(uint id);
			void add_vtx(const Vec2f& position, const Vec2f& uv, const Vec4c& color);
			void add_idx(uint idx);

			void begin_path() override;
			void move_to(const Vec2f& pos) override;
			void line_to(const Vec2f& pos) override;
			void close_path() override;

			void stroke(const Vec4c& col, float thickness, bool aa = false) override;
			void fill(const Vec4c& col, bool aa = false) override;
			void draw_text(uint res_id, const wchar_t* text_beg, const wchar_t* text_end, uint font_size, const Vec4c& col, const Vec2f& pos, const Mat2f& axes) override;
			void draw_image(uint res_id, uint tile_id, const Vec2f& LT, const Vec2f& RT, const Vec2f& RB, const Vec2f& LB, const Vec2f& uv0, const Vec2f& uv1, const Vec4c& tint_col) override;

			void set_camera(float fovy, float aspect, float zNear, float zFar, const Mat3f& axes, const Vec3f& coord) override;

			void draw_mesh(uint mod_id, uint mesh_idx, const Mat4f& transform, const Mat4f& normal_matrix, bool cast_shadow, ArmatureDeformer* deformer) override;
			void draw_terrain(uint height_tex_id, uint color_tex_id, const Vec2u& size, const Vec3f& extent, const Vec3f& coord, float tess_levels) override;
			void add_light(LightType type, const Mat4f& transform, const Vec3f& color, bool cast_shadow) override;

			Vec4f get_scissor() const override { return curr_scissor; }
			void set_scissor(const Vec4f& scissor) override;

			void add_blur(const Vec4f& range, uint radius) override;
			void add_bloom() override;

			void prepare() override;

			void record(CommandBufferPrivate* cb, uint image_index);
		};

		inline void CanvasBridge::set_target(uint views_count, ImageView* const* views)
		{
			((CanvasPrivate*)this)->set_target({ (ImageViewPrivate**)views, views_count });
		}

		inline int CanvasBridge::find_resource(ResourceType type, const char* name)
		{
			return ((CanvasPrivate*)this)->find_resource(type, name ? name : "");
		}

		inline uint CanvasBridge::set_resource(ResourceType type, int slot, void* p, const char* name)
		{
			return ((CanvasPrivate*)this)->set_resource(type, slot, p, name ? name : "");
		}

		inline void CanvasBridge::record(CommandBuffer* cb, uint image_index)
		{
			((CanvasPrivate*)this)->record((CommandBufferPrivate*)cb, image_index);
		}
	}
}
