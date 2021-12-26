#include <vector>
#include <memory>

struct A
{
	struct Create
	{
		A* operator()()
		{
			__FUNCTION__;
		}
	};
	static Create& create;
};

int main() 
{
    std::vector<A*> a;
    auto wtf = typeid(a).name();
    return 0;
}
