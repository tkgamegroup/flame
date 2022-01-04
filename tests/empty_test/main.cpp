#include <stdio.h>

template<int N>
struct A
{
	template<int nh>
	inline int set_var()
	{
		auto get_value = []() {
			printf("hello\n");
			return 10 + nh;
		};
		static auto v = get_value();
		return v;
	}
};

int main()
{
	A<10> a;
	A<20> b;
	auto wtf1 = a.set_var<5>();
	auto wtf2 = b.set_var<5>();
	return 0;
}
