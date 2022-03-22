#include <map>
#include <memory>
#include <vector>

struct A
{
	int v;

	A(int v);
	~A();
};

std::map<int, int> m;
std::vector<std::unique_ptr<A>> aa;

A::A(int v) :
	v(v)
{
	m[v] = v;
}

A::~A()
{
	if (auto it = m.find(v); it != m.end())
		m.erase(it);
}

int main(int argc, char** args) 
{ 
	for (auto i = 0; i < 100; i++)
		aa.emplace_back(new A(i));
	return 0;
}

