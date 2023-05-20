#include <flame/foundation/foundation.h>
#include <flame/foundation/typeinfo.h>

using namespace flame;

int main(int argc, char** args) 
{
	std::tuple<std::string, std::string, std::string> a;
	auto size = sizeof(a);
	auto wtf0 = (uint64)&std::get<0>(a);
	auto wtf1 = (uint64)&std::get<1>(a);
	auto wtf2 = (uint64)&std::get<2>(a);
	auto addr = (uint64)&a;
	return 0;
}

