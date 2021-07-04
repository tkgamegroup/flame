#pragma once

#include "../component.h"

namespace flame
{
	struct NodeDrawer
	{
		virtual void draw(sRendererPtr) = 0;
	};

	struct NodeMeasurer
	{
		virtual bool measure(AABB*) = 0;
	};

	struct cNode : Component
	{
		inline static auto type_name = "flame::cNode";
		inline static auto type_hash = ch(type_name);

		cNode() :
			Component(type_name, type_hash)
		{
		}

		virtual vec3 get_pos() const = 0;
		virtual void set_pos(const vec3& pos) = 0;
		virtual quat get_quat() const = 0;
		virtual void set_quat(const quat& quat) = 0;
		virtual vec3 get_scale() const = 0;
		virtual void set_scale(const vec3& scale) = 0;

		// yaw, pitch, roll, in angle
		virtual vec3 get_euler() const = 0;
		virtual void set_euler(const vec3& e) = 0;

		virtual vec3 get_local_dir(uint idx) = 0;

		virtual vec3 get_global_pos() = 0;
		virtual vec3 get_global_dir(uint idx) = 0;

		virtual bool get_assemble_sub() const = 0;
		virtual void set_assemble_sub(bool v) = 0;

		virtual float get_octree_length() const = 0;
		virtual void set_octree_length(float len) = 0;

		virtual void add_drawer(NodeDrawer* d) = 0;
		virtual void remove_drawer(NodeDrawer* d) = 0;
		virtual void add_measurer(NodeMeasurer* m) = 0;
		virtual void remove_measurer(NodeMeasurer* m) = 0;

		virtual bool is_any_within_circle(const vec2& c, float r, uint filter_tag = 0) = 0;
		virtual uint get_within_circle(const vec2& c, float r, EntityPtr* dst, uint max_count, uint filter_tag = 0) = 0;

		FLAME_UNIVERSE_EXPORTS static cNode* create(void* parms = nullptr);
	};
}
