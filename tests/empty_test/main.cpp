#include <flame/foundation/foundation.h>
#include <flame/foundation/system.h>

using namespace flame;

struct Node
{
	int tag;
	int a;
	int b;

	int some_job0()
	{
		a = rand();
		b = rand();
		return a + b;
	}

	int some_job1()
	{
		a = rand();
		b = rand();
		return a * b;
	}
};

int main(int argc, char** args) 
{
	srand(time(0));

	std::vector<Node> nodes;
	auto n = 1024 * 16;
	nodes.resize(n);
	for (auto i = 0; i < nodes.size(); i++)
		nodes[i].tag = rand() % 2;

	// test 1
	{
		std::vector<int> nodes_0;
		std::vector<int> nodes_1;
		nodes_0.reserve(nodes.size());
		nodes_1.reserve(nodes.size());

		auto t0 = performance_counter();
		for (auto i = 0; i < nodes.size(); i++)
		{
			switch (nodes[i].tag)
			{
			case 0:
				nodes_0.push_back(i);
				break;
			case 1:
				nodes_1.push_back(i);
				break;
			}
		}
		for (auto idx : nodes_0)
			nodes[idx].some_job0();
		for (auto idx : nodes_1)
			nodes[idx].some_job1();
		auto t1 = performance_counter();
		printf("test0: %lld\n", t1 - t0);
	}

	// test 2
	{
		auto t0 = performance_counter();
		for (auto& node : nodes)
		{
			if (node.tag == 0)
				node.some_job0();
		}
		for (auto& node : nodes)
		{
			if (node.tag == 1)
				node.some_job1();
		}
		auto t1 = performance_counter();
		printf("test1: %lld\n", t1 - t0);
	}

	return 0;
}

