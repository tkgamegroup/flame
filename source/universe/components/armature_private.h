#pragma once

#include "../../graphics/model.h"
#include "armature.h"
#include "node_private.h"

namespace flame
{
	struct cArmaturePrivate : cArmature, NodeDrawer
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
			std::vector<std::pair<uint, std::vector<graphics::BoneKey>>> tracks;

			void apply(Bone* bones, uint frame);
		};

		std::filesystem::path model_name;
		std::wstring animation_names;

		cNodePrivate* node = nullptr;
		sRendererPrivate* s_renderer = nullptr;
		int armature_id = -1;

		std::vector<Bone> bones;
		std::vector<mat4> bone_mats;
		std::vector<Action> actions;
		bool loop = false;
		float speed = 1.f;
		int playing = -1;
		int frame = -1;
		float frame_accumulate = 0.f;
		void* event = nullptr;
		std::pair<int, int> peeding_pose = { -1, -1 };
		std::vector<std::unique_ptr<Closure<void(Capture&, int)>>> callbacks;

		~cArmaturePrivate();

		const wchar_t* get_model() const override { return model_name.c_str(); }
		void set_model(const std::filesystem::path& src);
		void set_model(const wchar_t* src) override { set_model(std::filesystem::path(src)); }

		const wchar_t* get_animations() const override { return animation_names.c_str(); }
		void set_animations(const std::wstring& src);
		void set_animations(const wchar_t* src) override { set_animations(std::wstring(src)); }

		int get_playing() override { return playing; }
		void play(uint id) override;
		void stop() override;
		void stop_at(uint id, int frame) override;

		bool get_loop() const override { return loop; }
		void set_loop(bool l) override;

		float get_speed() const override { return speed; }
		void set_speed(float v) override { speed = v; }

		void* add_callback(void (*callback)(Capture& c, int frame), const Capture& capture) override;
		void remove_callback(void* cb) override;

		void apply_src();
		void advance();

		void draw(sRendererPtr s_renderer) override;

		void on_added() override;
		void on_removed() override;
		void on_entered_world() override;
		void on_left_world() override;
	};
}
