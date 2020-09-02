#include <flame/universe/app.h>
#include <flame/universe/components/text.h>

using namespace flame;
using namespace graphics;

App g_app;

Entity* root;

Entity* hierarchy;
Entity* scene;
Entity* inspector;

int main(int argc, char** args)
{
	g_app.create();
	auto w = new GraphicsWindow(&g_app, true, true, "Scene Editor", Vec2u(1280, 720), WindowFrame | WindowResizable);
	w->canvas->set_clear_color(Vec4c(255));
	root = w->root;

	auto e = Entity::create();
	e->load(L"main");
	root->add_child(e);

	hierarchy = e->find_child("hierarchy");
	scene = e->find_child("scene");
	inspector = e->find_child("inspector");

	auto prefab = Entity::create();
	prefab->load(L"tests/list_test");
	scene->add_child(prefab);

	std::function<void(Entity*, Entity*)> add_node;
	add_node = [&](Entity* dst, Entity* src) {
		auto e = Entity::create();
		e->load(L"tree_node");
		dst->add_child(e);
		
		e->find_child("title")->get_component_t<cText>()->set_text(
			(L"<" + std::wstring(src->get_src()) + L"> " + s2w(src->get_name())).c_str());
		auto child_count = src->get_children_count();
		auto items = e->find_child("items");
		for (auto i = 0; i < child_count; i++)
			add_node(items, src->get_child(i));
	};
	add_node(hierarchy, prefab);

	looper().add_event([](Capture& c) {
		printf("%d\n", looper().get_fps());
		c._current = INVALID_POINTER;
	}, Capture(), 1.f);

	g_app.run();

	return 0;
}
