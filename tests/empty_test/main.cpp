#include <flame/foundation/foundation.h>
#include <flame/foundation/typeinfo.h>

using namespace flame;

int main(int argc, char** args) 
{
	auto p = new std::vector<char>();
	p->resize(40);
	{
		auto d = (std::filesystem::path*)(p->data() + 0);
		new (d) std::filesystem::path;
		*d = L"adfjlksdafsdjkfjsl\\bdsfgbdfsgdfs\\bfdssbsdfhgdf\\sfgsdfg";
	}
	p->resize(80);
	{
		auto d = (std::filesystem::path*)(p->data() + 40);
		new (d) std::filesystem::path;
		*d = L"bfsdfy93\\xvbfdhsh\\sdgfsd\\fhsdjhsdgjsd";
	}

	{
		auto d = (std::filesystem::path*)(p->data() + 0);
		d->begin();
	}

	return 0;
}

