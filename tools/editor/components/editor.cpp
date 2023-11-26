#include "../app.h"
#include "../scene_window.h"

// Reflect ctor
struct cEditor : Component
{
	// Reflect
	bool show_outline = true;
	// Reflect
	bool show_axis = true;
	// Reflect
	bool use_gizmos = true;

	bool editor_show_outline;
	bool editor_show_axis;
	bool editor_use_gizmos;

	~cEditor()
	{
		if (auto fv = scene_window.first_view(); fv)
		{
			fv->show_outline = editor_show_outline;
			fv->show_axis = editor_show_axis;
			fv->use_gizmos = editor_use_gizmos;
		}
	}

	void start() override
	{
		if (auto fv = scene_window.first_view(); fv)
		{
			editor_show_outline = fv->show_outline;
			editor_show_axis = fv->show_axis;
			editor_use_gizmos = fv->use_gizmos;

			fv->show_outline = show_outline;
			fv->show_axis = show_axis;
			fv->use_gizmos = use_gizmos;
		}
	}

	struct Create
	{
		virtual cEditor* operator()(EntityPtr) = 0;
	};
	// Reflect static
	static Create& create;
};

struct cEditorCreate : cEditor::Create
{
	cEditor* operator()(EntityPtr e) override
	{
		return new cEditor();
	}
}cEditor_create;
cEditor::Create& cEditor::create = cEditor_create;
