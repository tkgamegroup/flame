#include "shader_editor.h"

ShaderEditor::ShaderEditor() :
	FileSelector("Shader Editor", flame::ui::FileSelectorOpen, "../shader")
{
	splitter.size[0] = 300;
}

ShaderEditor::~ShaderEditor()
{
	shader_editor = nullptr;
}

void ShaderEditor::on_bottom_area_show()
{
}

ShaderEditor *shader_editor = nullptr;
