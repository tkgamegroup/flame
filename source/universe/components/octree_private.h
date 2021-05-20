#pragma once

#include "octree.h"

namespace flame
{
	struct cOctreePrivate : cOctree
	{
		float length = 0.f;

		cNodePrivate* node = nullptr;

		std::unique_ptr<OctreeNode> octree;

		float get_length() const override { return length; }
		void set_length(float length) override;

		void add_object(cNodePrivate* obj);
		void remove_object(cNodePrivate* obj);

		void on_added() override;
		void on_removed() override;
		void on_entered_world() override;
		void on_left_world() override;
		void on_child_added(EntityPtr e) override;
		void on_child_removed(EntityPtr e) override;
	};
}
