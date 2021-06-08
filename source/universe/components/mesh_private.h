#pragma once

#include "mesh.h"

namespace flame
{
	struct cMeshPrivate : cMesh
	{
		struct Bone
		{
			std::string name;
			cNodePrivate* node;
		};

		struct AnimationLayer
		{
			struct Pose
			{
				vec3 p;
				quat q;
			};

			std::filesystem::path name;
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
		void* measurer = nullptr;
		sRendererPrivate* s_renderer = nullptr;

		int mesh_id = -1;
		graphics::Model* model = nullptr;
		graphics::Mesh* mesh = nullptr;
		std::vector<Bone> bones;
		std::vector<mat4> bone_mats;
		AnimationLayer animation_layers[2];

		~cMeshPrivate();

		const char* get_src() const override { return src.c_str(); }
		void set_src(const std::string& src);
		void set_src(const char* src) override { set_src(std::string(src)); }

		bool get_cast_shadow() const override { return cast_shadow; }
		void set_cast_shadow(bool v) override;

		void apply_src();

		void set_animation(const std::filesystem::path& name, bool loop, uint layer);
		void set_animation(const wchar_t* name, bool loop, uint layer) override { set_animation(std::filesystem::path(name), loop, layer); }
		void apply_animation(uint layer);
		void stop_animation(uint layer);

		void draw(sRenderer* s_renderer);
		bool measure(AABB* b);

		void on_added() override;
		void on_removed() override;
		void on_entered_world() override;
		void on_left_world() override;
	};
}
