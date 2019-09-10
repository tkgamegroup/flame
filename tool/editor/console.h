#include <flame/foundation/foundation.h>

using namespace flame;

namespace flame
{
	namespace graphics
	{
		struct FontAtlas;
	}

	struct Entity;
}

struct cConsole : Component
{
	cText* c_text_log;
	cEdit* c_edit_input;

	cConsole() :
		Component("Console")
	{
	}

	void print(const std::wstring& str);

	virtual void update() override;
};

Entity* open_console(void (*callback)(void* c, const std::wstring& cmd, cConsole* console), const Mail<>& capture, const Vec2f& pos, graphics::FontAtlas* font_atlas);
