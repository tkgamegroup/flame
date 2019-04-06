namespace ImGui
{
	struct Splitter
	{
		bool vertically;
		float min_size[2];
	};
}

namespace flame
{
	namespace ui
	{
		void init()
		{
			main_layout = new Layout;

			on_resize(0, 0);
			load_layout();
		}
	}
}
