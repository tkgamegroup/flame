#include <flame/universe/app.h>
#include <flame/universe/components/text.h>

using namespace flame;
using namespace graphics;

App g_app;

Entity* root;
Entity* tree;

void add_dir(Entity* dst, const std::filesystem::path& dir)
{
	for (std::filesystem::directory_iterator end, it(dir); it != end; it++)
	{
		if (it->is_directory())
		{
			auto e = Entity::create();
			e->load(L"tree_node");
			e->find_child("title")->get_component_t<cText>()->set_text(it->path().filename().c_str());
			dst->add_child(e);

			add_dir(e->find_child("items"), it->path());
		}
		else
		{
			auto e = Entity::create();
			e->load(L"tree_leaf");
			e->get_component_t<cText>()->set_text(it->path().filename().c_str());
			dst->add_child(e);
		}
	}
}

int main(int argc, char** args)
{
	g_app.create();
	auto w = new GraphicsWindow(&g_app, true, true, "Media Browser", Vec2u(1280, 720), WindowFrame | WindowResizable);
	w->canvas->set_clear_color(Vec4c(255));
	root = w->root;

	auto e = Entity::create();
	e->load(L"main");
	root->add_child(e);
	tree = e->find_child("tree");

	add_dir(tree, L"E:/ssss");

	looper().add_event([](Capture& c) {
		printf("%d\n", looper().get_fps());
		c._current = INVALID_POINTER;
	}, Capture(), 1.f);

	g_app.run();

	return 0;
}
