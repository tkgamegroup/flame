#pragma once

#include <flame/universe/component.h>

using namespace flame;

struct cHierarchy;
struct cInspector;

struct cSceneEditor : Component
{
	Entity* prefab;

	Entity* e_scene;
	cHierarchy* hierarchy;
	cInspector* inspector;

	Entity* selected;

	cSceneEditor() :
		Component("SceneEditor")
	{
	}

	virtual void update() override;
};

void open_scene_editor(const std::wstring& filename, const Vec2f& pos);
