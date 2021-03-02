#include <flame/foundation/foundation.h>
#include <flame/graphics/model.h>

using namespace flame;
using namespace graphics;

int main(int argc, char** args)
{
	std::filesystem::path input;
	std::filesystem::path output_model;
	std::filesystem::path output_prefab;
	if (argc == 2)
	{
		input = args[1];
		output_model = input;
		output_model.replace_extension(L".fmodb");
		output_prefab = input;
		output_prefab.replace_extension(L".prefab");
	}
	else
	{
		auto ap = pack_args(argc, args);
		if (!ap.get_item("-i", input))
			goto show_usage;
		ap.get_item("-o", output_model);
		ap.get_item("-p", output_prefab);
	}

	goto process;

show_usage:
	printf("usage: model_converter -i filename [-o filename] [-p filename] \n"
		"-i: specify the input model path\n"
		"-o: specify the output model path\n"
		"-p: specify the output prefab path\n"
		"or: model_converter filename\n"
		"to output model and prefab in input's directory\n");
	return 0;

process:

	auto m = Model::create(input.c_str());
	if (!m)
		return 0;
	if (!output_model.empty())
		m->save(output_model.c_str());
	if (!output_prefab.empty())
		m->generate_prefab(output_prefab.c_str());
	printf("converted\n");
	return 0;
}
