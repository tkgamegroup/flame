#include "text_editor.h"

TextEditor::TextEditor() :
	Window("")
{
}

TextEditor::~TextEditor()
{
	text_editor = nullptr;
}

void TextEditor::on_show()
{
}

TextEditor *text_editor = nullptr;
