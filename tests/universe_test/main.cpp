#include <flame/utils/app.h>
#include <flame/utils/fps.h>

using namespace flame;
using namespace graphics;

App g_app;

auto res_path = std::filesystem::path(getenv("FLAME_PATH")) / "art";
auto test_prefab = L"vscroller_test.prefab";

Entity* root;

int main(int argc, char** args)
{
	g_app.create();
	auto w = new GraphicsWindow(&g_app, true, true, "Universe Test", Vec2u(600, 400), WindowFrame | WindowResizable);
	w->canvas->set_clear_color(Vec4c(255));
	root = w->root;

	//auto res = ResMap::create();
	//res->load((res_path / L"res.ini").c_str());
	//res->traversal([](Capture&, const char* name, const wchar_t* _path) {
	//	auto path = std::filesystem::path(_path);
	//	canvas->set_resource(-1, Image::create(d, path.c_str())->get_default_view(), nullptr, name);
	//}, Capture());

	auto e = Entity::create();
	//{
	//	auto ce = cElement::create();
	//	e->add_component(ce);
	//	auto ct = cText::create();
	//	ct->set_text(L"Hello World");
	//	e->add_component(ct);
	//}
	e->load((res_path / test_prefab).c_str());
	root->add_child(e);

	//add_file_watcher(res_path.c_str(), [](Capture& c, FileChangeType, const wchar_t* filename) {
	//	auto path = std::filesystem::path(filename);
	//	if (path.filename() == test_prefab)
	//	{
	//		auto e = Entity::create();
	//		e->load(filename);
	//		auto root = c.thiz<Entity>();
	//		root->remove_all_children();
	//		root->add_child(e);
	//	}
	//}, Capture().set_thiz(root), false, false);

	add_fps_listener([](Capture&, uint fps) {
		printf("%d\n", fps);
	}, Capture());

	g_app.run();

	return 0;
}
