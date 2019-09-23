#pragma once

#include <flame/universe/component.h>

using namespace flame;

struct cSceneEditor;

struct cHierarchy : Component
{
	cDockerTab* tab;

	cSceneEditor* editor;

	Entity* e_tree;

	cHierarchy() :
		Component("Hierarchy")
	{
	}

	~cHierarchy();

	Entity* find_item(Entity* e) const;

	virtual void update() override;
};

void open_hierachy(cSceneEditor* editor, const Vec2f& pos);
