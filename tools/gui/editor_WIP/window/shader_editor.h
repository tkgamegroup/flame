#pragma once

#include <flame/engine/ui/fileselector.h>

struct Shader
{
	std::string filename;
	std::string name;
};

struct ShaderEditor : flame::ui::FileSelector
{
	std::vector<std::unique_ptr<Shader>> shaders;

	ShaderEditor();
	~ShaderEditor();
	virtual void on_bottom_area_show();
};

extern ShaderEditor *shader_editor;
