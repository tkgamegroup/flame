#include <flame/graphics/device.h>
#include <flame/graphics/shader.h>

using namespace flame;
using namespace graphics;

void compile(const std::filesystem::path& path, const std::filesystem::path& filter)
{
	if (!std::filesystem::is_directory(path))
		return;

	auto ext = path.extension();

	if (!filter.empty() && ext != filter)
		return;

	if (ext == L".dsl")
		DescriptorSetLayout::get(nullptr, path.c_str());
	else if (ext == L".pll")
		PipelineLayout::get(nullptr, path.c_str());
	else if (ext == L".pipeline")
		Pipeline::get(nullptr, path.c_str());
	else
	{
		if (ext == L".vert" || ext == L".frag" ||
			ext == L".tesc" || ext == L".tese" || ext == L".geom" ||
			ext == L".comp")
			Shader::get(nullptr, path.c_str(), "", "");
	}
}

int main(int argc, char **args)
{
	std::filesystem::path path;
	std::filesystem::path filter;

	auto ap = parse_args(argc, args);
	path = ap.get_item("-path");
	filter = ap.get_item("-filter");

	if (path.empty())
		path = std::filesystem::current_path();

	if (!std::filesystem::is_directory(path))
		compile(path, L"");
	else
	{
		std::filesystem::path filter;
		if (argc > 2)
			filter = args[2];

		for (auto& it : std::filesystem::recursive_directory_iterator(path))
			compile(it.path(), filter);
	}

	return 0;
}
