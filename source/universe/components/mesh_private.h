#pragma once

#include "../entity_private.h"
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

	struct cMeshBridge : cMesh
	{
		void set_src(const char* src) override;
		void set_animation_name(const char* name) override;
	};

	struct cMeshPrivate : cMeshBridge
	{
		struct FramePose
		{
			vec3 p;
			quat q;
		};

		struct Bone
		{
			std::string name;
			cNodePrivate* node;
			void* changed_listener;
			std::vector<FramePose> frames;
		};

		std::string src;

		bool cast_shadow = true;

		std::string animation_name;

		cNodePrivate* node = nullptr;
		graphics::Canvas* canvas = nullptr;

		int model_id = -1;
		int mesh_id = -1;
		graphics::Model* model = nullptr;
		graphics::Mesh* mesh = nullptr;
		graphics::ArmatureDeformer* deformer = nullptr;
		std::vector<Bone> bones;
		int animation_frame = -1;
		uint animation_max_frame;
		void* animation_event = nullptr;

		~cMeshPrivate();

		const char* get_src() const override { return src.c_str(); }
		void set_src(const std::string& src);

		bool get_cast_shadow() const override { return cast_shadow; }
		void set_cast_shadow(bool v) override;

		const char* get_animation_name() const override { return animation_name.c_str(); }
		void set_animation_name(const std::string& name);

		void destroy_deformer();
		void stop_animation();

		void apply_src();
		void apply_animation();

		void on_gain_canvas();

		void draw(graphics::Canvas* canvas);
	};

	inline void cMeshBridge::set_src(const char* src)
	{
		((cMeshPrivate*)this)->set_src(src);
	}

	inline void cMeshBridge::set_animation_name(const char* name)
	{
		((cMeshPrivate*)this)->set_animation_name(name);
	}
}
