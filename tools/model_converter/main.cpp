#include <flame/foundation/foundation.h>
#include <flame/graphics/model.h>

using namespace flame;
using namespace graphics;

int main(int argc, char** args)
{
	auto ap = pack_args(argc, args);
	if (!ap.has("-i") ||
		(ap.get_items("-i").size() == 0) ||
		(ap.has("-o") && ap.get_items("-o").size() == 0) ||
		(ap.has("-p") && ap.get_items("-p").size() == 0))
	{
		printf("usage: model_converter -i filename [-o filename] [-p filename]\n"
			"-i: specify the input model path\n"
			"-o: specify the output model path\n"
			"-p: specify the output prefab path\n");
		return 0;
	}
	auto model_name = std::filesystem::path(ap.get_items("-i")[0]);
	if (!std::filesystem::exists(model_name))
	{
		printf("model does not exist: %s\n", model_name.string().c_str());
		return 0;
	}
	auto m = Model::create(model_name.c_str());
	if (ap.has("-o"))
	{
		auto& its = ap.get_items("-o");
		if (its.size() > 0)
			m->save(s2w(its[0]).c_str());
	}
	if (ap.has("-p"))
	{
		auto& its = ap.get_items("-p");
		if (its.size() > 0)
			m->generate_prefab(s2w(its[0]).c_str());
	}
	return 0;
}
