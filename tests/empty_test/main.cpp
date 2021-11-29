#include <flame/foundation/system.h>
#include <flame/foundation/typeinfo.h>

using namespace flame;

struct SomeClass
{
    std::string a;
    int b;
}; 

template<unsigned N>
struct FixedString {
    char buf[N + 1]{};
    constexpr FixedString(char const* s) {
        for (unsigned i = 0; i != N; ++i) buf[i] = s[i];
    }
    constexpr operator char const* () const { return buf; }
};
template<unsigned N> FixedString(char const (&)[N])->FixedString<N - 1>;

template<FixedString T>
class Foo {
    static constexpr char const* Name = T;
public:
    void hello() const
    {
        printf(Name);
    }
};

int entry(int argc, char** args)
{
    Foo<"Hello!"> foo;
    foo.hello();

    SomeClass c;
    c.a = "Hello World";
    c.b = 5;
    auto str = TypeInfo::serialize_t(&c);
    return 0;
}

FLAME_EXE_MAIN(entry)
