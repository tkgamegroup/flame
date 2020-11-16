#include <flame/serialize.h>
#include <flame/graphics/model.h>

using namespace flame;
using namespace graphics;

int main(int argc, char** args)
{
	auto m = Model::create(s2w(args[1]).c_str());
	for (auto i = 2; i < argc; i++)
	{
		if (args[i][0] == '-')
		{
			switch (args[i][1])
			{
			case 'o':
				m->save(s2w(args[i] + 2).c_str());
				break;
			case 'p':
				m->generate_prefab(s2w(args[i] + 2).c_str());
				break;
			}
		}
	}
	return 0;
}
