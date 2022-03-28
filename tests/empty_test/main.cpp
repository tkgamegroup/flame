#include <algorithm>
#include <vector>

int main(int argc, char** args) 
{
	std::vector<int> a;
	a.push_back(1);
	a.push_back(2);
	a.push_back(4);
	auto t = 3;
	auto it = std::lower_bound(a.begin(), a.end(), t);
	if (*it != t)
		a.emplace(it, t);
	return 0;
}

