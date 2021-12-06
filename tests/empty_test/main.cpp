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

template<typename...>
struct type_list
{
};

template<typename U, typename T, typename... Args>
constexpr bool is_one_of(type_list<T, Args...>)
{
    return std::is_same_v<T, U> || is_one_of<U>(type_list<Args...>());
}

template<typename U, typename T>
constexpr bool is_one_of(type_list<T>) 
{
    return std::is_same_v<T, U>;
}

using basic_types = type_list<void, bool, char, wchar_t, short, int, float>;

template<typename T>
concept basic_type = is_one_of<T>(basic_types());

template<typename T>
concept not_basic_type = !is_one_of<T>(basic_types());

template<basic_type T>
void print(T v)
{
    std::cout << v << std::endl;
}

template<typename T>
void print(T v)
{
    std::cout << "UDT type?: " << typeid(T).name() << std::endl;
}

struct A
{

};

int main() 
{
    print(123);
    print(A());
    return 0;
}
