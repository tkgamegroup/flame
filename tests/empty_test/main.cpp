#include <flame/foundation/system.h>
#include <flame/foundation/typeinfo.h>
#include <flame/foundation/typeinfo_serialize.h>

using namespace flame;

//struct SomeClass
//{
//    std::string a;
//    std::vector<int> b;
//    int c = 5;
//};
//
//template<unsigned N>
//struct FixedString 
//{
//    char buf[N + 1] {};
//
//    constexpr FixedString(const char (&str)[N])
//    {
//        for (unsigned i = 0; i != N; i++) 
//            buf[i] = str[i];
//    }
//
//    constexpr operator char const* () const { return buf; }
//};
//
//template<FixedString T>
//struct Foo 
//{
//    static constexpr char const* Name = T;
//
//    void hello() const
//    {
//        printf(Name);
//    }
//};

int entry(int argc, char** args)
{

    return 0;
}

FLAME_EXE_MAIN(entry)
