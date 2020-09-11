#include <flame/serialize.h>
#include <flame/graphics/model.h>

using namespace flame;
using namespace graphics;

int main(int argc, char** args)
{
	auto m = Model::create(s2w(args[1]).c_str());
	std::string name;
	std::vector<std::pair<std::string, std::filesystem::path>> substitute_materials;
	for (auto i = 3; i < argc; i++)
	{
		if (args[i][0] == '-')
		{
			auto option = std::string(args[i]);
			if (option == "-name")
			{
				name = args[i + 1];
				i += 1;
			}
			else if (option == "-mtl")
			{
				substitute_materials.emplace_back(args[i + 1], args[i + 2]);
				i += 2;
			}
		}
	}
	for (auto& s : substitute_materials)
		m->substitute_material(s.first.c_str(), s.second.c_str());
	m->save(s2w(args[2]).c_str(), name.c_str());
	return 0;
}
