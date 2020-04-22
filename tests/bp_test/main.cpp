#include <flame/foundation/blueprint.h>
#include <flame/utils/app.h>
#include <flame/utils/fps.h>

#include <flame/universe/utils/entity_impl.h>
#include <flame/universe/utils/ui_impl.h>

using namespace flame;
using namespace graphics;

struct MyApp : App
{
	BP* bp;
}app;

int main(int argc, char** args)
{
	app.create("BP Test", Vec2u(1280, 720), WindowFrame | WindowResizable, true);

	utils::push_parent(app.root);

	utils::e_text(L"");
	utils::c_aligner(AlignMin, AlignMax);
	add_fps_listener([](void* c, uint fps) {
		(*(cText**)c)->set_text(std::to_wstring(fps).c_str());
	}, Mail::from_p(utils::current_entity()->get_component(cText)));

	utils::next_element_pos = 100.f;
	utils::next_element_size = 10.f;
	auto patch = utils::e_element()->get_component(cElement);
	patch->pivot = 0.5f;
	patch->color = Vec4c(255);
	cElement::set_linked_object(patch);

	utils::pop_parent();

	app.bp = BP::create_from_file((app.resource_path / L"test.bp").c_str());
	auto nr = app.bp->add_node("", "flame::cElement", BP::ObjectRefRead);
	auto nw = app.bp->add_node("", "flame::cElement", BP::ObjectRefWrite);
	auto nt = app.bp->add_node("", "flame::R_Time");
	auto na = app.bp->add_node("", "flame::R_Add");
	na->find_input("a")->link_to(nr->find_output("scale"));
	na->find_input("b")->link_to(nt->find_output("total"));
	nw->find_input("scale")->link_to(na->find_output("out"));

	looper().loop([](void*) {
		app.bp->update();
		app.run();
	}, Mail());

	return 0;
}
