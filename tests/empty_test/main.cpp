#include <list>

int main()
{
	std::list<int> a;
	auto it = a.begin();

	it = a.emplace(it, 1);
	it++;

	if (it == a.end())
		int cut = 1;

	return 0;
}
