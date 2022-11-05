#include <flame/foundation/foundation.h>
#include <flame/foundation/typeinfo.h>

using namespace flame;

int main(int argc, char** args) 
{
	static std::vector<std::pair<std::string, std::string>> default_defines;
	TypeInfo::get<decltype(default_defines)>();

	return 0;
}

