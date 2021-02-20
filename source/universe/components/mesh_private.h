#pragma once

#include <flame/universe/components/mesh.h>

namespace flame
{
	namespace graphics
	{
		struct Model;
		struct Mesh;
		struct ArmatureDeformer;
		struct Canvas;
	}

	struct cNodePrivate;
	struct sRendererPrivate;

	struct cMeshBridge : cMesh
	{
		void set_src(const char* src) override;
		void set_animation(const char* name, bool loop, uint layer) override;
	};

	struct cMeshPrivate : cMeshBridge
	{
		struct Bone
		{
			std::string name;
			cNodePrivate* node;
			void* changed_listener;
		};

		struct AnimationLayer
		{
			struct Pose
			{
				vec3 p;
				quat q;
			};

			std::string name;
			bool loop = false;
			int frame = -1;
			uint max_frame = 0;
			std::vector<std::pair<uint, std::vector<Pose>>> poses;

			void* event = nullptr;

			void stop();

			std::pair<uint, std::vector<Pose>>& add_track()
			{
				poses.emplace_back();
				return poses.back();
			}
		};

		std::string src;

		bool cast_shadow = true;

		cNodePrivate* node = nullptr;
		void* drawer = nullptr;
		sRendererPrivate* renderer = nullptr;
		graphics::Canvas* canvas = nullptr;

		int model_id = -1;
		int mesh_id = -1;
		graphics::Model* model = nullptr;
		graphics::Mesh* mesh = nullptr;
		graphics::ArmatureDeformer* deformer = nullptr;
		std::vector<Bone> bones;
		AnimationLayer animation_layers[2];

		~cMeshPrivate();

		const char* get_src() const override { return src.c_str(); }
		void set_src(const std::string& src);

		bool get_cast_shadow() const override { return cast_shadow; }
		void set_cast_shadow(bool v) override;

		void apply_src();

		void destroy_deformer();

		void set_animation(const std::string& name, bool loop, uint layer);
		void apply_animation(uint layer);
		void stop_animation(uint layer);

		void draw(graphics::Canvas* canvas);

		void on_added() override;
		void on_removed() override;
		void on_entered_world() override;
		void on_left_world() override;
	};

	inline void cMeshBridge::set_src(const char* src)
	{
		((cMeshPrivate*)this)->set_src(src);
	}

	inline void cMeshBridge::set_animation(const char* name, bool loop, uint layer)
	{
		((cMeshPrivate*)this)->set_animation(name, loop, layer);
	}
}
