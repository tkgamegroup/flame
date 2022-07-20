//#include <flame/foundation/foundation.h>

//using namespace flame;

#include <string>
#include <tuple>

struct A
{
	double a;
	float b;
};

int main(int argc, char** args) 
{
	std::tuple<std::string, std::string, float> a;
	std::get<0>(a) = "123";
	std::get<1>(a) = "abc";
	std::get<2>(a) = 0.5f;

	std::tuple<A, float> b;


	return 0;
}

