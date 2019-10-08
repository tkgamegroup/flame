#pragma once

#include <flame/universe/component.h>

using namespace flame;

struct cSceneEditor;
struct cDataTracker;

struct cInspector : Component 
{
	cDockerTab* tab;

	cSceneEditor* editor;

	Entity* e_layout;

	cInspector() :
		Component("Inspector")
	{
	}

	void update_data_tracker(uint component_hash, uint data_offset) const;
	void refresh();

	virtual void update() override;
};

void open_inspector(cSceneEditor* editor, const Vec2f& pos);
