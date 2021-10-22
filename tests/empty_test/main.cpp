#include <vector>
#include <list>
#include <map>
#include <memory>

struct T
{
	int* d;

	T(int v)
	{
		d = new int;
		*d = v;
	}

	~T()
	{
		delete d;
	}

	T(T&& oth) noexcept
	{
		d = oth.d;
		oth.d = nullptr;
	}

	T(const T&) = delete;
	T& operator=(const T&) = delete;
};

int main()
{
	std::vector<T> v;
	std::list<T> l;
	std::map<int, T> m;

	//for (auto i = 0; i < 1000; i++)
	//	v.emplace_back();
	//for (auto i = 0; i < 1000; i++)
	//	l.emplace_back();
	for (auto i = 0; i < 100000; i++)
		m.emplace(0, 0);

	return 0;
}
