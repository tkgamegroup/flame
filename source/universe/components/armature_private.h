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
			cNodePtr node = nullptr;
			mat4 offmat;

			inline mat4 calc_mat();
		};

		struct Animation
		{
			uint total_frame;
			std::vector<std::pair<uint, std::vector<graphics::Channel::Key>>> tracks;

			void apply(Bone* bones, uint frame);
		};

		graphics::ModelPtr model = nullptr;

		std::vector<Bone> bones;
		std::vector<Animation> animations;
		float time = 0.f;

		int frame = -1;

		~cArmaturePrivate();
		void on_init() override;

		void set_model_name(const std::filesystem::path& src) override;
		void set_animation_names(const std::wstring& paths) override;

		void play(uint id) override;
		void stop() override;

		void apply_src();

		void draw(sNodeRendererPtr renderer);

		void on_active() override;
		void on_inactive() override;
	};
}
