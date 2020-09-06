#include <flame/serialize.h>
#include <flame/graphics/model.h>

using namespace flame;
using namespace graphics;

int main(int argc, char** args)
{
	auto m = Model::create(s2w(args[1]).c_str());
	m->save(s2w(args[2]).c_str());
	return 0;
}
