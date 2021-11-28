#include <flame/foundation/system.h>
#include <flame/foundation/typeinfo.h>

using namespace flame;

struct SomeClass
{
    std::string a;
    int b;
};

int entry(int argc, char** args)
{
    SomeClass c;
    c.a = "Hello World";
    c.b = 5;
    auto str = TypeInfo::serialize_t(&c);
    return 0;
}

FLAME_EXE_MAIN(entry)
