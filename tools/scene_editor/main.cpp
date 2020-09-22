#include <flame/universe/app.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/tree.h>

using namespace flame;
using namespace graphics;

App g_app;

Entity* root;

Entity* hierarchy;
cTree* hierarchy_tree;
Entity* scene;
Entity* inspector;

int main(int argc, char** args)
{
	g_app.create();
	auto w = new GraphicsWindow(&g_app, "Scene Editor", Vec2u(1280, 720), WindowFrame | WindowResizable);
	w->canvas->set_clear_color(Vec4c(255));
	root = w->root;

	auto e = Entity::create();
	e->load(L"main");
	root->add_child(e);

	hierarchy = e->find_child("hierarchy");
	hierarchy_tree = hierarchy->get_component_t<cTree>();
	scene = e->find_child("scene");
	inspector = e->find_child("inspector");

	auto prefab = Entity::create();
	prefab->load(L"tests/list_test");
	scene->add_child(prefab);

	std::function<void(Entity*, Entity*)> add_node;
	add_node = [&](Entity* dst, Entity* src) {
		auto e = Entity::create();
		auto title = L"<" + std::wstring(src->get_src()) + L"> " + s2w(src->get_name());
		auto child_count = src->get_children_count();
		if (child_count > 0)
		{
			e->load(L"tree_node");
			e->find_child("title")->get_component_t<cText>()->set_text(title.c_str());

			auto items = e->find_child("items");
			for (auto i = 0; i < child_count; i++)
				add_node(items, src->get_child(i));
		}
		else
		{
			e->load(L"tree_leaf");
			e->get_component_t<cText>()->set_text(title.c_str());
		}
		dst->add_child(e);
	};
	add_node(hierarchy, prefab);

	hierarchy->add_local_data_changed_listener([](Capture&, Component* t, uint64 h) {
		if (t == hierarchy_tree && h == S<ch("selected")>::v)
		{
			inspector->remove_all_children();
		}
	}, Capture());

	looper().add_event([](Capture& c) {
		printf("%d\n", looper().get_fps());
		c._current = INVALID_POINTER;
	}, Capture(), 1.f);

	g_app.run();

	return 0;
}
