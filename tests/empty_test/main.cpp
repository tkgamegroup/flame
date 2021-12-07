//#include <flame/foundation/system.h>
//#include <flame/foundation/typeinfo.h>
//
//using namespace flame;
//
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
//
//int entry(int argc, char** args)
//{
//    Foo<"Hello!"> foo;
//    foo.hello();
//
//    SomeClass c;
//    c.a = "Hello World";
//    c.b.push_back(123);
//    c.b.push_back(456);
//    c.c = 5;
//    auto str = TypeInfo::serialize_t(&c);
//    return 0;
//}
//
//FLAME_EXE_MAIN(entry)

#include <iostream>
#include <typeinfo>
#include <vector>
#include <string>

enum E
{

};

struct ABC
{

};

using VE = std::vector<E>;
using VD = std::vector<std::string>;
using VP = std::vector<ABC*>;

int main() 
{
    std::cout << typeid(VE).name() << std::endl;
    std::cout << typeid(VD).name() << std::endl;
    std::cout << typeid(VP).name() << std::endl;
    return 0;
}
