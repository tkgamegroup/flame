#include <flame/serialize.h>

using namespace flame;

std::wstring str;
int indent;

void add_line(const std::wstring& s)
{
	for (auto i = 0; i < indent; i++)
		str += L"\t";
	str += s;
	str += L"\n";
}

int main(int argc, char **args)
{
	return 0;
}
