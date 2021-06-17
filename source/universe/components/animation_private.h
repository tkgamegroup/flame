#pragma once

#include "animation.h"

namespace flame
{
	struct cAnimationPrivate : cAnimation
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

		std::filesystem::path model_name;
		std::filesystem::path src;

		cNodePrivate* node = nullptr;
		void* drawer = nullptr;
		sRendererPrivate* s_renderer = nullptr;
		int armature_id = -1;

		std::vector<Bone> bones;
		std::vector<mat4> bone_mats;
		bool loop = false;
		int frame = -1;
		uint frame_max = 0;
		std::vector<std::pair<uint, std::vector<Pose>>> tracks;
		void* event = nullptr;

		~cAnimationPrivate();

		const wchar_t* get_model_name() const override { return model_name.c_str(); }
		void set_model_name(const std::filesystem::path& src);
		void set_model_name(const wchar_t* src) override { set_model_name(std::filesystem::path(src)); }

		const wchar_t* get_src() const override { return src.c_str(); }
		void set_src(const std::filesystem::path& src);
		void set_src(const wchar_t* src) override { set_src(std::filesystem::path(src)); }

		bool get_loop() const override { return loop; }
		void set_loop(bool l) override;

		void apply_src();
		void stop();
		void advance();

		void draw(sRenderer* s_renderer);

		void on_added() override;
		void on_removed() override;
		void on_entered_world() override;
		void on_left_world() override;
	};
}
