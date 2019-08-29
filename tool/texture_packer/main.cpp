#include <flame/foundation/serialize.h>
#include <flame/foundation/bitmap.h>

using namespace flame;

int main(int argc, char **args)
{
	std::vector<std::string> inputs;
	std::wstring output;
	for (auto i = 1; i < argc; i++)
	{
		if (args[i] == std::string("-o"))
		{
			i++;
			if (i < argc)
				output = s2w(args[i]);
		}
		else
			inputs.push_back(args[i]);
	}

	std::vector<std::pair< Bitmap*, std::string>> textures;
	for (auto& i : inputs)
		textures.emplace_back(Bitmap::create_from_file(s2w(i)), i);

	auto data = bin_pack(textures);
	Bitmap::save_to_file(data.p->first, output);
	std::ofstream pack_file(output + L".pack");
	for (auto& p : data.p->second)
		pack_file << p.first + " " + to_string(p.second) + "\n";
	pack_file.close();
	delete_mail(data);

	return 0;
}
