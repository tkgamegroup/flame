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
			mat4 offmat;
		};

		struct Pose
		{
			vec3 p;
			quat q;
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
		std::filesystem::path ani_name;
		bool loop_ani = false;
		int ani_frame = -1;
		uint ani_frame_max = 0;
		std::vector<std::pair<uint, std::vector<Pose>>> ani_tracks;
		void* ani_event = nullptr;

		~cMeshPrivate();

		const char* get_src() const override { return src.c_str(); }
		void set_src(const std::string& src);
		void set_src(const char* src) override { set_src(std::string(src)); }

		bool get_cast_shadow() const override { return cast_shadow; }
		void set_cast_shadow(bool v) override;

		void apply_src();

		const wchar_t* get_animation() const override { return ani_name.c_str(); }
		void set_animation(const std::filesystem::path& name, bool loop);
		void set_animation(const wchar_t* name, bool loop) override { set_animation(std::filesystem::path(name), loop); }
		void apply_animation();
		void stop_animation();
		void advance_frame();

		void draw(sRenderer* s_renderer);
		bool measure(AABB* b);

		void on_added() override;
		void on_removed() override;
		void on_entered_world() override;
		void on_left_world() override;
	};
}
