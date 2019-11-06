#include <flame/foundation/blueprint.h>
#include <flame/foundation/serialize.h>

using namespace flame;

std::filesystem::path parent_path;
std::filesystem::path filename;

void print_usage()
{
	printf("usage: filename [-g]\n  g - only autogen code");
}

int main(int argc, char **args)
{
	if (argc < 2)
	{
		print_usage();
		return 0;
	}

	filename = std::filesystem::canonical(s2w(args[1]));
	if (!std::filesystem::exists(filename))
	{
		printf("bp do not exist\n");
		return 0;
	}
	parent_path = std::filesystem::path(filename).parent_path();

	auto only_autogen_code = false;
	if (argc >= 3 && args[2] == "-g")
		only_autogen_code = true;

	auto file = SerializableNode::create_from_xml_file(filename.wstring());



	SerializableNode::destroy(file);

	return 0;
}
