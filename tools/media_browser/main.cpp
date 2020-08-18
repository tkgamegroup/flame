#include <flame/universe/app.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/image.h>
#include <flame/universe/components/tree.h>

using namespace flame;
using namespace graphics;

App g_app;

Canvas* canvas;
Entity* root;

Entity* search;
Entity* tree;
Entity* image;

ImageView* white_view;

Entity* prev_selected = nullptr;

struct cTreeLeafPath : Component
{
	inline static auto type_name = "cTreeLeafPath";
	inline static auto type_hash = ch(type_name);

	std::filesystem::path path;

	cTreeLeafPath() : 
		Component("cTreeLeafPath", type_hash)
	{
	}
};

void add_dir(Entity* dst, const std::filesystem::path& dir)
{
	for (std::filesystem::directory_iterator end, it(dir); it != end; it++)
	{
		auto& path = it->path();
		if (it->is_directory())
		{
			auto e = Entity::create();
			e->load(L"tree_node");
			e->find_child("title")->get_component_t<cText>()->set_text(path.filename().c_str());
			e->get_component_t<cTreeNode>()->toggle_collapse();
			dst->add_child(e);

			add_dir(e->find_child("items"), path);
		}
		else
		{
			auto e = Entity::create();
			e->load(L"tree_leaf");
			auto ctfp = new cTreeLeafPath();
			ctfp->path = path;
			e->add_component(ctfp);
			e->get_component_t<cText>()->set_text(path.filename().c_str());
			dst->add_child(e);
		}
	}
}

int main(int argc, char** args)
{
	g_app.create();
	auto w = new GraphicsWindow(&g_app, true, true, "Media Browser", Vec2u(1280, 720), WindowFrame | WindowResizable);
	canvas = w->canvas;
	canvas->set_clear_color(Vec4c(255));
	root = w->root;

	auto e = Entity::create();
	e->load(L"main");
	root->add_child(e);

	search = e->find_child("search");
	search->add_local_data_changed_listener([](Capture&, Component* t, uint64 h) {
		if (t->type_hash == cText::type_hash && h == S<ch("text")>::v)
		{

		}
	}, Capture());
	tree = e->find_child("tree");
	image = e->find_child("image");
	{
		auto ci = image->get_component_t<cImage>();
		ci->set_res_id(9);
		ci->set_tile_id(0);
	}

	white_view = canvas->get_resource(9)->get_view();

	tree->add_local_data_changed_listener([](Capture&, Component* t, uint64 h) {
		if (t->type_hash == cTree::type_hash && h == S<ch("selected")>::v)
		{
			auto s = ((cTree*)t)->get_selected();

			if (prev_selected == s)
				return;

			auto prev_view = canvas->get_resource(9)->get_view();
			if (prev_view != white_view)
				prev_view->get_image()->release();
			canvas->set_resource(9, white_view);

			if (auto ctf = s ? s->get_component_t<cTreeLeafPath>() : nullptr; ctf)
			{
				auto path = s->get_component_t<cTreeLeafPath>()->path;
				auto ext = path.extension();
				if (ext == L".jpg" ||
					ext == L".png")
					canvas->set_resource(9, Image::create(g_app.graphics_device, path.c_str())->get_default_view());
			}

			image->on_message(MessageElementSizeDirty);

			prev_selected = s;
		}
	}, Capture());

	add_dir(tree, L"E:/music");

	looper().add_event([](Capture& c) {
		printf("%d\n", looper().get_fps());
		c._current = INVALID_POINTER;
	}, Capture(), 1.f);

	g_app.run();

	return 0;
}
