#pragma once

#include <flame/universe/component.h>

using namespace flame;

struct cInspector;

struct cSceneEditor : Component
{
	Entity* prefab;

	Entity* e_scene;
	cDockerTab* hierarchy_tab;
	cInspector* inspector;

	Entity* selected;

	cSceneEditor() :
		Component("SceneEditor")
	{
	}

	void on_selected_changed(Entity* e);

	virtual void update() override;
};

void open_scene_editor(const std::wstring& filename, const Vec2f& pos);
