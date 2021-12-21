#pragma once

#include "../../graphics/model.h"
#include "armature.h"
#include "node_private.h"

namespace flame
{
	struct cArmaturePrivate : cArmature
	{
		struct Bone
		{
			std::string name;
			cNodePrivate* node;
			mat4 offmat;
		};

		struct Action
		{
			uint total_frame;
			std::vector<std::pair<uint, std::vector<graphics::Channel::Key>>> tracks;

			void apply(Bone* bones, uint frame);
		};

		cNodePrivate* node = nullptr;
		sRendererPrivate* s_renderer = nullptr;
		int armature_id = -1;

		std::vector<Bone> bones;
		std::vector<mat4> bone_mats;
		std::vector<Action> actions;
		bool loop = false;
		float speed = 1.f;
		float frame_accumulate = 0.f;
		void* event = nullptr;
		std::pair<int, int> peeding_pose = { -1, -1 };

		~cArmaturePrivate();

		void set_model_path(const std::filesystem::path& src) override;
		void set_animation_paths(const std::wstring& paths) override;

		void play(uint id, float speed, bool loop) override;
		void stop() override;
		void stop_at(uint id, int frame) override;

		void apply_src();
		void advance();

		void on_added() override;
		void on_removed() override;
		void on_entered_world() override;
		void on_left_world() override;
	};
}
