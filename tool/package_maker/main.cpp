#include <flame/serialize.h>
#include <flame/foundation/foundation.h>

using namespace flame;

int main(int argc, char **args)
{
	std::filesystem::path root;
	std::filesystem::path out;
	std::vector<std::filesystem::path> items;

	std::ifstream package_description(L"package_description.txt");
	if (!package_description.good())
		return 0;

	std::string line;
	std::getline(package_description, line);
	root = line;
	std::getline(package_description, line);
	out = line;
	while (!package_description.eof())
	{
		std::getline(package_description, line);
		if (!line.empty())
			items.push_back(line);
	}
	package_description.close();

	for (auto& i : items)
	{
		auto p = out / i.parent_path();
		if (!std::filesystem::exists(p))
			std::filesystem::create_directories(p);
		std::filesystem::copy_file(root / i, out / i, std::filesystem::copy_options::overwrite_existing);
	}

	return 0;
}
